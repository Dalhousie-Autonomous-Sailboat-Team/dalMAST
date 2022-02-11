#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "servo_rs232.h"
#include "eeprom_24_1024_PrIn.h"
#include "gprmc.h"
#include "HMC6343.h"
#include "girouetteDriver.h"
#include "voilier.h"

// #use delay(clock=48000000)



//liaison RS232 Pickit2
#define pickit_stream 1
// #use rs232(baud=9600,parity=N,xmit=pin_b7,rcv=pin_b6,bits=8,errors,stream=pickit_stream)

//servos RS232
#define servo_tx pin_c1
#define servo_rx pin_c2
#define servo_stream 2
// #use rs232(baud=9600,parity=N,xmit=servo_tx,rcv=servo_rx,bits=8,errors,stream=servo_stream)


////////////           I²C           ////////////
#define EEPROM_SDA  PIN_b0
#define EEPROM_SCL  PIN_b1
// #use i2c(Master,/*Fast,*/sda=EEPROM_SDA,scl=EEPROM_SCL, FORCE_HW)
//EEPROM
#define PIN_READ_WRITE PIN_C0


//GPS
#define gps_stream 3
// #use rs232(baud=115200,parity=N,xmit=PIN_C7,rcv=PIN_C5,bits=8,stream=gps_stream,ERRORS)


//boussole                 HMC6352_read_heading()
#define angle_boussole_boitier    3530          //2690            //850
#define angle_cone_ok 0
#define angle_precision 200
                                //{0,300,600,900,1200,1500,1800,2100,2400,2700,3000,3300,3600};
//short const table_corection[13] = {0,320,550,820,1140,1420,1780,2170,2480,2110,3040,3360,3600};
//short const table_corection[13] = {0,440,740,1040,1310,1500,1760,2000,2270,2530,2810,3200,3600};
short const table_corection[13] = {0,305,555,800,1060,1340,1650,1960,2290,2640,2990,3320,3625};



//Girouette
#define girouetteCor 1800
#define girouette_stream 4
// #use rs232(baud=4800,parity=N,xmit=PIN_C7,rcv=PIN_C4,bits=8,stream=gps_stream,ERRORS)

//Voilier
#define direction 1
#define sensDirection 0  // ou 0
#define neutreDirection 130
#define minDirection 50
#define maxDirection 230
#define angle_max_pres 600 //200      
#define angle_max_portant 1000 //200
#define angle_limite_pres 1600 
#define angle_filtre0 130                   //sur 180°
#define angle_filtre1 280
#define angle_filtre2 550
#define angle_filtre3 800

#define voile 2
#define sensVoile 1  // ou 1
#define neutreVoile 200//177
#define minVoile 80 //100 //140 //160
#define maxVoile 220 //210//200 //247//255//240        //bordé à fond

#define angleBordMax 500        //cone de reglage de voile (limite cone bordé max, "camembert face au vent")


//Logic Voilier
//#define direction_vent 3150
#define rayon 2500//1800 //250 //150 (1unité ~ 1dm)   
#define angle_mort 500  //450 //"camembert interdit au près"
#define precision_validation_angle 450  //cone de validation de la bouée !!desactivé!!


//Adeunis
// #include "adeunis_report.c"

//LCD
//#include <nokia_projet_indus.h>
// #define LCD_DB4   PIN_B4 
// #define LCD_DB5   PIN_B5 
// #define LCD_DB6   PIN_B6 
// #define LCD_DB7   PIN_B7 

// #define LCD_E     PIN_B3 
// #define LCD_RS    PIN_B2 
// #include "flex_lcd.c"

//LEDs
#define led1 pin_a5
#define led2 pin_a4
#define led3 pin_a3
#define led4 pin_a5

#define UCFG = 0xF6F // byte
#define UTRDIS = UCFG.3 // bit



gps_gprmc *gprmc_main;
gps_deci_signed *ds_main;
gps_deci_marge *marge_main;
distance_deci_signed *distance_main;

void beep(int num);
void init_main();

#endif