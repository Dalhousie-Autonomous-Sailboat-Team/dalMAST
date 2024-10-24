/*
 * sail_imu2.c
 *
 * Created: 2024-09-28 5:48:55 PM
 *  Author: nader
 */ 

#include "sail_imu2.h"
#include "sail_i2c.h"
#include "sail_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ##################### Static Function Prototypes ##################### */

// Initialize SENTRAL
static enum status_code SENTRAL_init(void);

// Read data from SENTRAL
static enum status_code SENTRAL_read_quaternions(float * quaternions);

// Print SENTRAL values
static void SENTRAL_print_quaternions(float * quaternions);

/* ##################### Private Function Declarations ##################### */

// Initializes SENtral IMU
static enum status_code SENTRAL_init(void)
{
	
}

// Reads data from SENtral IMU to data buffer
static enum status_code SENTRAL_read_quaternions(float* quaternions)
{
	uint8_t subaddress = EM7180_QX;
	uint8_t raw_data[RAW_DATA_LENGTH];
	enum status_code retval;
	
	// Begin I2C transmission to SENTRAL IMU
	retval = I2C_WriteBuffer(I2C_SENTRAL, &subaddress, 1, I2C_WRITE_NORMAL);
	
	if (STATUS_OK != retval)
	{
		return retval;
	}
	
	// Read raw quaternion data into buffer
	retval = I2C_ReadBuffer(I2C_SENTRAL, raw_data, 16, I2C_READ_NORMAL);

	if (STATUS_OK != retval)
	{
		return retval;
	}
	
	memcpy(quaternions, raw_data, RAW_DATA_LENGTH);
	
	return STATUS_OK;
}

static void SENTRAL_print_quaternions(float* quaternions)
{
	// Print quaternions
	DEBUG_Write("Quaternion X (*1000): %d\r\n", quaternions[0]);
	DEBUG_Write("Quaternion Y (*1000): %d\r\n", quaternions[1]);
	DEBUG_Write("Quaternion Z (*1000): %d\r\n", quaternions[2]);
	DEBUG_Write("Quaternion W (*1000): %d\r\n", quaternions[3]);
}

/* ##################### Public Function Declarations ##################### */

// Test SENtral IMU
void Test_SENTRAL(void)
{
	TickType_t task_delay = pdMS_TO_TICKS(IMU2_TEST_PERIOD);
	float quaternions[IMU2_DATA_LENGTH];
	
	SENTRAL_init();

	while (1)
	{
		SENTRAL_read_quaternions(quaternions);
		SENTRAL_print_quaternions(quaternions);
		vTaskDelay(task_delay);
	}
}