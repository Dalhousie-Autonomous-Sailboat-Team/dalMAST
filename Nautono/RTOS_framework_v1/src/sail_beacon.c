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
		
		UART_TxString(UART_SATELLITE, "AT");
		
		UART_RxString(UART_SATELLITE, str, 128);
		
		DEBUG_Write("receieved string: %s\r\n", str);
		
		vTaskDelay(testDelay);
	}
}