
#include "sail_radio.h"

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "sail_nmea.h"
#include "sail_debug.h"
#include "sail_rudder.h"
#include "sail_actuator.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_tasksinit.h"


//config for custom formating
#define HEADER_FMT				"DALSAIL,%03"PRIu16
#define FIELD_FMT				",%"PRIi32

#define RADIO_BUFFER_LENGTH		NMEA_BUFFER_LENGTH

static const uint8_t RADIO_arg_counts[RADIO_NUM_MSG_TYPES] = {
	1,
	1,
	1,
	2,
	5,
	1,
	2,
	2,
	4,
	8,
	1
};

typedef struct RADIO_MsgRawData {
	uint16_t type;
	int32_t args[RADIO_MSG_MAX_ARGS];
} RADIO_MsgRawData;

static bool init_flag = false;

static char msg_buffer[RADIO_BUFFER_LENGTH];

static enum status_code RADIO_ExtractMsg(RADIO_GenericMsg *msg, RADIO_MsgRawData *data);
static enum status_code RADIO_ExtractData(RADIO_MsgRawData *data, RADIO_GenericMsg *msg);


// Variables to maintain controller state and mode
CTRL_Mode mode;
CTRL_State state;

//turns everything on
void Radio_On(void) {
	RADIO_Init();
	RADIO_Enable();
}


enum status_code RADIO_Init(void)
{
	// Return if the module has already been initialized
	if (init_flag) {
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Initialize NMEA channel
	switch (NMEA_Init(NMEA_RADIO)) {
		case STATUS_OK:							// Initialization complete, continue
		case STATUS_ERR_ALREADY_INITIALIZED:	// Already initialized, not a problem
			break;
		default:
		DEBUG_Write_Unprotected("NMEA module could not be initialized!\r\n");
			return STATUS_ERR_DENIED;
	}
	
	// Set the initialization flag
	init_flag = true;
	
	return STATUS_OK;
}


enum status_code RADIO_Enable(void)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the receiver cannot be started
	if (NMEA_Enable(NMEA_RADIO) != STATUS_OK) {
		//DEBUG_Write("NMEA receiver could not be started!\r\n");
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;
}


enum status_code RADIO_Disable(void)
{
	// Return if the module hasn't been initialized
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the receiver cannot be disabled
	if (NMEA_Disable(NMEA_RADIO) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;
}


enum status_code RADIO_RxMsg(RADIO_GenericMsg *msg)
{
	//DEBUG_Write("RADIO_RX\r\n");
	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	memset(msg_buffer, NULL, NMEA_BUFFER_LENGTH*sizeof(char));
	
	// Check the NMEA receiver for new data
	switch (NMEA_RxString(NMEA_RADIO, (uint8_t *)msg_buffer, RADIO_BUFFER_LENGTH)) {
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
	RADIO_MsgRawData raw_data;
	char *msg_ptr;

	DEBUG_Write("Rx_radio: %s\r\n", msg_buffer);
	
	// Check the header
	if ((msg_ptr = strtok(msg_buffer, ",")) == NULL)
		return STATUS_ERR_BAD_DATA;
	if (strcmp(msg_ptr, "DALSAIL") != 0)
		return STATUS_ERR_BAD_DATA;
	
	// Check the message type
	if ((msg_ptr = strtok(NULL, ",")) == NULL)
		return STATUS_ERR_BAD_DATA;
	if (sscanf(msg_ptr, "%"SCNu16, &raw_data.type) != 1)
		return STATUS_ERR_BAD_DATA;
	if (raw_data.type >= RADIO_NUM_MSG_TYPES)
		return STATUS_ERR_BAD_DATA;

	// Get each argument
	uint8_t arg_count = 0;
	while ((msg_ptr = strtok(NULL, ",")) != NULL) {
		if (sscanf(msg_ptr, "%"SCNi32, &raw_data.args[arg_count++]) != 1) {
			return STATUS_ERR_BAD_DATA;
		}
	}
	
	// Compare the argument count to its expected value
	if (arg_count != RADIO_arg_counts[raw_data.type]) {
		return STATUS_ERR_BAD_DATA;
	}
	
	// Parse the message
	if (RADIO_ExtractMsg(msg, &raw_data) != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}
	
	return STATUS_OK;
}


enum status_code RADIO_DEBUG_TxMsg(const char * format, ... ){
	if (!init_flag)
		return STATUS_ERR_NOT_INITIALIZED;
	
