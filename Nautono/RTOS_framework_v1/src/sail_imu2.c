/*
 * sail_imu2.c
 *
 * Created: 2024-09-28 5:48:55 PM
 *  Author: nader
 */ 

#include "sail_imu2.h"
#include "sail_i2c.h"
#include "sail_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ##################### Static Function Prototypes ##################### */

// Initialize SENTRAL
static enum status_code SENTRAL_init(void);

// Read data from SENTRAL
static enum status_code SENTRAL_read_quaternions(float * quaternions);

// Print SENTRAL values
static void SENTRAL_print_quaternions(float * quaternions);

/* ##################### Private Function Declarations ##################### */

// Initializes SENtral IMU
static enum status_code SENTRAL_init(void)
{
	// Check that EEPROM data was uploaded
	enum status_code status = SENTRAL_EEPROM_validate();
	
	// Place SENTRAL in pass-through mode
	
	
}

static enum status_code SENTRA_EEPROM_validate(void)
{
	uint8_t write_buff[];
	uint8_t read_buff[];
	uint8_t count = 0, EEPROM_STATUS = 0;
	
	// Read data from EEPROM and make sure it was loaded
	while(!(EEPROM_STATUS & 0x01))
	{
		I2C->writeByte(EM7180_ADDRESS, EM7180_ResetRequest, 0x01);
		_delay(500);
		count++;
		EEPROM_STATUS = I2C->readByte(EM7180_ADDRESS, EM7180_SentralStatus);
		if(count > 5)
		{
			return ERR_BAD_DATA;
		}
	}
	 
	return STATUS_OK;	
}

// Reads data from SENtral IMU to data buffer
static enum status_code SENTRAL_read_quaternions(float* quaternions)
{
	uint8_t subaddress = EM7180_QX;
	uint8_t raw_data[RAW_DATA_LENGTH];
	enum status_code retval;
	
	// Begin I2C transmission to SENTRAL IMU
	retval = I2C_WriteBuffer(I2C_SENTRAL, &subaddress, 1, I2C_WRITE_NORMAL);
	
	if (STATUS_OK != retval)
	{
		return retval;
	}
	
	// Read raw quaternion data into buffer
	retval = I2C_ReadBuffer(I2C_SENTRAL, raw_data, 16, I2C_READ_NORMAL);

	if (STATUS_OK != retval)
	{
		return retval;
	}
	
	memcpy(quaternions, raw_data, RAW_DATA_LENGTH);
	
	return STATUS_OK;
}

static void SENTRAL_print_quaternions(float* quaternions)
{
	// Print quaternions
	DEBUG_Write("Quaternion X (*1000): %d\r\n", quaternions[0]);
	DEBUG_Write("Quaternion Y (*1000): %d\r\n", quaternions[1]);
	DEBUG_Write("Quaternion Z (*1000): %d\r\n", quaternions[2]);
	DEBUG_Write("Quaternion W (*1000): %d\r\n", quaternions[3]);
}

/* ##################### Public Function Declarations ##################### */

// Test SENtral IMU
void Test_SENTRAL(void)
{
	TickType_t task_delay = pdMS_TO_TICKS(IMU2_TEST_PERIOD);
	float quaternions[IMU2_DATA_LENGTH];
	
	SENTRAL_init();

	while (1)
	{
		SENTRAL_read_quaternions(quaternions);
		SENTRAL_print_quaternions(quaternions);
		vTaskDelay(task_delay);
	}
}