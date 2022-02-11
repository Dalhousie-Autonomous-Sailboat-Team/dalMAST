/*
 * main.c
 *
 *  Created on: Jun 14, 2016
 *      Author: UW-Stream
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "sail_gps.h"

//#define TEST_STRING "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W"
#define TEST_STRING "GPRMC,143232.000,A,4438.5401,N,06334.3165,W,0.14,231.83,160616,,,D"
#define TEST_EEPROM "58218300,12616600" //TODO Find out how the EEPROM is storing the Lat/Lon data.

int main(void) {
	uint8_t *buffer_ptr, *eeprom_ptr;
	size_t buffer_length;
	uint8_t *data_flag, *cmd_flag;
	GPS_Coords coords;
	GPS_Coords wp;
	GPS_Diff distance;

	// Set up GPS
	GPS_Init();
	GPS_GetBuffer(&buffer_ptr, &eeprom_ptr, &buffer_length);
	GPS_GetFlags(&cmd_flag, &data_flag);



	// Simulate new data arriving
	strcpy(buffer_ptr, TEST_STRING);
	*data_flag = 1;
	strcpy(eeprom_ptr, TEST_EEPROM);
	GPS_ParseEEPROM(&wp);
	// Update GPS
	if (GPS_Update() == GPS_SUCCESS) {
		GPS_GetCoords(&coords);
		printf("Latitude: %"PRIi8" deg\n", coords.lat);
		printf("Latitude: %"PRIu32" umin\n", coords.lat_mmn);
		printf("Latitude: %"PRIi32" udeg\n", coords.lat_final);
		printf("Longitude: %"PRIi8" deg\n", coords.lon);
		printf("Longitude: %"PRIu32" umin\n", coords.lon_mmn);
		printf("Longitude: %"PRIi32" udeg\n", coords.lon_final);
	}
	else {
		printf("No new data available :(\n");
	}
	//check distance calculations

	GPS_Distance(&coords, &wp, &distance);
	printf("Distance: %li change in lat, %li change in lon, %hi deg (unsure of unit), %li distance change", distance.delta_lat, distance.delta_lon, distance.angle, distance.distance);
	return 0;
}



