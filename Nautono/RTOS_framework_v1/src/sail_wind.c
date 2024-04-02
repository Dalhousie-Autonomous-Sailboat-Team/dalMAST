/* sail_wind.c
 * Implementation of wind vane module for autonomous sailboat.
 * Created on June 20, 2016.
 * Created by Julia Sarty.
 *
 * Belive this is replaced by new "weather station" is Sail_WEATHERSTATION under Sailboat deploy
 * 
 */

#include "sail_wind.h"

#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <status_codes.h>
#include <math.h>

#include "sail_tasksinit.h"
#include "sail_nmea.h"
#include "sail_uart.h"
#include "sail_debug.h"
#include "sail_ctrl.h"

// Not sure if this stuff is needed anymore... - KT
#define WS_ON_OFF_PIN		PIN_PA04
#define WS_ON_STATE		true

#define WS_BUFFER_LENGTH	NMEA_BUFFER_LENGTH

#define MWV_HEADER			"IIMWV"
#define MWV_HEADER_LENGTH	5

// MWV format string
#define MWV_FMT				"IIMWV,%"SCNu16".%"SCNu32",%c,"\
							"%"SCNu32".%"SCNu32",%c,"\
							"%c\n"
// Number of fields in the format string							
#define MWV_FMT_LENGTH		7 

#define KNOTS_TO_MS 0.51444444

// Buffer to hold wind vane strings from NMEA module
// static uint8_t wind_buffer[WS_BUFFER_LENGTH];
// static uint8_t mwv_buffer[WS_BUFFER_LENGTH];
// Flag to indicate if the module has been initialized
// static uint8_t init_flag = 0;
// static uint8_t enable_flag = 0;

static bool init_flag = false;
static char msg_buffer[WIND_BUFFER_LENGTH];
static uint8_t ON_OFF_PIN;

WIND_AllMsgs WIND_data;

///// Needed? - KT ///////
// Structure to hold parsed wind vane data
static struct WIND_MWVData {
	uint16_t angle;				//angle 0-360 deg
	uint32_t angle_dec;			//decimal of angle
	uint8_t	 ref;				//R relative, T true
	uint32_t wind_speed;		//wind speed
	uint32_t wind_speed_dec;	//decimal of wind speed
	uint8_t  wind_speed_unit;	//wind speed units k/m/n
	uint8_t  status;			//A active, data valid
} mwv_data;

static enum status_code WIND_ParseMWV(void);
static void WEATHERSTATION_InitPin(void);
static void WS_TurnOn(void);
static void WS_TurnOff(void);
///// Needed? - KT ^^^ ///////

static void init_pins()
{
	ON_OFF_PIN = WS_ON_OFF_PIN;
	
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);

	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;

	port_pin_set_config(ON_OFF_PIN, &config_port_pin);
}

static void WIND_PWR_ON() {
	port_pin_set_output_level(ON_OFF_PIN, true);
}

static void WIND_PWR_OFF() {
	port_pin_set_output_level(ON_OFF_PIN, false);
}

void ReadWIND(void){
    DEBUG_Write("Reading GPS...\r\n");
    uint16_t loop_cnt = 0;
    // Set msg type sum to 0 since no messages processed yet
    WIND_data.msg_type_sum = 0;
    
    // Event bits for holding the state of the event group
    EventBits_t event_bits;
    
    TickType_t read_wind_delay = pdMS_TO_TICKS(WIND_SLEEP_PERIOD_MS);
	
	NMEA_GenericMsg msg;
	
	//init_pins();
	
	WIND_On();
	
	DEBUG_Write("************ Performing Wind Vane Reading ****************\r\n");
    
    while(1) {
        
        event_bits = xEventGroupWaitBits(mode_event_group,
        CTRL_MODE_AUTO_BIT | CTRL_MODE_REMOTE_BIT,  // Wait until sailboat is in AUTO or REMOTE mode
        pdFALSE,                                    // Bits should not be cleared before returning
        pdFALSE,                                    // Don't Wait for both bits, either bit will do
        portMAX_DELAY);                             // Wait time does not expire
        
        taskENTER_CRITICAL();
        watchdog_counter |= 0x01;
        taskEXIT_CRITICAL();
      
        
        // TODO: Add code to make the wind run and try to collect data. See gps.c for reference implementation
        // - Kamden Thebeau (08-02-2023
        
        running_task = eReadWIND;
		
		if(!init_flag) {
			WIND_On();
		}
        
		enum status_code code = WIND_RxMsg(&msg);
		
		if(code == STATUS_VALID_DATA) {
			WIND_data.msg_array[msg.type] = msg;
			//DEBUG_Write("Received Wind data\r\n");
			
			assing_wind_readings();
		}
		
		
		
        vTaskDelay(read_wind_delay);
    }
}


void WIND_On(void){
    WIND_Init();
    WIND_Enable();
	WIND_PWR_ON();
}


enum status_code WIND_Init(void)
{
	// Check if the module has been initialized already
	if (init_flag) {
		// Already successfully initialized, return
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Initialize NMEA channel
	switch (NMEA_Init(NMEA_WIND)) {
		case STATUS_OK:							// Initialization complete
            break;
		case STATUS_ERR_ALREADY_INITIALIZED:	// Already initialized
			break;								// Continue initializing
		default:
			DEBUG_Write_Unprotected("NMEA module could not be initialized!\r\n");
			return STATUS_ERR_DENIED;   		// Return error code
	}
	
	// Initialize the on-off control pin
	// WEATHERSTATION_InitPin();
	
	// Turn off wind vane until enabled
	// WS_TurnOff();
	
	// Initialization successful, set flag
	init_flag = true;
	
	// Clear enable flag
	// enable_flag = 0;

	return STATUS_OK;
}

//enable wind vane
enum status_code WIND_Enable(void)
{
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the module has already been enabled
	//if (enable_flag) {
	//	return STATUS_NO_CHANGE;
	//}
	
