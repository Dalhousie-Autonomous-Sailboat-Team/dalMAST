
/*
 * sail_led.c
 *
 * Created: 10/29/2023 12:53:36 PM
 *  Author: manav
 */ 
#include <asf.h>
#include "sail_tasksinit.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "sail_debug.h"
#include "sail_uart.h"

static uint8_t _directionPin = PIN_PA25;

#define DEBUG_LED_DELAY 1000

static void LED_init(void) 
{
#ifdef PCB
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);

	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;

	port_pin_set_config(_directionPin, &config_port_pin);
#endif
}

// LED blink Task:
void Debug_LED(void)
{
	//UART_Init(UART_VCOM);
	
	TickType_t testDelay = pdMS_TO_TICKS(DEBUG_LED_DELAY);

	LED_init();
	
	while(1)
	{
		//DEBUG_Write("LED is blinking\r\n");
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		
#ifdef PCB //Only blink when using PCB.
		port_pin_set_output_level(_directionPin, true);
		
		delay_ms(1000);
		
		port_pin_set_output_level(_directionPin, false);
#endif
		vTaskDelay(testDelay);
	}
}
