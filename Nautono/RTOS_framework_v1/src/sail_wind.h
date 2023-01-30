// sail_wind.h
// Header file for wind vane module for autonomous sailboat.
// Created on June 20, 2016.
// Created by Julia Sarty & Thomas Gwynne-Timothy.
//

#ifndef SAIL_WIND_H_
#define SAIL_WIND_H_

#include <stdint.h>
#include <stdlib.h>

#include "sail_types.h"

// WIND_Init
// Initialize wv module by clearing flags and buffer.
// Should be called before any other WIND functions.
// Status:
//   STATUS_OK - WV initialization was successful
//   STATUS_ERR_DENIED - a driver error has occurred
//   STATUS_ERR_ALREADY_INITIALIZED - the module should only be initialized once
//
enum status_code WEATHERSTATION_Init(void);


// WIND_Enable
// Calls NMEA_RXStart to trigger the Wind channel to begin acquiring data
// Status:
//   STATUS_OK - data acquisition was started successfully
//   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
//   STATUS_ERR_DENIED - a driver error has occurred
//
enum status_code WS_Enable(void);


// WIND_Disable
// Calls NMEA_RXStop to stop the Wind channel from acquiring data.
// Status:
//   STATUS_OK - data acquisition was started successfully
//   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
//   STATUS_ERR_DENIED - a driver error has occurred
//
enum status_code WS_Disable(void);


// WIND_GetReading
// Gets the most recent angle from the wind vane. Parses NMEA string to get
// data.
// Inputs/Outputs:
//   reading - pointer to struct to store wind vane reading
// Status:
//   STATUS_OK - command was valid and sent successfully and data was received successfully
//   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
//   STATUS_ERR_DENIED - a driver error has occurred
//   STATUS_ERR_BAD_ADDRESS - an invalid address was provided
//
enum status_code WIND_GetReading(WIND_Reading *reading);

#endif /* SAIL_WIND_H_ */
