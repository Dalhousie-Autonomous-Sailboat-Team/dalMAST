/* sail_eeprom.c
* Implementation for the EEPROM module.
* Created on June 23, 2016.
* Created by Thomas Gwynne-Timothy/Julia.
*/

#include "sail_EEPROM.h"

#include <asf.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "sail_i2c.h"
#include "sail_debug.h"

static union {
	struct {
		uint16_t addr;
		uint8_t data[EEPROM_ENTRY_LENGTH];
		uint16_t reserved;
	} fields;
	uint8_t bytes[EEPROM_ENTRY_LENGTH + 4];
} i2c_buffer;

// Static functions to utilize I2C driver
static enum status_code ReadBufferFromAddr(uint16_t addr, uint8_t *data, uint16_t data_length);
static enum status_code WriteBufferToAddr(uint16_t addr, uint8_t *data, uint16_t data_length);
static enum status_code ReadByteFromAddr(uint16_t addr, uint8_t *data);
static enum status_code WriteByteToAddr(uint16_t addr, uint8_t data);
static enum status_code ReadWordFromAddr(uint16_t addr, uint16_t *data);
static enum status_code WriteWordToAddr(uint16_t addr, uint16_t data);

static uint16_t Idx2Addr(uint16_t idx);

static enum EEPROM_States {
	EEPROM_UNINITIALIZED,
	EEPROM_READY,
	EEPROM_CONFIG,
	EEPROM_NUM_STATES
} state = EEPROM_UNINITIALIZED;

static uint16_t config_idx;

enum status_code EEPROM_Init(void)
{
	// Return if the EEPROM has already been initialized
	if (state != EEPROM_UNINITIALIZED) {
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

	// Set the EEPROM module's state to READY
	state = EEPROM_READY;

	return STATUS_OK;
}


enum status_code EEPROM_GetCurrentWayPoint(EEPROM_WayPoint *wp)
{
	uint16_t wp_addr, wp_idx;
	uint8_t checksum;
	uint16_t i;
	EEPROM_Entry entry;
	
	// Return if the module is in an incorrect state
	if (state != EEPROM_READY) {
		return STATUS_ERR_DENIED;
	}

	// Return if provided a null pointer
	if (wp == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Read way point index from EEPROM
	if (ReadWordFromAddr(EEPROM_IDX_ADDR, &wp_idx) != STATUS_OK) {
		return STATUS_ERR_IO;
	}

	// Give the EEPROM time before next action
	delay_ms(10);

	// Read way point from EEPROM
	wp_addr = Idx2Addr(wp_idx);
	if (ReadBufferFromAddr(wp_addr, entry.data, EEPROM_ENTRY_LENGTH) != STATUS_OK) {
		return STATUS_ERR_IO;
	}

	// Calculate the expected checksum
	checksum = entry.data[0];
	for (i = 1; i < EEPROM_CHECKSUM_IDX; i++) {
		checksum ^= entry.data[i];
	}

	// Return if the checksum doesn't match
	if (checksum != entry.checksum) {
		return STATUS_ERR_BAD_DATA;
	}
	
	// Return the way point
	*wp = entry.wp;

	return STATUS_OK;
}


enum status_code EEPROM_CompleteCurrentWayPoint(void)
{
	uint16_t wp_idx, wp_addr;
	uint8_t checksum;
	EEPROM_Entry entry;

	// Return if the module is in an incorrect state
	if (state != EEPROM_READY) {
		return STATUS_ERR_DENIED;
	}

	// Read way point index from EEPROM
	if (ReadWordFromAddr(EEPROM_IDX_ADDR, &wp_idx) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	// Give the EEPROM time before next action
	delay_ms(10);	

	// Read way point from EEPROM
	wp_addr = Idx2Addr(wp_idx);
	if (ReadBufferFromAddr(wp_addr, entry.data, EEPROM_ENTRY_LENGTH) != STATUS_OK) { 
		return STATUS_ERR_IO;
	}

	// Calculate the checksum
	checksum = entry.data[0];
	int i;
	for (i = 1; i < EEPROM_CHECKSUM_IDX; i++) {
		checksum ^= entry.data[i];
	}

	// Return if the checksum doesn't match
	if (checksum != entry.checksum) {
		return STATUS_ERR_BAD_DATA;
	}
	
	// Give the EEPROM time before next action
	delay_ms(10);

	// Write new way point index to the EEPROM
	if (WriteWordToAddr(EEPROM_IDX_ADDR, entry.next_wp_idx) != STATUS_OK) {
		return STATUS_ERR_IO;
	}

	return STATUS_OK;
}


enum status_code EEPROM_StartMissionConfig(void)
{
	// Return if the module is in an incorrect state
	if (state != EEPROM_READY) {
		return STATUS_ERR_DENIED;
	}
	
