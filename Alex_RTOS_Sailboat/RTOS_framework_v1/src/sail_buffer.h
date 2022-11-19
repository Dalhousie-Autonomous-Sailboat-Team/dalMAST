/** sail_buffer.h
 * @file
 * @brief 
 * Header file for the buffer module used to manage the producer-consumer problem for the
 * interrupt driven UART driver.
 * Created on July 11, 2016.
 * @author Thomas Gwynne-Timothy.
 */


#ifndef SAIL_BUFFER_H_
#define SAIL_BUFFER_H_

#include <stdint.h>
#include <status_codes.h>
#include <stdbool.h>


/** BUFF_Module
 * @struct BUFF_Module
 * @brief Structure to manage the configuration and state of a buffer.
 * @var buffer    - pointer to the start of the buffer
 * @var read_ptr  - pointer last unread byte
 * @var write_ptr - pointer to the first empty byte
 * @var capacity  - maximum capacity of the buffer
 * @var length	  - current number of elements in the buffer
 */
typedef struct BUFF_Module {
	volatile uint8_t *buffer;
	volatile uint16_t read_idx;
	volatile uint16_t write_idx;
	uint16_t capacity;
	volatile uint16_t length;
} BUFF_Module;


/**
 * @brief
 * Initialize new empty ring buffer 
 * @param mod - Pointer to the ring buffer structure 
 * @param buffer - Pointer to the buffer data
 * @param size - size of the ring buffer 
 * @return STATUS_ERR_BAD_ADDRESS - When a null pointer is passed 
 * @return STATUS_OK - Buffer was successfully implemented 
 */
enum status_code BUFF_Init(BUFF_Module *mod, volatile uint8_t *buffer, uint16_t size);


/**
 * @brief
 * Reads a byte of data from the buffer
 * @param mod - Pointer to the buffer module
 * @param data - Pointer to fill byte read 
 * @return STATUS_VALID_DATA - When a byte has been read successfully 
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is passed 
 * @return STATUS_NO_CHANGE - If no new data is in the buffer
 */
enum status_code BUFF_ReadByte(BUFF_Module *mod, uint8_t *data);


/**
 * @brief
 * Writes a byte to a buffer
 * @param mod - Pointer to the buffer 
 * @param data - byte to be written
 * @return STATUS_VALID_DATA - When a byte has been read successfully
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is passed
 */
enum status_code BUFF_WriteByte(BUFF_Module *mod, uint8_t data);


/**
 * @brief
 * Read a string of bytes from a buffer. 
 * Will return early if a terminating character is detected in buffer
 * @param mod - Pointer to ring buffer
 * @param data - Pointer to string to be filled 
 * @param length - Size of string to be filled 
 * @return STATUS_VALID_DATA - When a byte has been read successfully
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is passed
 * @return STATUS_NO_CHANGE - If no new data is in the buffer
 * @return STATUS_ERR_NO_MEMORY - If the string found is to big
 */
enum status_code BUFF_ReadString(BUFF_Module *mod, uint8_t *data, uint16_t length);


/**
 * @brief
 * Read bytes of data from a buffer module 
 * @param mod - Pointer to ring buffer
 * @param data - Pointer to buffer to be filled
 * @param length - Size of buffer to be filled
 * @return STATUS_VALID_DATA - When a byte has been read successfully
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is passed
 * @return STATUS_NO_CHANGE - If no new data is in the buffer
 * @return STATUS_ERR_NO_MEMORY - If the string found is to big
 */
enum status_code BUFF_ReadBuffer(BUFF_Module *mod, uint8_t *data, uint16_t length);


/**
 * @brief
 * Write a string of bytes to a buffer module  
 * @param mod - Pointer to the buffer module
 * @param data - Pointer to string of bytes being written
 * @param length - Number of bytes to be written 
 * @return STATUS_OK - Data has been transmitted
 * @return STATUS_ERR_BAD_ADDRESS - A null pointer was passed
 * @return STATUS_ERR_NO_MEMORY - Buffer does not have sufficient space to store message
 */
enum status_code BUFF_WriteBuffer(BUFF_Module *mod, uint8_t *data, uint16_t length);


/**
 * @brief
 * Write a string of bytes to a buffer module, without using the mutex.
 * @note This API is not thread safe 
 * @param data - Pointer to string of bytes being written
 * @param length - Number of bytes to be written
 * @return STATUS_OK - Data has been transmitted
 * @return STATUS_ERR_BAD_ADDRESS - A null pointer was passed
 * @return STATUS_ERR_NO_MEMORY - Buffer does not have sufficient space to store message
 */
enum status_code BUFF_WriteBufferUnprotected(BUFF_Module *mod, uint8_t *data, uint16_t length);


/**
 * @brief
 * Returns the length of a buffer module
 * @param mod - Pointer to buffer module
 * @param length - Pointer to length value to be assigned
 * @return STATUS_ERR_BAD_ADDRESS - If a null pointer is passed
 * @return STATUS_OK - If length of buffer is returned
 */
enum status_code BUFF_GetLength(BUFF_Module *mod, uint16_t *length);

#endif /* SAIL_BUFFER_H_ */