	va_list args;
	//Load the buffer with string data
	va_start(args,format);
	
	vsnprintf((char*)msg_buffer, NMEA_BUFFER_LENGTH, format, args);
	va_end(args);
	
	
	// Transmit the string
	if (NMEA_TxString(NMEA_RADIO, (uint8_t*)msg_buffer) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	return STATUS_OK;
}


enum status_code RADIO_TxMsg(RADIO_GenericMsg *msg)
{
	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	for(int i = 0; i < NMEA_BUFFER_LENGTH; i++) {
		msg_buffer[i] = NULL;
	}
	
	// Generate the raw data from the message
	RADIO_MsgRawData raw_data;
	if (RADIO_ExtractData(&raw_data, msg) != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}  
	
	// Write the message header to the buffer
	sprintf(msg_buffer, "DALSAIL,%03"PRIu16, raw_data.type);
	
	// Write each argument to the buffer
	char arg_buffer[20];
	int i;
	for (i = 0; i < RADIO_arg_counts[raw_data.type]; i++) {
		// Write the argument to the buffer
		sprintf(arg_buffer, ",%"PRIi32, raw_data.args[i]);
		// Concatenate the argument string to the message string
		strcat(msg_buffer, arg_buffer);
	}
	
	DEBUG_Write("Tx_Radio: %s\r\n", msg_buffer);

	
	// Transmit the string
	if (NMEA_TxString(NMEA_RADIO, (uint8_t*)msg_buffer) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	return STATUS_OK;
}

enum status_code RADIO_TxMsg_Unprotected(RADIO_GenericMsg *msg)
{
	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	for(int i = 0; i < NMEA_BUFFER_LENGTH; i++) {
		msg_buffer[i] = NULL;
	}
	
	// Generate the raw data from the message
	RADIO_MsgRawData raw_data;
	if (RADIO_ExtractData(&raw_data, msg) != STATUS_OK) {
		return STATUS_ERR_BAD_DATA;
	}  
	
	// Write the message header to the buffer
	sprintf(msg_buffer, "DALSAIL,%03"PRIu16, raw_data.type);
	
	// Write each argument to the buffer
	char arg_buffer[20];
	int i;
	for (i = 0; i < RADIO_arg_counts[raw_data.type]; i++) {
		// Write the argument to the buffer
		sprintf(arg_buffer, ",%"PRIi32, raw_data.args[i]);
		// Concatenate the argument string to the message string
		strcat(msg_buffer, arg_buffer);
	}
	
	DEBUG_Write_Unprotected("Tx_Radio: %s\r\n", msg_buffer);

	
	// Transmit the string
	if (NMEA_TxString_Unprotected(NMEA_RADIO, (uint8_t*)msg_buffer) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	return STATUS_OK;
}


enum status_code RADIO_Ack(RADIO_Status status)
{
	// Return if an invalid status is provided
	if (status >= RADIO_NUM_STATUSES) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Build a generic message
	RADIO_GenericMsg msg;
	msg.type = RADIO_ACK;
	msg.fields.ack.status = status;
	
	// Send the message
	return RADIO_TxMsg(&msg);
}


static enum status_code RADIO_ExtractMsg(RADIO_GenericMsg *msg, RADIO_MsgRawData *data) {
	// Return if the pointers are null
	if (msg == NULL || data == NULL) 
		return STATUS_ERR_BAD_ADDRESS;
	
