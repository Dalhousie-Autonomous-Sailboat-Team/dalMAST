/* sail_i2c.h
 * Header file for the I2C driver module that handles communication with the 
 * EEPROM and compass.
 * Created on June 29, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#ifndef SAIL_I2C_H_
#define SAIL_I2C_H_

#include <status_codes.h>
#include <stdint.h>

typedef enum I2C_DeviceIDs {
	I2C_EEPROM,
	I2C_COMPASS,
	I2C_NUM_DEVICES
} I2C_DeviceID;

typedef enum I2C_ReadFormats {
	I2C_READ_NORMAL,
	I2C_READ_NO_NACK,
	I2C_READ_NO_STOP,
	I2C_NUM_READ_FMTS
} I2C_ReadFormat;

typedef enum I2C_WriteFormats {
	I2C_WRITE_NORMAL,
	I2C_WRITE_NO_STOP,
	I2C_NUM_WRITE_FMTS
} I2C_WriteFormat;


// I2C_Init
// Initialize the I2C master module.
// Status:
//
enum status_code I2C_Init(void);

// I2C_ReadBuffer
// Send a command to the specified I2C device and store the response.
// Inputs:
//   id - ID of the specified I2C device
//   fmt - format of read
// Inputs/Outputs:
//   data - pointer to location where response should be stored
//   data_len - number of bytes to store
// Status:
//   STATUS_OK - data was read successfully
//   STATUS_ERR_NOT_INITIALIZED - I2C module was not initialized
//   STATUS_ERR_INVALID_ARG - an invalid ID or format was provided
//   STATUS_ERR_BAD_ACCRESS - a null pointer was provided
//
enum status_code I2C_ReadBuffer(I2C_DeviceID id, uint8_t *data, uint16_t data_len, I2C_ReadFormat fmt);


// I2C_WriteBuffer
// Write the provided data to the specified I2C device.
// Inputs:
//   id - ID of the specified I2C device
//   data - pointer to location of data to send
//   data_len - number of bytes to send
//   fmt - format of the write
// Status:
//   STATUS_OK - data was read successfully
//   STATUS_ERR_NOT_INITIALIZED - I2C module was not initialized
//   STATUS_ERR_INVALID_ARG - an invalid ID was provided
//   STATUS_ERR_BAD_ACCRESS - a null pointer was provided
//
enum status_code I2C_WriteBuffer(I2C_DeviceID id, uint8_t *data, uint16_t data_len, I2C_WriteFormat fmt);

#endif /* SAIL_I2C_H_ */

