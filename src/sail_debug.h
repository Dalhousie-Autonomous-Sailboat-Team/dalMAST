/* sail_debug.h
 * Header file for the debug module for the autonomous sailboat project.
 * Created on June 27, 2016.
 * Created by Thomas Gwynne-Timothy.
 */


#ifndef SAIL_DEBUG_H_
#define SAIL_DEBUG_H_

#include <stdarg.h>
#include <status_codes.h>
#include <stdint.h>

#include "sail_uart.h"

#define DEBUG_BUFFER_LENGTH	1024

/* DEBUG_Init
 * Initialize the debug module.
 * Status:
 *	 STATUS_OK - initialization was successful
 *   STATUS_ERR_DENIED - a driver error has occurred 
 *   STATUS_ERR_ALREADY_INITIALIZED - the module should only be initialized once
 */
enum status_code DEBUG_Init(void);


/* DEBUG_Write
 * Transmit a command to the NMEA device.
 * Inputs:
 *   id - ID of the desired NMEA channel
 *	 cmd - pointer to the command (without $ or *checksum)
 * Status:
 *   STATUS_OK - command was valid and sent successfully and data was received successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 *   STATUS_ERR_BAD_ADDRESS - an invalid address was provided 
 *   STATUS_ERR_TIMEOUT - no response was received before the timeout counter expired
 */
enum status_code DEBUG_Write(const char *format, ...) __attribute__ ((format (gnu_printf, 1, 2)));


#endif /* SAIL_NMEA_H_ */