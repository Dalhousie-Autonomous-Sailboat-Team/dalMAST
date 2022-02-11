#ifndef GPSSENSOR_H_
#define GPSSENSOR_H_

#include "gps_log.h"

typedef struct gps_gprmc_        //malloc(28)
{
	char time_h;
	char time_mn;
	char time_s;
	short time_ds;
	
	char error;
	
	char lat;
	char lat_mn;
	short lat_mn_dec;
	char nord;
	
	char lon;
	char lon_mn;
	short lon_mn_dec;
	char est;
	
	short speed;
	int speed_dec;
	
	short course;
	int course_dec;
	
	int date_d;
	int date_m;
	int date_y;
	
	short compas;
	char compas_est;
}gps_gprmc;

typedef struct gps_ensieta_
{
	char lon1;        //longitude
	char lon2;        //deg
	char lon3;        //deg_dec
	char lat1;
	char lat2;
	char lat3;
}gps_ensieta;

typedef struct gps_deci_unsigned_        //malloc(10)
{
	char nord;              //1 si positif
	long lat;
	char est;               //1 si positif
	long lon;
}gps_deci_unsigned;

typedef struct gps_deg_unsigned_        //malloc(12)
{
	char nord;              //1 si positif
	int lat;                //partie entiere
	long lat_mn;
	
	char est;               //1 si positif
	int lon;                //partie entiere
	long lon_mn;
}gps_deg_unsigned;

typedef struct distance_deci_signed_        //malloc(14)
{
	long delta_lon;
	long delta_lat;
	long distance; // en dm je suppose ???
	short angle;
}distance_deci_signed;



///////////////////////////////////////////////////
//
//       récapitulation des différents fonctions
//
///////////////////////////////////////////////////

void init_gps_link(void);

void receive_usart(void);
int get_gprmc(gps_gprmc *gps);
void calcul_distance(gps_deci_signed *ds_dep, gps_deci_signed *ds_arr, distance_deci_signed *distance_calcul);
void convert_gprmc_to_deci_signed(gps_gprmc *gps,gps_deci_signed *ds);

long read_ext_eeprom_coord(long position,char lat);



#endif /* GPSSENSOR_H_ */