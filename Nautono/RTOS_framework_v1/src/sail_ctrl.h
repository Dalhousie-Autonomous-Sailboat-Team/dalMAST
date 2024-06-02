
#ifndef SAIL_CTRL_H_
#define SAIL_CTRL_H_

#include "delay.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"



#include <asf.h>
#include <status_codes.h>
#include "sail_radio.h"

#define LOG_DATA_SLEEP_PERIOD_MS 5000

#define UPDATECOURSE_ON_TIME_SEC 2
#define UPDATECOURSE_LOOP_LIM  UPDATECOURSE_ON_TIME_SEC * configTICK_RATE_HZ
#define UPDATE_COURSE_SLEEP_PERIOD_MS 60000

#define CONTROLRUDDER_ON_TIME_SEC 2
#define CONTROLRUDDER_LOOP_LIM CONTROLRUDDER_ON_TIME_SEC * configTICK_RATE_HZ
#define CONTROL_RUDDER_SLEEP_PERIOD_MS 1000

#define READ_AS_SLEEP_PERIOD_MS 500
#define READ_COMPASS_SLEEP_PERIOD_MS 1000

typedef enum Sensor_Types {
	SENSOR_GPS,
	SENSOR_WIND,
	SENSOR_COMP,
	SENSOR_COUNT
} Sensor_Type;


/* CTRL_InitSystem
 * Initialize the sail boat controller.
 * Status:
 *		STATUS_OK - sail boat controller initialization was successful	
 * Will generate process information in DEBUG mode
 */
enum status_code CTRL_InitSystem(void);


/* CTRL_InitSensors
 * Initialize each sensor
 * Status:
 *		STATUS_OK - Sensors initialization was successful
 * Will generate process information in DEBUG mode
 */
enum status_code CTRL_InitSensors(void);


/* startup
 * Initialize WeatherStation
 * Status:
 *		STATUS_OK - WeatherStation initialization was successful
 * Will generate process information in DEBUG mode
 */ 
enum status_code startup(void);


/* init_tasks
 * Initialize all tasks
 * Status:
 *		STATUS_ERR_INSUFFICIENT_RTOS_HEAP - Contains error when system create task
 */ 
enum status_code init_tasks();

extern CTRL_Mode mode;
extern CTRL_State state;


/* UpdateCourse
 * Update Navigation Parameters
 */ 
void UpdateCourse(void);


/* ControlRudder
 * Commented Function, set rubber degree
 */ 
void ControlRudder(void);


/* LogData
 * Record all sensors data
 */ 
void LogData(void);

/* ReadSailAngle
 * Get reading from angle sensor for sail angle.
 */ 
void ReadSailAngle(void);

/* ReadCompass
 * Get reading from compass
 */ 
void ReadCompass(void);


/* check_waypoint_state
 * Check the difference between the location of the ship and the scheduled route
 */ 
void check_waypoint_state(void);

/* assign_gps_readings
 * Assign GPS reading
 */ 
void assign_gps_readings(void);

/* process_wind_readings
 * Wind unit convert
 */ 
void process_wind_readings(void);


/* process_heading_readings
 * Heading unit convert
 */ 
void process_heading_readings(void);


void assing_wind_readings(void); 


/* CTRL_Sleep
 * Set the sleep time of the control unit
 * Input:
 *	 time_sec - sleep time length
 */ 
static void CTRL_Sleep(unsigned time_sec);

void beaconTaskTest(void);

#endif /* SAIL_CTRL_H_ */ 