/*
 * sail_beacon.c
 *
 * Created: 9/27/2023 2:59:28 PM
 *  Author: manav
 */ 
#include "sail_debug.h"
#include "sail_tasksinit.h"
#include "sail_uart.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include <string.h>
#include <delay.h>


#define TEST_BEACON_DELAY_MS 30000

void WriteBeacon(void){
	//RADIO_GenericMsg rxBeaconCmd;
	
	char rxCmd[128];
	
	DEBUG_Write("Reading Beacon Data.\n");
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	
	//figure out what event bits are
	
	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x01;
		taskEXIT_CRITICAL();
		
		DEBUG_Write("Receiving beacon command over radio.\n");
		
		running_task = eUpdateCourse;
		
		//Receive command over radio.
		//This has to somehow be translated into the string form of the command that he beacon can process.
		//RADIO_RxMsg(rxBeaconCmd);
		DEBUG_Write("Translating received radio signal into beacon command.\n");
		//SET rxCmd to the data string processed in the RADIO_RxMsg function.
		
		//extract rxCmd somehow.
		
		
		//After rxCmd has the command loaded into it and is terminated with a \r and \n.
		DEBUG_Write("Transmitting command to beacon, expect an email if relevant command was sent.\n");
		UART_TxString(UART_WIND,rxCmd);
		
		vTaskDelay(testDelay);
	}
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
	//char sentString[256];
	char cmd3 = "at+cier=1,1,1\r";
	char cmd4 = "at+sbdwt=hello\r\n";
	char cmd5 = "at+sbdix\r\n";
	char sentString = "hello";
	char delimit = "\r\n";
	//char rxString[0] = '\0';
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	
	UART_Init(UART_WIND);
	
	
	
	while(1){
		running_task = eUpdateCourse;
		
		//cmd4 = "at+sbdwt=";
		
		//if(strcmp(sentString, "hello") == 0){
			//UART_TxString(UART_WIND,cmd3);
		//}
		
		//strcat(cmd4,sentString);
		//strcat(cmd4,delimit);
		UART_TxString(UART_WIND,cmd3);
		delay_ms(10000);
		UART_TxString(UART_WIND,cmd4);
		delay_ms(10000);
		UART_TxString(UART_WIND,cmd5);
		//strcat(sentString,sentString);
		vTaskDelay(testDelay);
	}
}

void beaconStringResponse(void){
	char rxString[256];
	rxString[0] = '\0';
	TickType_t testDelay = pdMS_TO_TICKS(TEST_BEACON_DELAY_MS);
	char sentString[256];
	
	UART_Init(UART_WIND);
	
	while(1){
		running_task = eUpdateCourse;
		DEBUG_Write("----------Sending string----------\r\n");
		
		//Send Test string
		UART_TxString(UART_WIND, sentString);
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



