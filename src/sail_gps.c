/* sail_gps.c
 * Implementation of GPS module for autonomous sailboat.
 * Created on June 13, 2016.
 * Created by Julia Sarty & Thomas Gwynne-Timothy.
 */

#include "sail_gps.h"

#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include "sail_nmea.h"
#include "sail_debug.h"

#define GPS_ON_OFF_PIN		PIN_PA00
#define GPS_ON_STATE		false

#define GPS_BUFFER_LENGTH	NMEA_BUFFER_LENGTH
#define PI					M_PI
#define R_EARTH				6371e3
#define GPS_MAX_ATTEMPTS	5

#define RMC_FMT "GPRMC,%"SCNu32".%"SCNu32",%c,"\
				"%"SCNu16".%"SCNu32",%c,"\
				"%"SCNu16".%"SCNu32",%c,"\
				"%"SCNu16".%"SCNu16","\
				"%"SCNu16".%"SCNu16","\
				"%"SCNu32
							
// Command to get RMC data with a period of 5 seconds
static char gps_cmd[] = "PMTK314,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
static char gps_ack[] = "PMTK001,314,3";

// Buffer to hold GPS strings from NMEA module
static uint8_t gps_buffer[GPS_BUFFER_LENGTH];
// Flag to indicate if the module has been initialized
// TODO Convert this to an enumerated state
static uint8_t init_flag = 0;

static enum status_code GPS_ParseRMC(void);
static void GPS_InitPin(void);
static void GPS_TurnOn(void);
static void GPS_TurnOff(void);

// Structure to hold parsed GPS data
static struct GPS_RMCData {
	uint32_t fix_time;			// Time of fix (hour:minute:second)
	uint32_t fix_time_dec;		// Time of fix (hour:minute:second)
	uint8_t fix_status;			// Status of fix
	uint16_t lat_mn;			// Latitude with integer minutes
	uint32_t lat_mn_dec;		// Decimal part of latitude minutes
	uint8_t lat_card;			// Latitude cardinality (N or S)
	uint16_t lon_mn;			// Longitude with integer minutes
	uint32_t lon_mn_dec;		// Decimal part of longitude minutes
	uint8_t lon_card;			// Longitude cardinality (E or W)
	uint16_t speed_knot;		// Speed (knots)
	uint16_t speed_knot_dec;	// Decimal part of speed
	uint16_t course_deg;		// Course w.r.t True North (degrees)
	uint16_t course_deg_dec;	// Decimal part of course
	uint32_t date;				// Date
} rmc_data;


enum status_code GPS_Init(void) {
	// Check if the module has been initialized already
	if (init_flag) {
		// Already successfully initialized, return and indicate
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Initialize NMEA channel
	switch (NMEA_Init(NMEA_GPS)) {
		case STATUS_OK:							// Initialization complete, continue
		case STATUS_ERR_ALREADY_INITIALIZED:	// Already initialized, not a problem
			break;
		default:
			DEBUG_Write("NMEA module could not be initialized!\r\n");
			return STATUS_ERR_DENIED;   //returns error code
	}
	
	// Initialize the on-off control pin
	GPS_InitPin();
	
	// Turn off the GPS
	GPS_TurnOff();
	
	// Initialization successful, set flag
	init_flag = 1;

	return STATUS_OK;
}


enum status_code GPS_Enable(void) {
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the receiver cannot be started
	if (NMEA_Enable(NMEA_GPS) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Turn on the GPS
	GPS_TurnOn();

	// Give the GPS some time to warm up
	delay_ms(1000);
	
	// Configure the GPS (up to GPS_MAX_ATTEMPTS)
	for (int attempts = 0; attempts < GPS_MAX_ATTEMPTS; attempts++) {
		// Send a command to get RMC data
		if (NMEA_TxString(NMEA_GPS, (uint8_t *)gps_cmd) != STATUS_OK) {
			return STATUS_ERR_DENIED;
		}
		
		// Give the GPS time to respond
		delay_ms(50);
		
		// Loop through the receive buffer until there's nothing in it
		while (NMEA_RxString(NMEA_GPS, gps_buffer, GPS_BUFFER_LENGTH) == STATUS_VALID_DATA) {
			// Return if the ack was found
			if (strcmp((char *)gps_buffer, gps_ack) == 0) {
				return STATUS_OK;
			}		
		}
		
		// No ack was found - send the config message again
	}
	
	// Number attempts exceeded - error
	return STATUS_ERR_DENIED;
}


enum status_code GPS_Disable(void) {
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Turn off the GPS
	GPS_TurnOff();
	
