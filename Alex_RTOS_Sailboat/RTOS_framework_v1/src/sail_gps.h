/*
 * sail_gps.h
 *
 * Created: 2022-06-02 8:04 PM
 * Author: Kamden Thebeau
 */

#ifndef SAIL_GPS_H
#define SAIL_GPS_H

#include <stdint.h>
#include <stdbool.h>
#include "sail_nmea.h"

#define GPS_MSG_MAX_ARGS 9 
#define PREFIX_LIM 6
#define NUM_MSGS 4

#define MSG_TYPE_TOTAL 6 //sum of all expected msg type ids, eg. 0 + 1 + 2 + 3 = 6.. MAX value = 153 (all types enabled)
#define GPS_LOOP_LIM 5000 //max number of loops allowed before weather station task is put to sleep
#define GPS_SLEEP_PERIOD_MS 1000	 //length of time in sec that puts weather sensor to sleep
#define GPS_BUFFER_LENGTH 30

typedef struct GPS_MsgRawData {
	uint8_t type;
	char* args[GPS_MSG_MAX_ARGS];
}GPS_MsgRawData_t;

typedef struct GPS_AllMsgs {
	NMEA_GenericMsg msg_array[NUM_MSGS];
	/*used to check if all messages have been assigned by
	comparing its value to the sum of all message types*/
	uint8_t msg_type_sum;
} GPS_AllMsgs;

extern GPS_AllMsgs GPS_data;

void enable_gps_msg(eNMEA_TRX_t msg_type);
void write_to_gps(const char* format, ...);

void ReadGPS(void);
void GPS_On(void);
void GPS_Sleep_Sec(unsigned time_sec);

enum status_code GPS_Init(void);
enum status_code GPS_Enable(void);
enum status_code GPS_Disable(void);
enum status_code GPS_RxMsg(NMEA_GenericMsg* msg);

static enum status_code GPS_ExtractMsg(NMEA_GenericMsg* msg, GPS_MsgRawData_t* data);

enum status_code GPS_cmd(const char* format, ...);

#endif /* SAIL_GPS_H */