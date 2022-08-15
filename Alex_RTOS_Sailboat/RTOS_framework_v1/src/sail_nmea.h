/* sail_nmea.h
 * Header file for the NMEA module that takes raw data buffers from the UART module
 * and verifies them before passing them along to the application layer.
 * Created on June 22, 2016.
 * Created by Thomas Gwynne-Timothy.
 */


#ifndef SAIL_NMEA_H_
#define SAIL_NMEA_H_

#include <status_codes.h>
#include <stdint.h>
#include <stdbool.h>

#include "sail_uart.h"

#define NMEA_BUFFER_LENGTH 128
#define PREFIX_LIM 6

typedef enum NMEA_ChannelIDs {
	NMEA_GPS,
	//NMEA_WEATHERSTATION,
	NMEA_RADIO,
	NMEA_NUM_CHANNELS
} NMEA_ChannelID;

/********** NMEA Message Types **********/

/* List the various NMEA message types */
typedef enum eNMEA_TRX {
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
	NUM_NMEA_TYPES
} eNMEA_TRX_t;

typedef struct NMEA_TYPES_INFO {
	eNMEA_TRX_t MSG_id;
	char NMEA_Prefix[PREFIX_LIM];
} NMEA_TYPE_MAP;

static NMEA_TYPE_MAP NMEA_TYPE_TABLE[NUM_NMEA_TYPES] = {
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
	double lat;
	enum north_south ns;
} latitude;

typedef struct longitude_info {
	double lon;
	enum west_east we;
} longitude;

typedef struct eNMEA_GPGGA {
	latitude lat;
	longitude lon;
	float alt;
} eNMEA_GPGGA;

/*
typedef struct eNMEA_HCHDT {
	float bearing;
} eNMEA_HCHDT;
*/

typedef struct eNMEA_GPVTG {
	float course_over_ground;
} eNMEA_GPVTG;

typedef struct eNMEA_WIMWV {
	//float wind_dir_true, wind_dir_mag, wind_speed_ms, wind_speed_knot;
	float wind_dir_rel, wind_speed_ms; //relative wind direction and wind speed
} eNMEA_WIMWV;

/*YXXDR_B*/
typedef struct eNMEA_YXXDR {
	float roll_deg;
	float pitch_deg;
} eNMEA_YXXDR;

/*HCHDT*/
typedef struct eNMEA_HCHDT {
	float bearing;
} eNMEA_HCHDT;

typedef struct NMEA_GenericMsg {
	eNMEA_TRX_t type;
	union WEATHERSENSOR_GenericDataUnion {
		eNMEA_GPGGA		gpgga;
		eNMEA_HCHDT		hchdt;
		//eNMEA_GPVTG		gpvtg;
		eNMEA_WIMWV		wimwv;
		eNMEA_YXXDR		yxxdr;
		/* the remaining fields
		eNMEA_GPDTM		gpdtm;
		eNMEA_GPGLL		gpgll;
		eNMEA_GPGSA		gpgsa; //Add this in
		eNMEA_GPGSV		gpgsv;
		eNMEA_GPRMC		gprmc;
		eNMEA_GPVTG		gpvtg;
		eNMEA_GPZDA		gpzda;
		eNMEA_HCHDG		hchdg;
		eNMEA_HCTHS		hcths;
		eNMEA_TIROT		tirot;
		eNMEA_WIMDA		wimda;
		eNMEA_WIMWV		wiwmv;
		eNMEA_WIMWR		wimwr;
		eNMEA_WIMWT		wimwt;
		*/
	} fields;
} NMEA_GenericMsg;

