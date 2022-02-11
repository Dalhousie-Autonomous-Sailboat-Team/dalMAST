/*
 * sail_gps.c
 *
 *  Created on: Jun 13, 2016
 *      Author: Thomas
 */

#include "sail_gps.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#define PI 3.141592653589793
#define BUFFER_LENGTH 128

#define RMC_FMT "GPRMC,%"SCNu32".%"SCNu16",%c,"\
				"%"SCNu32".%"SCNu32",%c,"\
				"%"SCNu32".%"SCNu32",%c,"\
				"%"SCNu8".%"SCNu16","\
				"%"SCNu16".%"SCNu16","\
				"%"SCNu32",%s"\
				//"%"SCNu8".%"SCNu16",%"SCNu8""

static GPS_Status GPS_ParseRMC();

static uint8_t rmc_buffer[BUFFER_LENGTH];
static uint8_t rx_flag, tx_flag;
static uint8_t eeprom_buffer[BUFFER_LENGTH];

struct GPS_RMCData {
	uint32_t fix_time;			// Time of fix (hour:minute:second)
	uint16_t fix_time_dec;		// Time of fix (millisecond)
	uint8_t fix_status;			// Status of fix
	uint32_t lat_mn;			// Latitude with integer minutes
	uint32_t lat_mn_dec;		// Decimal part of latitude minutes
	uint8_t lat_card;			// Latitude cardinality (N or S)
	uint32_t lon_mn;			// Longitude with integer minutes
	uint32_t lon_mn_dec;		// Decimal part of longitude minutes
	uint8_t lon_card;			// Longitude cardinality (E or W)
	uint8_t speed_knot;			// Speed (knots)
	uint16_t speed_knot_dec;	// Decimal part of speed
	uint16_t course_deg;		// Course w.r.t True North (degrees)
	uint16_t course_deg_dec;	// Decimal part of course
	uint32_t date;				// Date
	char compass[10];			// Just the whole compass string unparsed
	//uint8_t compass;			// Compass variation
	//uint16_t compass_dec;		// Decimal part of compass variation			COMPASS TURNED OFF
	//uint8_t compass_card;		// Cardinality of compass variation (E or W)
} rmc_data;


GPS_Status GPS_Init() {
		// Clear flags
		tx_flag = 0;
		rx_flag = 0;

		// Clear buffer
		memset(rmc_buffer, 0, BUFFER_LENGTH);


	return GPS_SUCCESS;
}

GPS_Status GPS_Update() {
	// Check the RX flag
		if (rx_flag == 0) {
			return GPS_UNCHANGED;
		}

		// Parse the data in the buffer
		GPS_ParseRMC();

		// Clear the flag
		rx_flag = 0;

	return GPS_SUCCESS;
}

GPS_Status GPS_GetCoords(GPS_Coords *coords) {

		if(coords->lat == (uint16_t)(rmc_data.lat_mn/100) && coords->lat_mmn == ((uint16_t)(rmc_data.lat_mn % 100)*1000 + (uint16_t)rmc_data.lat_mn_dec)){
			return GPS_UNCHANGED;
		}
		// Get latitude
		coords->lat = (int8_t)(rmc_data.lat_mn/100.0);
		coords->lat_mmn = (uint32_t)(rmc_data.lat_mn % 100)*1000000 + (uint32_t)rmc_data.lat_mn_dec*100;
		coords->lat_final = (int32_t)coords->lat*1000000 + (int32_t)(coords->lat_mmn*100)/6;
		if (rmc_data.lat_card == 'S') {
			coords->lat *= -1;
			coords->lat_final *= -1;
		}

		// Get longitude
		coords->lon = (int8_t)(rmc_data.lon_mn/100.0);
		coords->lon_mmn = (uint32_t)(rmc_data.lon_mn % 100)*1000000 + (uint32_t)rmc_data.lon_mn_dec*100;
		coords->lon_final = (int32_t)coords->lon*1000000.0 + (int32_t)(coords->lon_mmn*100)/6;
		if (rmc_data.lon_card == 'W') {
			coords->lon *= -1;
			coords->lon_final *= -1;
		}

	return GPS_SUCCESS;
}

GPS_Status GPS_GetBuffer(uint8_t **buffer_ptr, uint8_t **eeprom_ptr, size_t *buffer_length) {
		*buffer_ptr = rmc_buffer;
		*buffer_length = BUFFER_LENGTH;
		*eeprom_ptr = eeprom_buffer;

		if(*buffer_ptr==NULL||*eeprom_buffer==NULL){
			return GPS_ERROR;
		}
	return GPS_SUCCESS;
}


