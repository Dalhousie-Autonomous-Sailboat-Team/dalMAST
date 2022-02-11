
#include "sail_ctrl.h"

#include <math.h>
#include <inttypes.h>
#include <stdbool.h>

#include "sail_math.h"
#include "sail_debug.h"
#include "sail_radio.h"
#include "sail_comp.h"
#include "sail_wind.h"
#include "sail_gps.h"
#include "sail_eeprom.h"
#include "sail_types.h"
#include "sail_nav.h"
#include "sail_motor.h"

typedef enum CTRL_TaskIDs {
	CTRL_LOG_DATA,
	CTRL_READ_GPS,
	CTRL_READ_WIND,
	CTRL_ENABLE_WIND,
	CTRL_DISABLE_WIND,
	CTRL_READ_COMP,
	CTRL_SET_RUDDER,
	CTRL_SET_COURSE,
	CTRL_NUM_TASKS
} CTRL_TaskID;

// Time since last reset
static uint64_t t_ms;
// Time step (resolution of RTC)
static uint16_t dt_ms = 200;
// RTC timer
struct rtc_module rtc_timer;

// Task functions
static void LogData(void);
static void ReadGPS(void);
static void ReadWindVane(void);
static void EnableWindVane(void);
static void DisableWindVane(void);
static void ReadCompass(void);
static void ControlRudder(void);
static void UpdateCourse(void);

// Load the task functions into an array
typedef void (*Task)(void);
static Task tasks[] = {
	LogData,
	ReadGPS,
	ReadWindVane,
	EnableWindVane,
	DisableWindVane,
	ReadCompass,
	ControlRudder,
	UpdateCourse
};

// Main loop
static void ControlTimerTick(void);

// Array of interrupts periods, in milliseconds (these can be set, according to the restrictions in the comments below)
static uint16_t task_periods_ms[CTRL_NUM_TASKS] = {
	5000,	// Data log period
	10000,	// GPS read period
	5000,	// Wind read period
	60000,  // Wind vane enable
	60000,  // Wind vane disable
	500,	// Compass read period
	1000,	// Rudder control period
	60000	// Course update period
};

// Array of timestamps, in milliseconds
static uint64_t task_stamps_ms[CTRL_NUM_TASKS];

// Timer utility function
static void StartTimer(void);

// Watchdog timer utilities
static void StartWatchDog(void);
static void KickWatchDog(void);


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
static void HandleMessage(RADIO_GenericMsg *msg);
static RADIO_Status ChangeMode(CTRL_Mode new_mode);
static RADIO_Status ChangeState(CTRL_State new_state);
static RADIO_Status ChangeLogPeriod(uint8_t new_period);
static RADIO_Status AddWayPoint(RADIO_WayPointData *wp_data);
static RADIO_Status AdjustMotors(int8_t sail_angle, int8_t rudder_angle);

// Variables to maintain controller state and mode
static CTRL_Mode mode;
static CTRL_State state;

// Sensor status
static bool sensor_statuses[SENSOR_COUNT] = {
	false,
	false,
	false
};

// Sensor readings
static GPS_Reading gps;
static WIND_Reading wind, avg_wind;
static COMP_Reading comp;

// Radio messages
static RADIO_GenericMsg tx_msg, rx_msg;

// EEPROM data
static EEPROM_Entry wp_entry;
static uint16_t wp_count;

// Current way point
static EEPROM_WayPoint wp;
// Counter to track # of times distance < radius
static uint16_t wp_complete_count;
// Distance between boat and way point
static double wp_distance;

static double course, bearing, sail_deg, rudder_deg; 
static double avg_heading_deg = 0.0;

enum status_code CTRL_InitSystem(void)
{
	// Initialize SAMD20
	system_init();
	
	// Initialize debug UART
	DEBUG_Init();
	
	// Initialize the radio
	if (RADIO_Init() != STATUS_OK) {
		DEBUG_Write("Radio not initialized!\r\n");
		} else {
		DEBUG_Write("Radio initialized!\r\n");
	}
	
