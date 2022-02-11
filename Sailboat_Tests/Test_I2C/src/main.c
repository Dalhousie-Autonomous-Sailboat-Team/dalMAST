
#include <asf.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "sail_debug.h"
#include "sail_i2c.h"

static enum status_code ReadBufferFromAddr(uint16_t addr, uint8_t *data, uint16_t data_length);
static enum status_code WriteBufferToAddr(uint16_t addr, uint8_t *data, uint16_t data_length);
static enum status_code ReadByteFromAddr(uint16_t addr, uint8_t *data);
static enum status_code WriteByteToAddr(uint16_t addr, uint8_t data);
static enum status_code ReadWordFromAddr(uint16_t addr, uint16_t *data);
static enum status_code WriteWordToAddr(uint16_t addr, uint16_t data);

#define EEPROM_ENTRY_LENGTH 32
#define BUFFER_LENGTH		254
#define NUM_STRINGS			4
#define DELAY_TIME			100

uint8_t str_in[NUM_STRINGS][BUFFER_LENGTH] = {
	"Hello,",
	"My name is Thomas.",
	"This is the sail boat project.",
	"Wow!"
};

uint8_t str_out[BUFFER_LENGTH];

static union {
	struct {
		uint16_t addr;
		uint8_t data[BUFFER_LENGTH];
	} fields;
	uint8_t bytes[BUFFER_LENGTH + 2];
} i2c_buffer;


int main(void)
{
	// Initialize the system
	system_init();
	
	// Turn on system interrupts
	system_interrupt_enable_global();	
	
	// Initialize the debug module
	DEBUG_Init();
	
	// Initialize the I2C module
	DEBUG_Write("Initializing I2C...\r\n");
	if (I2C_Init() != STATUS_OK) {
		DEBUG_Write("Initialization failed!\r\n");
		// Stop
		return 0;
	}
	
	uint16_t idx, addr, chunk;	
	
	DEBUG_Write("Testing buffer read and write\r\n");
	
	uint8_t byte_buffer_in[1024];
	uint8_t byte_buffer_out[1024];
	
	// Fill the buffer
	srand(1234);
	for (idx = 0; idx < 1024; idx++) {
		byte_buffer_in[idx] = rand() % UINT8_MAX;
	}
	
	// Write the data in chunks
	for (chunk = 0; chunk < 32; chunk++) {
		DEBUG_Write("Writing chunk %d:\r\n", chunk);
		idx = 32*chunk;
		addr = 32*chunk;
		WriteBufferToAddr(addr, &byte_buffer_in[idx], 32);
		delay_ms(10);
		for (; idx < 32*(chunk + 1); idx++) {
			DEBUG_Write("%02x", byte_buffer_in[idx]);
			if ((idx + 1) % 4 == 0) DEBUG_Write("\r\n");
			else DEBUG_Write(" | ");
		}
	}
	
	// Read the data in chunks
	for (chunk = 0; chunk < 32; chunk++) {
		DEBUG_Write("Reading chunk %d:\r\n", chunk);
		idx = 32*chunk;
		addr = 32*chunk;
		ReadBufferFromAddr(addr, &byte_buffer_out[idx], 32);
		delay_ms(10);
		for (; idx < 32*(chunk + 1); idx++) {
			DEBUG_Write("%02x %02x", byte_buffer_in[idx], byte_buffer_out[idx]);
			if ((idx + 1) % 4 == 0) DEBUG_Write("\r\n");
			else DEBUG_Write(" | ");
			if (byte_buffer_in[idx] != byte_buffer_out[idx]) {
				DEBUG_Write("\r\nFailure!\r\n");
				return 0;
			}
		}
	}
	
	DEBUG_Write("Buffer read and write tests passed!\r\n");
	
	/********************* WORD TEST *********************/
	
	DEBUG_Write("Testing word read and write\r\n");
	
	uint16_t word_buffer_in[1024];
	uint16_t word_buffer_out[1024];

	// Fill the buffer
	for (idx = 0; idx < 1024; idx++) {
		word_buffer_in[idx] = rand() % UINT16_MAX;
	}

	// Write the data word by word
	chunk = 0;
	for (idx = 0; idx < 1024; idx++) {
		// Write the chunk label
		if (idx % 32 == 0) DEBUG_Write("Writing chunk #%"PRIu16"\r\n", chunk++);
		// Write to EEPROM
		addr = 2 * idx;
		if (WriteWordToAddr(addr, word_buffer_in[idx]) != STATUS_OK) {
			DEBUG_Write("Failure!\r\n");
			return 0;
		}
		// Print written data to screen
		DEBUG_Write("%04x", word_buffer_in[idx]);
		if ((idx + 1) % 4 == 0) DEBUG_Write("\r\n");
		else DEBUG_Write(" | ");
		// Wait until next write
		delay_ms(10);
	}

	// Read the data word by word
	chunk = 0;
	for (idx = 0; idx < 1024; idx++) {
		// Write the chunk label
		if (idx % 32 == 0) DEBUG_Write("Writing chunk #%"PRIu16"\r\n", chunk++);
		// Read from EEPROM
		addr = 2 * idx;
		if (ReadWordFromAddr(addr, &word_buffer_out[idx]) != STATUS_OK) {
			DEBUG_Write("Failure!\r\n");
			return 0;
		}
		// Print the data to the screen
		DEBUG_Write("%04x %04x", word_buffer_in[idx], word_buffer_out[idx]);
		if ((idx + 1) % 4 == 0) DEBUG_Write("\r\n");
		else DEBUG_Write(" | ");
		// Compare the data
		if (word_buffer_in[idx] != word_buffer_out[idx]) {
			DEBUG_Write("\r\nFailure!\r\n");
			return 0;
		}
		// Wait until next read 
		delay_ms(10);
	}
	
	return 0;
}


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