	// Apply parsing based on message type
	switch (data->type) {
		case RADIO_ACK:
			msg->type = RADIO_ACK;
			msg->fields.ack.status = data->args[0];
			break;
		case RADIO_MODE:
			msg->type = RADIO_MODE;
			msg->fields.mode.mode = data->args[0];	
			break;
		case RADIO_STATE:
			msg->type = RADIO_STATE;
			msg->fields.state.state = data->args[0];		
			break;
		case RADIO_REMOTE:
			msg->type = RADIO_REMOTE;
			msg->fields.remote.rudder_angle = data->args[0];
			msg->fields.remote.sail_angle = data->args[1];
			break;
		case RADIO_WAY_POINT:
			msg->type = RADIO_WAY_POINT;
			msg->fields.wp.idx = data->args[0];
			msg->fields.wp.data.pos.lat = ((double)data->args[1])/1000000.0;
			msg->fields.wp.data.pos.lon = ((double)data->args[2])/1000000.0;
			msg->fields.wp.data.rad = (double)data->args[3];
			msg->fields.wp.next_idx = data->args[4];
			break;
		case RADIO_CONFIG:
			msg->type = RADIO_CONFIG;
			msg->fields.config.period = data->args[0];
			break;
		case RADIO_GPS:
			msg->type = RADIO_GPS;
			msg->fields.gps.data.lat = ((double)data->args[0])/1000000.0;
			msg->fields.gps.data.lon = ((double)data->args[1])/1000000.0;
			break;
		case RADIO_WIND:
			msg->type = RADIO_WIND;
			msg->fields.wind.data.angle = ((double)data->args[0])/10.0;
			msg->fields.wind.data.speed = ((double)data->args[1])/10.0;
			break;
		case RADIO_COMP:
			msg->type = RADIO_COMP;
			msg->fields.comp.data.type = data->args[0];
			msg->fields.comp.data.data.fields[0] = ((double)data->args[1])/10.0;
			msg->fields.comp.data.data.fields[1] = ((double)data->args[2])/10.0;
			msg->fields.comp.data.data.fields[2] = ((double)data->args[3])/10.0;
			break;
		case RADIO_NAV:
			msg->type = RADIO_NAV;
			msg->fields.nav.wp.pos.lat = ((double)data->args[0])/1000000.0;
			msg->fields.nav.wp.pos.lon = ((double)data->args[1])/1000000.0;
			msg->fields.nav.wp.rad = (double)data->args[2];
			msg->fields.nav.distance = (double)data->args[3];
			msg->fields.nav.bearing = ((double)data->args[4])/10.0;
			msg->fields.nav.course = ((double)data->args[5])/10.0;
			msg->fields.nav.rudder_angle = data->args[6];
			msg->fields.nav.sail_angle = data->args[7];
			break;
		case RADIO_RESET:
			msg->type = RADIO_RESET;
			msg->fields.reset.cause = data->args[0];
			break;
		default:
			return STATUS_ERR_BAD_DATA;
	}
	
	return STATUS_OK;
}

//can this function and the one above be combined? seems like repeating
static enum status_code RADIO_ExtractData(RADIO_MsgRawData *data, RADIO_GenericMsg *msg)
{
	// Return if the pointers are null
	if (data == NULL || msg == NULL)
		return STATUS_ERR_BAD_ADDRESS;	
		
