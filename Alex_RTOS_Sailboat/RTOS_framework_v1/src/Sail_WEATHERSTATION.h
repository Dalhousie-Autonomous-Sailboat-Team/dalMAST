/*
 * Sail_WEATHERSTATION.h
 *
 * Created: 2019-04-11 5:29:07 PM
 *  Author: Antho, Serge
 */ 


#ifndef SAIL_WEATHERSTATION_H_
#define SAIL_WEATHERSTATION_H_

#define WEATHERSENSOR_MSG_MAX_ARGS 9 
#define PREFIX_LIM 6
#define NUM_MSGS 4


#define MSG_TYPE_TOTAL 6 //sum of all expected msg type ids, eg. 0 + 1 + 2 + 3 = 6.. MAX value = 153 (all types enabled)
#define WS_LOOP_LIM 5000 //max number of loops allowed before weather station task is put to sleep
#define WS_SLEEP_PERIOD_MS 1000	 //length of time in sec that puts weather sensor to sleep
#define WS_BUFFER_LENGTH 30

#include <stdint.h>
#include <stdbool.h>

/* eWeatherstationTRX_t
 * List of various Weatherstation message types.
 */

typedef enum eWEATHERSTATION_TRX {
	eGPGGA,
	eWIMWV,
	eYXXDR,
	eHCHDT,
	//the rest of the types are listed below, but not currently used
	/*
	eGPVTG,
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
	WEATHERSENSOR_NUM_MSG_TYPES
} eWeatherstationTRX_t;


typedef struct WEATHERSENSOR_MsgRawData {
	uint8_t type;
	float args[WEATHERSENSOR_MSG_MAX_ARGS];
} WEATHERSENSOR_MsgRawData;

typedef struct WEATHERSTATION_TYPES_INFO {
	eWeatherstationTRX_t WS_id;
	char WS_Prefix[PREFIX_LIM];
} WEATHERSTATION_TYPE_MAP;

static WEATHERSTATION_TYPE_MAP WEATHERSTATION_TYPE_TABLE[WEATHERSENSOR_NUM_MSG_TYPES] = {
	{ eGPGGA, "GPGGA"}, 
	{ eWIMWV, "WIMWV"}, 
	{ eYXXDR, "YXXDR"}, 
	{ eHCHDT, "HCHDT"}
	
	//remaining fields
	/*
	{ eGPDTM, "GPDTM"}, 
	{ eGPGLL, "GPGLL"}, 
	{ eGPGSA, "GPGSA"},
	{ eGPGSV, "GPGSV"}, 
	{ eGPRMC, "GPRMC"},
	{ eGPVTG, "GPVTG"}, 
	{ eGPZDA, "GPZDA"},
	{ eHCHDG, "HCHDG"}, 
	{ eHCTHS, "HCTHS"}, 
	{ eTIROT, "TIROT"},
	{ eWIMDA, "WIMDA"}, 
	{ eWIMWV, "WIMWV"}, 
	{ eWIMWR, "WIMWR"},
	{ eWIMWT, "WIMWT"}, 
	{ eYXXDR, "YXXDR"},
		*/
};



enum west_east {west, east};
enum north_south {north, south};	
	
typedef struct latitude_info {
	float lat;
	enum north_south ns;
} latitude;

typedef struct longitude_info {
	float lon;
	enum west_east we;
} longitude;

typedef struct eWEATHERSTATION_GPGGA {
	latitude lat;
	longitude lon;
	float alt;
} eWEATHERSTATION_GPGGA;

/*
typedef struct eWEATHERSTATION_HCHDT {
	float bearing;
} eWEATHERSTATION_HCHDT;
*/

typedef struct eWEATHERSTATION_GPVTG {
	float course_over_ground;
} eWEATHERSTATION_GPVTG;

typedef struct eWEATHERSTATION_WIMWV {
	//float wind_dir_true, wind_dir_mag, wind_speed_ms, wind_speed_knot;
	float wind_dir_rel, wind_speed_ms; //relative wind direction and wind speed
} eWEATHERSTATION_WIMWV;

