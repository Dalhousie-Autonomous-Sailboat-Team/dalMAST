// sail_gps.h
// Header file for GPS module for autonomous sailboat.
// Created on June 13, 2016.
// Created by Julia Sarty & Thomas Gwynne-Timothy.
//

#ifndef SAIL_GPS_H_
#define SAIL_GPS_H_

#include <status_codes.h>
#include <stdint.h>

#include "sail_types.h"

// GPS_Init
// Initialize GPS module by clearing flags and buffer. 
// Should be called before any other GPS functions.
// Status:
//	 STATUS_OK - GPS initialization was successful
//   STATUS_ERR_DENIED - a driver error has occurred
//   STATUS_ERR_ALREADY_INITIALIZED - the module should only be 
//                                    initialized once
//
enum status_code GPS_Init(void);


// GPS_Enable
// Calls NMEA_RXStart to trigger the GPS channel to begin 
// acquiring data.
// Status:
//   STATUS_OK - data acquisition was started successfully
//   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been 
//                                initialized yet
//   STATUS_ERR_DENIED - a driver error has occurred
//
enum status_code GPS_Enable(void);


// GPS_Disable
// Calls NMEA_RXStop to stop the GPS channel from acquiring data.
// Status:
//   STATUS_OK - data acquisition was started successfully
//   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been 
//                                initialized yet
//   STATUS_ERR_DENIED - a driver error has occurred
//
enum status_code GPS_Disable(void);


// GPS_GetReading
// Get the most recent angle from the GPS, parses NMEA string
// to get data.
// Inputs/Outputs:
//	 reading - pointer to a struct to assign most recent reading
// Status:
//	 STATUS_OK - data was retrieved and parsed successfully
//   STATUS_ERR_BAD_DATA - data was retrieved but it couldn't 
//                         be parsed
//   STATUS_ERR_NOT_INITIALIZED - the GPS module hasn't been 
//                                initialized yet
//   STATUS_ERR_DENIED - a driver error has occurred
//   STATUS_ERR_BAD_ADDRESS - a NULL pointer was provided
//
enum status_code GPS_GetReading(GPS_Reading *reading);


#endif /* SAIL_GPS_H_ */


