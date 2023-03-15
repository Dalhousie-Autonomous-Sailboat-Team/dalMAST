/* sail_uart.h
 * Header file for the UART driver module for the autonomous sailboat project.
 * Created on June 13, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#ifndef SAIL_UART_H
#define SAIL_UART_H

#include <stdint.h>
#include <status_codes.h>

typedef enum UART_ChannelIDs {
	UART_GPS,
	UART_WIND,
	UART_RADIO,
	UART_XEOS,
	UART_NUM_CHANNELS
} UART_ChannelID;


//UART_Init
// Initialize a specific UART port.
// Inputs:
//   id - ID of the specified UART port
// Status:
//   STATUS_OK - UART was initialized successfully
//   STATUS_ERR_INVALID_ARG - The provided ID is invalid
//   STATUS_ERR_DENIED - A driver error occurred
//
enum status_code UART_Init(UART_ChannelID id);


// UART_Enable
// Starts the receive job for the specified buffer.
//
enum status_code UART_Enable(UART_ChannelID id);


// UART_Disable
// Stops the receiver from acquiring anymore data.
//
enum status_code UART_Disable(UART_ChannelID id);


// UART_TxString
// Sends a null-terminated string with the specified UART.
//
enum status_code UART_TxString(UART_ChannelID id, uint8_t *data);

// UART_TxString_Unprotected
// The same as UART_TxString except it does not call the mutex protecting the
// circular buffer, so the function can be used before the freeRTOS scheduler is started
//
enum status_code UART_TxString_Unprotected(UART_ChannelID id, uint8_t *data);


// UART_RxString
// Parses the FIFO buffer for a string (delimited with \r, \n, or \0)
// and copies it to the provided memory.
//

enum status_code UART_RxString(UART_ChannelID id, uint8_t *data, uint16_t length);

#endif // SAIL_UART_H

