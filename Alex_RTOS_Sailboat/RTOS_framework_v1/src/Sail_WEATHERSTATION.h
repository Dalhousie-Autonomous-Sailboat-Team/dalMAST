/*
 * Sail_WEATHERSTATION.h
 *
 * Created: 2019-04-11 5:29:07 PM
 *  Author: Antho, Serge
 */ 


#ifndef SAIL_WEATHERSTATION_H_
#define SAIL_WEATHERSTATION_H_

#define WEATHERSENSOR_MSG_MAX_ARGS 9 
#define NUM_MSGS 4

#define MSG_TYPE_TOTAL 6 //sum of all expected msg type ids, eg. 0 + 1 + 2 + 3 = 6.. MAX value = 153 (all types enabled)
#define WS_LOOP_LIM 5000 //max number of loops allowed before weather station task is put to sleep
#define WS_SLEEP_PERIOD_MS 1000	 //length of time in sec that puts weather sensor to sleep
#define WS_BUFFER_LENGTH 30

#include <stdint.h>
#include <stdbool.h>
#include "sail_nmea.h"

typedef struct WEATHERSENSOR_MsgRawData {
	uint8_t type;
	float args[WEATHERSENSOR_MSG_MAX_ARGS];
} WEATHERSENSOR_MsgRawData;

typedef struct NMEA_AllMsgs {
	NMEA_GenericMsg msg_array[NUM_MSGS];
	/*used to check if all messages have been assigned by 
	comparing its value to the sum of all message types*/
	uint8_t msg_type_sum; 
} NMEA_AllMsgs;

void enable_ws_msg(eNMEA_TRX_t msg_type);
void write_to_ws(const char *format, ...);

void ReadWeatherSensor(void);
void WeatherStation_On(void);
void WeatherStation_Sleep_Sec(unsigned time_sec);

enum status_code WeatherStation_Init(void);
enum status_code WeatherStation_Enable(void);
enum status_code WeatherStation_Disable(void);
enum status_code WEATHERSTATION_RxMsg(NMEA_GenericMsg *msg);

static enum status_code WEATHERSENSOR_ExtractMsg(NMEA_GenericMsg *msg, WEATHERSENSOR_MsgRawData *data);

//void ReadWeatherSensor(void);
enum status_code weatherstation_cmd(const char *format, ...);

#endif /* SAIL_WEATHERSTATION_H_ */