	// Apply parsing based on message type
	switch (msg->type) {
		case RADIO_ACK:
			data->type = RADIO_ACK;
			data->args[0] = msg->fields.ack.status;
			break;
		case RADIO_MODE:
			data->type = RADIO_MODE;
			data->args[0] = msg->fields.mode.mode;
			break;
		case RADIO_STATE:
			data->type = RADIO_STATE;
			data->args[0] = msg->fields.state.state;
			break;
		case RADIO_REMOTE:
			data->type = RADIO_REMOTE;
			data->args[0] = msg->fields.remote.rudder_angle;
			data->args[1] = msg->fields.remote.sail_angle;
			break;
		case RADIO_WAY_POINT:
			data->type = RADIO_WAY_POINT;
			data->args[0] = msg->fields.wp.idx;
			data->args[1] = (int32_t)(msg->fields.wp.data.pos.lat*10000.0);
			data->args[2] = (int32_t)(msg->fields.wp.data.pos.lon*10000.0);
			data->args[3] = (int32_t)msg->fields.wp.data.rad;
			data->args[4] = msg->fields.wp.next_idx;
			break;
		case RADIO_CONFIG:
			data->type = RADIO_CONFIG;
			data->args[0] = msg->fields.config.period;
			break;
		case RADIO_GPS:
			data->type = RADIO_GPS;
			data->args[0] = (int32_t)(msg->fields.gps.data.lat*10000.0);
			data->args[1] = (int32_t)(msg->fields.gps.data.lon*10000.0);
			break;
		case RADIO_WIND:
			data->type = RADIO_WIND;
			data->args[0] = (int32_t)(msg->fields.wind.data.angle*10.0);
			data->args[1] = (int32_t)(msg->fields.wind.data.speed*10.0);
			break;
		case RADIO_COMP:
			data->type = RADIO_COMP;
			data->args[0] = msg->fields.comp.data.type = COMP_HEADING;
			data->args[1] = (int32_t)(msg->fields.comp.data.data.heading.heading*10.0);
			data->args[2] = (int32_t)(msg->fields.comp.data.data.heading.pitch*10.0);
			data->args[3] = (int32_t)(msg->fields.comp.data.data.heading.roll*10.0);
			/* This used to be here .
			data->args[1] = (int32_t)(msg->fields.comp.data.data.fields[0]*10.0);
			data->args[2] = (int32_t)(msg->fields.comp.data.data.fields[1]*10.0);
			data->args[3] = (int32_t)(msg->fields.comp.data.data.fields[2]*10.0);
			*/
			break;
		case RADIO_NAV:
			data->type = RADIO_NAV;
			data->args[0] = (int32_t)(msg->fields.nav.wp.pos.lat*10000.0);
			data->args[1] = (int32_t)(msg->fields.nav.wp.pos.lon*10000.0);		
			data->args[2] = (int32_t)msg->fields.nav.wp.rad;	
			data->args[3] = (int32_t)msg->fields.nav.distance;
			data->args[4] = (int32_t)(msg->fields.nav.bearing*10.0);
			data->args[5] = (int32_t)(msg->fields.nav.course*10.0);
			data->args[6] = (uint16_t)msg->fields.nav.rudder_angle;
			data->args[7] = (uint16_t)msg->fields.nav.sail_angle;
			break;
		case RADIO_RESET:
			data->type = RADIO_RESET;
			data->args[0] = msg->fields.reset.cause;
			break;		
		default:
			return STATUS_ERR_BAD_DATA;
	}

	return STATUS_OK;
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
	
