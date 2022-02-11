
#include <asf.h>
// #include <stdio_serial.h>
#include <string.h>
#include <math.h>
// #include "conf_test.h"
#include "GPSSensor.h"
#include "Queue.h"

#define PI 3.141592653589793
#define STRING_LEN 75
#define GPS_SPEED 115200

/*
 * GPSSensor.c
 *
 * Created: 6/8/2015 8:48:14 AM
 *  Author: JFB
 */ 

// In this module we will initiate an RS-232 configuration to GPS, 
// and read its data through a FIFO. 

struct usart_module usart_module;
struct usart_config usart_config;

volatile uint8_t rx_string[STRING_LEN] = "";

QueueC QueueGPS;

volatile bool transfer_complete;

static int GetNextGPSData(void);

static int GetNextGPSData(void)
{
	char NMEA_char;
	int e = QueueGet(&NMEA_char, &QueueGPS);
	if (e != 0)
		NMEA_char = 0;
	return NMEA_char;
}

/******************************************************************************
******************************************************************************/
static void usart_callback(struct usart_module *const module)
{		
	// copy it into a FIFO
	for(int i = 0; i < STRING_LEN; i++)
		QueuePut(rx_string[i], &QueueGPS);
}

/******************************************************************************
******************************************************************************/
void init_gps_link(void)
{

	usart_get_config_defaults(&usart_config);
	usart_config.mux_setting = EXT1_UART_SERCOM_MUX_SETTING;
	usart_config.pinmux_pad0 = PINMUX_PA04D_SERCOM0_PAD0; // PA04, normally an SPI MISO, used for TX
	usart_config.pinmux_pad1 = PINMUX_PA05D_SERCOM0_PAD1; // PB05, normally SERCOM 0, SPI SS
	usart_config.pinmux_pad2 = PINMUX_UNUSED;
	usart_config.pinmux_pad3 = PINMUX_UNUSED;
	usart_config.baudrate    = GPS_SPEED;
	/* Apply configuration */
	usart_init(&usart_module, SERCOM0, &usart_config);
	/* Enable USART */
	usart_enable(&usart_module);

	/* Configure TX USART */
/*	usart_get_config_defaults(&usart_tx_config);
	usart_tx_config.mux_setting = CONF_TX_USART_SERCOM_MUX;
	usart_tx_config.pinmux_pad0 = CONF_TX_USART_PINMUX_PAD0;
	usart_tx_config.pinmux_pad1 = CONF_TX_USART_PINMUX_PAD1;
	usart_tx_config.pinmux_pad2 = CONF_TX_USART_PINMUX_PAD2;
	usart_tx_config.pinmux_pad3 = CONF_TX_USART_PINMUX_PAD3;
	usart_tx_config.baudrate    = TEST_USART_SPEED;
	// Apply configuration 
	usart_init(&usart_tx_module, CONF_TX_USART, &usart_tx_config);
	// Enable USART 
	usart_enable(&usart_tx_module);*/
	
	usart_register_callback(&usart_module, usart_callback,
		USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&usart_module, 
		USART_CALLBACK_BUFFER_RECEIVED);

	transfer_complete = false;
	
	QueueInit(&QueueGPS);
}

/******************************************************************************
******************************************************************************/
void receive_usart(void)
{
//	volatile uint8_t tx_string[TEST_STRING_LEN] = TEST_STRING;
	usart_read_buffer_job(&usart_module, (uint8_t*)rx_string, STRING_LEN);


}

// private functions declarations
void convert_unsigned_to_ensieta(gps_deci_unsigned *du,gps_ensieta *en);
void convert_ensieta_to_unsigned(gps_ensieta *en,gps_deci_unsigned *du);
void convert_ensieta_to_signed(gps_ensieta *en,gps_deci_signed *ds);
void convert_deci_signed_to_unsigned(gps_deci_signed *ds,gps_deci_unsigned *du);
void convert_deci_unsigned_to_signed(gps_deci_unsigned *du,gps_deci_signed *ds);
void convert_deg_unsigned_to_deci_unsigned(gps_deg_unsigned *dgu,gps_deci_unsigned *du);
void convert_gprmc_to_deg_unsigned(gps_gprmc *gps,gps_deg_unsigned *dgu);
int gprmc_header(void);
long traduction_ascii_to_num(int taille);

