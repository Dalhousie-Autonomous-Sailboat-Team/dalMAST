/*
 * sail_beacon.c
 *
 * Created: 9/27/2023 2:59:28 PM
 *  Author: manav
 */ 
#include "sail_debug.h"
#include "sail_tasksinit.h"
#include "sail_uart.h"
#include "sail_beacon.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include <string.h>
#include <delay.h>


#define TEST_BEACON_DELAY_MS 1000

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
	uint8_t rxString[256];
	uint8_t at_cmd[] = "AT";
	rxString[0] = '\0';
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	
	UART_Init(UART_WIND);
	
	while(1){
		
		running_task = eUpdateCourse;
		DEBUG_Write("Sending >at<\n");
		UART_TxString(UART_WIND, at_cmd);
		UART_RxString(UART_WIND,rxString, 128);
		DEBUG_Write("String received: >%s<\n", rxString);
		vTaskDelay(testDelay);
	}
}

void beaconStringResponse(void){
	uint8_t rxString[256];
	uint8_t sent_at_cmd[]		= "AT+SBDWT=we made it";
	uint8_t ok_response[]		= "OK";
	uint8_t sbd_session_cmd[]	= "AT+SBDIX";
	rxString[0] = '\0';
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	
	
	UART_Init(UART_WIND);
	
	while(1){
		running_task = eUpdateCourse;
		DEBUG_Write("----------Sending string----------\r\n");
		
		//Send Test string
		UART_TxString(UART_WIND, sent_at_cmd);
		//Receive string
		UART_RxString(UART_WIND,rxString, sizeof(rxString)>>1);
		//If received string is not expected message
		if(strcmp(rxString, ok_response)){
			DEBUG_Write("Received message: %s\r\n", rxString);
			DEBUG_Write("Expected message: 'OK'\r\n");
			DEBUG_Write("Attempting to resend string.\r\n");
			//DEBUG_Write("Attempting to resend string.\r\n");
			//Attempt to resend string
			UART_TxString(UART_WIND, sent_at_cmd);
			DEBUG_Write("Finished sending\r\n");
		}else{
			//Initiating SBD session to receive string sent earlier back from modem
			DEBUG_Write("Attempting to open SBD session.\r\n");
			UART_TxString(UART_WIND, sbd_session_cmd);
			UART_RxString(UART_WIND,rxString, sizeof(rxString)>>1);
			DEBUG_Write("String received: %s\r\n", rxString);
		}
		
		vTaskDelay(testDelay);
	}
	
}

