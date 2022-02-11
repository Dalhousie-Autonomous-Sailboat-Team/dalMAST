#include <asf.h>
#include <samd20_xplained_pro.h>
#include "MonitorRadio.h"
#include "Queue.h"


#define STRING_LEN 75
#define RADIO_SPEED 9600

/******************************************************************************
******************************************************************************/
void init_monitor_radio(void)
{

	// this one should be on SERCOM 1, PA17, PA16
	usart_get_config_defaults(&usart_config);
	usart_config.mux_setting = EXT2_UART_SERCOM_MUX_SETTING;
	usart_config.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0; // used for TX
	usart_config.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1; // 
	usart_config.pinmux_pad2 = PINMUX_UNUSED;
	usart_config.pinmux_pad3 = PINMUX_UNUSED;
	usart_config.baudrate    = RADIO_SPEED;
	/* Apply configuration */
	usart_init(&usart_module, SERCOM1, &usart_config);
	/* Enable USART */
	usart_enable(&usart_module);

	
	usart_register_callback(&usart_module, usart_callback,
		USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&usart_module, 
		USART_CALLBACK_BUFFER_RECEIVED);

	transfer_complete = false;
	
	QueueInit(&QueueGPS);
}

/******************************************************************************
******************************************************************************/
void transmit_radio(gps_gprmc *gprmc, gps_deci_signed *ds_robot, gps_deci_signed *ds_bouee, int16 angleRobot, distance_deci_signed *distance, int correctionCap,int16 dirVent, int servoVoile, int poi_actuel)
{
	int32 temp32,temp32_2;
	int16 angle1=0,angle2=0;
	// Rapport Adeunis
	//heure
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "%02uh%02um%02u,%03lu", gprmc->time_h, gprmc->time_mn, gprmc->time_s, gprmc->time_ds);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////date
	//fprintf(pickit_stream, "%02u/%02u/%02u", gprmc->date_d, gprmc->date_m, gprmc->date_y);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////angles
	//fprintf(pickit_stream, "angle robot: %5lu  cor: %3u", angleRobot, correctionCap);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "angle dest: %5lu", distance->angle);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "angle vent: %5lu", dirVent);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////logiciel roumain
	//fprintf(pickit_stream, "#data ");
	//if(!gprmc->nord)
	//fprintf(pickit_stream,"-");
	//temp32 = gprmc->lat_mn_dec;
	//temp32*=60;
	//temp32_2=temp32;
	//temp32/=10000;
	//temp32_2 = temp32_2 - temp32*10000;
	////tempf *= 0.0006;
	//fprintf(pickit_stream, "%u %u %lu,%03lu ", gprmc->lat, gprmc->lat_mn, temp32, temp32_2);
	//if(!gprmc->est);
	//fprintf(pickit_stream,"-");
	//temp32 = gprmc->lon_mn_dec;
	//temp32*=60;
	//temp32_2=temp32;
	//temp32/=10000;
	//temp32_2 = temp32_2 - temp32*10000;
	////tempf *= 0.0006;
	//fprintf(pickit_stream, "%u %u %lu,%03lu ", gprmc->lon, gprmc->lon_mn, temp32, temp32_2);
	//if(angleRobot!=-1);
	//angle1 = angleRobot/10;
	//if(distance->angle!=-1);
	//angle2 = distance->angle/10;
	//fprintf(pickit_stream, "%lu %lu ", angle1, angle2);
	//// les deux lignes modifiées pour envoyer l'heure et la date au logiciel des roumains
	//fprintf(pickit_stream, "%02uh %02um %02u,%03lu ", gprmc->time_h, gprmc->time_mn, gprmc->time_s, gprmc->time_ds);
	//fprintf(pickit_stream, "%02u/ %02u/ %02u ", gprmc->date_d, gprmc->date_m, gprmc->date_y);
	//// envoie de la vitesse du bateau
	//fprintf(pickit_stream, "%02lu,%03u ", gprmc->speed, gprmc->speed_dec);
	//// envoie de le cap fond
	//fprintf(pickit_stream, "%02lu,%03u ", gprmc->course, gprmc->course_dec);
	//// angle de direction du vent
	//dirVent = dirVent / 10;
	//fprintf(pickit_stream,"%lu ", dirVent);
	//// correction du cap
	//fprintf(pickit_stream, "%u ", correctionCap);
	//// POI visé
	//fprintf(pickit_stream, "%u", (poi_actuel+1));
	////fprintf(pickit_stream,"90 0 ");
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////GPS
	//fprintf(pickit_stream, "gprmc err: %2u",gprmc->error);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "gprmc lat: %2u,%02u.%05lu", gprmc->lat, gprmc->lat_mn, gprmc->lat_mn_dec);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "gprmc lon: %2u,%02u.%05lu", gprmc->lon, gprmc->lon_mn, gprmc->lon_mn_dec);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////fprintf(pickit_stream, "posi: %8LX,%8LX", ds_robot->lat,ds_robot->lon);
	////fputc(0x0d,pickit_stream);
	////fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "POI : %9ld,%9ld", (ds_bouee->lat-2^31), (ds_bouee->lon-2^31));
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream, "delt: %9lu,%9lu", distance->delta_lat, distance->delta_lon);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//fprintf(pickit_stream,  "dist: %9lu", distance->distance);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////servo
	//fprintf(pickit_stream,"servoVoile: %4u", servoVoile);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	////servo
	//fprintf(pickit_stream,"POI en cours: %4u", poi_actuel+1);
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//
	//fputc(0x0d,pickit_stream);
	//fputc(0x0a,pickit_stream);
	//printf(lcd_putc,"lat:%lu lon:%lu ",distance_main->delta_lat,distance_main->delta_lon);

}