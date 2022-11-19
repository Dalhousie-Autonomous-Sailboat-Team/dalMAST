/** sail_uart.h
 * @file
 * @brief
 * UART driver API's for the autonomous sailboat project.
 * Created on June 13, 2016.
 * @author Thomas Gwynne-Timothy.
 */

#ifndef SAIL_UART_H
#define SAIL_UART_H

#include <stdint.h>
#include <status_codes.h>

typedef enum UART_ChannelIDs {
	UART_GPS,
	UART_WEATHERSTATION,
	UART_RADIO,
	UART_XEOS,
	UART_BEACON,
	UART_NUM_CHANNELS
} UART_ChannelID;



/**
 * @brief Initialize a specific UART port.
 * @param id - ID of the specified UART port
 * @return STATUS_OK - UART was initialized successfully 
 * @return STATUS_ERR_DENIED - A driver error occurred
 * @return STATUS_ERR_INVALID_ARG - The provided ID is invalid  
 */
enum status_code UART_Init(UART_ChannelID id);


/**
 * @brief Starts the receive job for the specified buffer.
 * @param id - ID of the specified UART port
 * @return STATUS_OK - UART was enabled successfully
 * @return STATUS_ERR_NOT_INITIALIZED - If the UART Module is not initialized 
 * @return STATUS_NO_CHANGE - If the UART Module is already Enabled 
 */
enum status_code UART_Enable(UART_ChannelID id);


/**
 * @brief Stops the receiver from acquiring anymore data.
 * @param id - ID of the specified UART port
 * @return STATUS_OK - UART was disabled successfully
 * @return STATUS_ERR_NOT_INITIALIZED - If the UART Module is not initialized 
 * @return STATUS_NO_CHANGE - If the UART Module is already disabled 
 */
enum status_code UART_Disable(UART_ChannelID id);


/**
 * @brief Sends a null-terminated string with the specified UART.
 * @param id - ID of the specified UART port
 * @param data - Pointer to string of data to be transmitted
 * @return STATUS_OK - Data was transmitted successfully 
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is detected 
 * @return STATUS_ERR_INVALID_ARG - If the ID passed is invalid   
 */
enum status_code UART_TxString(UART_ChannelID id, uint8_t *data);


/**
 * @brief The same as UART_TxString except it does not call the mutex protecting the circular buffer,
 * so the function can be used before the freeRTOS scheduler is started
 *
 * @param id - ID of the specified UART port
 * @param data - Pointer to string of data to be transmitted
 * @return STATUS_OK - Data was transmitted successfully
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is detected
 * @return STATUS_ERR_INVALID_ARG - If the ID passed is invalid
 */

enum status_code UART_TxString_Unprotected(UART_ChannelID id, uint8_t *data);


/**
 * @brief Parses the FIFO buffer for a string (delimited with \r, \n, or \0)
 * and copies it to the provided memory.
 *
 * @param id - ID of the specified UART port
 * @param data - Pointer to string of data to be transmitted
 * @param length - Length of the amount of bytes to be read from buffer 
 * @return STATUS_VALID_DATA - If buffer has been filled
 * @return STATUS_NO_CHANGE - No new data is in the UART buffer	
 * @return STATUS_ERR_NO_MEMORY - Not enough memory allocated for the buffer
 * @return STATUS_ERR_DENIED - Error with UART driver layer has occurred 
 */
enum status_code UART_RxString(UART_ChannelID id, uint8_t *data, uint16_t length);

#endif // SAIL_UART_H

