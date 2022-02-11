/* sail_buffer.c
 * Implementation of the buffer module used to manage the producer-consumer problem for the
 * interrupt driven UART driver.
 * Created on July 11, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#include "sail_buffer.h"

#include <stdlib.h>
#include <string.h>
#include <system_interrupt.h>

#include "sail_debug.h"

//#define USE_CRITICAL_SECTION_PROTECTION

enum status_code BUFF_Init(BUFF_Module *mod, volatile uint8_t *buffer, uint16_t size) {
	// Check if the pointers are valid
	if (mod == NULL || buffer == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Configure the buffer structure
	mod->buffer = buffer;
	mod->read_idx = 0;
	mod->write_idx = 0;
	mod->capacity = size;
	mod->length = 0;
	
	// Clear the buffer
	memset((uint8_t *)buffer, 0, size);
	
	return STATUS_OK;
}


enum status_code BUFF_ReadByte(BUFF_Module *mod, uint8_t *data) {
	// Check if the pointers are valid
	if (mod == NULL || data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_enter_critical_section();	
	#endif
	
	// Check if there's data available
	if (mod->length == 0) {
		// There's no data in the buffer
		#ifdef USE_CRITICAL_SECTION_PROTECTION
		system_interrupt_leave_critical_section();
		#endif
		return STATUS_NO_CHANGE;
	}
	
	// Get the data at the read position
	*data = mod->buffer[mod->read_idx];
	
	// Move the read position
	mod->read_idx = (mod->read_idx + 1) % mod->capacity;
	// Decrement the length
	mod->length--;
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_leave_critical_section();
	#endif
	
	return STATUS_VALID_DATA;
}


enum status_code BUFF_WriteByte(BUFF_Module *mod, uint8_t data) {
	// Check if the pointer is valid
	if (mod == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_enter_critical_section();	
	#endif
	
	// Put the data in the write position
	mod->buffer[mod->write_idx] = data; 
	
	// Move the write position
	mod->write_idx = (mod->write_idx + 1) % mod->capacity;
	// Increment the length
	if (mod->length < mod->capacity) {
		mod->length++;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_leave_critical_section();	
	#endif
	
	return STATUS_OK;	
}


enum status_code BUFF_ReadString(BUFF_Module *mod, uint8_t *data, uint16_t length) {
	// Temporary variables
	uint16_t temp_read_idx, temp_length, str_idx, str_length, seg_length;
	uint8_t found_start, found_stop;
	
	// Return if null pointers are provided
	if (mod == NULL || data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_enter_critical_section();
	#endif
	
	// Set the read index
	temp_read_idx = mod->read_idx;
	// Set the length
	temp_length = mod->length;
	// Clear the found start flag
	found_start = 0;
	
	// Skip over any carriage returns, newlines, or NULLs
	while (temp_length > 0) {
		// Check for carriage return, newline, or NULL
		if (mod->buffer[temp_read_idx] != '\r' && mod->buffer[temp_read_idx] != '\n' && mod->buffer[temp_read_idx] != '\0') {
			// Found an actual character, stop looking
			found_start = 1;
			break;
		}
		// Skip the character
		temp_length--;
		temp_read_idx = (temp_read_idx + 1) % mod->capacity;
	}
	
	// Return if no start could be found
	if (!found_start) {
		// Update the length
		mod->length = temp_length;
		// Update the read position
		mod->read_idx = temp_read_idx;
		// Return and indicate no data was found
		#ifdef USE_CRITICAL_SECTION_PROTECTION
		system_interrupt_leave_critical_section();
		#endif
		return STATUS_NO_CHANGE;
	}
	
	// Clear the string length
	str_length = 0;
	// Store the start of the string
	str_idx = temp_read_idx;
	// Clear the found stop flag
	found_stop = 0;
	
	// Loop through the string until a carriage return, newline, or NULL is found
	while (temp_length > 0) {
		// Check for a carriage return, newline, or NULL
		if (mod->buffer[temp_read_idx] == '\r' || mod->buffer[temp_read_idx] == '\n' || mod->buffer[temp_read_idx] == '\0') {
			// Found a carriage return, stop looking
			found_stop = 1;
			break;
		}
		// Keep looking
		temp_length--;
		str_length++;
		temp_read_idx = (temp_read_idx + 1) % mod->capacity;	
	}
	
	// Return if the stop couldn't be found
	if (!found_stop) {
		#ifdef USE_CRITICAL_SECTION_PROTECTION
		system_interrupt_leave_critical_section();	
		#endif
		return STATUS_NO_CHANGE;
	}
	
	// Return if the provided buffer isn't big enough
	// TODO Think about this a little more
	if (str_length + 1 > length) {
		#ifdef USE_CRITICAL_SECTION_PROTECTION
		system_interrupt_leave_critical_section();	
		#endif
		return STATUS_ERR_NO_MEMORY;
	}
	
	// Check if the data can be copied in one piece (+1 for null terminator)
	if (str_length + str_idx + 1 <= mod->capacity) {
		// Copy all the data
		memcpy(data, (uint8_t *)&mod->buffer[str_idx], str_length);
	} else {
		// Compute the length of the first segment of data
		seg_length = mod->capacity - str_idx;
		// Copy the first half of the data
		memcpy(data, (uint8_t *)&mod->buffer[str_idx], seg_length);
		// Copy the second half of the data
		memcpy(&data[seg_length], (uint8_t *)mod->buffer, str_length - seg_length);
	}
	
	// Put a null terminator at the end
	data[str_length] = '\0';
	
	// Update the length
	mod->length = temp_length;
	// Update the read position
	mod->read_idx = temp_read_idx;
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_leave_critical_section();	
	#endif
	
	return STATUS_VALID_DATA;	
}


enum status_code BUFF_ReadBuffer(BUFF_Module *mod, uint8_t *data, uint16_t length) {
	// Temporary variables
	uint16_t seg_length;
		
	// Check if the pointers are valid
	if (mod == NULL || data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_enter_critical_section();	
	#endif
	
	// Return if there's no data in the buffer
	if (mod->length == 0) {
		// There's no data in the buffer
		#ifdef USE_CRITICAL_SECTION_PROTECTION
		system_interrupt_leave_critical_section();		
		#endif	
		return STATUS_NO_CHANGE;
	}
	
	// Ensure the read length isn't longer than the amount of data in the buffer
	length = (length > mod->length ? mod->length : length);
	
	// Check if the data can be copied in one piece
	if (mod->read_idx + length <= mod->capacity) {
		// Copy all the data
		memcpy(data, (uint8_t *)&mod->buffer[mod->read_idx], length);
	} else {
		// Compute the length of the first segment of data
		seg_length = mod->capacity - mod->read_idx;
		// Copy the first half of the data
		memcpy(data, (uint8_t *)&mod->buffer[mod->read_idx], seg_length);
		// Copy the second half of the data
		memcpy(&data[seg_length], (uint8_t *)mod->buffer, length - seg_length);
	}
	
	// Move the read position
	mod->read_idx = (mod->read_idx + length) % mod->capacity;
	// Decrement the length
	mod->length -= length;	
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_leave_critical_section();		
	#endif
	
	return STATUS_VALID_DATA;
}


enum status_code BUFF_WriteBuffer(BUFF_Module *mod, uint8_t *data, uint16_t length) {
	// Temporary variables
	uint16_t seg_length;

	// Check if the pointers are valid
	if (mod == NULL || data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Return if the buffer isn't large enough to hold the provided data
	if (length > mod->capacity) {
		return STATUS_ERR_NO_MEMORY;		
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_enter_critical_section();	
	#endif
	
	// Check if the data can be copied in one piece
	if (mod->write_idx + length <= mod->capacity) {
		// Copy all the data
		memcpy((uint8_t *)&mod->buffer[mod->write_idx], data, length);
	} else {
		// Compute the length of the first segment of data
		seg_length = mod->capacity - mod->write_idx;
		// Copy the first half of the data
		memcpy((uint8_t *)&mod->buffer[mod->write_idx], data, seg_length);
		// Copy the second half of the data
		memcpy((uint8_t *)mod->buffer, &data[seg_length], length - seg_length);
	}
	
	// Adjust the write position
	mod->write_idx = (mod->write_idx + length) % mod->capacity;
	// Increase the length
	mod->length += length;
	if (mod->length > mod->capacity) {
		mod->length = mod->capacity;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_leave_critical_section();		
	#endif
	
	return STATUS_OK;	
}


enum status_code BUFF_GetLength(BUFF_Module *mod, uint16_t *length) {
	// Check if the pointer is valid
	if (mod == NULL || length == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_enter_critical_section();	
	#endif
	
	*length = mod->length;
	
	#ifdef USE_CRITICAL_SECTION_PROTECTION
	system_interrupt_leave_critical_section();		
	#endif
	
	return STATUS_OK;	
}
//
//bool BUFF_IsEmpty();
//
//bool BUFF_IsFull();



