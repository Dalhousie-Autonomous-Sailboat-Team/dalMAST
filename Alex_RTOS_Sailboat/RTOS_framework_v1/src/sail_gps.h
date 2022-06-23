/*
 * sail_gps.h
 *
 * Created: 2022-06-02 8:04 PM
 *  Author: secul
 */

#ifndef SAIL_GPS_H
#define SAIL_GPS_H

#define GPS_MSG_MAX_ARGS 9 
#define PREFIX_LIM 6
#define NUM_MSGS 4


#define MSG_TYPE_TOTAL 6 //sum of all expected msg type ids, eg. 0 + 1 + 2 + 3 = 6.. MAX value = 153 (all types enabled)
#define GPS_LOOP_LIM 5000 //max number of loops allowed before weather station task is put to sleep
#define GPS_SLEEP_PERIOD_MS 1000	 //length of time in sec that puts weather sensor to sleep
#define GPS_BUFFER_LENGTH 30

#include <stdint.h>
#include <stdbool.h>

 /* eGPSTRX_t
  * List of various GPS message types.
  */

typedef enum eGPS_TRX {
	eGPGGA,
	eGPVTG,
	eWIMWV,
	eYXXDR,
	eHCHDT,
	//the rest of the types are listed below, but not currently used
	/*
	eGPDTM,
	eGPGLL,
	eGPGSA,
	eGPGSV,
	eGPRMC,
	eGPVTG,
	eGPZDA,
	eHCHDT,
	eHCHDG,
	eHCTHS,
	eTIROT,
	eWIMDA,
	eWIMWV,
	eWIMWR,
	eWIMWT,
	*/
	GPS_NUM_MSG_TYPES
} eGPSTRX_t;


typedef struct GPS_MsgRawData {
	uint8_t type;
	float args[GPS_MSG_MAX_ARGS];
} GPS_MsgRawData;

typedef struct GPS_TYPES_INFO {
	eGPSTRX_t GPS_id;
	char GPS_Prefix[PREFIX_LIM];
} GPS_TYPE_MAP;

static GPS_TYPE_MAP GPS_TYPE_TABLE[GPS_NUM_MSG_TYPES] = {
	{ eGPGGA, "GPGGA"}, { eWIMWV, "WIMWV"},
	{ eYXXDR, "YXXDR"}, { eHCHDT, "HCHDT"}

	//remaining fields
	/*
	{ eGPDTM, "GPDTM"},
	{ eGPGLL, "GPGLL"}, { eGPGSA, "GPGSA"},
	{ eGPGSV, "GPGSV"}, { eGPRMC, "GPRMC"},
	{ eGPVTG, "GPVTG"}, { eGPZDA, "GPZDA"},
	{ eHCHDG, "HCHDG"},
	{ eHCTHS, "HCTHS"}, { eTIROT, "TIROT"},
	{ eWIMDA, "WIMDA"},
	{ eWIMWV, "WIMWV"}, { eWIMWR, "WIMWR"},
	{ eWIMWT, "WIMWT"}, { eYXXDR, "YXXDR"},
		*/
};



enum west_east { west, east };
enum north_south { north, south };

typedef struct latitude_info {
	float lat;
	enum north_south ns;
} latitude;

typedef struct longitude_info {
	float lon;
	enum west_east we;
} longitude;

typedef struct eGPS_GPGGA {
	latitude lat;
	longitude lon;
	float alt;
} eGPS_GPGGA;

/*
typedef struct eWEATHERSTATION_HCHDT {
	float bearing;
} eWEATHERSTATION_HCHDT;
*/

typedef struct eGPS_GPVTG {
	float course_over_ground;
} eGPS_GPVTG;

typedef struct eGPS_WIMWV {
	//float wind_dir_true, wind_dir_mag, wind_speed_ms, wind_speed_knot;
	float wind_dir_rel, wind_speed_ms; //relative wind direction and wind speed
} eGPS_WIMWV;

/*YXXDR_B*/
typedef struct eGPS_YXXDR {
	float roll_deg;
	float pitch_deg;
} eGPS_YXXDR;

/*HCHDT*/
typedef struct eGPS_HCHDT {
	float bearing;
} eGPS_HCHDT;



typedef struct GPS_GenericMsg {
	eGPSTRX_t type;
	union GPS_GenericDataUnion {
		eGPS_GPGGA		gpgga;
		eGPS_HCHDT		hchdt;
		//eWEATHERSTATION_GPVTG		gpvtg;
		eGPS_WIMWV		wimwv;
		eGPS_YXXDR		yxxdr;
		/* the remaining fields
		eWEATHERSTATION_GPDTM		gpdtm;
		eWEATHERSTATION_GPGLL		gpgll;
		eWEATHERSTATION_GPGSA		gpgsa;
		eWEATHERSTATION_GPGSV		gpgsv;
		eWEATHERSTATION_GPRMC		gprmc;
		eWEATHERSTATION_GPVTG		gpvtg;
		eWEATHERSTATION_GPZDA		gpzda;
		eWEATHERSTATION_HCHDG		hchdg;
		eWEATHERSTATION_HCTHS		hcths;
		eWEATHERSTATION_TIROT		tirot;
		eWEATHERSTATION_WIMDA		wimda;
		eWEATHERSTATION_WIMWV		wiwmv;
		eWEATHERSTATION_WIMWR		wimwr;
		eWEATHERSTATION_WIMWT		wimwt;
		*/
	} fields;
} GPS_GenericMsg;


typedef struct GPS_AllMsgs {
	GPS_GenericMsg msg_array[NUM_MSGS];
	/*used to check if all messages have been assigned by
	comparing its value to the sum of all message types*/
	uint8_t msg_type_sum;
} GPS_AllMsgs;

void enable_gps_msg(eGPSTRX_t msg_type);
void write_to_gps(const char* format, ...);

void ReadGPS(void);
void GPS_On(void);
void GPS_Sleep_Sec(unsigned time_sec);

enum status_code GPS_Init(void);
enum status_code GPS_Enable(void);
enum status_code GPS_Disable(void);
enum status_code GPS_RxMsg(GPS_GenericMsg* msg);

static enum status_code GPS_ExtractMsg(GPS_GenericMsg* msg, GPS_MsgRawData* data);

bool get_NMEA_type(eGPSTRX_t* type);
//void ReadWeatherSensor(void);
enum status_code GPS_cmd(const char* format, ...);

extern volatile GPS_AllMsgs gps_data;


#endif /* SAIL_GPS_H */