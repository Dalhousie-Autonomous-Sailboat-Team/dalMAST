/*
 * sail_gps.c
 *
 * Created: 2022-06-02 8:04 PM
 *  Author: secul
 */

#include "sail_gps.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_ctrl.h"
#include "sail_tasksinit.h"
#include "usart_interrupt.h"



#include "sail_nmea.h"
#include "sail_debug.h"

#define HEADER_FMT				"DALSAIL,%03"PRIu16
#define FIELD_FMT				",%"PRIi32

#define RADIO_BUFFER_LENGTH		NMEA_BUFFER_LENGTH




static bool init_flag = false;
static char msg_buffer[RADIO_BUFFER_LENGTH];

//stores all the msg types of the weather sensor
volatile GPS_AllMsgs gps_data;



void ReadGPS(void) {
	DEBUG_Write("Reading GPS...\r\n");
	uint16_t loop_cnt = 0;
	//set msg type sum to 0 since no messages processed yet
	gps_data.msg_type_sum = 0;

	// Event bits for holding the state of the event group
	EventBits_t event_bits;

	TickType_t read_gps_delay = pdMS_TO_TICKS(GPS_SLEEP_PERIOD_MS);

	while (1) {

		event_bits = xEventGroupWaitBits(mode_event_group,                        /* Test the mode event group */
			CTRL_MODE_AUTO_BIT | CTRL_MODE_REMOTE_BIT, /* Wait until the sailboat is in AUTO or REMOTE mode */
			pdFALSE,                                 /* Bits should not be cleared before returning. */
			pdFALSE,                                 /* Don't wait for both bits, either bit will do. */
			portMAX_DELAY);                          /* Wait time does not expire */
			
		taskENTER_CRITICAL();
		watchdog_counter |= 0x01;
		taskEXIT_CRITICAL();

		GPS_On();

		running_task = eReadWeatherSensor; //Replace this with gps, see tasksinit.c/.h

		GPS_GenericMsg msg;

		if (GPS_RxMsg(&msg) == STATUS_OK) {
			loop_cnt++;
			//DEBUG_Write("\nStatus OK\n");
			//DEBUG_Write("loop count %d\r\n", loop_cnt);
			//store msg into msg array at index corresponding to msg type
			gps_data.msg_array[msg.type] = msg;
			//add type to msg type sum to keep track of the saved messages
			gps_data.msg_type_sum += msg.type;

			DEBUG_Write("\nAll fields obtained... Going to sleep...\r\n");
			gps_data.msg_type_sum = 0;

			//store weather station data into appropriate structs
			assign_gps_readings();
			//check if waypoint was reached and affect as necessary
			DEBUG_Write("checking waypoint...\r\n");
			check_waypoint_state();

			//Necessary??????????????
			DEBUG_Write("processing wind...\r\n");
			//calculate wind parameters
			process_wind_readings();
			DEBUG_Write("processing heading...\r\n");
			//calculate heading parameters
			process_heading_readings();

			//put thread to sleep until a specific tick count is reached
			vTaskDelay(read_gps_delay);
		}
	}
}

void GPS_Sleep_Sec(unsigned time_sec) {
	GPS_Disable();
	//put thread to sleep for number of ticks specified
	vTaskDelay(time_sec * configTICK_RATE_HZ);
}

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
	case STATUS_OK:// Initialization complete, continue
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
		//DEBUG_Write("NMEA receiver could not be started!\r\n");
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


/*identifies the prefix from the NMEA message and assigns it to the message type*/
bool get_NMEA_type(eGPSTRX_t* type) {
	//read prefix
	char* msg_ptr = strtok(msg_buffer, ",");
	int i;
	for (i = 0; i < GPS_NUM_MSG_TYPES; i++) {
		//return true if prefix found in weather station type table
		if (strcmp(msg_ptr, GPS_TYPE_TABLE[i].GPS_Prefix) == 0) {
			//assign type to matched prefix string
			*type = GPS_TYPE_TABLE[i].GPS_id;
			return true;
		}
	}
	return false;
}
char tmp_ptr[8];
int tmp_cntr = 0;
/*request to receive message by weather sensor*/
enum status_code GPS_RxMsg(GPS_GenericMsg* msg)
{
	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Check the NMEA receiver for new data
	switch (NMEA_RxString(NMEA_GPS, (uint8_t*)msg_buffer, NMEA_BUFFER_LENGTH)) {
		// Data was found, continue and process
	case STATUS_VALID_DATA:
		break;
		// Data was not found
	case STATUS_NO_CHANGE:
		return STATUS_NO_CHANGE;
		// An error occurred
	default:
		return STATUS_ERR_DENIED;
	}


