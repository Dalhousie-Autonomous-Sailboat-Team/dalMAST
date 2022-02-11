/* main.c
 * Main file for project to test the function the NMEA module. This module sits between
 * the UART module and the GPS module, so both of those modules will also be tested in
 * this project.
 * Created on June 23, 2016.
 * Created by Thomas Gwynne-Timothy w JULIA.
 */

#include <asf.h>

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "sail_gps.h"
#include "sail_types.h"
#include "sail_debug.h"

#define COORDS_FMT  "latitude:  %"PRIi16".%"PRIu32" deg\r\n"\
					"longitude: %"PRIi16".%"PRIu32" deg\r\n"

int main (void) {
	// Initialize SAMD20
	system_init();
	
	// Initialize the debug UART
	DEBUG_Init();
	
	// Send a greeting
	DEBUG_Write("Initializing GPS...\r\n");
	
	// Initialize the GPS
	if (GPS_Init() != STATUS_OK) {
		DEBUG_Write("GPS initialization failed!\r\n");
		return 0;
	}
	
	DEBUG_Write("GPS initialized.\r\n");
	
	// Send a greeting
	DEBUG_Write("Enabling GPS...\r\n");
		
	// Start the GPS
	if (GPS_Enable() != STATUS_OK) {
		DEBUG_Write("GPS enable failed!\r\n");
		return 0;
	}
	
	DEBUG_Write("GPS enabled.\r\n");
			
	// Create a variable to hold current coordinates
	GPS_Reading coords;

	while (1) {
		// Try to read from the GPS
		if (GPS_GetReading(&coords) == STATUS_OK) {
			// Print a debug statement
			DEBUG_Write("Processing data...\r\n");
			// Write to the debug port
			DEBUG_Write(COORDS_FMT, (int16_t)coords.lat,
									(uint32_t)(fmod(fabs(coords.lat), 1.0)*1000000.0),
									(int16_t)coords.lon,
									(uint32_t)(fmod(fabs(coords.lon), 1.0)*1000000.0));
		}
	}
}
