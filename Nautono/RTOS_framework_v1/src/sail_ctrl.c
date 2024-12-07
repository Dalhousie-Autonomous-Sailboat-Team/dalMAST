
#include "sail_ctrl.h"

#include <math.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "sail_radio.h"
#include "sail_wind.h"
#include "sail_eeprom.h"
#include "sail_rudder.h"
#include "sail_gps.h"
#include "sail_rudder.h"
#include "sail_actuator.h"

#include "sail_nav.h"
#include "sail_debug.h"

#include "sail_math.h"
#include "sail_types.h"

#include "sail_tasksinit.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sail_wdt.h"

// RTC timer
struct rtc_module rtc_timer;

// Private function declarations

/* CTRL_Sleep
 * Set the sleep time of the control unit
 * Input:
 *	 time_sec - sleep time length
 */ 
static void CTRL_Sleep(unsigned time_sec);

static void EnableWeatherStation(void);
static void DisableWeatherStation(void);



/* Message handling functions:
 * HandleMessage()
 * - checks the message type and calls the appropriate function
 * ChangeMode()
 * - changes the operating mode of the controller
 * - starts and stops EEPROM loading process when required
 * ChangeState()
 * - change the state of the controller
 * ChangeLogPeriod()
 * - change the period at which log messages are sent
 * AddWayPoint()
 * - add a way point to the EEPROM
 * AdjustMotors()
 * - apply remotely controlled adjustments to the motors
 */ 

// Sensor readings
static GPS_Reading gps;
static WIND_Reading wind, avg_wind;
static COMP_Reading comp;

// Radio messages
RADIO_GenericMsg tx_msg;

// EEPROM data
EEPROM_Entry wp_entry;
uint16_t wp_count;

// Current way point
EEPROM_WayPoint wp;
// Counter to track # of times distance < radius
uint16_t wp_complete_count;
// Distance between boat and way point
double wp_distance;

uint16_t rudder_deg;
float course, bearing, sail_deg; 
float avg_heading_deg = 0.0;


enum status_code CTRL_InitSystem(void)
{
	// Initialize SAMD20
	system_init();
	
	// Initialize debug UART
	DEBUG_Init();
	
	// Initialize the radio
	if (RADIO_Init() != STATUS_OK) {
		DEBUG_Write_Unprotected("Radio not initialized!\r\n");
		} else {
		DEBUG_Write_Unprotected("Radio initialized!\r\n");
	}
	
	// Enable the radio receiver
	if (RADIO_Enable() != STATUS_OK) {
		DEBUG_Write_Unprotected("Radio not enabled!\r\n");
		} else {
		DEBUG_Write_Unprotected("Radio enabled!\r\n");
	}	

