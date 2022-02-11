/* sail_buffer.h
 * Header file for the buffer module used to manage the producer-consumer problem for the
 * interrupt driven UART driver.
 * Created on July 11, 2016.
 * Created by Thomas Gwynne-Timothy.
 */


#ifndef SAIL_BUFFER_H_
#define SAIL_BUFFER_H_

#include <stdint.h>
#include <status_codes.h>
#include <stdbool.h>

/* BUFF_Module
 * Structure to manage the configuration and state of a buffer.
 * buffer		- pointer to the start of the buffer
 * read_ptr		- pointer last unread byte
 * write_ptr	- pointer to the first empty byte
 * capacity		- maximum capacity of the buffer
 * length		- current number of elements in the buffer
 */
typedef struct BUFF_Module {
	volatile uint8_t *buffer;
	volatile uint16_t read_idx;
	volatile uint16_t write_idx;
	uint16_t capacity;
	volatile uint16_t length;
} BUFF_Module;

enum status_code BUFF_Init(BUFF_Module *mod, volatile uint8_t *buffer, uint16_t size);

enum status_code BUFF_ReadByte(BUFF_Module *mod, uint8_t *data);
enum status_code BUFF_WriteByte(BUFF_Module *mod, uint8_t data);

enum status_code BUFF_ReadString(BUFF_Module *mod, uint8_t *data, uint16_t length);
enum status_code BUFF_ReadBuffer(BUFF_Module *mod, uint8_t *data, uint16_t length);
enum status_code BUFF_WriteBuffer(BUFF_Module *mod, uint8_t *data, uint16_t length);

enum status_code BUFF_GetLength(BUFF_Module *mod, uint16_t *length);

#endif /* SAIL_BUFFER_H_ */