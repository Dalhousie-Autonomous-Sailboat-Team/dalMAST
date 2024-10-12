/* sail_tasksinit.c
 * Creates all of the freeRTOS tasks for controlling the sailboat
 * and starts the task scheduler
 * Header file for the task initialization module of the sailboat
 * Created on April 11, 2019
 * Created by Serge Toutsenko
 */

#include "sail_tasksinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sail_debug.h"
#include "usart_interrupt.h"

#include "sail_types.h"
#include "sail_ctrl.h"
#include "sail_led.h"

// Header files of different devices containing RTOS tasks.
#include "sail_actuator.h"
#include "sail_anglesensor.h"
#include "sail_eeprom.h"
#include "sail_gps.h"
#include "sail_imu.h"
#include "sail_ina.h"
#include "sail_nav.h"
#include "sail_nmea.h"
#include "sail_radio.h"
#include "sail_rudder.h"
#include "sail_wind.h"
#include "sail_beacon.h"
#include "sail_wdt.h"

enum all_tasks running_task;

EventGroupHandle_t mode_event_group = NULL;
SemaphoreHandle_t write_buffer_mutex[UART_NUM_CHANNELS];


enum status_code init_tasks(void) {
	
	// Initialize the mode event group
	mode_event_group = xEventGroupCreate();
	
	// Initialize the mutexes for writing to the uart buffers
	int i;
	for(i = 0; i < UART_NUM_CHANNELS; i++){
		write_buffer_mutex[i] = xSemaphoreCreateMutex();
	}
	
	// Task for reading incoming data from the GPS
	//xTaskCreate( ReadGPS, NULL, GPS_STACK_SIZE, NULL, GPS_PRIORITY, NULL );	

	// Task for reading incoming data from the weather station
	//xTaskCreate( ReadWeatherSensor, NULL, WEATHER_SENSOR_STACK_SIZE, NULL, WEATHER_SENSOR_PRIORITY, NULL );
	
	// Task for updating the course of the sailboat
	//xTaskCreate( UpdateCourse, NULL, UPDATE_COURSE_STACK_SIZE, NULL, UPDATE_COURSE_PRIORITY, NULL );
	
	// Task for changing the position of the rudder
	//xTaskCreate( ControlRudder, NULL, CONTROL_RUDDER_STACK_SIZE, NULL, CONTROL_RUDDER_PRIORITY, NULL );
	
	// Task for handling incoming messages to the radio
	//TaskCreate( RadioHandler, NULL, RADIO_HANDLER_STACK_SIZE, NULL, RADIO_HANDLER_PRIORITY, NULL );
	
	// Task for transmitting logs using the radio
	//xTaskCreate( LogData, NULL, LOG_DATA_STACK_SIZE, NULL, LOG_DATA_PRIORITY, NULL );
	
	// Task for getting the heading from the compass
	//xTaskCreate( ReadCompass, NULL, READ_COMPASS_STACK_SIZE, NULL, READ_COMPASS_PRIORITY, NULL );
	
	//Internal watchdog task
	xTaskCreate(intWDT_Task, NULL, WATCHDOG_STACK_SIZE, NULL, WATCHDOG_PRIORITY, NULL);
	
	//External watchdog task
	//xTaskCreate(extWDT_Task, NULL, configMINIMAL_STACK_SIZE, WATCHDOG_PRIORITY, 1, NULL);
	
	/* Device Testing tasks: */
	
	//xTaskCreate(Test_Actuator, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	//xTaskCreate(Test_IMU, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	//xTaskCreate(Test_AS, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	//xTaskCreate(Test_EEPROM, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	//xTaskCreate( ReadWIND, NULL, WIND_STACK_SIZE, NULL, WIND_PRIORITY, NULL );
	//xTaskCreate(Test_Rudder, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	xTaskCreate(Test_INA, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);

	// Task to blink an LED on the pcb, to ensure that the CPU is working.
	//xTaskCreate(Debug_LED, NULL, configMINIMAL_STACK_SIZE ,NULL, 1, NULL);
	
	//xTaskCreate(beaconStringResponse, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	//xTaskCreate(beaconTaskTest, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	
	//pass control to FreeRTOS kernel
	vTaskStartScheduler();
	
	// The program should not reach this point
	// If it does, more freeRTOS heap memory must be allocated
	return STATUS_ERR_INSUFFICIENT_RTOS_HEAP;
}

void vApplicationDaemonTaskStartupHook(void) {
	xEventGroupSetBits(mode_event_group, CTRL_MODE_AUTO_BIT);
}