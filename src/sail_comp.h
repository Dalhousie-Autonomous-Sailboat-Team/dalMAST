// sail_comp.h
// Header file for compass module for autonomous sailboat.
// Created on June 28, 2016.
// Created by Julia Sarty & Thomas Gwynne-Timothy.
//

#ifndef SRC_SAIL_COMP_H_
#define SRC_SAIL_COMP_H_

#include <status_codes.h>

#include "sail_types.h"
	
// COMP_Init
// Turn on compass and initialize driver.
// Status: 
//   STATUS_OK - Compass initialization was successful
//   STATUS_ERR_DENIED - a driver error has occurred
//   STATUS_ERR_ALREADY_INITIALIZED - the module should only be initialized once
//
enum status_code COMP_Init(void);


// COMP_GetReading
// Get specified reading from the compass via I2C interface
// Inputs/Outputs:
//   type - type of data to read (accel, heading, etc.)
//   reading - pointer to struct containing specified compass values
// Status:
//   STATUS_OK - data was read successfully
//   STATUS_ERR_NOT_INITIALIZED - compass module was not initialized
//   STATUS_ERR_DENIED - a driver error has occurred
//
enum status_code COMP_GetReading(COMP_ReadingType type, COMP_Reading *reading);


// COMP_StartCalibration
// Put the compass into calibration mode
// Status:
//   STATUS_OK - the compass was successfully placed into calibration mode.
//   STATUS_ERR_NOT_INITIALIZED - the compass could not be placed into calibration 
//                                mode because it was not previously initialized
//   STATUS_NO_CHANGE - the compass is already in calibration mode
//
enum status_code COMP_StartCalibration(void);


// COMP_StopCalibration
// Get the compass out of calibration mode
// Status:
//   STATUS_OK - calibration mode was successfully stopped
//   STATUS_ERR_NOT_INITIALIZED - the compass was not initialized
//   STATUS_NO_CHANGE - the compass wasn't even in calibration mode
//
enum status_code COMP_StopCalibration(void);

#endif /* SRC_SAIL_COMP_H_ */
