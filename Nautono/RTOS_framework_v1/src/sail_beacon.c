/*
 * sail_beacon.c
 *
 * Created: 9/27/2023 2:59:28 PM
 *  Author: manav
 */ 
#include "sail_debug.h"
#include "sail_tasksinit.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include <string.h>
#include <delay.h>
#include <asf.h>

#define TEST_BEACON_DELAY_MS 1000
#define BEACON_ENABLE PIN_PA10

void enableBeacon(void){
	// Get config struct for GPIO pin
	struct port_config config_port_pin;
	
	// Select UART MUX channel to initialize:
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(BEACON_ENABLE, &config_port_pin);
	
	port_pin_set_output_level(BEACON_ENABLE, true);
	
}

void disableBeacon(void){
	// Get config struct for GPIO pin
	struct port_config config_port_pin;
	
	// Select UART MUX channel to initialize:
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(BEACON_ENABLE, &config_port_pin);
	
	port_pin_set_output_level(BEACON_ENABLE, false);
}

void Test_Beacon(void)
{

	char str[256];
	
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Beacon >>>>>>>>>>\n\r");
		
		//UART_TxString(UART_SATELLITE, "AT");
		
	//	UART_RxString(UART_SATELLITE, str, 128);
		
		DEBUG_Write("received string: %s\r\n", str);
		
		vTaskDelay(testDelay);
	}
}

void beaconOk(void){
	char rxString[256];
	rxString[0] = '\0';
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	
	
	UART_Init(UART_WIND);
	
	while(1){
		
		running_task = eUpdateCourse;
		DEBUG_Write("Sending >at<\n");
		UART_TxString(UART_WIND,"AT");
		UART_RxString(UART_WIND,rxString, 128);
		DEBUG_Write("String received: >%s<\n", rxString);
		vTaskDelay(testDelay);
	}
}

void beaconStringResponse(void){
	char rxString[256];
	rxString[0] = '\0';
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	
	enableBeacon();
	UART_Init(UART_WIND);
	
	while(1){
		running_task = eUpdateCourse;
		DEBUG_Write("----------Sending string----------\r\n");
		
		//Send Test string
		UART_TxString(UART_WIND,"AT+SBDWT=we made it");
		//Receive string
		UART_RxString(UART_WIND,rxString, sizeof(rxString)>>1);
		//If received string is not expected message
		if(strcmp(rxString, "OK")){
			DEBUG_Write("Received message: %s\r\n", rxString);
			DEBUG_Write("Expected message: 'OK'\r\n");
			DEBUG_Write("Attempting to resend string.\r\n");
			//DEBUG_Write("Attempting to resend string.\r\n");
			//Attempt to resend string
			UART_TxString(UART_WIND,"AT+SBDWT=we made it");
			DEBUG_Write("Finished sending\r\n");
		}else{
			//Initiating SBD session to receive string sent earlier back from modem
			DEBUG_Write("Attempting to open SBD session.\r\n");
			UART_TxString(UART_WIND,"AT+SBDIX");
			UART_RxString(UART_WIND,rxString, sizeof(rxString)>>1);
			DEBUG_Write("String received: %s\r\n", rxString);
		}
		
		vTaskDelay(testDelay);
	}
	
}