/******************************************************************************
******************************************************************************/
void convert_unsigned_to_ensieta(gps_deci_unsigned *du,gps_ensieta *en){
   int temp1,temp2,temp3,i;
   long temp32,val_temp;
   
   val_temp=du->lon;
   
   for(i=0;i<2;i++){
      temp32=du->lon/1000000;
      temp1=temp32;
      temp32=du->lon%1000000;
      temp32*=6;
      temp32/=10;
      temp2=temp32/10000;
      temp32=temp32%10000;
      temp32=(temp32+50)/100;
      temp3=temp32;
      
      if(val_temp==du->lon){
         if(du->est){
            en->lon1=128+temp1;
            en->lon2=128+temp2;
            en->lon3=128+temp3;
         }
         else{
            en->lon1=128-temp1;
            en->lon2=128-temp2;
            en->lon3=128-temp3;   
         }
      }
      else{
         if(du->est){
            en->lat1=128+temp1;
            en->lat2=128+temp2;
            en->lat3=128+temp3;
         }
         else{
            en->lat1=128-temp1;
            en->lat2=128-temp2;
            en->lat3=128-temp3;   
         }
      }
      val_temp=du->lat;
   }
}

/******************************************************************************
******************************************************************************/
void convert_ensieta_to_unsigned(gps_ensieta *en,gps_deci_unsigned *du){
   int temp1,temp2,temp3;
   long temp32;
   if((en->lon1>=128)&&(en->lon2>=128)&&(en->lon3>=128)){
      temp1=en->lon1-128;
      temp2=en->lon2-128;
      temp3=en->lon3-128;
      du->est=1;
   }
   else{
      temp1=128-en->lon1;
      temp2=128-en->lon2;
      temp3=128-en->lon3;
      du->est=0;
   }
   temp32=en->lon2;
   temp32=temp32*100;
   temp32+=en->lon3;
   temp32*=1000;
   temp32/=6;
   du->lon=en->lon1;
   du->lon*=1000000;
   du->lon+=temp32;
   
   if((en->lat1>=128)&&(en->lat2>=128)&&(en->lat3>=128)){
      temp1=en->lat1-128;
      temp2=en->lat2-128;
      temp3=en->lat3-128;
      du->nord=1;
   }
   else{
      temp1=128-en->lat1;
      temp2=128-en->lat2;
      temp3=128-en->lat3;
      du->nord=0;
   }
   temp32=en->lat2;
   temp32=temp32*100;
   temp32+=en->lat3;
   temp32*=1000;
   temp32/=6;
   du->lat=en->lat1;
   du->lat*=1000000;
   du->lat+=temp32;
   
}

/******************************************************************************
******************************************************************************/
void convert_ensieta_to_signed(gps_ensieta *en,gps_deci_signed *ds){
   gps_deci_unsigned du;
   convert_ensieta_to_unsigned(en,&du);
   convert_deci_unsigned_to_signed(&du,ds);
}

/******************************************************************************
******************************************************************************/
void convert_deci_signed_to_unsigned(gps_deci_signed *ds,gps_deci_unsigned *du){
   if(ds->lon>=0x80000000){
      du->est=1;
      du->lon=(ds->lon)-0x80000000;
   }
   else{
      du->est=0;
      du->lon=0x80000000-(ds->lon);
   }
   if(ds->lat>=0x80000000){
      du->nord=1;
      du->lat=(ds->lat)-0x80000000;
   }
   else{
      du->nord=0;
      du->lat=0x80000000-(ds->lat);
   }
}

/******************************************************************************
******************************************************************************/
void convert_deci_unsigned_to_signed(gps_deci_unsigned *du,gps_deci_signed *ds){
   if(du->est)
      ds->lon=0x80000000+(du->lon); //2^31+...
   else
      ds->lon=0x80000000-(du->lon);
   if(du->nord)
      ds->lat=0x80000000+(du->lat);
   else
      ds->lat=0x80000000-(du->lat);
}

/******************************************************************************
******************************************************************************/
void convert_deg_unsigned_to_deci_unsigned(gps_deg_unsigned *dgu,gps_deci_unsigned *du){
   long temp32;
   
   du->nord=dgu->nord;
   du->est=dgu->est;
   
   //temp32=dgu->lon;
   du->lon=dgu->lon*1000000;
   temp32=dgu->lon_mn*10;
   temp32/=6;
   du->lon=du->lon+temp32;
   
   //temp32=dgu->lat;
   du->lat=dgu->lat*1000000;
   temp32=dgu->lat_mn*10;
   temp32/=6;
   du->lat=du->lat+temp32;
}