	// Try to stop the NMEA channel
	if (NMEA_Disable(NMEA_GPS) != STATUS_OK) {
		return STATUS_ERR_DENIED;	// Return error code
	}
	
	return STATUS_OK;
}


enum status_code GPS_GetReading(GPS_Reading *coords) {
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the pointer is NULL
	if (coords == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Get the most recent reading from the GPS
	uint8_t found_reading = 0;
	do {
		switch (NMEA_RxString(NMEA_GPS, gps_buffer, GPS_BUFFER_LENGTH)) {
			// Data was found
			case STATUS_VALID_DATA:
				found_reading = 1;
				// Check if there's something even more recent
				break;
			// Data was not found				
			case STATUS_NO_CHANGE:
				// If data was not found before, return
				if (!found_reading) {
					// DEBUG_Write("No data\r\n");
					return STATUS_NO_CHANGE;
				}
				// Otherwise, break from the loop and process the data
				found_reading = 0;
				break;
			// An error occurred
			default:
				return STATUS_ERR_DENIED;
		}
	} while (found_reading);
	
	// Return an error if the data could not be parsed
	if (GPS_ParseRMC() != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}
	
	// Get latitude
	coords->lat = (double)(rmc_data.lat_mn/100);
	coords->lat += (double)(rmc_data.lat_mn % 100)/60.0;
	coords->lat += (double)rmc_data.lat_mn_dec/10000.0/60.0;
	if (rmc_data.lat_card == 'S') {
		coords->lat *= -1.0;
	}

	// Get longitude
	coords->lon = (double)(rmc_data.lon_mn/100);
	coords->lon += (double)(rmc_data.lon_mn % 100)/60.0;
	coords->lon += (double)rmc_data.lon_mn_dec/10000.0/60.0;
	if (rmc_data.lon_card == 'W') {
		coords->lon *= -1.0;
	}

	return STATUS_OK;
}

/*
enum status_code GPS_DirectionCalc(GPS_Reading *coords, GPS_Reading *wp, GPS_Direction *direction) {

	double lat1, lat2, delta_lat, lon1, lon2, delta_lon, a, b, x, y;
	
	// Calculate distance
	lat1 = coords->lat*PI/180.0;	// Convert current frame to radians
	lat2 = wp->lat*PI/180.0;		// Convert waypoint to radians
	lon1 = coords->lon*PI/180.0;	// Convert current frame to radians
	lon2 = wp->lon*PI/180.0;		// Convert waypoint to radians
	delta_lat = lat1 - lat2;		// Delta lat in radians
	delta_lon = lon2 - lon1;		// Delta lon in radians

	a = sin(delta_lat/2)*sin(delta_lat/2) + cos(lat1)*cos(lat2)*sin(delta_lon/2)*sin(delta_lon/2);
	y = sqrt(a);
	x = sqrt(1-a);
	b = 2 * atan2(y, x);
	direction->distance = R_EARTH * b;
	
	// Calculate bearing
	y = sin(delta_lon)*cos(lat2);
	x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(delta_lon);
	direction->bearing = atan2(y, x);
	direction->bearing *= (180.0/PI);								// Result in degrees
	direction->bearing = fmod(direction->bearing + 360.0, 360.0);	// Result in the range 0-360

	return STATUS_OK;
}
*/


enum status_code GPS_ParseRMC(void) {
	int scanCnt = sscanf((char *)gps_buffer, RMC_FMT, &rmc_data.fix_time,
													  &rmc_data.fix_time_dec,
											 (char * )&rmc_data.fix_status,
													  &rmc_data.lat_mn,
													  &rmc_data.lat_mn_dec,
											 (char * )&rmc_data.lat_card,
													  &rmc_data.lon_mn,
													  &rmc_data.lon_mn_dec,
											 (char * )&rmc_data.lon_card,
													  &rmc_data.speed_knot,
													  &rmc_data.speed_knot_dec,
													  &rmc_data.course_deg,
													  &rmc_data.course_deg_dec,
													  &rmc_data.date);
	
	return (scanCnt == 14 ? STATUS_OK : STATUS_ERR_BAD_DATA);										  
}


static void GPS_InitPin(void) {
    struct port_config config_port_pin;
    port_get_config_defaults(&config_port_pin);
	
    config_port_pin.direction = PORT_PIN_DIR_OUTPUT_WTH_READBACK;
	
    port_pin_set_config(GPS_ON_OFF_PIN, &config_port_pin);
}


static void GPS_TurnOn(void) {
	port_pin_set_output_level(GPS_ON_OFF_PIN, GPS_ON_STATE);
}


static void GPS_TurnOff(void) {
	port_pin_set_output_level(GPS_ON_OFF_PIN, !GPS_ON_STATE);	
}



