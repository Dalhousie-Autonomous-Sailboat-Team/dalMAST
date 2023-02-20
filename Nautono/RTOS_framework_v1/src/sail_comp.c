/*
 * sail_comp.c
 *
 *  Created on: Jun 28, 2016
 *      Author: JULIA SARTY
 */

#include "sail_comp.h"

#include <asf.h>

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "sail_debug.h"
#include "sail_i2c.h"

#define COMP_READ_EEPROM	0xE1
#define COMP_WRITE_EEPROM	0xF1

#define COMP_POST_ACCEL		0x40
#define COMP_POST_MAG		0x45
#define COMP_POST_HEADING	0x50
#define COMP_POST_TILT		0x55

#define COMP_ENTER_STANDBY	0x76
#define COMP_EXIT_STANDBY	0x75

#define COMP_ENTER_SLEEP	0x83
#define COMP_EXIT_SLEEP		0x84

static uint8_t read_cmds[COMP_NUM_TYPES] = {
	COMP_POST_ACCEL,
	COMP_POST_MAG,
	COMP_POST_HEADING,
	COMP_POST_TILT
};

static double scaling_factors[COMP_NUM_TYPES][3] = {
	{0.0009765625,	0.0009765625,	0.0009765625},
	{1.0,			1.0,			1.0},
	{0.1,			0.1,			0.1},	
	{0.1,			0.1,			1.0},
};

static bool init_flag = false;


// Static functions to work with I2C driver
static enum status_code SendCommand(uint8_t cmd);
static enum status_code ReadGeneric(int16_t *data);
static enum status_code WriteEEPROM(uint8_t addr, uint8_t data);
static enum status_code ReadEEPROM(uint8_t addr, uint8_t *data);

static bool calibrate_flag = false;


enum status_code COMP_Init(void)
{
	// Return if the module has already been initialized
	if (init_flag) {
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Return if an error occurred while initializing the I2C driver
	switch (I2C_Init()) {
		case STATUS_OK:
		case STATUS_ERR_ALREADY_INITIALIZED:
			break;
		default:
			return STATUS_ERR_DENIED;
	}
	
	// TODO Turn on the compass
	
	// Give the compass time to warm up
	delay_ms(500);
	
	// Check for the presence of the HMC6343 by trying to read its slave address
	uint8_t slave_addr;
	if (ReadEEPROM(0x00, &slave_addr) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Return if an incorrect slave address is provided
	if (slave_addr != 0x32) {
		return STATUS_ERR_DENIED;
	}
	
	// Set the flag to indicate the module has been initialized
	init_flag = true;
	
	return STATUS_OK;
}

enum status_code COMP_StartCalibration(void)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the compass already in calibration mode
	if (calibrate_flag) {
		return STATUS_NO_CHANGE;
	}	
	
	// Send the command to start the calibration routine
	SendCommand(0x71);
	
	calibrate_flag = true;
	
	return STATUS_OK;
}

enum status_code COMP_StopCalibration(void)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the compass was not previously set into calibration mode
	if (!calibrate_flag) {
		return STATUS_NO_CHANGE;
	}
	
	// Send the command to stop the calibration routine
	SendCommand(0x7E);
	
	// Clear the calibration flag
	calibrate_flag = false;
	
	return STATUS_OK;
}



enum status_code COMP_GetReading(COMP_ReadingType type, COMP_Reading *reading)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the pointer is NULL
	if (reading == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Return if the requested data type is invalid
	if (type >= COMP_NUM_TYPES) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Create a buffer to store the raw data
	int16_t raw_data[3];
	
	// Request the appropriate data from the compass
	if (SendCommand(read_cmds[type]) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Give the compass time to prepare the data (see data sheet)
	delay_ms(1);
	
	// Read the data from the compass
	if (ReadGeneric(raw_data) != STATUS_OK) {
		DEBUG_Write("Error!\r\n");		
		return STATUS_ERR_DENIED;
	}
	
	// Loop through each field
	int i;
	for (i = 0; i < 3; i++) {
		// Convert to double precision floating point
		reading->data.fields[i] = (double)raw_data[i];
		// Apply scaling based on data type
		reading->data.fields[i] *= scaling_factors[type][i];
	}
	
	reading->type = type;
	
	return STATUS_OK;
}


static enum status_code SendCommand(uint8_t cmd)
{
	// Write to the I2C slave
	if (I2C_WriteBuffer(I2C_COMPASS, &cmd, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;
}


static enum status_code ReadGeneric(int16_t *data)
{
	// Request the data from the compass
	if (I2C_ReadBuffer(I2C_COMPASS, (uint8_t *)data, 6, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Swap the bytes to change endianness
	int i;
	for (i = 0; i < 3; i++) {
		data[i] = ((uint16_t)data[i] >> 8) | ((uint16_t)data[i] << 8);
	}
	
	return STATUS_OK;
}


static enum status_code WriteEEPROM(uint8_t addr, uint8_t data)
{
	/* Prepare buffer with the following fields:
	 * 1. Command to write to the built-in EEPROM,
	 * 2. EEPROM address to write to, and 
	 * 3. Data to be written
	 */
	uint8_t buffer[3];
	buffer[0] = COMP_WRITE_EEPROM;
	buffer[1] = addr;
	buffer[2] = data;	
	
	// Write to the compass
	if (I2C_WriteBuffer(I2C_COMPASS, buffer, 3, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;	
}


static enum status_code ReadEEPROM(uint8_t addr, uint8_t *data)
{
	// Return if the pointer is NULL
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	/* Prepare buffer with the following fields:
	 * 1. Command to read from the built-in EEPROM and
	 * 2. EEPROM address to read from
	 */
	uint8_t buffer[2];
	buffer[0] = COMP_READ_EEPROM;
	buffer[1] = addr;
	
	// Write the command to the compass
	if (I2C_WriteBuffer(I2C_COMPASS, buffer, 2, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Give the compass time to prepare the data (see data sheet)
	delay_ms(10);
	
	// Read the data back from the compass
	if (I2C_ReadBuffer(I2C_COMPASS, data, 1, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}	
	
	return STATUS_OK;		
}

