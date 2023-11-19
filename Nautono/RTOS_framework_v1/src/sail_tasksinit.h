/* sail_tasksinit.h
 * Header file for the task initialization module of the sailboat
 * Created on April 11, 2019
 * Created by Serge Toutsenko
 */

#ifndef SAIL_TASKSINIT_H
#define SAIL_TASKSINIT_H

#include "config.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"

#include "sail_nmea.h"

// The event group used for setting sailboat modes
extern EventGroupHandle_t mode_event_group;

// The mutex for writing to the uart buffer
extern SemaphoreHandle_t write_buffer_mutex[UART_NUM_CHANNELS];

// The variable use to test if all of the tasks are have run
// so the watchdog timer does not need to reset the microcontroller
extern unsigned char watchdog_counter;

// The value that watchdog_counter must equal for the watchdog to be reset
// This changes depending on the tasks that are running in a specific mode
extern unsigned char watchdog_reset_value;

// The event bits for the mode event group
#define CTRL_MODE_AUTO_BIT     (0x01UL)
#define CTRL_MODE_REMOTE_BIT   (0x02UL)
#define CTRL_MODE_LOAD_BIT     (0x04UL)
#define CTRL_ALL_BITS          (0x07UL)

#define GPS_PRIORITY					  tskIDLE_PRIORITY + 3
#define WEATHER_SENSOR_PRIORITY           tskIDLE_PRIORITY + 4
#define WIND_PRIORITY					  tskIDLE_PRIORITY + 4
#define UPDATE_COURSE_PRIORITY            tskIDLE_PRIORITY + 3
#define CONTROL_RUDDER_PRIORITY           tskIDLE_PRIORITY + 3
#define RADIO_HANDLER_PRIORITY            tskIDLE_PRIORITY + 1
#define LOG_DATA_PRIORITY                 tskIDLE_PRIORITY + 2
#define READ_COMPASS_PRIORITY             tskIDLE_PRIORITY + 3
#define READ_AS_PRIORITY				  tskIDLE_PRIORITY + 3
#define WATCHDOG_PRIORITY                 tskIDLE_PRIORITY

#define GPS_STACK_SIZE					  configMINIMAL_STACK_SIZE + 100
#define WEATHER_SENSOR_STACK_SIZE         configMINIMAL_STACK_SIZE
#define WIND_STACK_SIZE					  configMINIMAL_STACK_SIZE + 100
#define UPDATE_COURSE_STACK_SIZE          configMINIMAL_STACK_SIZE
#define CONTROL_RUDDER_STACK_SIZE         configMINIMAL_STACK_SIZE
// The radio stack size was increased because the small stack size caused the radio task stack to overflow
#define RADIO_HANDLER_STACK_SIZE          configMINIMAL_STACK_SIZE + 100
#define LOG_DATA_STACK_SIZE               configMINIMAL_STACK_SIZE 
#define READ_COMPASS_STACK_SIZE           configMINIMAL_STACK_SIZE 
#define READ_AS_STACK_SIZE           configMINIMAL_STACK_SIZE 
#define WATCHDOG_STACK_SIZE               configMINIMAL_STACK_SIZE

// TODO: update this file
enum all_tasks { eReadGPS, eReadWIND, eUpdateCourse, eControlRudder, eRadioHandler, eLogData, eReadCompass, eReadAS};

extern enum all_tasks running_task;


#endif