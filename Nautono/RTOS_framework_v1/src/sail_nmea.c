/* sail_nmea.c
 * Implementation for the NMEA module that handles communication with the GPS and
 * wind vane devices.
 * Created on June 23, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#include "sail_nmea.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sail_debug.h"

#define NMEA_CMD_FMT "$%s*%02x\r\n"

// Mapping between NMEA channels and UART channels
static UART_ChannelID uart_channels[] = {
	UART_GPS,
	UART_WIND,
	UART_RADIO,
    UART_XEOS
};

// Buffers to hold raw data from the UART
static uint8_t rx_buffers[NMEA_NUM_CHANNELS][NMEA_BUFFER_LENGTH];
// Buffers to hold data being processed
static uint8_t proc_buffers[NMEA_NUM_CHANNELS][NMEA_BUFFER_LENGTH];
// Flags to track if which NMEA modules have been initialized (flag == 1)
static bool init_flags[NMEA_NUM_CHANNELS];
// Flags to track which NMEA modules have been started
static bool start_flags[NMEA_NUM_CHANNELS];
// Buffers for commands
static uint8_t cmd_buffers[NMEA_NUM_CHANNELS][NMEA_BUFFER_LENGTH];

/*identifies the prefix from the NMEA message and assigns it to the message type*/
bool get_NMEA_type(eNMEA_TRX_t* type, char* msg_ptr) {
	int i;
	for (i = 0; i < NUM_NMEA_TYPES; i++) {
		//return true if prefix found in weather station type table
		if (strcmp(msg_ptr, NMEA_TYPE_TABLE[i].NMEA_Prefix) == 0) {
			*type = NMEA_TYPE_TABLE[i].MSG_id;
			return true;
		}
	}
	return false;
}

