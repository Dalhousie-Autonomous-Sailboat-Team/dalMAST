/* sail_radio.h
 * Header file for the radio module for the autonomous sailboat.
 * The messages follow the format below. 
 * The first field is common to all messages. 
 * The second field is a three-digit number denoting the message type.
 * The following optional fields provide message specific data
 * DALSAIL,[message type],[arg 0],[arg 1],[arg 2],[arg 3],[arg 4],[arg 5],[arg 6],[arg 7]
 * 
 * Created on July 19, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#ifndef SAIL_RADIO_H_
#define SAIL_RADIO_H_

#include "sail_eeprom.h"
#include "sail_wind.h"
#include "sail_types.h"

#include <stdint.h>
#include <status_codes.h>

#define RADIO_MSG_MAX_ARGS	8 
#define RADIO_SLEEP_PERIOD_MS 3000
//#define RADIO_LOOP_LIM 100000

/* RADIO_MsgType
 * List of various DALSAIL message types.
 */
typedef enum RADIO_MsgTypes {
	RADIO_ACK,
	RADIO_MODE,
	RADIO_STATE,
	RADIO_REMOTE,
	RADIO_WAY_POINT,
	RADIO_CONFIG,
	RADIO_GPS,
	RADIO_WIND,
	RADIO_COMP,
	RADIO_NAV,
	RADIO_RESET,
	RADIO_NUM_MSG_TYPES
} RADIO_MsgType;

/* RADIO_Status
 * List of various acknowledgment statuses.
 */
typedef enum RADIO_Statuses {
	RADIO_STATUS_SUCCESS,
	RADIO_STATUS_FAILURE,
	RADIO_STATUS_ERROR,
	RADIO_STATUS_UNCHANGED,
	RADIO_NUM_STATUSES
} RADIO_Status;

typedef struct RADIO_AckData {
	RADIO_Status		status;
} RADIO_AckData;

typedef struct RADIO_ModeData {
	CTRL_Mode			mode;
} RADIO_ModeData;

typedef struct RADIO_StateData {
	CTRL_State			state;
} RADIO_StateData;

typedef struct RADIO_RemoteData {
	uint16_t				rudder_angle;
	uint16_t				sail_angle;
} RADIO_RemoteData;

typedef struct RADIO_WayPointData {
	uint16_t			idx;
	EEPROM_WayPoint		data;
	uint16_t			next_idx;
} RADIO_WayPointData;

typedef struct RADIO_ConfigData {
	uint8_t				period;
} RADIO_ConfigData;

typedef struct RADIO_GPSData {
	GPS_Reading			data;
} RADIO_GPSData;

typedef struct RADIO_WindData {
	WIND_Reading		data;
} RADIO_WindData;

typedef	struct RADIO_CompData {
	COMP_Reading		data;
} RADIO_CompData;

typedef struct RADIO_NavData {
	EEPROM_WayPoint		wp;
	double				distance;
	double				bearing;
	double				course;
	uint16_t			rudder_angle;
	uint16_t			sail_angle;
} RADIO_NavData;

typedef struct RADIO_ResetData {
	CTRL_ResetCause		cause;
} RADIO_ResetData;

typedef struct RADIO_GenericMsg {
	RADIO_MsgType			type;
	union RADIO_GenericDataUnion {
		RADIO_AckData		ack;
		RADIO_ModeData		mode;
		RADIO_StateData		state;
		RADIO_RemoteData	remote;
		RADIO_WayPointData	wp;
		RADIO_ConfigData	config;
		RADIO_GPSData		gps;
		RADIO_WindData		wind;
		RADIO_CompData		comp;
		RADIO_NavData		nav;
		RADIO_ResetData		reset;
	} fields;
} RADIO_GenericMsg;

enum status_code RADIO_Init(void);
enum status_code RADIO_Enable(void);
enum status_code RADIO_Disable(void);
enum status_code RADIO_RxMsg(RADIO_GenericMsg *msg);
enum status_code RADIO_TxMsg(RADIO_GenericMsg *msg);
enum status_code RADIO_TxMsg_Unprotected(RADIO_GenericMsg *msg);
enum status_code RADIO_Ack(RADIO_Status status);

static RADIO_Status ChangeMode(CTRL_Mode new_mode);
static RADIO_Status ChangeState(CTRL_State new_state);
static RADIO_Status ChangeLogPeriod(uint8_t new_period);
static RADIO_Status AddWayPoint(RADIO_WayPointData *wp_data);
static RADIO_Status AdjustMotors(uint16_t sail_angle, uint16_t rudder_angle);

void Radio_On(void);
void RadioHandler(void);


//extern RADIO_GenericMsg tx_msg, rx_msg;
extern RADIO_GenericMsg tx_msg;

// EEPROM data
extern EEPROM_Entry wp_entry;
extern uint16_t wp_count;

// Current way point
extern EEPROM_WayPoint wp;
// Counter to track # of times distance < radius
extern uint16_t wp_complete_count;
// Distance between boat and way point
extern double wp_distance;

extern float course, bearing, sail_deg;
extern uint16_t rudder_deg;
extern float avg_heading_deg;




void Radio_Sleep_Sec(unsigned time_sec);

#endif /* SAIL_RADIO_H_ */