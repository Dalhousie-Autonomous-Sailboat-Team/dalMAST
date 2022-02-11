/*
 * sail_sensor.c
 *
 *  Created on: Jul 8, 2016
 *      Author: Julia SARTY
 */

#include <asf.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <inttypes.h>

#include "sail_comp.h"
#include "sail_debug.h"
#include "sail_eeprom.h"
#include "sail_gps.h"
#include "sail_wind.h"

enum status_code SENS_WindGetAverage(WIND_Info *info);


enum status_code SENS_GPSnav(GPS_Direction *direction){
	
	//TODO add GPS timer
	if (GPS_Init() != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	if (GPS_Enable() != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	if (GPS_GetReading(&coords) != STATUS_OK){
		return STATUS_ERR_DENIED;
	}
	
	if(GPS_DirectionCalc(&coords, &wp, &direction) != STATUS_OK){
		//TODO replace GPS information with "HEAD EAST"
		return STATUS_ERR_DENIED;
	}
	return STATUS_OK;
}

enum status_code SENS_WINDnav(WIND_Info *av){
	double angle_history, speed_history;
	
	if (WIND_Init() != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	if (WIND_Enable() != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	SENS_WindGetAverage(&info);
	
	av->angle = angle_history;
	av->speed = speed_history;
	
	return STATUS_OK;
}

enum status_code SENS_WindGetAverage(WIND_Info *info){

int i;
double *angle_history=0, *speed_history=0;

for(i=0;i<=100;i++){ //TODO define the amount of loops we want this to complete before moving on
	WIND_GetReading(&info);
	*angle_history += info->angle;
	*speed_history += info->speed;
	}
	
	*angle_history /= i;
	*speed_history /= i;

	return STATUS_OK;
}

enum status_code SENS_COMPnav(COMP_HeadingData *info){
	
	if(COMP_Init() != STATUS_OK){
		return STATUS_ERR_DENIED;
	}
	if(COMP_GetReading(&info) != STATUS_OK){
		return STATUS_ERR_DENIED;
	}
		
	return STATUS_OK;
}

