/*
 * sail_wdt.c
 *
 * Created: 2024-09-16 8:42:51 PM
 *  Author: Nader Hdeib
 */ 

#include "sail_wdt.h"

// Internal watchdog init
void intWDT_Init(void)
{
	struct wdt_conf int_wdt_config;
	
	// Get default watchdog configuration values
	wdt_get_config_defaults(&int_wdt_config);
	
	/*
	The default config values are as follows:
	• Not locked, to allow for further (re-)configuration
	• Enable WDT
	• Watchdog timer sourced from Generic Clock Channel 4
	• A timeout period of 16384 clocks of the Watchdog module clock
	• No window period, so that the Watchdog count can be reset at any time
	• No early warning period to indicate the Watchdog will soon expire
	*/
	
	// Lock the hardware module to avoid corruption
	int_wdt_config.always_on = true;
	
	// Save configuration values
	wdt_set_config(&int_wdt_config);
	
	DEBUG_Write_Unprotected("Internal watchdog initialized.\r\n");
	
}

// External watchdog init
void extWDT_Init(void)
{
	struct port_config config_port_pin;
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(EXT_WDT_PIN, &config_port_pin);
	
	DEBUG_Write_Unprotected("External watchdog initialized.\r\n");
}


// Internal watchdog task
void intWDT_Task(void)
{
	TickType_t int_wdt_delay = pdMS_TO_TICKS(INT_WDT_SLEEP_PERIOD);
	
	while(1){
		taskENTER_CRITICAL();
		intWDT_Kick();
		DEBUG_Write("#################Kicked the internal watchdog######################\r\n");
		taskEXIT_CRITICAL();
		vTaskDelay(int_wdt_delay);
	}
}


// External watchdog task
void extWDT_Task(void)
{
	TickType_t ext_wdt_delay = pdMS_TO_TICKS(EXT_WDT_SLEEP_PERIOD);
	
	while(1){
		taskENTER_CRITICAL();
		extWDT_Kick();
		DEBUG_Write("#################Kicked the external watchdog######################\r\n");
		taskEXIT_CRITICAL();
		vTaskDelay(ext_wdt_delay);
	}
}


// Reset internal watchdog timer
static void intWDT_Kick(void)
{
	// Use driver code to reset internal WDT
	wdt_reset_count();
}


// Reset external watchdog timer
static void extWDT_Kick(void)
{
	//Turn on and off very quickly (pulse)
	//This should be sufficient time as WDT only needs 50 ns pulse to reset timer.
	port_pin_set_output_level(EXT_WDT_PIN, true);
	delay_ms(50);
	port_pin_set_output_level(EXT_WDT_PIN, false);
}