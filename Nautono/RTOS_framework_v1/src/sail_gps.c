/*
 * sail_gps.c
 *
 * Created: 2022-06-02 8:04 PM
 * Author: Kamden Thebeau
 * 
 * Implementation of the sail_gps.h header file. Responsible for storing NMEA gps data 
 * received from gps hardware. 
 */

#include "sail_gps.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_ctrl.h"
#include "sail_tasksinit.h"
#include "usart_interrupt.h"

#include "sail_debug.h"

#define HEADER_FMT				"DALSAIL,%03"PRIu16
#define FIELD_FMT				",%"PRIi32

#define GPS_BUFFER_LENGTH		NMEA_BUFFER_LENGTH

static bool init_flag = false;
static char msg_buffer[GPS_BUFFER_LENGTH];

//stores all the msg types of the gps sensor
GPS_AllMsgs GPS_data;

// Private function declarations
static enum status_code GPS_ExtractMsg(NMEA_GenericMsg* msg, GPS_MsgRawData_t* data);

// Function definitions

void ReadGPS(void) {
	DEBUG_Write("Reading GPS...\r\n");
	uint16_t loop_cnt = 0;
	//set msg type sum to 0 since no messages processed yet
	GPS_data.msg_type_sum = 0;

	TickType_t read_gps_delay = pdMS_TO_TICKS(GPS_SLEEP_PERIOD_MS);

	GPS_On();

	while (1) {

		xEventGroupWaitBits(mode_event_group,                        /* Test the mode event group */
			CTRL_MODE_AUTO_BIT | CTRL_MODE_REMOTE_BIT, /* Wait until the sailboat is in AUTO or REMOTE mode */
			pdFALSE,                                 /* Bits should not be cleared before returning. */
			pdFALSE,                                 /* Don't wait for both bits, either bit will do. */
			portMAX_DELAY);                          /* Wait time does not expire */

		DEBUG_Write("\r\n********** Performing GPS Reading **********\r\n");

		running_task = eReadGPS; //Replace this with gps, see tasksinit.c/.h

		NMEA_GenericMsg msg;

		if (GPS_RxMsg(&msg) == STATUS_OK) {
			loop_cnt++;
			//DEBUG_Write("\nStatus OK\n");
			//DEBUG_Write("loop count %d\r\n", loop_cnt);
			//store msg into msg array at index corresponding to msg type
			DEBUG_Write("message type: %d\r\n", msg.type);
			GPS_data.msg_array[msg.type] = msg;
			//add type to msg type sum to keep track of the saved messages
			GPS_data.msg_type_sum += msg.type;

			DEBUG_Write("\nAll fields obtained... Going to sleep...\r\n");
			GPS_data.msg_type_sum = 0;

			//store weather station data into appropriate structs
			assign_gps_readings();
			//check if waypoint was reached and affect as necessary
			//DEBUG_Write("checking waypoint...\r\n");
			//check_waypoint_state();
//
			////Necessary??????????????
			//DEBUG_Write("processing wind...\r\n");
			////calculate wind parameters
			//process_wind_readings();
			//DEBUG_Write("processing heading...\r\n");
			////calculate heading parameters
			//process_heading_readings();			
		}
		vTaskDelay(read_gps_delay);
	}
}

/*void GPS_Sleep_Sec(unsigned time_sec) {
	GPS_Disable();
	//put thread to sleep for number of ticks specified
	vTaskDelay(time_sec * configTICK_RATE_HZ);
}*/

void GPS_On(void) {
	GPS_Init();
	GPS_Enable();
}

enum status_code GPS_Init(void)
{
	// Return if the module has already been initialized
	if (init_flag) {
		return STATUS_ERR_ALREADY_INITIALIZED;
	}

	// Initialize NMEA channel
	switch (NMEA_Init(NMEA_GPS)) {
	case STATUS_OK: // Initialization complete, continue
		break; 
	case STATUS_ERR_ALREADY_INITIALIZED:	// Already initialized, not a problem
		break;
	default:
		DEBUG_Write_Unprotected("NMEA module could not be initialized!\n");
		return STATUS_ERR_DENIED;
	}

	// Set the initialization flag
	init_flag = true;

	return STATUS_OK;
}