/******************************************************************************
******************************************************************************/
void convert_gprmc_to_deg_unsigned(gps_gprmc *gps,gps_deg_unsigned *dgu){
   long temp32;
   
   dgu->nord=gps->nord;
   dgu->est=gps->est;
   
   dgu->lat=gps->lat;
   dgu->lon=gps->lon;
   
   temp32=gps->lat_mn;
   dgu->lat_mn=(temp32*10000)+gps->lat_mn_dec;
   
   temp32=gps->lon_mn;
   dgu->lon_mn=(temp32*10000)+gps->lon_mn_dec;
}

/******************************************************************************
******************************************************************************/
void convert_gprmc_to_deci_signed(gps_gprmc *gps,gps_deci_signed *ds){
   gps_deg_unsigned *dgu;
   gps_deci_unsigned *du;
   
   dgu=malloc(12);
   du=malloc(10);
   
   convert_gprmc_to_deg_unsigned(gps,dgu);
   convert_deg_unsigned_to_deci_unsigned(dgu,du);
   convert_deci_unsigned_to_signed(du,ds);
   
   free(dgu);
   free(du);
}

/******************************************************************************
******************************************************************************/
int compare_ensieta_lon(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       longitude puis latitude si necessaire
   if((en1->lon1)>(en2->lon1))
      return 0;
   if((en1->lon1)==(en2->lon1)){
      if((en1->lon2)>(en2->lon2))
         return 0;
      if((en1->lon2)==(en2->lon2)){
         if((en1->lon3)>(en2->lon3))
            return 0;
         if((en1->lon3)==(en2->lon3)){
            if((en1->lat1)>(en2->lat1))
               return 0;
            if((en1->lat1)==(en2->lat1)){
               if((en1->lat2)>(en2->lat2))
                  return 0;
               if((en1->lat2)==(en2->lat2)){
                  if((en1->lat3)>(en2->lat3))
                     return 0;
               }
            }
         }
      }
   }
   return 1;
}

/******************************************************************************
******************************************************************************/
int compare_ensieta_lat(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       latitude puis longitude si necessaire
   if((en1->lat1)>(en2->lat1))
      return 0;
   if((en1->lat1)==(en2->lat1)){
      if((en1->lat2)>(en2->lat2))
         return 0;
      if((en1->lat2)==(en2->lat2)){
         if((en1->lat3)>(en2->lat3))
            return 0;
         if((en1->lat3)==(en2->lat3)){
            if((en1->lon1)>(en2->lon1))
               return 0;
            if((en1->lon1)==(en2->lon1)){
               if((en1->lon2)>(en2->lon2))
                  return 0;
               if((en1->lon2)==(en2->lon2)){
                  if((en1->lon3)>(en2->lon3))
                     return 0;
               }
            }
         }
      }
   }
   return 1;
}

/******************************************************************************
******************************************************************************/
int compare_ensieta_lat_seul(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       latitude puis longitude si necessaire
   if((en1->lat1)>(en2->lat1))
      return 0;
   if((en1->lat1)==(en2->lat1)){
      if((en1->lat2)>(en2->lat2))
         return 0;
      if((en1->lat2)==(en2->lat2)){
         if((en1->lat3)>(en2->lat3))
            return 0;
      }
   }
   return 1;
}

/******************************************************************************
******************************************************************************/
int compare_ensieta_lon_seul(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       latitude puis longitude si necessaire
   if((en1->lon1)>(en2->lon1))
      return 0;
   if((en1->lon1)==(en2->lon1)){
      if((en1->lon2)>(en2->lon2))
         return 0;
      if((en1->lon2)==(en2->lon2)){
         if((en1->lon3)>(en2->lon3))
            return 0;
      }
   }
   return 1;
}



/******************************************************************************
******************************************************************************/
int compare_deci_signed_lat_seuil(gps_deci_signed *ds_lu,gps_deci_signed *ds_mem,short seuil){       //        ds_mem-seuil <= ds_lu <= ds_mem+seuil ?       latitude puis longitude si necessaire
   if(ds_lu->lat < (ds_mem->lat-seuil))
      return 2;                                             //latitude lu trop petite -> 2
   else if(ds_lu->lat > (ds_mem->lat+seuil))
      return 3;                                             //latitude lu trop grande -> 3
   else if(ds_lu->lon < (ds_mem->lon-seuil))
      return 4;                                             //longitude lu trop petite -> 4
   else if(ds_lu->lon > (ds_mem->lon+seuil))
      return 5;                                             //longitude lu trop grande -> 5
   else
      return 1;                                             //dans le cadre  -> 1
}