	// Enable the radio receiver
	if (RADIO_Enable() != STATUS_OK) {
		DEBUG_Write("Radio not enabled!\r\n");
		} else {
		DEBUG_Write("Radio enabled!\r\n");
	}	

	// Determine reset cause
	tx_msg.type = RADIO_RESET;
	switch (system_get_reset_cause()) {
		case  SYSTEM_RESET_CAUSE_WDT:
			DEBUG_Write("WDT reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_WDT;
			break;
		case SYSTEM_RESET_CAUSE_POR:
			DEBUG_Write("Power on reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_POWER;
			break;
		case SYSTEM_RESET_CAUSE_SOFTWARE:
			DEBUG_Write("Software reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_SW;
			break;
		case SYSTEM_RESET_CAUSE_EXTERNAL_RESET:
			DEBUG_Write("External reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_EXT;
			break;
		default:
			DEBUG_Write("Other type of reset detected!\r\n");
			tx_msg.fields.reset.cause = CTRL_RESET_OTHER;
			break;
	}
	RADIO_TxMsg(&tx_msg);
	
	// Initialize the EEPROM
	if (EEPROM_Init() != STATUS_OK) {
		DEBUG_Write("EEPROM not initialized!\r\n");
	}
	
	// Set default mode and state
	mode = CTRL_MODE_AUTO;
	state = CTRL_STATE_DEPLOY;
	
	// Set the time to 0 ms
	t_ms = 0;
	
	return STATUS_OK;
}


enum status_code CTRL_InitSensors(void)
{
	// Initialize each sensor
	if (GPS_Init() != STATUS_OK) {
		DEBUG_Write("GPS not initialized...\r\n");
	}
	
	if (WIND_Init() != STATUS_OK) {
		DEBUG_Write("Wind vane not initialized...\r\n");
	}
	
	if (COMP_Init() != STATUS_OK) {
		DEBUG_Write("Compass not initialized...\r\n");
	}
	
	return STATUS_OK;
}


enum status_code CTRL_RunLoop(void)
{
	// Enable GPS
	if (GPS_Enable() != STATUS_OK) {
		DEBUG_Write("GPS not enabled...\r\n");
	} else {
		DEBUG_Write("GPS enabled...\r\n");
	}
	
	// Enable wind vane
	if (WIND_Enable() != STATUS_OK) {
		DEBUG_Write("Wind vane not enabled...\r\n");
	} else {
		DEBUG_Write("Wind vane enabled...\r\n");
	}
	
	// Get the current way point
	EEPROM_GetCurrentWayPoint(&wp);
	
	// Reset the way point complete count
	wp_complete_count = 0;
	
	DEBUG_Write("way point: lat - %d\r\n", (int)(wp.pos.lat * 1000.0));
	
	// Initialize the task time stamps
	task_stamps_ms[CTRL_LOG_DATA]     =  2000;
	task_stamps_ms[CTRL_READ_GPS]     =   500;
	task_stamps_ms[CTRL_READ_WIND]    =  1000;
	task_stamps_ms[CTRL_ENABLE_WIND]  = 63000;
	task_stamps_ms[CTRL_DISABLE_WIND] = 33000;
	task_stamps_ms[CTRL_READ_COMP]    =  1500;
	task_stamps_ms[CTRL_SET_RUDDER]   =  2500;
	task_stamps_ms[CTRL_SET_COURSE]   =  2000;

	// Start the motor controller
	MOTOR_Init();
	
	// Start the timer
	StartTimer();
	StartWatchDog();
		
	// Loop and let interrupts take over
	while (true) {
		switch (RADIO_RxMsg(&rx_msg)) {
			case STATUS_OK:
				DEBUG_Write("Received a message!\r\n");
				HandleMessage(&rx_msg);
				break;
			case STATUS_ERR_BAD_DATA:
				DEBUG_Write("Received a corrupt message!\r\n");
				RADIO_Ack(RADIO_STATUS_ERROR);
				break;
			default:
				break;
		}
		delay_ms(50);
	}
	
	return STATUS_OK;
}

static void ControlTimerTick(void)
{
	// Update the system clock
	t_ms += dt_ms;
	
	// Iterate through tasks
	for (int i = 0; i < CTRL_NUM_TASKS; i++) {
		// Check if it is time to run the task
		if (t_ms >= task_stamps_ms[i]) {
			// Run the task
			(*tasks[i])();
			// Update the time stamp
			task_stamps_ms[i] += task_periods_ms[i];
		}
	}
	// Reset the watchdog
	KickWatchDog();
}


/**** TIMER UTILITIES ************************************************************
 * Utility functions to control each of the timers.
 *
 * StartTimer()
 * Configures the control timer and starts it.
 */

static void StartTimer(void)
{
	struct rtc_count_config config_rtc_count;
	rtc_count_get_config_defaults(&config_rtc_count);

	config_rtc_count.prescaler           = RTC_COUNT_PRESCALER_DIV_1;
	config_rtc_count.mode                = RTC_COUNT_MODE_16BIT;
	config_rtc_count.continuously_update = true;		

	rtc_count_init(&rtc_timer, RTC, &config_rtc_count);
	rtc_count_enable(&rtc_timer);
	
	// Register and enable the new callback
	rtc_count_register_callback(&rtc_timer, ControlTimerTick, RTC_COUNT_CALLBACK_OVERFLOW);
	rtc_count_enable_callback(&rtc_timer, RTC_COUNT_CALLBACK_OVERFLOW);
	
	// Set the period
	rtc_count_set_period(&rtc_timer, dt_ms);
}


static void StartWatchDog(void)
{
	struct wdt_conf config_wdt;

	wdt_get_config_defaults(&config_wdt);

	config_wdt.always_on            = false;
	config_wdt.clock_source         = GCLK_GENERATOR_4;
	config_wdt.timeout_period       = WDT_PERIOD_512CLK;
	config_wdt.early_warning_period = WDT_PERIOD_256CLK;

	wdt_set_config(&config_wdt);
}


static void KickWatchDog(void)
{
	wdt_reset_count();
}


/**** MESSAGE HANDLERS ************************************************************/


static void HandleMessage(RADIO_GenericMsg *msg)
{
	// Switch according to the type of message received
	switch (msg->type) {
		case RADIO_ACK:
			// Do nothing
			break;
		case RADIO_MODE:
			// Acknowledge the mode change
			RADIO_Ack(ChangeMode(msg->fields.mode.mode));
			break;
		case RADIO_STATE:
			// Acknowledge the state change
			RADIO_Ack(ChangeState(msg->fields.state.state));
			break;
		case RADIO_REMOTE:
			AdjustMotors(msg->fields.remote.sail_angle, msg->fields.remote.rudder_angle);
			break;
		case RADIO_WAY_POINT:
			RADIO_Ack(AddWayPoint(&msg->fields.wp));
			break;
		case RADIO_CONFIG:
			RADIO_Ack(ChangeLogPeriod(msg->fields.config.period));
			break;
		default:
			// Do nothing
			break;
	}
}


static RADIO_Status ChangeMode(CTRL_Mode new_mode)
{
	/* The operational mode of the controller can be changed at anytime. Depending on
	 * the controller's current mode, different actions may need to be taken.
	 */
	
	// Return if the mode is invalid
	if (new_mode >= CTRL_NUM_MODES) {
		return RADIO_STATUS_FAILURE;
	}
		
	// Return if the new mode is the same as the current mode
	if (new_mode == mode) {
		DEBUG_Write("No mode change!\r\n");
		return RADIO_STATUS_UNCHANGED;
	}
	
	// Start the loading process the new mode is LOAD
	if (new_mode == CTRL_MODE_LOAD) {
		// Reset the way point counter
		wp_count = 0;
		// Start the EEPROM mission configuration
		if (EEPROM_StartMissionConfig() != STATUS_OK) {
			return RADIO_STATUS_FAILURE;
		}
	}
	
	// Stop the loading process if the old mode is LOAD
	else if (mode == CTRL_MODE_LOAD) {
		// End mission configuration
		if (EEPROM_EndMissionConfig() != STATUS_OK) {
			return RADIO_STATUS_FAILURE;
		}
		// Get the current way point
		EEPROM_GetCurrentWayPoint(&wp);
			
		// Reset the way point complete count
		wp_complete_count = 0;
		
		DEBUG_Write("way point: lat - %d\r\n", (int)(wp.pos.lat * 1000.0));		
	}	
	
	switch (new_mode) {
		case CTRL_MODE_AUTO:
			DEBUG_Write("Entering AUTO mode\r\n");
			break;
		case CTRL_MODE_LOAD:
			DEBUG_Write("Entering LOAD mode\r\n");
			break;
		case CTRL_MODE_REMOTE:
			DEBUG_Write("Entering REMOTE mode\r\n");
			break;	
		default:
			// This won't happen because of the check at the top
			break;			
	}
	
	// Apply the mode change
	mode = new_mode;
	
	return RADIO_STATUS_SUCCESS;
}


static RADIO_Status ChangeState(CTRL_State new_state)
{
	/* The state of the controller can be changed when the device is not in LOAD
	 * mode. 
	 */	
	
	// Return if the state is invalid
	if (new_state >= CTRL_NUM_STATES) {
		return RADIO_STATUS_FAILURE;
	}
	
	// Return if the state is unchanged
	if (new_state == state) {
		return RADIO_STATUS_UNCHANGED;
	}
	
	// Return if the mode is LOAD
	if (mode == CTRL_MODE_LOAD) {
		return RADIO_STATUS_FAILURE;
	}
	
	// Apply the state change
	state = new_state;
	
	return RADIO_STATUS_SUCCESS;
}


static RADIO_Status ChangeLogPeriod(uint8_t new_period)
{
	// Return if the log period is out of range
	if (new_period > 60) {
		return RADIO_STATUS_FAILURE;
	}

	task_periods_ms[CTRL_LOG_DATA] = new_period * 1000;
	
	return RADIO_STATUS_SUCCESS;
}


static RADIO_Status AddWayPoint(RADIO_WayPointData *wp_data)
{
	// A way point can only be added when the controller is in LOAD mode.
	
	// Return if the controller is not in LOAD mode
	if (mode != CTRL_MODE_LOAD) {
		return RADIO_STATUS_FAILURE;
	}
	
	// Return if the index does not match the expected
	if (wp_data->idx != wp_count) {
		return RADIO_STATUS_FAILURE;
	}
	
	// Load the data into the entry
	wp_entry.wp = wp_data->data;
	wp_entry.next_wp_idx = wp_data->next_idx;
	
	// Load the entry into the EEPROM
	if (EEPROM_LoadWayPoint(&wp_entry) != STATUS_OK) {
		return RADIO_STATUS_FAILURE;
	}
	
	// Update the way point counter
	wp_count++;
	
	return RADIO_STATUS_SUCCESS;
}


static RADIO_Status AdjustMotors(int8_t sail_angle, int8_t rudder_angle)
{
	MOTOR_SetSail((double)sail_angle);
	MOTOR_SetRudder((double)rudder_angle);
	DEBUG_Write("Setting rudder angle to %d\r\n", rudder_angle);
	return RADIO_STATUS_SUCCESS;	
}


/**** TIMER CALLBACKS ************************************************************/
static void LogData(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}
	
	// Log the GPS coordinates
	tx_msg.type = RADIO_GPS;
	tx_msg.fields.gps.data = gps;
	RADIO_TxMsg(&tx_msg);
	
	// Log the wind speed and direction
	tx_msg.type = RADIO_WIND;
	tx_msg.fields.wind.data = wind;
	// Correct wind angle with average heading
	tx_msg.fields.wind.data.angle += avg_heading_deg;
	RADIO_TxMsg(&tx_msg);
	
	// Log the compass data
	tx_msg.type = RADIO_COMP;
	tx_msg.fields.comp.data = comp;
	RADIO_TxMsg(&tx_msg);

	// Log the navigation data
	tx_msg.type = RADIO_NAV;
	tx_msg.fields.nav.wp = wp;
	tx_msg.fields.nav.distance = wp_distance;
	tx_msg.fields.nav.bearing = bearing;
	tx_msg.fields.nav.course = course;
	tx_msg.fields.nav.sail_angle = sail_deg;
	tx_msg.fields.nav.rudder_angle = rudder_deg;
	RADIO_TxMsg(&tx_msg);
	
	//DEBUG_Write("Logging data...\r\n");
}

static void ReadGPS(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}
	
	// Get the GPS reading
	GPS_GetReading(&gps);
	
	// Get the distance between boat and way point
	NAV_GetDistance(wp.pos, gps, &wp_distance);
	NAV_GetBearing(wp.pos, gps, &bearing);
	
	// Check if the boat is in the zone
	if (wp_distance < wp.rad) {
		// Decrease gps read period
		task_periods_ms[CTRL_READ_GPS] = 2000;
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
		// Increase way point interval
		task_periods_ms[CTRL_READ_GPS] = 10000;
	}
}

static void ReadWindVane(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}

	// Get the wind vane reading
	if (WIND_GetReading(&wind) != STATUS_OK) {
		return;
	}
	
	// Force the wind to range [0, 360)
	wind.angle = MATH_ForceAngleTo360(wind.angle);
	
	// Convert wind vector to rectangular coordinates
	double wind_x = wind.speed * cos(wind.angle * M_PI / 180.0);
	double wind_y = wind.speed * sin(wind.angle * M_PI / 180.0);
	double avg_x = avg_wind.speed * cos(avg_wind.angle * M_PI / 180.0);
	double avg_y = avg_wind.speed * sin(avg_wind.angle * M_PI / 180.0);
	
	// Update average wind
	avg_x = 0.667 * avg_x + 0.333 * wind_x;
	avg_y = 0.667 * avg_y + 0.333 * wind_y;
	avg_wind.speed = sqrt(avg_x * avg_x + avg_y * avg_y);
	avg_wind.angle = 180.0 * atan2(avg_y, avg_x) / M_PI;
	
	DEBUG_Write("cur_wind.speed = %4d cm/s | cur_wind.angle = %4d deg\r\n", (int)(wind.speed * 100), (int)wind.angle);
	DEBUG_Write("avg_wind.speed = %4d cm/s | avg_wind.angle = %4d deg\r\n", (int)(avg_wind.speed * 100), (int)avg_wind.angle);
}

static void EnableWindVane(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}

