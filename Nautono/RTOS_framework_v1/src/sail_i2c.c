/* sail_i2c.c
 * Implementation of the I2C driver module that handles communication with the 
 * EEPROM and compass.
 * Created on June 29, 2016.
 * Created by Thomas Gwynne-Timothy.
 */
#include "config.h"
#include "sail_i2c.h"
#include "sail_debug.h"

#include <asf.h>

// Functions to initialize I2C module
static void configure_i2c(void);

// I2C software module instance
static struct i2c_master_module i2c_master;

// R/W packets
static struct i2c_master_packet write_packet;
static struct i2c_master_packet read_packet;

// Flag to track if the module has been initialized
uint8_t init_flag = 0;

// Addresses of slave devices
static uint8_t slave_addrs[I2C_NUM_DEVICES] = {
	0x50,	// EEPROM address
	0x28,	// IMU address
	0x40,	// INA 1
	0x41,	// INA 2
	0x42,	// INA 3
	0x76,	// Temperature 1 BME280
	0x77,	// Temperature 2 BME680
	0x36,	// sail angle sensor (or 0x38)
	0x07,	// TBD for 2nd IMU
	0x00	// TBD for 2nd GPS
};


enum status_code I2C_Init(void) {
	// Return if the module has already been initialized
	if (init_flag) {
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Configure the I2C master module
	configure_i2c();
	
	// Set the initialization flag
	init_flag = 1;
	
	return STATUS_OK;
}


enum status_code I2C_ReadBuffer(I2C_DeviceID id, uint8_t *data, uint16_t data_len, I2C_ReadFormat read_fmt) {
	// Return if the module hasn't been initialized yet
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the device ID is invalid
	if (id >= I2C_NUM_DEVICES) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if the pointer is null
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Configure the read packet
	read_packet.address = slave_addrs[id];
	read_packet.data = data;
	read_packet.data_length = data_len;
	
	// Read from the slave
	enum status_code code;
	switch(read_fmt) {
		case I2C_READ_NORMAL:
			code = i2c_master_read_packet_wait(&i2c_master, &read_packet);
			break;
		case I2C_READ_NO_NACK:
			code = i2c_master_read_packet_wait_no_nack(&i2c_master, &read_packet);
			break;
		case I2C_READ_NO_STOP:
			code = i2c_master_read_packet_wait_no_stop(&i2c_master, &read_packet);
			break;	
		default:
			return STATUS_ERR_INVALID_ARG;			
	}
	
	return (code == STATUS_OK ? STATUS_OK : STATUS_BUSY);
}


enum status_code I2C_WriteBuffer(I2C_DeviceID id, uint8_t *data, uint16_t data_len, I2C_WriteFormat fmt) {
	// Ensure the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Ensure the device ID is valid
	if (id >= I2C_NUM_DEVICES) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if the pointer is NULL
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Configure the write packet
	write_packet.address = slave_addrs[id];
	write_packet.data = data;
	write_packet.data_length = data_len;	
	
	// Write the packet
	enum status_code code;
	switch (fmt) {
		case I2C_WRITE_NORMAL:
			code = i2c_master_write_packet_wait(&i2c_master, &write_packet);
			break;
		case I2C_WRITE_NO_STOP:
			code = i2c_master_write_packet_wait_no_stop(&i2c_master, &write_packet);
			break;
		default:
			// Return if an invalid write format was provided
			return STATUS_ERR_INVALID_ARG;
	}
	
	return (code == STATUS_OK ? STATUS_OK : STATUS_BUSY);
}


static void configure_i2c(void) {
	// Initialize config structure and software module
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);

	// Change buffer timeout to something longer
	config_i2c_master.buffer_timeout = 65535;
	
	// Select SERCOM port
	#ifdef PCB
	
	config_i2c_master.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0;
	config_i2c_master.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1;

	// Apply configuration
	// TODO Put a timeout here
	while (i2c_master_init(&i2c_master, SERCOM1, &config_i2c_master) != STATUS_OK);
	
	#else
	
	config_i2c_master.pinmux_pad0 = PINMUX_PA12C_SERCOM2_PAD0;
	config_i2c_master.pinmux_pad1 = PINMUX_PA13C_SERCOM2_PAD1;

	// Apply configuration
	// TODO Put a timeout here
	while (i2c_master_init(&i2c_master, SERCOM2, &config_i2c_master) != STATUS_OK);
	
	#endif

	// Enable the module
	i2c_master_enable(&i2c_master);
}

