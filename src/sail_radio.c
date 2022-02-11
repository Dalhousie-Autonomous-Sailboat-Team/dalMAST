
#include "sail_radio.h"

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "sail_nmea.h"
#include "sail_debug.h"

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
			DEBUG_Write("NMEA module could not be initialized!\r\n");
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
		DEBUG_Write("NMEA receiver could not be started!\r\n");
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
	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
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

	DEBUG_Write("radio: %s\r\n", msg_buffer);
	
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





enum status_code RADIO_TxMsg(RADIO_GenericMsg *msg)
{
	// Return if a null pointer is provided
	if (msg == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
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
	
	// Transmit the string
	if (NMEA_TxString(NMEA_RADIO, (uint8_t*)msg_buffer) != STATUS_OK) {
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
			data->args[1] = (int32_t)(msg->fields.wp.data.pos.lat*1000000.0);
			data->args[2] = (int32_t)(msg->fields.wp.data.pos.lon*1000000.0);
			data->args[3] = (int32_t)msg->fields.wp.data.rad;
			data->args[4] = msg->fields.wp.next_idx;
			break;
		case RADIO_CONFIG:
			data->type = RADIO_CONFIG;
			data->args[0] = msg->fields.config.period;
			break;
		case RADIO_GPS:
			data->type = RADIO_GPS;
			data->args[0] = (int32_t)(msg->fields.gps.data.lat*1000000.0);
			data->args[1] = (int32_t)(msg->fields.gps.data.lon*1000000.0);
			break;
		case RADIO_WIND:
			data->type = RADIO_WIND;
			data->args[0] = (int32_t)(msg->fields.wind.data.angle*10.0);
			data->args[1] = (int32_t)(msg->fields.wind.data.speed*10.0);
			break;
		case RADIO_COMP:
			data->type = RADIO_COMP;
			data->args[0] = msg->fields.comp.data.type;
			data->args[1] = (int32_t)(msg->fields.comp.data.data.fields[0]*10.0);
			data->args[2] = (int32_t)(msg->fields.comp.data.data.fields[1]*10.0);
			data->args[3] = (int32_t)(msg->fields.comp.data.data.fields[2]*10.0);
			break;
		case RADIO_NAV:
			data->type = RADIO_NAV;
			data->args[0] = (int32_t)(msg->fields.nav.wp.pos.lat*1000000.0);
			data->args[1] = (int32_t)(msg->fields.nav.wp.pos.lon*1000000.0);		
			data->args[2] = (int32_t)msg->fields.nav.wp.rad;	
			data->args[3] = (int32_t)msg->fields.nav.distance;
			data->args[4] = (int32_t)(msg->fields.nav.bearing*10.0);
			data->args[5] = (int32_t)(msg->fields.nav.course*10.0);
			data->args[6] = msg->fields.nav.rudder_angle;
			data->args[7] = msg->fields.nav.sail_angle;
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