	// Enable the wind vane
	WIND_Enable();
}

static void DisableWindVane(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}

	// Disable the wind vane
	WIND_Disable();
}

static void ReadCompass(void)
{
	// Return if the controller is in LOAD mode
	if (mode == CTRL_MODE_LOAD) {
		return;
	}

	// Get the compass reading
	COMP_GetReading(COMP_HEADING, &comp);

	// Update the averaged heading
	avg_heading_deg = 0.9 * avg_heading_deg + 0.1 * comp.data.heading.heading;
}

static void ControlRudder(void)
{
	// Return if the controller is not in autonomous mode
	if (mode != CTRL_MODE_AUTO) {
		return;
	}
	
	NAV_CalculateRudderPosition(course, comp.data.heading.heading, &rudder_deg);
	DEBUG_Write("Setting rudder - %d\r\n", (int)rudder_deg);
	MOTOR_SetRudder(rudder_deg);
}

static void UpdateCourse(void)
{
	// Return if the controller is not in autonomous mode
	if (mode != CTRL_MODE_AUTO) {
		return;
	}
	
	NAV_UpdateCourse(wp.pos, gps, avg_wind, avg_heading_deg, &course, &sail_deg);

	DEBUG_Write("course: %6d  sail: %d\r\n", (int)(course*1000.0), (int)(sail_deg*1000.0));

	MOTOR_SetSail(sail_deg);
}



