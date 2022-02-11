/*
 * main.c
 *
 *  Created on: Jun 17, 2016
 *      Author: UW-Stream
 */
#include"sail_gps_float.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

int main(void){

	GPS_Coords coords, wp;
	GPS_Direction direction;
	double bearing, distance;

	wp.lat=40.0;
	wp.lon=-60.0;

	coords.lat= 45.0;
	coords.lon= -65.0;

	GPS_DirectionCalc(&coords, &wp, &direction, &distance, &bearing);
	//GPS_Bearing(&coords, &wp, &bearing);
	printf("distance = %.3lf m\n", direction.distance);
	printf("bearing  = %.3lf deg", direction.bearing);
	return 0;
}