	// Set the state to CONFIG
	state = EEPROM_CONFIG;
	
	// Initialize the index 
	config_idx = 0;
	
	return STATUS_OK;
}


enum status_code EEPROM_LoadWayPoint(EEPROM_Entry *entry)
{
	// Return if the module is in an incorrect state
	if (state != EEPROM_CONFIG) {
		return STATUS_ERR_DENIED;
	}	
	
	// Return if the pointer is null
	if (entry == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Compute the address of the way point entry to be written
	uint16_t entry_addr = Idx2Addr(config_idx);
	
	// Compute the checksum
	uint8_t checksum = entry->data[0];
	int i;
	for (i = 1; i < EEPROM_CHECKSUM_IDX; i++) {
		checksum ^= entry->data[i];
	}
	
	// Add the checksum
	entry->checksum = checksum;
	
	delay_ms(10);
	
	// Write the data
	if (WriteBufferToAddr(entry_addr, entry->data, EEPROM_ENTRY_LENGTH) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	// Increment the index
	config_idx++;
	
	return STATUS_OK;
}


enum status_code EEPROM_EndMissionConfig(void)
{
	
	// Return if the module is in an incorrect state
	if (state != EEPROM_CONFIG) {
		return STATUS_ERR_DENIED;
	}
	
	// Write the starting index to the EEPROM
	WriteWordToAddr(EEPROM_IDX_ADDR, 0);
	
	delay_ms(10);
	
	// Write the entry count to the EEPROM
	WriteWordToAddr(EEPROM_COUNT_ADDR, config_idx);
		
	// Exit configuration state
	state = EEPROM_READY;
	
	return STATUS_OK;
}


enum status_code EEPROM_GetWayPointCount(uint16_t *wp_count)
{
	// Return if the EEPROM module has not been initialized
	if (state == EEPROM_UNINITIALIZED) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if a null pointer is provided
	if (wp_count == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Read the way point count
	if (ReadWordFromAddr(EEPROM_COUNT_ADDR, wp_count) != STATUS_OK) {
		return STATUS_ERR_IO;
	}

	return STATUS_OK;
}


/*************** STATIC FUNCTIONS ***************/

static enum status_code WriteBufferToAddr(uint16_t addr, uint8_t *data, uint16_t data_length)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Return if the provided data won't fit in the internal buffer
	if (data_length > EEPROM_ENTRY_LENGTH) {
		return STATUS_ERR_NO_MEMORY;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Load the data into the write module
	memcpy(i2c_buffer.fields.data, data, data_length);

	// Repeat until the data is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the data
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, data_length + 2, I2C_WRITE_NORMAL)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	return STATUS_OK;
}


static enum status_code ReadBufferFromAddr(uint16_t addr, uint8_t *data, uint16_t data_length)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);

	// Repeat until the address is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the address
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 2, I2C_WRITE_NO_STOP)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	// Request a read from the device
	if (I2C_ReadBuffer(I2C_EEPROM, data, data_length, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


static enum status_code WriteByteToAddr(uint16_t addr, uint8_t data)
{
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Load the data into the write module
	i2c_buffer.fields.data[0] = data;
	
	// Repeat until the data is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the data
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 3, I2C_WRITE_NORMAL)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	return STATUS_OK;
}


static enum status_code ReadByteFromAddr(uint16_t addr, uint8_t *data)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);

	// Repeat until the address is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the address
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 2, I2C_WRITE_NO_STOP)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	// Request a read from the device
	if (I2C_ReadBuffer(I2C_EEPROM, data, 1, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


static enum status_code WriteWordToAddr(uint16_t addr, uint16_t data)
{
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Load the data into the write module
	i2c_buffer.fields.data[0] = 0xff & data;
	i2c_buffer.fields.data[1] = 0xff & (data >> 8);

	// Repeat until the data is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the data
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 4, I2C_WRITE_NORMAL)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	return STATUS_OK;
}


static enum status_code ReadWordFromAddr(uint16_t addr, uint16_t *data)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Repeat until the address is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the address
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 2, I2C_WRITE_NO_STOP)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	// Request a read from the device
	if (I2C_ReadBuffer(I2C_EEPROM, (uint8_t *)data, 2, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


static uint16_t Idx2Addr(uint16_t idx)
{
	return EEPROM_START_OFFSET + EEPROM_ENTRY_OFFSET + EEPROM_ENTRY_LENGTH *idx;
}