	// Extract the raw data from the message
	GPS_MsgRawData raw_data;
	char* msg_ptr;

	//DEBUG_Write("WS: %s\r\n", msg_buffer);

	//assign NMEA string prefix to raw_data type
	if (!get_NMEA_type(&raw_data.type)) {
		return STATUS_DATA_NOT_NEEDED;
	}

	// Get each argument after the type
	uint8_t arg_count = 0;

	while ((msg_ptr = strtok(NULL, ",")) != NULL) {
		//store msg_ptr as float value into arg array ..
		//if *msg_ptr is alphabetic, store directly, else convert string to float value
		raw_data.args[arg_count++] = isalpha(*msg_ptr) ? *msg_ptr : atof(msg_ptr);

		if (arg_count == GPS_MSG_MAX_ARGS) break;
	}

	/*
	// Compare the argument count to its expected value
	if (arg_count != WEATHERSENSOR_arg_counts[raw_data.type]) {
		return STATUS_ERR_BAD_DATA;
	}
	*/

	// Parse the message
	if (GPS_ExtractMsg(msg, &raw_data) != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}

	return STATUS_OK;
}


int last_type = 1337;
int vals[4];

//extern int was_here;
static enum status_code GPS_ExtractMsg(GPS_GenericMsg* msg, GPS_MsgRawData* data) {
	msg->type = data->type;
	last_type = data->type;

	// args[0] is the first argument after the NMEA type
	//ex. GPGGA,<arg[0]>,<arg[1]>...
	switch (data->type)
	{
	case eGPGGA:
		msg->fields.gpgga.lat.lat = data->args[1];
		msg->fields.gpgga.lat.ns = ((char)data->args[2] == 'N') ? north : south;
		msg->fields.gpgga.lon.lon = data->args[3];
		msg->fields.gpgga.lon.we = ((char)data->args[4] == 'W') ? west : east;
		msg->fields.gpgga.alt = data->args[8];
		vals[0] = 1;
		break;

		/* case eGPVTG:
			msg->fields.gpvtg.course_over_ground = data->args[0];
			break; */

	case eWIMWV:
		msg->fields.wimwv.wind_dir_rel = data->args[0];
		msg->fields.wimwv.wind_speed_ms = data->args[2];
		break;

		// This the YXXDR-B type NMEA message
	case eYXXDR:
		if (data->args[0] == 'A') {
			msg->fields.yxxdr.pitch_deg = data->args[1];
			msg->fields.yxxdr.roll_deg = data->args[5];
			vals[3] = 1;
		}

		break;

	case eHCHDT:
		msg->fields.hchdt.bearing = data->args[0];
		vals[1] = 1;
		break;
		/*
		case eWIMWD:
			msg->fields.wimwd.wind_dir_true = data->args[0];
			msg->fields.wimwd.wind_dir_mag = data->args[2];
			msg->fields.wimwd.wind_speed_knot = data->args[4];
			msg->fields.wimwd.wind_speed_ms = data->args[6];
			vals[2] = 1;
		break;

		case eYXXDR:
		//the yxxdr type we are interested in
			if(data->args[0] == 'A') {
				msg->fields.yxxdr.pitch_deg = data->args[1];
				msg->fields.yxxdr.roll_deg = data->args[5];
				vals[3] = 1;
			//	DEBUG_Write("Assigning data!\n");
			}

		break;

		//the remaining message cases are listed below

		case eGPDTM:
		break;
		case eGPGLL:
		break;
		case eGPGSA:
		break;
		case eGPGSV:
		break;
		case eGPRMC:
		break;
		case eGPVTG:
		break;
		case eGPZDA:
		break;
		case eHCHDG:
		break;
		case eHCTHS:
		break;
		case eTIROT:
		break;
		case eWIMDA:
		break;
		case eWIMWV:
		break;
		case eWIMWR:
		break;
		case eWIMWT:
		break;
		*/


	default:
		return STATUS_ERR_BAD_DATA;
		break;
	}
	return STATUS_OK;
}