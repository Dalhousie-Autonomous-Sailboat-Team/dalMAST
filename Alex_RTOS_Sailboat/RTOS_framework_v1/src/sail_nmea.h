/* sail_nmea.h
 * Header file for the NMEA module that takes raw data buffers from the UART module
 * and verifies them before passing them along to the application layer.
 * Created on June 22, 2016.
 * Created by Thomas Gwynne-Timothy.
 */


#ifndef SAIL_NMEA_H_
#define SAIL_NMEA_H_

#include <status_codes.h>
#include <stdint.h>

#include "sail_uart.h"

#define NMEA_BUFFER_LENGTH 128

typedef enum NMEA_ChannelIDs {
	NMEA_GPS,
	NMEA_WEATHERSTATION,
	NMEA_RADIO,
	NMEA_NUM_CHANNELS
} NMEA_ChannelID;


/* NMEA_Init
 * Initialize a specific NMEA channel.
 * Inputs:
 *   id - ID of the desired NMEA channel to initialize
 * Status:
 *	 STATUS_OK - initialization was successful
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided
 *   STATUS_ERR_DENIED - a driver error has occurred 
 *   STATUS_ERR_ALREADY_INITIALIZED - the module should only be initialized once
 */
enum status_code NMEA_Init(NMEA_ChannelID id);


/* NMEA_Enable
 * Trigger the NMEA channel to begin acquiring data.
 * Inputs:
 *   id - ID of the desired NMEA channel
 * Status:
 *   STATUS_OK - data acquisition was started successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided  
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code NMEA_Enable(NMEA_ChannelID id);


/* NMEA_Disable
 * Trigger the NMEA channel to stop acquiring data.
 * Inputs:
 *   id - ID of the desired NMEA channel
 * Status:
 *   STATUS_OK - data acquisition was stopped successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized and/or started
 *   STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code NMEA_Disable(NMEA_ChannelID id);


/* NMEA_TxCommand
 * Transmit a string to the NMEA device.
 * Inputs:
 *   id - ID of the desired NMEA channel
 *	 str - pointer to the command (without $ or *checksum)
 * Status:
 *   STATUS_OK - command was valid and sent successfully and data was received successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 *   STATUS_ERR_BAD_ADDRESS - an invalid address was provided 
 *   STATUS_ERR_TIMEOUT - no response was received before the timeout counter expired
 */
enum status_code NMEA_TxString(NMEA_ChannelID id, uint8_t *str);

/* NMEA_TxString_Unprotected
 * Transmit a string to the NMEA device.
 * The same as NMEA_TxString except the mutex on the write buffer is not used so
 * the function can be called before the FreeRTOS scheduler is started
 * Inputs:
 *   id - ID of the desired NMEA channel
 *	 str - pointer to the command (without $ or *checksum)
 * Status:
 *   STATUS_OK - command was valid and sent successfully and data was received successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 *   STATUS_ERR_BAD_ADDRESS - an invalid address was provided 
 *   STATUS_ERR_TIMEOUT - no response was received before the timeout counter expired
 */
enum status_code NMEA_TxString_Unprotected(NMEA_ChannelID id, uint8_t *str);

/* NMEA_RxString
 * Trigger the NMEA channel to check its buffer and process the data if there is anything
 * available. If there is new, valid data available, the processed data will be copied
 * into the output buffer and the output flag will be set.
 * Inputs:
 *   id - ID of the desired NMEA channel
 * Status:
 *   STATUS_VALID_DATA - valid data has been placed into the output buffer
 *   STATUS_NO_CHANGE - no new data is available
 *	 STATUS_ERR_BAD_DATA - data was available, but the checksum was incorrect
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 */
enum status_code NMEA_RxString(NMEA_ChannelID id, uint8_t *str, uint16_t length);

#endif /* SAIL_NMEA_H_ */