/******************************************************************************
******************************************************************************/
/*int32 traduction_ascii_to_num(int taille){
   int i;
   int32 val;
   
   val=fgetc(gps_stream)-48;
   for(i=1;i<taille;i++){
      val=(val*10)+fgetc(gps_stream)-48;
   }
   return val;
}*/

/******************************************************************************
******************************************************************************/
int get_gprmc(gps_gprmc *gps){
   int error=0;
   int car,h,m,s,la,laM,lo,loM,N,E,vit,vit_dc,day,mon,year,course_dc,compas_E;
   short lodM,ladM,ds,course,compas;

	if(!gprmc_header())
		return 0;
	if(GetNextGPSData() !=',')
		error=2;
	
	// hours
	h=GetNextGPSData()-'0';
	h=h*10+GetNextGPSData()-'0';
   
	// minutes
	m=GetNextGPSData()-'0';
	m=m*10+GetNextGPSData()-'0';
	s=GetNextGPSData()-'0';
	s=s*10+GetNextGPSData()-'0';
	
	if(GetNextGPSData()!='.')
		error=3;
	  
	
	ds=GetNextGPSData()-'0';
	ds=ds*10+GetNextGPSData()-'0';
	ds=ds*10+GetNextGPSData()-'0';
   
	if(GetNextGPSData()!=',')    //precision pour les secondes
		error=4;
	if(GetNextGPSData()!='A')                     
		error=5;  
	if(GetNextGPSData() !=',')
		error=6;                           
	  
	//   latitude
	la=GetNextGPSData()-'0';
	la=la*10+GetNextGPSData()-'0';
	laM=GetNextGPSData()-'0';
	laM=laM*10+GetNextGPSData()-'0';
   
	if(GetNextGPSData() !='.')
		error=7;	  
	ladM=GetNextGPSData() -'0';
	ladM=ladM*10+GetNextGPSData()-'0';
	ladM=ladM*10+GetNextGPSData()-'0';
	ladM=ladM*10+GetNextGPSData()-'0';
                 				 
	if(GetNextGPSData() != ',')
		error=8;
		
	N=GetNextGPSData(); // North?
   
	if(GetNextGPSData() !=',')
		error=9;
		          
	lo=GetNextGPSData()-'0';
	lo=lo*10+GetNextGPSData()-'0';
	lo=lo*10+GetNextGPSData()-'0';
	loM=GetNextGPSData()-'0';
	loM=loM*10+GetNextGPSData()-'0';
  
	if(GetNextGPSData() != '.')
		error=10;	  
	lodM=GetNextGPSData()-'0';
	lodM=lodM*10+GetNextGPSData()-'0';
	lodM=lodM*10+GetNextGPSData()-'0';
	lodM=lodM*10+GetNextGPSData()-'0';
				  
	if(GetNextGPSData() !=',')
		error=11;
	
	E=GetNextGPSData(); // East?
   
   if(GetNextGPSData() != ',')
      error=12;
   
   //vitesse
   vit=0;             
   car=GetNextGPSData();
   while (car!='.'){
      vit=vit*10+car-'0';
      car=GetNextGPSData();
   }
   vit_dc=0;
   
   car= GetNextGPSData();
   while (car!=','){
		vit_dc=vit_dc*10+car-'0';
		car=GetNextGPSData();
   }
   
   //Course
   car = GetNextGPSData();
   course=0;
   while (car!='.'){
      course=course*10+car-'0';
	  car=GetNextGPSData();
   }

   car = GetNextGPSData();
   course_dc=0;
   while (car!=','){
      course_dc=course_dc*10+car-'0';
      car=GetNextGPSData();
   }
   
   
   //date
   day=GetNextGPSData()-'0';
   day=day*10+GetNextGPSData()-'0';
   mon=GetNextGPSData()-'0';
   mon=mon*10+GetNextGPSData()-'0';
   year=GetNextGPSData()-'0';
   year=year*10+GetNextGPSData()-'0';
   
   
   //compas
   if(GetNextGPSData()!=',')
      error=14;
	 
   car=GetNextGPSData();
   compas=0;
   while (car!=','){
      compas=compas*10+car-'0';
      car=GetNextGPSData();
   }
   compas_E=0;
   compas_E = GetNextGPSData();
                  
   //affectation à *GPG
   gps->time_h = h;
   gps->time_mn = m;
   gps->time_s = s;
   gps->time_ds = ds;
   gps->lat = la;
   gps->lat_mn=laM;
   gps->lat_mn_dec=ladM;
   if(N=='N')                     
      gps->nord=1;
   else
      gps->nord=0;
   gps->lon=lo;
   gps->lon_mn=loM;
   gps->lon_mn_dec=lodM;
   if(E=='E')                     
      gps->est=1;
   else
      gps->est=0;
   
   gps->speed=vit;
   gps->speed_dec=vit_dc;
   gps->course=course;
   gps->course_dec=course_dc;
   gps->date_d =day;
   gps->date_m =mon;
   gps->date_y =year;
   gps->compas =compas;
   if(compas_E=='E')                     
      gps->compas_est=1;
   else
      gps->compas_est=0;
   
   gps->error=error;
   if(error==0)
      return 1;
   else
      return 0;
}