/* The struct templates for the remaining NMEA fields are listed below

typedef struct eNMEA_GPGLL {

} eNMEA_GPGLL;

typedef struct eNMEA_GPGSA {

} eNMEA_GPGSA;

typedef struct eNMEA_GPGSV {

} eNMEA_GPGSV;

typedef struct eNMEA_GPRMC {

} eNMEA_GPRMC;

typedef struct eNMEA_GPVTG {

} eNMEA_GPVTG;

typedef struct eNMEA_GPZDA {

} eNMEA_GPZDA;

typedef struct eNMEA_HCHDG {

} eNMEA_HCHDG;

typedef struct eNMEA_HCTHS {

} eNMEA_HCTHS;

typedef struct eNMEA_TIROT {

} eNMEA_TIROT;

typedef struct eNMEA_WIMDA {

} eNMEA_WIMDA;

typedef struct eNMEA_WIMWV {

} eNMEA_WIMWV;

typedef struct eNMEA_WIMWR {

} eNMEA_WIMWR;

typedef struct eNMEA_WIMWT {

} eNMEA_WIMWT;

typedef enum eLOCAL_DATUM_CODE {
	eWGS84,
	eWGS72,
	eSGS85,
	ePE90,
	eUSER_DEFINED
} eLocalDatumCode;

typedef struct eNMEA_GPDTM {
	eLocalDatumCode eDatumCode;
	uint8_t ucLocalDatumSubcode;
	double xLatitudeOffset;
	//SEE ONLINE
} eNMEA_GPDTM;
*/

bool get_NMEA_type(eNMEA_TRX_t* type, char* msg_ptr);

/* NMEA_Init
 * Initialize a specific NMEA channel.
 * Inputs:
 *   id - ID of the desired NMEA channel to initialize
 * Status:
 *	 STATUS_OK - initialization was successful
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided
 *   STATUS_ERR_DENIED - a driver error has occurred 
 *   STATUS_ERR_ALREADY_INITIALIZED - the module should only be initialized once
 */
enum status_code NMEA_Init(NMEA_ChannelID id);


/* NMEA_Enable
 * Trigger the NMEA channel to begin acquiring data.
 * Inputs:
 *   id - ID of the desired NMEA channel
 * Status:
 *   STATUS_OK - data acquisition was started successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided  
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code NMEA_Enable(NMEA_ChannelID id);


/* NMEA_Disable
 * Trigger the NMEA channel to stop acquiring data.
 * Inputs:
 *   id - ID of the desired NMEA channel
 * Status:
 *   STATUS_OK - data acquisition was stopped successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized and/or started
 *   STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code NMEA_Disable(NMEA_ChannelID id);


/* NMEA_TxCommand
 * Transmit a string to the NMEA device.
 * Inputs:
 *   id - ID of the desired NMEA channel
 *	 str - pointer to the command (without $ or *checksum)
 * Status:
 *   STATUS_OK - command was valid and sent successfully and data was received successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 *   STATUS_ERR_BAD_ADDRESS - an invalid address was provided 
 *   STATUS_ERR_TIMEOUT - no response was received before the timeout counter expired
 */
enum status_code NMEA_TxString(NMEA_ChannelID id, uint8_t *str);

/* NMEA_TxString_Unprotected
 * Transmit a string to the NMEA device.
 * The same as NMEA_TxString except the mutex on the write buffer is not used so
 * the function can be called before the FreeRTOS scheduler is started
 * Inputs:
 *   id - ID of the desired NMEA channel
 *	 str - pointer to the command (without $ or *checksum)
 * Status:
 *   STATUS_OK - command was valid and sent successfully and data was received successfully
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_DENIED - a driver error has occurred
 *   STATUS_ERR_BAD_ADDRESS - an invalid address was provided 
 *   STATUS_ERR_TIMEOUT - no response was received before the timeout counter expired
 */
enum status_code NMEA_TxString_Unprotected(NMEA_ChannelID id, uint8_t *str);

/* NMEA_RxString
 * Trigger the NMEA channel to check its buffer and process the data if there is anything
 * available. If there is new, valid data available, the processed data will be copied
 * into the output buffer and the output flag will be set.
 * Inputs:
 *   id - ID of the desired NMEA channel
 * Status:
 *   STATUS_VALID_DATA - valid data has been placed into the output buffer
 *   STATUS_NO_CHANGE - no new data is available
 *	 STATUS_ERR_BAD_DATA - data was available, but the checksum was incorrect
 *   STATUS_ERR_NOT_INITIALIZED - the channel hasn't been initialized yet
 *   STATUS_ERR_INVALID_ARG - an invalid channel ID was provided   
 */
enum status_code NMEA_RxString(NMEA_ChannelID id, uint8_t *str, uint16_t length);

#endif /* SAIL_NMEA_H_ */