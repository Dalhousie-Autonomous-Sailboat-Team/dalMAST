/* sail_wind.c
 * Implementation of wind vane module for autonomous sailboat.
 * Created on June 20, 2016.
 * Created by Julia Sarty.
 *
 * Belive this is replaced by new "weather station" is Sail_WEATHERSTATION under Sailboat deploy
 * 
 */

#include "sail_wind.h"

#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <status_codes.h>
#include <math.h>

#include "sail_nmea.h"
#include "sail_uart.h"
#include "sail_debug.h"

// TODO Check with Mark to see if these have changed
//ws = weatherstation
#define WS_ON_OFF_PIN		PIN_PA22
#define WS_ON_STATE		true

#define WS_BUFFER_LENGTH	NMEA_BUFFER_LENGTH

#define MWV_HEADER			"IIMWV"
#define MWV_HEADER_LENGTH	5

// MWV format string
#define MWV_FMT				"IIMWV,%"SCNu16".%"SCNu32",%c,"\
							"%"SCNu32".%"SCNu32",%c,"\
							"%c\n"
// Number of fields in the format string							
#define MWV_FMT_LENGTH		7 

// Buffer to hold wind vane strings from NMEA module
static uint8_t wind_buffer[WS_BUFFER_LENGTH];
static uint8_t mwv_buffer[WS_BUFFER_LENGTH];
// Flag to indicate if the module has been initialized
static uint8_t init_flag = 0;
static uint8_t enable_flag = 0;


// Structure to hold parsed wind vane data
static struct WIND_MWVData {
	uint16_t angle;				//angle 0-360 deg
	uint32_t angle_dec;			//decimal of angle
	uint8_t	 ref;				//R relative, T true
	uint32_t wind_speed;		//wind speed
	uint32_t wind_speed_dec;	//decimal of wind speed
	uint8_t  wind_speed_unit;	//wind speed units k/m/n
	uint8_t  status;			//A active, data valid
} mwv_data;

static enum status_code WIND_ParseMWV(void);
static void WEATHERSTATION_InitPin(void);
static void WS_TurnOn(void);
static void WS_TurnOff(void);

enum status_code WEATHERSTATION_Init(void)
{
	// Check if the module has been initialized already
	if (init_flag) {
		// Already successfully initialized, return
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Initialize NMEA channel
	/*
	switch (NMEA_Init(NMEA_WEATHERSTATION)) {
		case STATUS_OK:							// Initialization complete
		case STATUS_ERR_ALREADY_INITIALIZED:	// Already initialized
			break;								// Continue initializing
		default:
			DEBUG_Write_Unprotected("NMEA module could not be initialized!\r\n");
			return STATUS_ERR_DENIED;   		// Return error code
	} */ // I AM COMMENTED OUT!!!
	
	// Initialize the on-off control pin
	WEATHERSTATION_InitPin();
	
	// Turn off wind vane until enabled
	WS_TurnOff();
	
	// Initialization successful, set flag
	init_flag = 1;
	
	// Clear enable flag
	enable_flag = 0;

	return STATUS_OK;
}

//enable weatherstation
enum status_code WS_Enable(void)
{
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the module has already been enabled
	if (enable_flag) {
		return STATUS_NO_CHANGE;
	}
	
	// Return if the receiver cannot be started
	/*
	if (NMEA_Enable(NMEA_WEATHERSTATION) != STATUS_OK) {
		DEBUG_Write("NMEA receiver could not be started!\r\n");
		return STATUS_ERR_DENIED;
	} */ // I AM COMMENTED OUT!!!
	
	// Turn on the device
	WS_TurnOn();
	
	enable_flag = 1;
	
	return STATUS_OK;
}


enum status_code WS_Disable(void)
{
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the module has already been disabled
	if (!enable_flag) {
		return STATUS_NO_CHANGE;
	}
	
	// Turn off the device
	WS_TurnOff();
	
	// Try to stop the NMEA channel
	/*
	if (NMEA_Disable(NMEA_WEATHERSTATION) != STATUS_OK) {
		return STATUS_ERR_DENIED;	// Return error code
	}*/ //I AM COMMENTED OUT!!!
	
	enable_flag = 0;
	
	return STATUS_OK;
}

enum status_code WIND_GetReading(WIND_Reading *reading)
{
	// Check if the module has been initialized
	if (!init_flag || !enable_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the pointer is NULL
	if (reading == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Get the most recent reading from the wind vane
	uint8_t found_reading = 0;
	uint8_t found_mwv = 0;
	
	// This will repeat, switch breaks will break the switch, not the loop...
	/*
	do {
		switch (NMEA_RxString(NMEA_WEATHERSTATION, wind_buffer, WS_BUFFER_LENGTH)) {
			// Data was found
			case STATUS_VALID_DATA:
				found_reading = 1;
				// Copy the string if the message type is correct
				if (strncmp((char *)wind_buffer, (char *)MWV_HEADER, MWV_HEADER_LENGTH) == 0) {
					strcpy((char *)mwv_buffer, (char *)wind_buffer);
					found_mwv = 1;
				}
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
				found_reading = 1;
				break;
		}
	} while (found_reading);
	*/ // I AM COMMENTED OUT!!!
	
	// Return if the message type is incorrect
	if (!found_mwv) {
		return STATUS_NO_CHANGE;
	}
	
	// Return an error if the data could not be parsed
	if (WIND_ParseMWV() != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}

	//calculates values
	reading->angle = (double)mwv_data.angle + (double)mwv_data.angle_dec/10.0;
	reading->speed = (double)mwv_data.wind_speed + (double)mwv_data.wind_speed_dec/10.0;
	// TODO Read from the wind vane to see what scaling factor should be applied
	reading->speed *= 0.514444;

	return STATUS_OK;
}


enum status_code WIND_ParseMWV(void)
{
	int scanCnt = sscanf((char *)mwv_buffer, MWV_FMT, &mwv_data.angle,
													  &mwv_data.angle_dec,
											 (char *) &mwv_data.ref,
													  &mwv_data.wind_speed,
													  &mwv_data.wind_speed_dec,
											 (char *) &mwv_data.wind_speed_unit,
											 (char *) &mwv_data.status);

	return (scanCnt == MWV_FMT_LENGTH ? STATUS_OK : STATUS_ERR_BAD_DATA);
}


static void WEATHERSTATION_InitPin(void) {
    struct port_config config_port_pin;
    port_get_config_defaults(&config_port_pin);
    config_port_pin.direction = PORT_PIN_DIR_OUTPUT_WTH_READBACK;
    port_pin_set_config(WS_ON_OFF_PIN, &config_port_pin);	
}


static void WS_TurnOn(void) {
	port_pin_set_output_level(WS_ON_OFF_PIN, WS_ON_STATE);	
}


static void WS_TurnOff(void) {
	port_pin_set_output_level(WS_ON_OFF_PIN, !WS_ON_STATE);		
}