/*YXXDR_B*/
typedef struct eWEATHERSTATION_YXXDR {
	float roll_deg;
	float pitch_deg;
} eWEATHERSTATION_YXXDR;

/*HCHDT*/
typedef struct eWEATHERSTATION_HCHDT {
	float bearing;
} eWEATHERSTATION_HCHDT;



typedef struct WEATHERSENSOR_GenericMsg {
	eWeatherstationTRX_t type;
	union WEATHERSENSOR_GenericDataUnion {
		eWEATHERSTATION_GPGGA		gpgga;
		eWEATHERSTATION_HCHDT		hchdt;
		//eWEATHERSTATION_GPVTG		gpvtg;
		eWEATHERSTATION_WIMWV		wimwv;
		eWEATHERSTATION_YXXDR		yxxdr;
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
} WEATHERSENSOR_GenericMsg;


typedef struct WEATHERSENSOR_AllMsgs {
	WEATHERSENSOR_GenericMsg msg_array[NUM_MSGS];
	/*used to check if all messages have been assigned by 
	comparing its value to the sum of all message types*/
	uint8_t msg_type_sum; 
} WEATHERSENSOR_AllMsgs;

void enable_ws_msg(eWeatherstationTRX_t msg_type);
void write_to_ws(const char *format, ...);

void ReadWeatherSensor(void);
void WeatherStation_On(void);
void WeatherStation_Sleep_Sec(unsigned time_sec);

enum status_code WeatherStation_Init(void);
enum status_code WeatherStation_Enable(void);
enum status_code WeatherStation_Disable(void);
enum status_code WEATHERSTATION_RxMsg(WEATHERSENSOR_GenericMsg *msg);

static enum status_code WEATHERSENSOR_ExtractMsg(WEATHERSENSOR_GenericMsg *msg, WEATHERSENSOR_MsgRawData *data);

bool get_NMEA_type(eWeatherstationTRX_t *type);
//void ReadWeatherSensor(void);
enum status_code weatherstation_cmd(const char *format, ...);

extern volatile WEATHERSENSOR_AllMsgs weathersensor_data;

//the structs for the remaining fields are listed below

/* 
typedef struct eWEATHERSTATION_GPGLL {

} eWEATHERSTATION_GPGLL;

typedef struct eWEATHERSTATION_GPGSA {

} eWEATHERSTATION_GPGSA;

typedef struct eWEATHERSTATION_GPGSV {

} eWEATHERSTATION_GPGSV;

typedef struct eWEATHERSTATION_GPRMC {

} eWEATHERSTATION_GPRMC;

typedef struct eWEATHERSTATION_GPVTG {

} eWEATHERSTATION_GPVTG;

typedef struct eWEATHERSTATION_GPZDA {

} eWEATHERSTATION_GPZDA;

typedef struct eWEATHERSTATION_HCHDG {

} eWEATHERSTATION_HCHDG;



typedef struct eWEATHERSTATION_HCTHS {

} eWEATHERSTATION_HCTHS;

typedef struct eWEATHERSTATION_TIROT {

} eWEATHERSTATION_TIROT;

typedef struct eWEATHERSTATION_WIMDA {

} eWEATHERSTATION_WIMDA;



typedef struct eWEATHERSTATION_WIMWV {

} eWEATHERSTATION_WIMWV;

typedef struct eWEATHERSTATION_WIMWR {

} eWEATHERSTATION_WIMWR;

typedef struct eWEATHERSTATION_WIMWT {

} eWEATHERSTATION_WIMWT;

typedef enum eLOCAL_DATUM_CODE {
	eWGS84,
	eWGS72,
	eSGS85,
	ePE90,
	eUSER_DEFINED
} eLocalDatumCode;

typedef struct eWEATHERSTATION_GPDTM {
	eLocalDatumCode eDatumCode;
	uint8_t ucLocalDatumSubcode;
	double xLatitudeOffset;
	//SEE ONLINE
} eWEATHERSTATION_GPDTM;


*/

#endif /* SAIL_WEATHERSTATION_H_ */