enum status_code NMEA_Init(NMEA_ChannelID id) {
	// Check that the id is valid
	if (id >= NMEA_NUM_CHANNELS) {
		// Error, invalid ID
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Check if the module has already been initialized
	if (init_flags[id]) {
		// Error, already initialized
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Initialize the appropriate UART and check for an error
	if (UART_Init(uart_channels[id]) != STATUS_OK) {
		// Indicate a hardware error
		return STATUS_ERR_DENIED;
	}
	
	// Set the initialization flag
	init_flags[id] = true;
	
	// Clear the start flag
	start_flags[id] = false;
	
	return STATUS_OK;	
}


enum status_code NMEA_Enable(NMEA_ChannelID id) {
	// Check that the id is valid
	if (id >= NMEA_NUM_CHANNELS) {
		// Error, invalid ID
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Check if the module hasn't been initialized
	if (!init_flags[id]) {
		// Error, not initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the channel has already been started
	if (start_flags[id]) {
		// OK, but already started
		return STATUS_NO_CHANGE;
	}
	
	// Start the UART receiver
	if (UART_Enable(uart_channels[id]) != STATUS_OK) {
		// Indicate a hardware error
		DEBUG_Write("UART FAILED for ID >>%d<<\r\n", id);
		return STATUS_ERR_DENIED;
	}
	
	// Set the start flag
	start_flags[id] = true;
	
	return STATUS_OK;
}


enum status_code NMEA_Disable(NMEA_ChannelID id) {
	// Check that the id is valid
	if (id >= NMEA_NUM_CHANNELS) {
		// Error, invalid ID
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Check if the module hasn't been initialized
	if (!init_flags[id]) {
		// Error, not initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the channel has already been started
	if (!start_flags[id]) {
		// OK, but already stopped
		return STATUS_NO_CHANGE;
	}
	
	// Stop the UART receiver
	if (UART_Disable(uart_channels[id]) != STATUS_OK) {
		// Indicate a hardware error
		return STATUS_ERR_DENIED;
	}
	
	// Clear the start flag
	start_flags[id] = false;	
	
	return STATUS_OK;
}


enum status_code NMEA_TxString(NMEA_ChannelID id, uint8_t *str) {
	size_t str_length;
	uint8_t checksum;
	uint16_t idx;
	
	// Check that the id is valid
	if (id >= NMEA_NUM_CHANNELS) {
		// Error, invalid ID
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Check if the module hasn't been initialized
	if (!init_flags[id]) {
		// Error, not initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Make sure the pointer isn't invalid
	if (str == NULL) {
		// Error, invalid pointer
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Check the length of the command
	str_length = strlen((char *)str);
	// Make sure the command (with overhead) is small enough for the buffer to hold
	// Overhead includes '$', '*', checksum, '\r', '\n', '\0' --> 7 bytes
	if (str_length + 7 > NMEA_BUFFER_LENGTH) {
		// Not enough memory for this command
		return STATUS_ERR_NO_MEMORY;
	}
	
	// Compute the checksum
	checksum = str[0];
	for (idx = 1; idx < str_length; idx++) {
		checksum ^= str[idx];
	}
	
	
	// Send the command
	sprintf((char *)&cmd_buffers[id][0], NMEA_CMD_FMT, (char *)str, checksum);
	
	
	// transmits
	UART_TxString(uart_channels[id], &cmd_buffers[id][0]);
		
	return STATUS_OK;
}

enum status_code NMEA_TxString_Unprotected(NMEA_ChannelID id, uint8_t *str) {
	size_t str_length;
	uint8_t checksum;
	uint16_t idx;
	
	// Check that the id is valid
	if (id >= NMEA_NUM_CHANNELS) {
		// Error, invalid ID
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Check if the module hasn't been initialized
	if (!init_flags[id]) {
		// Error, not initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Make sure the pointer isn't invalid
	if (str == NULL) {
		// Error, invalid pointer
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Check the length of the command
	str_length = strlen((char *)str);
	// Make sure the command (with overhead) is small enough for the buffer to hold
	// Overhead includes '$', '*', checksum, '\r', '\n', '\0' --> 7 bytes
	if (str_length + 7 > NMEA_BUFFER_LENGTH) {
		// Not enough memory for this command
		return STATUS_ERR_NO_MEMORY;
	}
	
	// Compute the checksum
	checksum = str[0];
	for (idx = 1; idx < str_length; idx++) {
		checksum ^= str[idx];
	}
	
	
	// Send the command
	sprintf((char *)&cmd_buffers[id][0], NMEA_CMD_FMT, (char *)str, checksum);
	
	
	// transmits
	UART_TxString_Unprotected(uart_channels[id], &cmd_buffers[id][0]);
		
	return STATUS_OK;
}


enum status_code NMEA_RxString(NMEA_ChannelID id, uint8_t *str, uint16_t length) {
	uint8_t check, sum;
	uint8_t *rx_ptr;	// Pointer to the raw data from the UART
	uint8_t *proc_ptr;	// Pointer to the buffer used to process the data
	uint16_t rx_idx, proc_length, rx_length;
	
		
	// Return if the ID is invalid
	if (id >= NMEA_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if the module hasn't been initialized
	if (!init_flags[id]) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the pointer is NULL
	if (str == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Try to get a string from the UART
	switch (UART_RxString(uart_channels[id], &rx_buffers[id][0], NMEA_BUFFER_LENGTH)) {
		// Continue if some valid data was found
		case STATUS_VALID_DATA:
			break;
		// Return if there's no new data to return
		case STATUS_NO_CHANGE:
			return STATUS_NO_CHANGE;
		// Return if there's an error
		default:
			return STATUS_ERR_DENIED;
	}
	
	// Get the length of the received data
	rx_length = strlen((char *)&rx_buffers[id][0]);
	
	
	// Get pointer to process buffer
	proc_ptr = &proc_buffers[id][0];
	
	// Get pointer to receive buffer
	rx_ptr = &rx_buffers[id][0];
	
	// Initialize the index, sum, and length to zero
	rx_idx = sum = proc_length = 0;
	// Find the start of the NMEA string ('$')
	while(*rx_ptr++ != '$') {
		// Return if we've reached the end of the buffer prematurely
		if(++rx_idx == rx_length) {
			//DEBUG_Write("reach end of buffer\r\n\n");
			return STATUS_ERR_BAD_DATA;
		}
	}
	// Return if we've reached the end of the buffer prematurely
	if(rx_idx >= rx_length) {
		return STATUS_ERR_BAD_DATA;
	}
		
	// Loop through each character until the end of the NMEA string ('*')
	while (*rx_ptr != '*') {
		proc_length++;
		sum ^= *rx_ptr;
		*proc_ptr++ = *rx_ptr++;
		// Return if we've reached the end of the buffer prematurely
		if(++rx_idx == rx_length) {
			return STATUS_ERR_BAD_FORMAT;
		}
	}
	
	// Return if we've reached the end of the buffer
	if(rx_idx >= rx_length) {
		// Error, end of buffer reached before checksum
		return STATUS_ERR_BAD_FORMAT;
	}

	// Convert 2-char ASCII checksum to a single byte value
	rx_ptr++;
	check = 0xff & strtol((char *)rx_ptr, NULL, 16);

	// Return if the checksum does not match
	if(check != sum) {
		return STATUS_ERR_BAD_DATA;
	}
	
	// Add a NULL termination
	*proc_ptr = '\0';
	proc_length++;
	
	// Return if there is not enough memory to hold the processed string
	if (proc_length > length) {
		return STATUS_ERR_NO_MEMORY;
	}
	
	// Copy the data into the specified memory
	memcpy(str, &proc_buffers[id][0], proc_length);
	return STATUS_VALID_DATA;
}