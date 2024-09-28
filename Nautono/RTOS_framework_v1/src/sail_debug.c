/* sail_debug.c
 * Implementation of the debug module.
 * Created on June 27, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#include "sail_debug.h"


#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG

#ifdef DEBUG
//static uint8_t debug_buffer[DEBUG_BUFFER_LENGTH];
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
	if (UART_Init(UART_VCOM) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	//if (UART_Init(UART_RADIO) != STATUS_OK) {
		//return STATUS_ERR_DENIED;
	//}
	
	// Set the initialization flag
	init_flag = true;
#endif	
	return STATUS_OK;
}

//static uint8_t debug_buffer[DEBUG_BUFFER_LENGTH];

enum status_code DEBUG_Write(const char *format, ...) {
#ifdef DEBUG
	uint8_t debug_buffer[DEBUG_BUFFER_LENGTH];
	// Check if the module has not been initialized
	if (!init_flag) {
		// Error, already initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
		
	va_list args;
	// Load the buffer
	va_start(args, format);
	vsnprintf((char *)debug_buffer, DEBUG_BUFFER_LENGTH, format, args);
	va_end(args);
	
	// Send the string
	// This prints to a serial terminal when the SAMD20 is powered by a laptop
	UART_TxString(UART_VCOM, debug_buffer); 
	//UART_TxString(UART_GPS, debug_buffer);
	// This will print the debug statements over the Xbee radios
	//UART_TxString(UART_RADIO, debug_buffer);
	//UART_TxString(UART_WEATHERSTATION, debug_buffer);
#endif	
	
	return STATUS_OK;
}

enum status_code DEBUG_Write_Unprotected(const char *format, ...) {
	#ifdef DEBUG
	uint8_t debug_buffer[DEBUG_BUFFER_LENGTH];
	// Check if the module has not been initialized
	if (!init_flag) {
		// Error, already initialized
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	va_list args;
	// Load the buffer
	va_start(args, format);
	vsnprintf((char *)debug_buffer, DEBUG_BUFFER_LENGTH, format, args);
	va_end(args);
	
	// Send the string
	// This prints to a serial terminal when the SAMD20 is powered by a laptop
	UART_TxString_Unprotected(UART_VCOM, debug_buffer);
	//UART_TxString_Unprotected(UART_XEOS, debug_buffer);
	//UART_TxString(UART_GPS, debug_buffer);
	// This will print the debug statements over the Xbee radios
	//UART_TxString(UART_RADIO, debug_buffer);
	//UART_TxString(UART_WEATHERSTATION, debug_buffer);
	#endif
	
	return STATUS_OK;
}



void float_to_string(float value, char * string_buffer)
{
	uint8_t digits = 0;
	char c_digits[2];
	
	// Isolate decimal digits
	float decimal = value - (int)value;
	
	while( digits < 2 )
	{
		digits *= 10;
		digits -= (int)digits;
		switch((int)digits)
		{
			case 0:
			c_digits[digits] = '0';
			break;
			case 1:
			c_digits[digits] = '1';
			break;
			case 2:
			c_digits[digits] = '2';
			break;
			case 3:
			c_digits[digits] = '3';
			break;
			case 4:
			c_digits[digits] = '4';
			break;
			case 5:
			c_digits[digits] = '5';
			break;
			case 6:
			c_digits[digits] = '6';
			break;
			case 7:
			c_digits[digits] = '7';
			break;
			case 8:
			c_digits[digits] = '8';
			break;
			case 9:
			c_digits[digits] = '9';
			break;
		}
		digits ++;
	}
	
	sprintf(string_buffer, "%d.%c%c", (int)value, c_digits[0], c_digits[1]);
}