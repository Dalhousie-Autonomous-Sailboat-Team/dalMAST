/* main.c
 * Main file for project to test the function the compass module. 
 * This module sits on top of the I2C module.
 * Created on June 30, 2016.
 * Created by JULIA SARTY.
 */

#include <asf.h>

#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "sail_debug.h"
#include "sail_comp.h"

#define COMP_ACCEL_FMT		"x:        %"PRIi16".%"PRIu32" g\r\n"\
							"y:        %"PRIi16".%"PRIu32" g\r\n"\
							"z:        %"PRIi16".%"PRIu32" g\r\n"	
#define COMP_MAG_FMT		"x:        %"PRIi16".%"PRIu32" \r\n"\
							"y:        %"PRIi16".%"PRIu32" \r\n"\
							"z:        %"PRIi16".%"PRIu32" \r\n"							
#define COMP_HEADING_FMT	"heading:  %"PRIi16".%"PRIu32" deg\r\n"\
							"pitch:    %"PRIi16".%"PRIu32" deg\r\n"\
							"roll:     %"PRIi16".%"PRIu32" deg\r\n"	
#define COMP_TILT_FMT		"pitch:    %"PRIi16".%"PRIu32" deg\r\n"\
							"roll:     %"PRIi16".%"PRIu32" deg\r\n"\
							"temp:     %"PRIi16".%"PRIu32" deg\r\n"

int main (void) {
	// Initialize SAMD20J18
	system_init();
	
	// Initialize the debug module
	DEBUG_Init();
	
	// Initialize the compass
	DEBUG_Write("Initializing Compass ... ");
	if (COMP_Init() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete\r\n");
	
	
	DEBUG_Write("Starting calibration routine...\r\n");
	
	// Calibrate the compass
	COMP_StartCalibration();
	
	DEBUG_Write("Rotate the compass around its y-axis...\r\n");
	
	// Give the compass some time
	for (int t = 30; t > 0; t -= 5) {
		DEBUG_Write("time remaining: %d seconds\r\n", t);
		delay_ms(5000);
	}
	
	DEBUG_Write("Rotate the compass around its z-axis...\r\n");
	
	// Give the compass some time
	for (int t = 30; t > 0; t -= 5) {
		DEBUG_Write("time remaining: %d seconds\r\n", t);
		delay_ms(5000);
	}
	
	DEBUG_Write("Stopping calibration...\r\n");
	
	// Complete the calibration
	COMP_StopCalibration();	
	
	COMP_Reading reading[4];
	
	while (1) {
		// Get accelerometer data
		if (COMP_GetReading(COMP_ACCEL, &reading[0]) != STATUS_OK) {
			DEBUG_Write("Error getting accelerometer data!\r\n");
		}
		
		// Get magnetometer data
		if (COMP_GetReading(COMP_MAG, &reading[1]) != STATUS_OK) {
			DEBUG_Write("Error getting magnetometer data!\r\n");
		}	
			
		// Get heading data
		if (COMP_GetReading(COMP_HEADING, &reading[2]) != STATUS_OK) {
			DEBUG_Write("Error getting heading data!\r\n");
		}
				
		// Get tilt data
		if (COMP_GetReading(COMP_HEADING, &reading[3]) != STATUS_OK) {
			DEBUG_Write("Error getting tilt data!\r\n");
		}
		
		// Print the data
		//DEBUG_Write(COMP_ACCEL_FMT,
			//(int16_t)reading[0].data.accel.x,
			//(uint32_t)(fmod(fabs(reading[0].data.accel.x), 1.0)*1000000.0),
			//(int16_t)reading[0].data.accel.y,
			//(uint32_t)(fmod(fabs(reading[0].data.accel.y), 1.0)*1000000.0),
			//(int16_t)reading[0].data.accel.z,
			//(uint32_t)(fmod(fabs(reading[0].data.accel.z), 1.0)*1000000.0));
		//DEBUG_Write(COMP_MAG_FMT,
			//(int16_t)reading[1].data.mag.x,
			//(uint32_t)(fmod(fabs(reading[1].data.mag.x), 1.0)*1000000.0),
			//(int16_t)reading[1].data.mag.y,
			//(uint32_t)(fmod(fabs(reading[1].data.mag.y), 1.0)*1000000.0),
			//(int16_t)reading[1].data.mag.z,
			//(uint32_t)(fmod(fabs(reading[1].data.mag.z), 1.0)*1000000.0));
		//DEBUG_Write(COMP_HEADING_FMT,
			//(int16_t)reading[2].data.heading.heading,
			//(uint32_t)(fmod(fabs(reading[2].data.heading.heading), 1.0)*1000000.0),
			//(int16_t)reading[2].data.heading.pitch,
			//(uint32_t)(fmod(fabs(reading[2].data.heading.pitch), 1.0)*1000000.0),
			//(int16_t)reading[2].data.heading.roll,
			//(uint32_t)(fmod(fabs(reading[2].data.heading.roll), 1.0)*1000000.0));
		//DEBUG_Write(COMP_TILT_FMT,
			//(int16_t)reading[3].data.tilt.pitch,
			//(uint32_t)(fmod(fabs(reading[3].data.tilt.pitch), 1.0)*1000000.0),
			//(int16_t)reading[3].data.tilt.roll,
			//(uint32_t)(fmod(fabs(reading[3].data.tilt.roll), 1.0)*1000000.0),
			//(int16_t)reading[3].data.tilt.temp,
			//(uint32_t)(fmod(fabs(reading[3].data.tilt.temp), 1.0)*1000000.0));
			
		DEBUG_Write("%d\r\n", (int)(reading[2].data.heading.heading * 10.0));
		
		// Wait for 1 second
		delay_s(1);
	}
}