	// Return if the receiver cannot be started
    enum status_code s = {0};
    s = NMEA_Enable(NMEA_WIND);
    if (s != STATUS_OK || s != STATUS_NO_CHANGE) {
        DEBUG_Write_Unprotected("NMEA receiver could not be started!\r\n");
        return STATUS_ERR_DENIED;
    }
	
	// Turn on the device
	// WS_TurnOn();
	
	// enable_flag = 1;
	
	return STATUS_OK;
}


enum status_code WIND_Disable(void)
{
	// Check if the module has been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Check if the module has already been disabled
	//if (!enable_flag) {
	//	return STATUS_NO_CHANGE;
	//}
	
	// Turn off the device
	//WS_TurnOff();
	
	// Try to stop the NMEA channel
	if (NMEA_Disable(NMEA_WIND) != STATUS_OK) {
    	return STATUS_ERR_DENIED;	// Return error code
	}
	
	// enable_flag = 0;
	
	return STATUS_OK;
}

// TODO: Both the gps.c and wind.c can be generalized and done in just the NMEA.c file
// Should look into refactoring and simplifying code/generalizing for any NMEA case. 
// If GPS or WIND is required, should pass that in as input, and return related result
// - Kamden Thebeau (08-02-2023)

enum status_code WIND_RxMsg(NMEA_GenericMsg* msg)
{
    // Return if a null pointer is provided
    if (msg == NULL){
        return STATUS_ERR_BAD_ADDRESS;
    }
    
    enum status_code rc;
    rc = NMEA_RxString(NMEA_WIND, (uint8_t*)&msg_buffer, NMEA_BUFFER_LENGTH);
    // Check the NMEA receiver for new data
    if (rc != STATUS_VALID_DATA){
        return rc;
    }
	
    
    // Extract the raw data from the message
    WIND_MsgRawData_t raw_data;
    char* msg_ptr;
    
    uint8_t arg_count = 0;
	
    #ifdef DEBUG_WIND
		DEBUG_Write("WIND: %s\r\n", msg_buffer);
	#endif
	
    msg_ptr = strtok(msg_buffer, ",");
    // Check that msg is valid NMEA type within the type list
    if (!get_NMEA_type(&raw_data.type, msg_ptr)){
        return STATUS_DATA_NOT_NEEDED;
    }
    
    while (msg_ptr != NULL) {
        
        msg_ptr = strtok(NULL, ",");
        if(msg_ptr == NULL){ // No more data
            // DEBUG_Write("NULL/n/r");
            break; 
        }
        
        raw_data.args[arg_count++] = msg_ptr;
        // DEBUG_Write("Msg ptr %s and count %d\r\n", msg_ptr, arg_count);
        // DEBUG_Write("In the raw data: >%s<\r\n", raw_data.args[arg_count-1]);
        
        if(arg_count == WIND_MSG_MAX_ARGS){
            break;
        }
    }
    
    // Parse the message and save to appropriate values
    if (WIND_ExtractMsg(msg, &raw_data) != STATUS_OK){
        return STATUS_ERR_BAD_DATA;
    }
    
    return rc;
}
    
static enum status_code WIND_ExtractMsg(NMEA_GenericMsg* msg, WIND_MsgRawData_t* data)
{
    msg->type = data->type;
    
    	// args[0] is the first argument after the NMEA type
	//ex. GPGGA,<arg[0]>,<arg[1]>...
	switch (data->type)
	{
	case eGPGGA:
		msg->fields.gpgga.lat.lat = atof(data->args[1]);
		msg->fields.gpgga.lat.ns = ((char)data->args[2] == 'N') ? north : south;
		msg->fields.gpgga.lon.lon = atof(data->args[3]);
		msg->fields.gpgga.lon.we = ((char)data->args[4] == 'W') ? west : east;
		msg->fields.gpgga.alt = atof(data->args[8]);
		
		if (0==atof(data->args[1])){
			DEBUG_Write("Was 0\r\n");
		}
		//TODO
		//wrap this so it only shows during debug config
		#ifdef DEBUG_GPS
		DEBUG_Write("LAT DATA: >%s<\r\n", data->args[1]);
		DEBUG_Write("LON DATA: >%s<\r\n", data->args[3]);
		
		DEBUG_Write("LAT DATA: >%d<\r\n", (int)msg->fields.gpgga.lat.lat);
		DEBUG_Write("LON DATA: >%f<\r\n", msg->fields.gpgga.lon.lon);
		#endif

		break;

		/* case eGPVTG:
			msg->fields.gpvtg.course_over_ground = data->args[0];
			break; */

	case eWIMWV:
	case eIIMWV:
		msg->fields.wimwv.wind_dir_rel = atof(data->args[0]);
		msg->fields.wimwv.wind_speed_ms = atof(data->args[2])*KNOTS_TO_MS;
		
		DEBUG_Write("\n\rRelative wind direction %s\r\n", data->args[0]);
		DEBUG_Write("\n\rWind Speed [m/s] %s\r\n", data->args[2]);
		
		break;
    // A bunch more NMEA message types exist here...
    
    default:
        return STATUS_ERR_BAD_DATA;
        break;
    }
    return STATUS_OK;    
}
 