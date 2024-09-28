/*
 * sail_imu2.h
 *
 * Created: 2024-09-28 5:49:20 PM
 *  Author: nader
 *	This module contains code for the second IMU onboard Nautono.
 *	The IMU is called the SENtral IMU.
 */ 

#include <asf.h>
#include <status_codes.h>

#define IMU2_TEST_PERIOD 1000		// milliseconds

// SENtral EM7180 defines
#define IMU2_DATA_LENGTH	4		// 4 quaternion values
#define SENTRAL_ADDRESS		0x28	// I2C address for SENTRAL IMU hub
#define RAW_DATA_LENGTH		16		// 16 8-bit registers (4 floats) worth of raw data


/* ##################### Public Function Prototypes ##################### */

// Test task for SENTRAL IMU
void Test_SENTRAL(void);