/*
 * sail_gps.h
 *
 *  Created on: Jun 13, 2016
 *      Author: Thomas
 */

#ifndef SRC_SAIL_GPS_H_
#define SRC_SAIL_GPS_H_

#include <stdint.h>
#include <stdlib.h>

typedef enum GPS_Statuses {
	GPS_SUCCESS,
	GPS_ERROR,
	GPS_UNCHANGED
} GPS_Status;

typedef struct GPS_Coords {
	int8_t lat;			// latitude (-90 to +90 degrees)
	uint32_t lat_mmn;	// latitude milliminutes (0 to 59999)
	int32_t lat_final;		// latitude  frame in microdegrees
	int16_t lon;		// longitude (-180 to +180 degrees)
	uint32_t lon_mmn;	// longitude milliminutes (0 59999)
	int32_t lon_final;		// longitude frame in microdegrees
} GPS_Coords;

typedef struct GPS_Difference
{
	long delta_lon;
	long delta_lat;
	long distance; //TODO define unit slash what is this
	short angle;
} GPS_Diff;

/* GPS_Init
 * Initialize GPS module by clearing flags and buffer.
 * Should be called before any other GPS functions.
 * Returns status by value.
 */
GPS_Status GPS_Init();

/* GPS_Update
 * Check if new data is available in the RMC buffer.
 * If there is new data, process it to get import fields (available via
 * GPS_GetCoords()).
 * If there was no new data, return GPS_UNCHANGED.
 */
GPS_Status GPS_Update();

/* GPS_GetCoords
 * Get the most recent location from the GPS.
 * Returns the coordinate data by reference through the provided pointer.
 * Returns the status by value. The status is GPS_UNCHANGED when no new data has
 * arrived since the last call.
 */
GPS_Status GPS_GetCoords(GPS_Coords *coords);

/* GPS_GetBuffer
 * Get a pointer to the GPS RMC data buffer, along with the length of the
 * buffer.
 * Returns an error if the buffer_ptr is a null pointer.
 */
GPS_Status GPS_GetBuffer(uint8_t **buffer_ptr, uint8_t **eeprom_ptr, size_t *buffer_length);

/* GPS_GetFlags
 * Get pointers to the transmit and receive flags.
 * Return an error if either of them are null pointers.
 */
GPS_Status GPS_GetFlags(uint8_t **tx_flag_ptr, uint8_t **rx_flag_ptr);

/*GPS_Distance
 * Takes EEPROM lat/lon data and compares it to current frame.
 * Calculates delta-lat and delta-lon
 */

GPS_Status GPS_ParseEEPROM(GPS_Coords *wp);
GPS_Status GPS_Distance(GPS_Coords *coords, GPS_Coords *wp, GPS_Diff *distance);

/* GPS_Heading
 * Gives angle between current position and desired location
 */

GPS_Status GPS_Heading(GPS_Coords *coords, GPS_Coords *wp, int32_t *heading); //TODO new struct for heading int?


#endif /* SRC_SAIL_GPS_H_ */


