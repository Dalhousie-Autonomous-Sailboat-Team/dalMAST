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
	UART_MUX1,			//MUX 1
	UART_MUX2,			//MUX 2
 	UART_WIND,			//Windvane 
	UART_VCOM,			//VCOM
	UART_XEOS,			//Beacon (Stream 211)
	UART_NUM_CHANNELS,	//Total number of UART channels
	
	UART_MPPT1,			//MUX 1 MPPT 1
	UART_BMS1,			//MUX 1 BMS 1
	UART_BMS2,			//MUX 1 BMS 2
	UART_MPPT2,			//MUX 1 MPPT 2
	
	UART_RADIO,			//MUX 2 EXTRA HEADER 1 (POSSIBLY RADIO)
	UART_GPS,			//GPS
	UART_PIXIE,			//PIXIE
	UART_XTRA			//MUX 2 EXTRA HEADER 2
	
} UART_ChannelID;



// Used to keep track of which channel settings are set up on the multiplexers
UART_ChannelID MUX1_CURRENT_CHANNEL;
UART_ChannelID MUX2_CURRENT_CHANNEL;


//MUX_Init
// Sets the UART multiplexer logic pins up for output.
//
void MUX_Init(void);

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

//UART_MUX_ChannelID
//Uses the enumerated table to associate a channel ID with components on the MUX's
//
UART_ChannelID UART_MUX_ChannelID(UART_ChannelID id);


// UART_RxString
// Parses the FIFO buffer for a string (delimited with \r, \n, or \0)
// and copies it to the provided memory.
//

enum status_code UART_RxString(UART_ChannelID id, uint8_t *data, uint16_t length);

#endif // SAIL_UART_H