GPS_Status GPS_GetFlags(uint8_t **tx_flag_ptr, uint8_t **rx_flag_ptr) {
		*tx_flag_ptr = &tx_flag;
		*rx_flag_ptr = &rx_flag;

		if(&tx_flag==NULL||&rx_flag==NULL){
			return GPS_ERROR;
		}
	return GPS_SUCCESS;
}


GPS_Status GPS_ParseRMC() {
	int scanCnt = sscanf(rmc_buffer, RMC_FMT, &rmc_data.fix_time,
									&rmc_data.fix_time_dec,
									&rmc_data.fix_status,
									&rmc_data.lat_mn,
									&rmc_data.lat_mn_dec,
									&rmc_data.lat_card,
									&rmc_data.lon_mn,
									&rmc_data.lon_mn_dec,
									&rmc_data.lon_card,
									&rmc_data.speed_knot,
									&rmc_data.speed_knot_dec,
									&rmc_data.course_deg,
									&rmc_data.course_deg_dec,
									&rmc_data.date,
									&rmc_data.compass);
									//&rmc_data.compass_dec,
									//&rmc_data.compass_card);

		printf(RMC_FMT"\n", rmc_data.fix_time,
			rmc_data.fix_time_dec,
			rmc_data.fix_status,
			rmc_data.lat_mn,
			rmc_data.lat_mn_dec,
			rmc_data.lat_card,
			rmc_data.lon_mn,
			rmc_data.lon_mn_dec,
			rmc_data.lon_card,
			rmc_data.speed_knot,
			rmc_data.speed_knot_dec,
			rmc_data.course_deg,
			rmc_data.course_deg_dec,
			rmc_data.date,
			rmc_data.compass);
			//rmc_data.compass_dec,
			//rmc_data.compass_card);



	return GPS_SUCCESS;
}

GPS_Status GPS_ParseEEPROM(GPS_Coords *wp){

	sscanf(eeprom_buffer, "%"SCNi32",%"SCNi32"", &wp->lat_final, &wp->lon_final);
	printf("%"PRIi32", %"PRIi32"\n", wp->lat_final, wp->lon_final);

}


GPS_Status GPS_Distance(GPS_Coords *coords, GPS_Coords *wp, GPS_Diff *distance){

	   float deg_float, temp_float, square_distance, lat_rad, cos_lat, lat;
	   int delta_lat, delta_lon;
	   int temp;
	   if(coords->lat_final >= wp->lat_final){

	         delta_lat = coords->lat_final - wp->lat_final;
	      }
	      else{
	         delta_lat = wp->lat_final - coords->lat_final;
	      }

	   if(coords->lon_final >= wp->lon_final){

	  	         delta_lon = coords->lon_final - wp->lon_final;
	  	      }
	  	      else{
	  	         delta_lon = wp->lon_final - coords->lon_final;
	  	      }

	      if(delta_lon > 180000000)

	      {
	         delta_lon = 360000000 - delta_lon;
	      }

	      //printf("%li, %li", distance->delta_lat, distance->delta_lon);

	      lat = coords->lat_final-2147483648;
	      lat_rad = lat*PI/180000000; 			//latitude in radians
	      cos_lat =1-lat_rad*lat_rad/2.0;		//cosine approximation using Maclaurin series
	      delta_lon = delta_lon*cos_lat;
	      deg_float = delta_lon;
	      temp_float = delta_lat;

	      if(delta_lat > 1)
	    	  deg_float /= temp_float;

	      	  deg_float = atan(deg_float) * 1800;
	      	  deg_float /= PI;

	        distance->delta_lon = delta_lon;
	        distance->delta_lat = delta_lat;

	        if(((delta_lon*delta_lon)+(delta_lat*delta_lat)) < 2147483648)
	           square_distance = ((delta_lon*delta_lon)+(delta_lat*delta_lat));
	        else
	           square_distance = 2147483648;

	        distance->distance = sqrt(square_distance);


	        switch (temp){
	           case 0b01   :  deg_float = 1800-deg_float;
	                          break;
	           case 0b11   :  deg_float = 1800+deg_float;
	                          break;
	           case 0b10   :  deg_float = 3600-deg_float;
	                          break;
	           default     :  break;
	        }

	        distance->angle = deg_float;
	     }


//}