/******************************************************************************
******************************************************************************/
int gprmc_header(){
	
	// TODO: lookfor frame header, until the frame is complete
	
	while(GetNextGPSData()!='$' && GetSize(&QueueGPS) >= 75){
		if(GetNextGPSData()=='G'){
			if(GetNextGPSData()=='P'){
				if(GetNextGPSData()=='R'){
				   if(GetNextGPSData()=='M'){
					  if(GetNextGPSData()=='C')
						 return 1;
				   }
				}
			}
		}
	}
	return 0;
}

/******************************************************************************
******************************************************************************/
//calcul de la distance entre le radar et la voiture
void calcul_distance(gps_deci_signed *ds_dep, gps_deci_signed *ds_arr, distance_deci_signed *distance_calcul)
{
   float deg_float,tempFloat,distance_carre,latrad, coslat,lat;
   int dif_lat,dif_lon;
   int temp;
   int lon_d_sup, lat_d_sup;
   
   if(ds_dep->lat >= ds_arr->lat){
      dif_lat = ds_dep->lat - ds_arr->lat;
      lat_d_sup = 1;
   }
   else{
      dif_lat = ds_arr->lat - ds_dep->lat;
      lat_d_sup = 0;
   }
   
   if(ds_dep->lon >= ds_arr->lon)
   {
      dif_lon = ds_dep->lon - ds_arr->lon;
      lon_d_sup = 1;
   }
   else
   {
      dif_lon = ds_arr->lon - ds_dep->lon;
      lon_d_sup = 0;
   }
   if(dif_lon > 180000000)
   {
      dif_lon = 360000000 - dif_lon;
      lon_d_sup++;
   }
   temp = lon_d_sup;
   temp = temp << 1; //multiplie par 2
   temp += lat_d_sup;
   
   // TO BE VERIFIED
   lat=ds_dep->lat-2147483648;
  latrad=lat*PI/180000000;//lattitude en radian
  coslat=1-latrad*latrad/2.0;//dl de cos de lat
  dif_lon=dif_lon*coslat;
   deg_float = dif_lon;
   tempFloat = dif_lat;
   if(dif_lat > 1)
      deg_float /= tempFloat;
      
   deg_float = atan(deg_float) * 1800;
   deg_float /= PI;
   
   distance_calcul->delta_lon = dif_lon;
   distance_calcul->delta_lat = dif_lat;
   if(((dif_lon*dif_lon)+(dif_lat*dif_lat)) < 2147483648) //2^31 et il me semble qu'il y a une erreur c'est plutôt (dif_lon*dif_lon)+(dif_lat*dif_lat)
      distance_carre = ((dif_lon*dif_lon)+(dif_lat*dif_lat));
   else
      distance_carre = 2147483648;
      
   distance_calcul->distance = sqrt(distance_carre);
   
   
   switch (temp){
      case 0b01   :  deg_float=1800-deg_float;
                     break;
      case 0b11   :  deg_float=1800+deg_float;
                     break;
      case 0b10   :  deg_float=3600-deg_float;
                     break;
      default     :  break;
   }
   
   distance_calcul->angle = deg_float;   
}
