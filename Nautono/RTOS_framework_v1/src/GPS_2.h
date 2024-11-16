/*
 * GPS_2.h
 *
 * Created: 6/29/2024 1:14:48 PM
 *  Author: 19024
 */ 


#ifndef GPS_2_H_
#define GPS_2_H_

#include "FreeRTOS.h"


//#define size_t unsigned long
//#define uint8_t unsigned char

//
//#include <inttypes.h>
//#include "sail_types.h"

extern enum status_code GPS2_init(void); // check if I2C bus is active
extern enum status_code write(uint8_t c); // write to slave 
extern enum status_code read(void); //read from slave
void DEBUG_GPS2(void); //freeRTOS task that will be running on top layer

#endif /* GPS_2_H_ */