	// Determine reset cause
	tx_msg.type = RADIO_RESET;
	switch (system_get_reset_cause()) {
		case  SYSTEM_RESET_CAUSE_WDT:
			DEBUG_Write_Unprotected("WDT reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_WDT;
			break;
		case SYSTEM_RESET_CAUSE_POR:
			DEBUG_Write_Unprotected("Power on reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_POWER;
			break;
		case SYSTEM_RESET_CAUSE_SOFTWARE:
			DEBUG_Write_Unprotected("Software reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_SW;
			break;
		case SYSTEM_RESET_CAUSE_EXTERNAL_RESET:
			DEBUG_Write_Unprotected("External reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_EXT;
			break;
		default:
			DEBUG_Write_Unprotected("Other type of reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_OTHER;
			break;
	}
	RADIO_TxMsg_Unprotected(&tx_msg);

	// Initialize watchdog timers
	extWDT_Init();
	intWDT_Init();
	
	// Initialize the EEPROM
	if (EEPROM_Init() != STATUS_OK) {
		DEBUG_Write_Unprotected("EEPROM not initialized!\r\n");
	}
	
	// Set default mode and state
	mode = CTRL_MODE_AUTO;
	state = CTRL_STATE_TEST;
	
	return STATUS_OK;
}


enum status_code CTRL_InitSensors(void)
{
	enum status_code status = 0;
	
	//todo: add initialization for AIS module
	status = WIND_Init();
    if(STATUS_OK != status){
        DEBUG_Write_Unprotected("Wind Vane not initialized... \r\n");
		return status;
	}else{
        DEBUG_Write_Unprotected("Wind Init Ok ...\r\n");
		return STATUS_OK;
    }
}



enum status_code startup(void)
{
	// Enable wind vane
	if (WIND_Enable() != STATUS_OK) {
		DEBUG_Write_Unprotected("WS not enabled...\r\n");
        // Return bad status? - KT
	} else {
		DEBUG_Write_Unprotected("WS enabled...\r\n");
	}
	
	// Get the current way point
	//EEPROM_GetCurrentWayPoint(&wp);
	
	// Reset the way point complete count
	//wp_complete_count = 0;
	
	//DEBUG_Write_Unprotected("way point: lat - %d lon - %d rad - %d\r\n", (int)(wp.pos.lat * 1000000.0), (int)(wp.pos.lon * 1000000.0), (int)(wp.rad));
	/*
	// Start the motor controller
	MOTOR_Init();
	*/
	
	RUDDER_Init();
	AC_init();
	
	
	return STATUS_OK;
}

/**** TIMER CALLBACKS ************************************************************/
void LogData(void)
{

	TickType_t log_data_delay = pdMS_TO_TICKS(LOG_DATA_SLEEP_PERIOD_MS);
	
	while(1) {
		
		xEventGroupWaitBits(			mode_event_group,								/* Test the mode event group */
										CTRL_MODE_AUTO_BIT | CTRL_MODE_REMOTE_BIT,		/* Wait until the sailboat is in AUTO or REMOTE mode */
										pdFALSE,										/* Bits should not be cleared before returning. */
									    pdFALSE,										/* Don't wait for both bits, either bit will do. */
									 	portMAX_DELAY);									/* Wait time does not expire */
										  
		DEBUG_Write("----------------------Do log data-------------------\r\n");
		
		RADIO_GenericMsg tx_msg_log;
	    running_task = eLogData;
		
		// Log the GPS coordinates
		tx_msg_log.type = RADIO_GPS;
		tx_msg_log.fields.gps.data = gps;
		RADIO_TxMsg(&tx_msg_log);
	
		// Log the wind speed and direction
		tx_msg_log.type = RADIO_WIND;
		tx_msg_log.fields.wind.data = wind;
		
		//not needed because wind is reported in relation to the vessel's center line
		/*
		// Correct wind angle with average heading
		tx_msg.fields.wind.data.angle += avg_heading_deg;
		*/
		
		RADIO_TxMsg(&tx_msg_log);
	
		// Log the compass data
		tx_msg_log.type = RADIO_COMP;
		tx_msg_log.fields.comp.data = comp;
		RADIO_TxMsg(&tx_msg_log);

		// Log the navigation data
		tx_msg.type = RADIO_NAV;
		tx_msg.fields.nav.wp = wp;
		tx_msg.fields.nav.distance = wp_distance;
		tx_msg.fields.nav.bearing = bearing;
		tx_msg.fields.nav.course = course;
		tx_msg.fields.nav.sail_angle = sail_deg;
		tx_msg.fields.nav.rudder_angle = rudder_deg;
		RADIO_TxMsg(&tx_msg);

		//put thread to sleep until a specific tick count is reached
		vTaskDelay(log_data_delay);
	}
}


static void beaconTxLogData(void){
	uint8_t cmd5[] = "AT+SBDIX\r\n";
	uint8_t at_cmd[] = "AT\r\n";
	uint8_t cier_cmd[] = "AT+CIER=1,1,1\r\n";
	uint8_t rxString[64];
	
	UART_TxString(UART_XEOS, at_cmd);
	vTaskDelay(6000 / portTICK_RATE_MS);
	UART_RxString(UART_XEOS,rxString, sizeof(rxString));
	DEBUG_Write((char *)rxString);
	
	UART_TxString(UART_XEOS, cier_cmd);
	vTaskDelay(6000 / portTICK_RATE_MS);
	UART_RxString(UART_XEOS,rxString, sizeof(rxString));
	DEBUG_Write((char *)rxString);
	
	uint8_t sbdwt_cmd[] = "at+sbdwt=";
	uint8_t data[200] = {'\0'};
	uint8_t delimit[3] = "\r\n";
#ifdef PCB
	sprintf((char *)data, "%5.3lf,%5.3lf", gps.lat, gps.lon);
//#ifdef SENSORREADINGS
	// Sensor readings output to the data string
	sprintf((char *)data, "%5.3lf,%5.3lf,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f"
	, gps.lat, gps.lon, wind.speed, wind.angle, comp.data.heading.roll, 
	comp.data.heading.pitch, bearing, sail_deg, avg_heading_deg);
#else
	// Sensor readings output to the data string
	sprintf((char *)data, "1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0, these are random numbers");

#endif
	//send the stream 211 command followed by data followed by delimiter
	
	
	UART_TxString(UART_XEOS, sbdwt_cmd);   
	UART_TxString(UART_XEOS, data);
	UART_TxString(UART_XEOS, delimit);
	
	vTaskDelay(6000/portTICK_RATE_MS);
	
	UART_RxString(UART_WIND,rxString, sizeof(rxString));
	DEBUG_Write((char *)rxString);
	
	vTaskDelay(6000 / portTICK_PERIOD_MS);

	UART_TxString(UART_XEOS, cmd5);
	
	vTaskDelay(6000 / portTICK_PERIOD_MS);
	
}

TaskFunction_t beaconTaskTest(void){
	TickType_t testDelay = pdMS_TO_TICKS(6000 / portTICK_RATE_MS);
	
	UART_Init(UART_VCOM);
	

	while(1){
		
#ifdef LOL		
	//	running_task = eUpdateCourse;
		//beaconTxLogData();
#endif
		DEBUG_Write("Idk what to write\r\n");
		vTaskDelay(testDelay);
	}
}


void process_wind_readings(void)
{

	// Force the wind to range [0, 360)
	wind.angle = MATH_ForceAngleTo360(wind.angle);
	
	// Convert wind vector to rectangular coordinates
	float wind_x = wind.speed * cos(wind.angle * M_PI / 180.0);
	float wind_y = wind.speed * sin(wind.angle * M_PI / 180.0);
	float avg_x = avg_wind.speed * cos(avg_wind.angle * M_PI / 180.0);
	float avg_y = avg_wind.speed * sin(avg_wind.angle * M_PI / 180.0);
	
	// Update average wind
	avg_x = 0.667 * avg_x + 0.333 * wind_x;
	avg_y = 0.667 * avg_y + 0.333 * wind_y;
	avg_wind.speed = sqrt(avg_x * avg_x + avg_y * avg_y);
	//avg_wind.angle = 180.0 * atan2(avg_y, avg_x) / M_PI;
	
	DEBUG_Write("cur_wind.speed = %4d cm/s | cur_wind.angle = %4d deg\r\n", (int)(wind.speed * 100), (int)wind.angle);
	DEBUG_Write("avg_wind.speed = %4d cm/s | avg_wind.angle = %4d deg\r\n", (int)(avg_wind.speed * 100), (int)avg_wind.angle);
}

static void EnableWeatherStation(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}

	// Enable the wind vane
	WIND_Enable();
}

static void DisableWeatherStation(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}

	// Disable the wind vane
	WIND_Disable();
}

void process_heading_readings(void)
{
	// Update the averaged heading
	avg_heading_deg = 0.9 * avg_heading_deg + 0.1 * comp.data.heading.heading;
}

void assign_gps_readings(void) {
	//assign gps weather sensor data to gps struct
	gps.lat = GPS_data.msg_array[eGPGGA].fields.gpgga.lat.lat;
	gps.lon = GPS_data.msg_array[eGPGGA].fields.gpgga.lon.lon;
	
	//assign distance between boat and waypoint to wp_distance
	//NAV_GetDistance(wp.pos, gps, &wp_distance);
	//assign bearing
	//NAV_GetBearing(wp.pos, gps, &bearing);
	
}

void ControlRudder(void)
{
	TickType_t control_rudder_delay = pdMS_TO_TICKS(CONTROL_RUDDER_SLEEP_PERIOD_MS);
	
	while(1) {
		
		
		xEventGroupWaitBits(			 mode_event_group,    /* Test the mode event group */
										 CTRL_MODE_AUTO_BIT,  /* Wait until the sailboat is in AUTO mode */
										 pdFALSE,             /* Bits should not be cleared before returning. */
									     pdFALSE,             /* Don't wait for both bits, either bit will do. */
										 portMAX_DELAY);      /* Wait time does not expire */
									 
	    DEBUG_Write("***************************Do rudder***********************\r\n");
		
		running_task = eControlRudder;
		/*NAV_CalculateRudderPosition(course, comp.data.heading.heading, &rudder_deg);
		DEBUG_Write("Setting rudder - %d\r\n", (int)rudder_deg);
		MOTOR_SetRudder(rudder_deg); */
		
		//put thread to sleep until a specific tick count is reached
		vTaskDelay(control_rudder_delay);
	}
}


void check_waypoint_state(void) {
		DEBUG_Write("wp_dist: %d, wp_rad: %d\r\n", (int)wp_distance, (int)wp.rad);
		// Check if the boat is in the zone
		if (wp_distance < wp.rad) {
			wp_complete_count++;
			DEBUG_Write("way point in range - #%d\r\n", (int)wp_complete_count);
			} else {
			// Reset the count
			wp_complete_count = 0;
		}
		
		// Check if we can move to the next way point
		if (wp_complete_count > 5) {
			DEBUG_Write("getting new way point!\r\n");
			EEPROM_CompleteCurrentWayPoint();
			EEPROM_GetCurrentWayPoint(&wp);
			wp_complete_count = 0;
		}
	
}


void UpdateCourse(void)
{
	TickType_t update_course_delay = pdMS_TO_TICKS(UPDATE_COURSE_SLEEP_PERIOD_MS);

	while(1) {
				
		xEventGroupWaitBits(			 mode_event_group,    /* Test the mode event group */
										 CTRL_MODE_AUTO_BIT,  /* Wait until the sailboat is in AUTO mode */
										 pdFALSE,             /* Bits should not be cleared before returning. */
										 pdFALSE,             /* Don't wait for both bits, either bit will do. */
										 portMAX_DELAY);      /* Wait time does not expire */
									 
	    DEBUG_Write("\n<<<<<<<<<<<<<<<<<<<<<<<Do update course>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
		
		running_task = eUpdateCourse;
		
		//update course
		NAV_UpdateCourse(wp.pos, gps, avg_wind, avg_heading_deg, &course, &sail_deg);
		/*
		DEBUG_Write("course: %6d  sail: %d\r\n", (int)(course*1000.0), (int)(sail_deg*1000.0));
		MOTOR_SetSail(sail_deg);
		*/
		vTaskDelay(update_course_delay);

	}
	
}

void ReadCompass(void)
{
	TickType_t read_compass_delay = pdMS_TO_TICKS(READ_COMPASS_SLEEP_PERIOD_MS);

	while(1) {
				
		xEventGroupWaitBits(			 mode_event_group,                        /* Test the mode event group */
										 CTRL_MODE_AUTO_BIT | CTRL_MODE_REMOTE_BIT, /* Wait until the sailboat is in AUTO or REMOTE mode */
										 pdFALSE,                                 /* Bits should not be cleared before returning. */
									     pdFALSE,                                 /* Don't wait for both bits, either bit will do. */
									 	 portMAX_DELAY);                          /* Wait time does not expire */
		 
	    DEBUG_Write("\n<<<<<<<<<<<<<<<<<<<<<<<Do read compass>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
		
		running_task = eReadCompass;
		

		// Get the compass reading
		
/* Updated this code for new compass:
		
		if(COMP_GetReading(COMP_HEADING, &comp) !=  STATUS_OK){
			DEBUG_Write("\nERROR\r\n");
		}

		// Update the averaged heading
		avg_heading_deg = 0.9 * avg_heading_deg + 0.1 * comp.data.heading.heading;
		
*/
		
		vTaskDelay(read_compass_delay);

	}


}