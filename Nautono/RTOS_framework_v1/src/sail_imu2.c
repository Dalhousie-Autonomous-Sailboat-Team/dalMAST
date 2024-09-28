/*
 * sail_imu2.c
 *
 * Created: 2024-09-28 5:48:55 PM
 *  Author: nader
 */ 

#include "sail_imu2.h"

/* ##################### Static Function Prototypes ##################### */

// Read data from SENTRAL
static enum status_code SENTRAL_read(float * data);

// Initialize SENTRAL
static enum status_code SENTRAL_init(void);

// Print SENTRAL values
static void SENTRAL_print(float * data);


/* ##################### Private Function Declarations ##################### */

// Initializes SENtral IMU
static enum status_code SENTRAL_init(void)
{
	
}

// Reads data from SENtral IMU to data buffer
static enum status_code SENTRAL_read(float* quaternions)
{
	uint8_t raw_data[RAW_DATA_LENGTH];
	
	
	
}

static void SENTRAL_print(float* quaternions)
{
	// Print raw quaternions
	//sprintf();
	
	// Calculate 
}

static void SENTRAL_calc(float * quaterions, float * calculated_values)
{
	// store roll, pitch, and yaw in to calculated values array
	// Calculate using the quaternions array
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
		SENTRAL_read(quaternions);
		SENTRAL_print(quaternions);
		vTaskDelay(task_delay);
	}
}