	switch (new_mode) {
		case CTRL_MODE_AUTO:
			DEBUG_Write("Entering AUTO mode\r\n");
			// Clear the bits except for 0x3F in the counter
			watchdog_counter &= 0x3F;
			// Set the reset value to be 0x3F
			watchdog_reset_value = 0x3F;
			xEventGroupClearBits(mode_event_group, CTRL_ALL_BITS);
			xEventGroupSetBits(mode_event_group, CTRL_MODE_AUTO_BIT);
		break;
		case CTRL_MODE_LOAD:
			DEBUG_Write("Entering LOAD mode\r\n");
			// Clear the bits except for 0x08 in the counter
			watchdog_counter &= 0x08;
			// Set the reset value to be 0x08
			watchdog_reset_value = 0x08;
			xEventGroupClearBits(mode_event_group, CTRL_ALL_BITS);
			xEventGroupSetBits(mode_event_group, CTRL_MODE_LOAD_BIT);
		break;
		case CTRL_MODE_REMOTE:
			DEBUG_Write("Entering REMOTE mode\r\n");
			// Clear the bits except for 1 8 16 and 32
			watchdog_counter &= 0x39;
			// Set the reset value to be 0x39
			watchdog_reset_value = 0x39;
			xEventGroupClearBits(mode_event_group, CTRL_ALL_BITS);
			xEventGroupSetBits(mode_event_group, CTRL_MODE_REMOTE_BIT);
		break;
		default:
		break;
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



static RADIO_Status AddWayPoint(RADIO_WayPointData *wp_data)
{
	DEBUG_Write("Adding waypoint...\r\n");
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


static RADIO_Status AdjustMotors(uint16_t sail_angle, uint16_t rudder_angle)
{
	DEBUG_Write("Setting rudder: %d\tsail: %d\r\n", rudder_angle, sail_angle);
	RudderSetPos((double)rudder_angle);
	LAC_set_pos((double)sail_angle);
	
	return RADIO_STATUS_SUCCESS;	
}

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
		RADIO_Ack(RADIO_STATUS_SUCCESS);
		AdjustMotors(msg->fields.remote.sail_angle, msg->fields.remote.rudder_angle);
		break;
		case RADIO_WAY_POINT:
		RADIO_Ack(AddWayPoint(&msg->fields.wp));
		break;
		default:
		// Do nothing
		break;
	}
	return;
}
#define TEST_XBEE

void RadioHandler(void) {
		unsigned int loop_cnt = 0;
		//uint16_t loop_cnt = 0;
		unsigned int loop_max = 100000; //500000;
		
		TickType_t radio_handler_delay = pdMS_TO_TICKS(RADIO_SLEEP_PERIOD_MS);
		
		//#ifndef TEST

		while (1) {
			running_task = eRadioHandler;
			RADIO_GenericMsg rx_msg;
			
			//DEBUG_Write("<<<<<<<<<<<<<<<<<<<< RADIO HANDLER >>>>>>>>>>>>>>>>>>>>>\r\n");
			
			switch (RADIO_RxMsg(&rx_msg)) {
				case STATUS_OK:
				
					taskENTER_CRITICAL();
					DEBUG_Write("Received a message!\r\n");
					HandleMessage(&rx_msg);
					taskEXIT_CRITICAL();
					
					break;
				
				case STATUS_ERR_BAD_DATA:
					DEBUG_Write("Received a corrupt message!\r\n");
					RADIO_Ack(RADIO_STATUS_ERROR);
					break;
				
				default:
					break;
			}
			//RADIO_Ack(RADIO_STATUS_SUCCESS);
			//vTaskDelay(radio_handler_delay);
			
			//loop_cnt++;
				//
			//if(loop_cnt > loop_max) {
				////DEBUG_Write("loop_cnt: %d\r\n", (int)loop_cnt);
				//taskENTER_CRITICAL();
				//watchdog_counter |= 0x08;
				//taskEXIT_CRITICAL();
				//
				//DEBUG_Write("Radio going to sleep...\r\n");
				//loop_cnt = 0;
								//
				////put thread to sleep until a specific tick count is reached
				//vTaskDelay(radio_handler_delay);
				//
				//DEBUG_Write("Radio waking up...\r\n");
			//}
				
		}
		//#else
		//UART_Init(UART_WIND);
		//while(1) {
			//running_task = eRadioHandler;
			//taskENTER_CRITICAL();
			//watchdog_counter |= 0x08;
			//taskEXIT_CRITICAL();
			//RADIO_GenericMsg rx_msg;
			//
			//
			//memset(msg_buffer, 0, RADIO_BUFFER_LENGTH*sizeof(char));
			//
			////NMEA_RxString(NMEA_RADIO, (uint8_t *)msg_buffer, RADIO_BUFFER_LENGTH);
			//UART_RxString(UART_RADIO, msg_buffer, RADIO_BUFFER_LENGTH);
			//DEBUG_Write("message: %s\r\n", msg_buffer);
			//HandleMessage(msg_buffer);
			//vTaskDelay(radio_handler_delay);
		//}
		//#endif
}

void Radio_Sleep_Sec(unsigned time_sec) {
	RADIO_Disable();
	//put thread to sleep for number of ticks specified
	vTaskDelay(time_sec * configTICK_RATE_HZ);
}


