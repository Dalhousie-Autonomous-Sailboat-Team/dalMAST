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
	
	init_flag = true;
	//delay(10);
	return STATUS_OK;
}



enum status_code write(uint8_t c) {
		
	uint8_t buffer = c; 
	
	if (I2C_WriteBuffer(I2C_GPS, &buffer, 1, I2C_WRITE_NORMAL)!= STATUS_OK) {
		DEBUG_Write("Write Error\r\n");
		//error in writing to GPS 2
		return STATUS_ERR_DENIED; 
	}
	//data written successfully
	return STATUS_OK;
}


size_t available(void){
		
	return 0;
}


size_t read(size_t *data){
	
	//size_t *data; // create buffer to read into
	
	if (I2C_ReadBuffer(I2C_GPS, data, 1, I2C_READ_NORMAL)!= STATUS_OK) {
		DEBUG_Write("Write Error\r\n");
		//error in writing to GPS 2
		return STATUS_ERR_DENIED;
	}
	//data written successfully
	return STATUS_OK;
}


