/* sail_debug.c
 * Implementation of the debug module.
 * Created on June 27, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#include "sail_debug.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef DEBUG
static uint8_t debug_buffer[DEBUG_BUFFER_LENGTH];
static bool init_flag = false;
#endif


enum status_code DEBUG_Init(void) {
#ifdef DEBUG
	// Check if the module has already been initialized
	if (init_flag) {
		// Error, already initialized
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Return if the UART cannot be initialized
	if (UART_Init(UART_XEOS) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Set the initialization flag
	init_flag = true;
#endif	
	return STATUS_OK;
}


enum status_code DEBUG_Write(const char *format, ...) {
#ifdef DEBUG
	va_list args;
	
	// Check if the module has not been initialized
	if (!init_flag) {
		// Error, already initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Load the buffer
	va_start(args, format);
	vsnprintf((char *)debug_buffer, DEBUG_BUFFER_LENGTH, format, args);
	va_end(args);
	
	// Send the string
	UART_TxString(UART_XEOS, debug_buffer);
#endif	
	
	return STATUS_OK;
}