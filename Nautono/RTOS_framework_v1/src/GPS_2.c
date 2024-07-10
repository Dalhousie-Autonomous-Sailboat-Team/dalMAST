/*
 * GPS2.c
 *
 * Created: 6/29/2024 1:15:18 PM
 *  Author: Shishir Ghosh 
 */ 
#include "GPS_2.h"

#include <asf.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "sail_debug.h"
#include "sail_i2c.h"
#include "sail_types.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_ctrl.h"
#include "sail_tasksinit.h"


static bool init_flag = false;

enum status_code GPS2_init(void) {
	
	if(init_flag)
	{
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	
	
	switch (I2C_Init()) {
		case STATUS_OK:
		case STATUS_ERR_ALREADY_INITIALIZED:
		break;
		default:
		return STATUS_ERR_DENIED;
	}
		
	delay_ms(500);
	
	
	// a basic scanner, see if it ACKs
	
	
	
//
	//gpsI2C->begin();
	//if (baud_or_i2caddr > 0x7F) {
		//_i2caddr = GPS_DEFAULT_I2C_ADDR;
		//} else {
		//_i2caddr = baud_or_i2caddr;
	//}
	//// A basic scanner, see if it ACK's
	//gpsI2C->beginTransmission(_i2caddr);
	//return (gpsI2C->endTransmission() == 0);
	//
	
	init_flag = true;
	//delay(10);
	return STATUS_OK;
}



size_t write(uint8_t c) {
		
	//#if (defined(__AVR__) || ((defined(ARDUINO_UNOR4_WIFI) || defined(ESP8266)) && 
	//!defined(NO_SW_SERIAL)))
	//if (gpsSwSerial) {
		//return gpsSwSerial->write(c);
	//}
	//#endif
	//
	//gpsI2C->beginTransmission(_i2caddr);
	//
	//if (gpsI2C->write(c) != 1) {
		//return 0;
	//}
	//if (gpsI2C->endTransmission(true) == 0) {
		//return 1;
	//}

	return 0;
}

//size_t write(uint8_t c){
	//
	//return 0;
//}






size_t available(void){
	
	
	return 0;
}