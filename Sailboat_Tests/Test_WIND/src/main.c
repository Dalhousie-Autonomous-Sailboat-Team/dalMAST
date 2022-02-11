/* main.c
*
*  Created on: Jun 20, 2016
*      Author: Julia :)
*/

#include <asf.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "sail_wind.h"
#include "sail_gps.h"
#include "sail_uart.h"
#include "sail_debug.h"

int main(void) {
	// Initialize the system
	system_init();
	
	// Initialize the debug port
	DEBUG_Init();
	
	// Initialize the wind vane
	DEBUG_Write("Initializing wind module... ");
	if (WIND_Init() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	// Initialize the GPS
	DEBUG_Write("Initializing GPS... ");
	if (GPS_Init() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	// Enable the wind vane
	DEBUG_Write("Enabling wind module... ");
	if (WIND_Enable() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	// Enable the GPS
	DEBUG_Write("Enabling GPS... ");
	if (GPS_Enable() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	WIND_Reading wind;
	GPS_Reading gps;
	
	while (1) {
		if (WIND_GetReading(&wind) == STATUS_OK) {
			DEBUG_Write("Wind speed: %d / 10 m/s\r\n", (int)(wind.speed * 10.0));
			DEBUG_Write("Wind angle: %d / 10 deg\r\n", (int)(wind.angle * 10.0));
		}
		if (GPS_GetReading(&gps) == STATUS_OK) {
			DEBUG_Write("GPS!\r\n");
		}
		delay_ms(5000);
	}
	
	return 0;
}
