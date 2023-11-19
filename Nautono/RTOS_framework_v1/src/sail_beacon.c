
/*
 * sail_beacon.c
 *
 * Created: 11/18/2023 6:15:12 PM
 *  Author: manav
 */ 

#include <stdbool.h>
#include <string.h>

#include "sail_uart.h"
#include "sail_debug.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_ctrl.h"
#include "sail_tasksinit.h"

static bool beacon_init = false;

#define MAX_LEN 120

#define cmd1 "AT+CIER=1,1,1\r"
#define msg_cmd "AT+SBDWT="
#define cmd2 "AT+SPDIX\r"

void Beacon_init(void)
{
	if(UART_Init(UART_XEOS) == STATUS_OK) {
		beacon_init = true;
	}
	
	UART_TxString(UART_XEOS, cmd1);
}

enum status_code BeaconTX(char *str)
{
	enum status_code rc;
	if(!beacon_init) {
		Beacon_init();
	}
	
	if(str == NULL) {
		return STATUS_ERR_BAD_DATA;
	}
	
	if(strlen(str) > 120) {
		return STATUS_ERR_BAD_FORMAT;
	}
	
	rc = UART_TxString(UART_XEOS, msg_cmd);
	rc = UART_TxString(UART_XEOS, str);
	rc = UART_TxString(UART_XEOS, "\r");
	
	delay_ms(5000);
	
	rc = UART_TxString(UART_XEOS, cmd2);
	
	return rc;
}

void TEST_BEACON(void)
{
	Beacon_init();
	
	delay_ms(5000);
	
	while(1)
	{
		running_task = eUpdateCourse;
		DEBUG_Write("<<<<<<<<<<<< Testing Beacon >>>>>>>>>>>>>>\r\n");
		
		BeaconTX("Testing beacon from PCB");
		
		vTaskDelay(10*60000);
	}
	
}
