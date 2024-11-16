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


#define GPS_RX_BUFFER_LENGTH 32

static bool init_flag = false;
static char rx_buffer[GPS_RX_BUFFER_LENGTH]; // create a buffer to hold 128 characters of a NMEA sentence
static char msg_buffer[GPS_RX_BUFFER_LENGTH];
static char c; 

#define DEBUG_GPS_DELAY 1000

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



//enum status_code write(uint8_t c) {
		//
	//uint8_t buffer = c; 
	//
	//if (I2C_WriteBuffer(I2C_GPS, &buffer, 1, I2C_WRITE_NORMAL)!= STATUS_OK) {
		//DEBUG_Write("Write Error\r\n");
		////error in writing to GPS 2
		//return STATUS_ERR_DENIED; 
	//}
	////data written successfully
	//return STATUS_OK;
//}
//


// read one character from the GPS --> call this very frequently
enum status_code read(void){
	

	if (I2C_ReadBuffer(I2C_GPS, (uint8_t)rx_buffer, GPS_RX_BUFFER_LENGTH, I2C_READ_NORMAL)== STATUS_OK) {
		DEBUG_Write("Read Error\r\n");
	
		return STATUS_ERR_DENIED;
	}

	//data read successfully
	return STATUS_OK;
}








void DEBUG_GPS2(void)
{
	TickType_t testDelay = pdMS_TO_TICKS(DEBUG_GPS_DELAY);

	//GPS2_init();
	
	while(1)
	{
		taskenter_critical();
		watchdog_counter |= 0x20;
		taskexit_critical();
		running_task = eupdatecourse;
		
		
		#ifdef PCB //Only blink when using PCB.
		port_pin_set_output_level(_directionPin, true);
		
		delay_ms(1000);
		
		port_pin_set_output_level(_directionPin, false);
		#endif
		
		
		
		//size_t stuff = 0;
		//DEBUG_Write("\r\n********** Performing GPS 2 Reading **********\r\n");
		
		//DEBUG_Write("%c", msg_buffer[i]);
		
		
		
		//read();
		//DEBUG_Write("%c", current_char);
		//
		//
		////delay_ms(100);
		//
		//
		//for (int i = 0; i <= GPS_BUFFER_LENGTH; i++){
		//DEBUG_Write("%c", msg_buffer[i]);
		////msg_buffer[i]=0; //clear the buffer?
		//}
		
		//
		//if (I2C_ReadBuffer(I2C_GPS, (uint8_t)rx_buffer, GPS_RX_BUFFER_LENGTH, I2C_READ_NORMAL)== STATUS_OK) {
			//DEBUG_Write("Read Error\r\n");
			////error in writing to GPS 2
			////return STATUS_ERR_DENIED;
		//}
		//
		//DEBUG_Write("WHATS UP BITCH");
		//vTaskDelay(testDelay);
	}
	
}


