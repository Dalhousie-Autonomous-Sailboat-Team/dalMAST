/* sail_tasksinit.c
 * Creates all of the freeRTOS tasks for controlling the sailboat
 * and starts the task scheduler
 * Header file for the task initialization module of the sailboat
 * Created on April 11, 2019
 * Created by Serge Toutsenko
 */

#define PCB

#include "sail_tasksinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sail_wind.h"
#include "sail_debug.h"
#include "usart_interrupt.h"
#include "sail_nmea.h"
//#include "Sail_WEATHERSTATION.h"
#include "sail_gps.h"
#include "sail_types.h"
#include "sail_nav.h"
#include "sail_ctrl.h"
#include "sail_radio.h"
#include "sail_actuator.h"
#include "sail_imu.h"
#include "sail_anglesensor.h"
#include "sail_eeprom.h"
#include "sail_wind.h"
#include "sail_motor.h"

void WatchDogTask(void);
static void StartWatchDog(void);
static void KickWatchDog(void);

enum all_tasks running_task;

EventGroupHandle_t mode_event_group = NULL;
SemaphoreHandle_t write_buffer_mutex[UART_NUM_CHANNELS];

unsigned char watchdog_counter;
unsigned char watchdog_reset_value = 0x3F;



#ifdef PCB
void Debug_LED(void);
#endif


enum status_code init_tasks(void) {
	
	// Initialize the mode event group
	mode_event_group = xEventGroupCreate();
	
	// Initialize the mutexes for writing to the uart buffers
	int i;
	for(i = 0; i < UART_NUM_CHANNELS; i++){
		write_buffer_mutex[i] = xSemaphoreCreateMutex();
	}
	
	// Initialize the watchdog counter
	watchdog_counter = 0;
	
	// Task for reading incoming data from the GPS
	//xTaskCreate( ReadGPS, NULL, GPS_STACK_SIZE, NULL, GPS_PRIORITY, NULL );	

	// Task for reading incoming data from the weather station
	//xTaskCreate( ReadWeatherSensor, NULL, WEATHER_SENSOR_STACK_SIZE, NULL, WEATHER_SENSOR_PRIORITY, NULL );
	
	// Task for updating the course of the sailboat
	//xTaskCreate( UpdateCourse, NULL, UPDATE_COURSE_STACK_SIZE, NULL, UPDATE_COURSE_PRIORITY, NULL );
	
	// Task for changing the position of the rudder
	//xTaskCreate( ControlRudder, NULL, CONTROL_RUDDER_STACK_SIZE, NULL, CONTROL_RUDDER_PRIORITY, NULL );
	
	// Task for handling incoming messages to the radio
	//xTaskCreate( RadioHandler, NULL, RADIO_HANDLER_STACK_SIZE, NULL, RADIO_HANDLER_PRIORITY, NULL );
	
	// Task for transmitting logs using the radio
	//xTaskCreate( LogData, NULL, LOG_DATA_STACK_SIZE, NULL, LOG_DATA_PRIORITY, NULL );
	
	// Task for getting the heading from the compass
	//xTaskCreate( ReadCompass, NULL, READ_COMPASS_STACK_SIZE, NULL, READ_COMPASS_PRIORITY, NULL );
	
	// Task for reseting the watchdog so that the microcontroller is not restarted
	//xTaskCreate( WatchDogTask, NULL, WATCHDOG_STACK_SIZE, NULL, WATCHDOG_PRIORITY, NULL );
	
	//xTaskCreate(Test_Actuator, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	
	//xTaskCreate(Test_IMU, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	
	//xTaskCreate(Test_AS, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	
	//xTaskCreate(Test_EEPROM, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	//xTaskCreate( ReadWIND, NULL, WIND_STACK_SIZE, NULL, WIND_PRIORITY, NULL );
	xTaskCreate(Test_Rudder, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);

#ifdef PCB
	xTaskCreate(Debug_LED, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
#endif
	//pass control to FreeRTOS kernel
	vTaskStartScheduler();
	
	
	// The program should not reach this point
	// If it does, more freeRTOS heap memory must be allocated
	return STATUS_ERR_INSUFFICIENT_RTOS_HEAP;
	
}

void WatchDogTask(void){
	
	const TickType_t xDelay = pdMS_TO_TICKS(30000);
	
	while(1){
		
		//KickWatchDog();
		//vTaskDelay(xDelay);
		
		taskENTER_CRITICAL();
		if (watchdog_counter == watchdog_reset_value) {
			watchdog_counter = 0x00;
			KickWatchDog();
			DEBUG_Write("#################Kicked the watchdog######################\r\n");
		}
		taskEXIT_CRITICAL(); 
		
		
		
	}
}

static void StartWatchDog(void)
{
	struct wdt_conf config_wdt;

	wdt_get_config_defaults(&config_wdt);

	config_wdt.always_on            = false;
	config_wdt.clock_source         = GCLK_GENERATOR_4;
	config_wdt.timeout_period       = WDT_PERIOD_16384CLK;
	config_wdt.early_warning_period = WDT_PERIOD_8192CLK;

	wdt_set_config(&config_wdt);
}


static void KickWatchDog(void)
{
	wdt_reset_count();
}

// Runs freeRTOS initialization code immediately after the scheduler is started
void vApplicationDaemonTaskStartupHook(void) {
	xEventGroupSetBits(mode_event_group, CTRL_MODE_AUTO_BIT);
	watchdog_reset_value = 0x3F;
	
	// Start the watchdog timer
	//StartWatchDog();
}

#ifdef PCB

static uint8_t _directionPin;
#define DEBUG_LED_DELAY 1000

void Debug_LED(void)
{
	TickType_t testDelay = pdMS_TO_TICKS(DEBUG_LED_DELAY);
	
	_directionPin = PIN_PA25;
	
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);

	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;

	port_pin_set_config(_directionPin, &config_port_pin);
	
	while(1)
	{
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		
		//DEBUG_Write_Unprotected("Flashing LED\r\n");
	
		port_pin_set_output_level(_directionPin, true);
		
		delay_ms(1000);
		
		port_pin_set_output_level(_directionPin, false);
		
		vTaskDelay(testDelay);
	}
}

#endif