enum status_code GPS_Enable(void)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	// Return if the receiver cannot be started
	if (NMEA_Enable(NMEA_GPS) != STATUS_OK) {
		DEBUG_Write("GPS could not be started!\r\n");
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


enum status_code GPS_Disable(void)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	// Return if the receiver cannot be disabled
	if (NMEA_Disable(NMEA_GPS) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}

/*request to receive message by weather sensor*/
enum status_code GPS_RxMsg(NMEA_GenericMsg* msg)
{

	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	enum status_code rc; 
	rc = NMEA_RxString(NMEA_GPS, (uint8_t*)msg_buffer, NMEA_BUFFER_LENGTH); 
	
	// Check the NMEA receiver for new data
	if (rc != STATUS_VALID_DATA){
		//DEBUG_Write("RAW INVALID GPS MESSAGE: %s\r\n", msg_buffer);
		//DEBUG_Write("RC code: %d\r\n", rc);
		return rc;
	}

	// Extract the raw data from the message
	GPS_MsgRawData_t raw_data;
	char* msg_ptr;
	
	//Show raw data and error code
	//DEBUG_Write("RAW GPS MESSAGE: %s\r\n", msg_buffer);
	//DEBUG_Write("RC code: %d\r\n", rc);
	
	// Get each argument after the type
	uint8_t arg_count = 0;
	
	//DEBUG_Write("WS: %s\r\n", msg_buffer);
	msg_ptr = strtok(msg_buffer, ",");
	// Check that msg is valid NMEA type within type list
	if(!get_NMEA_type(&raw_data.type, msg_ptr))
	{
		//DEBUG_Write("Msg not in list of types...\r\n");
		return STATUS_DATA_NOT_NEEDED;
	}
	//DEBUG_Write("Message type: %s\r\n", NMEA_TYPE_TABLE[raw_data.type]);
	//TODO
	//Verify that numbers are lat lon, a.k.a. do a checksum 
	//ALso check length (should be nominal)
	//TODO
	//lat and long are saved as ddmm.mmmm (dd degree, mm minutes) need to parse this! (or do you?)
	
	while (msg_ptr != NULL) {
		//store msg_ptr as float value into arg array ..
		//if *msg_ptr is alphabetic, store directly, else convert string to float value
		msg_ptr = strtok(NULL, ",");
		if(msg_ptr == NULL){
			//DEBUG_Write("NULL/n/r");
			break;
		}
		//raw_data.args[arg_count++] = isalpha(*msg_ptr) ? *msg_ptr : atof(msg_ptr);
		//memcpy(raw_data.args[arg_count++], msg_ptr, sizeof(msg_ptr));
		raw_data.args[arg_count++] = msg_ptr;
		//DEBUG_Write("msg ptr %s and count %d\r\n", msg_ptr, arg_count);
		//DEBUG_Write("In the raw data: >%s<\r\n", raw_data.args[arg_count-1]);
		if (arg_count == GPS_MSG_MAX_ARGS) break;
	}

	// Parse the message
	if (GPS_ExtractMsg(msg, &raw_data) != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}
	return STATUS_OK;
	//return rc;
}

//extern int was_here;
static enum status_code GPS_ExtractMsg(NMEA_GenericMsg* msg, GPS_MsgRawData_t* data) {
	msg->type = data->type;

	// args[0] is the first argument after the NMEA type
	//ex. GPGGA,<arg[0]>,<arg[1]>...
	switch (data->type)
	{
	case eGPGGA:
		msg->fields.gpgga.lat.lat = atof(data->args[1]);
		msg->fields.gpgga.lat.ns = (data->args[2][0] == 'N') ? north : south;
		msg->fields.gpgga.lon.lon = atof(data->args[3]);
		msg->fields.gpgga.lon.we = (data->args[4][0] == 'W') ? west : east;
		msg->fields.gpgga.alt = atof(data->args[8]);
		
		//if(atof(data->args[1]) == 0.00)
		//{
			//DEBUG_Write("Was 0\r\n");
		//}
		
		//wrap this so it only shows during debug config
		//#ifndef DEBUG_GPS
		
		DEBUG_Write("LAT DATA: >%s<\r\n", data->args[1]);
		DEBUG_Write("LON DATA: >%s<\r\n", data->args[3]);
			
		DEBUG_Write("LAT DATA: >%d<\r\n", (uint)(msg->fields.gpgga.lat.lat));
		DEBUG_Write("LON DATA: >%d<\r\n", (uint)(msg->fields.gpgga.lon.lon));
		
		//#endif

		break;

	/* case eGPVTG:
		msg->fields.gpvtg.course_over_ground = data->args[0];
		break; */

	case eWIMWV:
		msg->fields.wimwv.wind_dir_rel = atof(data->args[0]);
		msg->fields.wimwv.wind_speed_ms = atof(data->args[2]);
		break;

	// This the YXXDR-B type NMEA message
	case eYXXDR:
		if (data->args[0][0] == 'A') {
			msg->fields.yxxdr.pitch_deg = atof(data->args[1]);
			msg->fields.yxxdr.roll_deg = atof(data->args[5]);
		}

		break;

	case eHCHDT:
		msg->fields.hchdt.bearing = atof(data->args[0]);
		break;
		
	default:
		return STATUS_ERR_BAD_DATA;
		break;
	}
	return STATUS_OK;
}