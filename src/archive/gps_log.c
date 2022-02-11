///////////////////////////////////////////////////////////////////////////
////   Library for a 24XX1025 serial EEPROM                            ////
////                                                                   ////
////   init_ext_eeprom();    Call before the other functions are used  ////
////                                                                   ////
////   write_ext_eeprom(a, d);  Write the char d to the address a      ////
////                                                                   ////
////   d = read_ext_eeprom(a);   Read the char d from the address a    ////
////                                                                   ////
////   The main program may define eeprom_sda                          ////
////   and eeprom_scl to override the defaults below.                  ////
////                                                                   ////
////                            Pin Layout                             ////
////   -----------------------------------------------------------     ////
////   |                                                          |    ////
////   | 1: A0   Address Input 0| 8: VCC   +5V                    |    ////
////   |                        |                                 |    ////
////   | 2: A1   Address Input 1| 7: WP    Write Protect          |    ////
////   |                        |                                 |    ////
////   | 3: A2   Connect to VCC | 6: SCL   EEPROM_SCL and Pull-Up |    ////
////   |                        |                                 |    ////
////   | 4: VSS  GND            | 5: SDA   EEPROM_SDA and Pull-Up |    ////
////   -----------------------------------------------------------     ////
///////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2006 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS C  ////
//// compiler.  This source code may only be distributed to other      ////
//// licensed users of the CCS C compiler.  No other use, reproduction ////
//// or distribution is permitted without written permission.          ////
//// Derivative programs created using this software in object code    ////
//// form are not restricted in any way.                               ////
///////////////////////////////////////////////////////////////////////////                                                                                                                                                         

#include "gps_log.h"

// #define EEPROM_SDA  PIN_b0
// #define EEPROM_SCL  PIN_b1
// #use i2c(Master,/*Fast,*/sda=EEPROM_SDA,scl=EEPROM_SCL, FORCE_HW)
//EEPROM
// #define PIN_READ_WRITE PIN_C0

/******************************************************************************
******************************************************************************/
void init_ext_eeprom(void)
{
   
   //output_float(EEPROM_SCL);		// set the pin floating
   //output_float(EEPROM_SDA);		// set the pin floating
   //output_bit(PIN_READ_WRITE,1);	// set EEPROM read
}

/******************************************************************************
******************************************************************************/
void write_ext_eeprom(long address, char data)
{
   int  command;
   
//   output_bit(PIN_READ_WRITE,0);	// set the EEPROM to WRITE
   
   // Check if address is located in Block 0/1
   if (address>65535)command=address_eeprom+8; 
   else command=address_eeprom;

   //i2c_start();
   //i2c_write(command);
   //i2c_write(address>>8);
   //i2c_write(address);
   //i2c_write(data);

   //i2c_stop();
   
//   output_bit(PIN_READ_WRITE,1);	// set the EEPROM to READ
// delay_ms(4);
   
}

/******************************************************************************
read a single value from eeprom
******************************************************************************/
char read_ext_eeprom(long address) {
   char data = 0;
   int   command;

   // Check if address is located in Block 0/1
   if (address>65535)command=address_eeprom+8;
   else command=address_eeprom;

   //i2c_start();
   //i2c_write(command);
   //i2c_write(address>>8);
   //i2c_write(address);
   //i2c_start();
   //i2c_write(command+1);
   //data=i2c_read(0);
   //i2c_stop();
   return(data);
}


/******************************************************************************
// function to read the waypoint lat, lon, and error margin
// num is the waypoint number
******************************************************************************/
char read_ext_eeprom_POI(long num,gps_deci_signed *ds,gps_deci_marge *marge) {          //num commence à 0
   int   command,i;
   long address;
   
   address = num*16+16;
   if (address>65535)command=address_eeprom+8;
   else command=address_eeprom;

   //i2c_start();
   //i2c_write(command);
   //i2c_write(address>>8);
   //i2c_write(address);
   //i2c_start();
   //i2c_write(command+1);
   
   ds->lat=0;
   for(i=0;i<4;i++){
      ds->lat=(ds->lat<<8);
//      ds->lat+=i2c_read();
   }
   ds->lon=0;
   for(i=0;i<4;i++){
      ds->lon=(ds->lon<<8);
//      ds->lon+=i2c_read();
   }
   marge->delta_lat=0;
   for(i=0;i<4;i++){
      marge->delta_lat=(marge->delta_lat<<8);
 //     marge->delta_lat+=i2c_read();
   }
   marge->delta_lon=0;
   for(i=0;i<3;i++){
      marge->delta_lon=(marge->delta_lon<<8);
//      marge->delta_lon+=i2c_read();
   }
   marge->delta_lon=(marge->delta_lon<<8);
//   marge->delta_lon+=i2c_read(0);
                                 
//   i2c_stop();
   
   return 1;
   
}

/******************************************************************************
writes the waypoint info to EEPROM
******************************************************************************/
char write_ext_eeprom_POI(long num,gps_deci_signed *ds,gps_deci_marge *marge) {         //num commence à 0
   int   command;
   char sans_errors;
   long address;
   
   sans_errors=1;
      
//   output_bit(PIN_READ_WRITE,0);

   address = num*16+16;
   
   if (address>65535)command=address_eeprom+8;
   else command=address_eeprom;

   //i2c_start();
   //i2c_write(command);
   //i2c_write(address>>8);
   //i2c_write(address);
   //
   //i2c_write(ds->lat>>24);
   //i2c_write(ds->lat>>16);
   //i2c_write(ds->lat>>8);
   //i2c_write(ds->lat);
   //i2c_write(ds->lon>>24);
   //i2c_write(ds->lon>>16);
   //i2c_write(ds->lon>>8);
   //i2c_write(ds->lon);
   //
   //i2c_write(marge->delta_lat>>24);
   //i2c_write(marge->delta_lat>>16);
   //i2c_write(marge->delta_lat>>8);
   //i2c_write(marge->delta_lat);
   //i2c_write(marge->delta_lon>>24);
   //i2c_write(marge->delta_lon>>16);
   //i2c_write(marge->delta_lon>>8);
   //i2c_write(marge->delta_lon);
   //
   //i2c_stop();
   //
   //output_bit(PIN_READ_WRITE,1);
   //delay_ms(4);
   return 1;
}

/******************************************************************************
writes the number of waypoints to eeprom
******************************************************************************/
void write_ext_eeprom_NB_POI(int data)
{
   int  command;
   
//   output_bit(PIN_READ_WRITE,0);
   
   command=address_eeprom;
   
   //i2c_start();
   //i2c_write(command);
   //i2c_write(0);
   //i2c_write(0);
   //i2c_write(data);
//
   //i2c_stop();
 //
   //output_bit(PIN_READ_WRITE,1);
   //delay_ms(4);
}

/******************************************************************************
reads the number of waypoints from eeprom. 2nd line. 
******************************************************************************/
int read_ext_eeprom_NB_POI(void) {
   int data = 0;
   int   command;

   command=address_eeprom;

   //i2c_start();
   //i2c_write(command);
   //i2c_write(0);
   //i2c_write(0);
   //i2c_start();
   //i2c_write(command+1);
   //data=i2c_read(0);
   //i2c_stop();
   return(data);
}


/******************************************************************************
reads the current position from EEPROM (waypoint 0). 2nd line
******************************************************************************/
int read_ext_eeprom_POI_ACTUEL() {
   int data = 0;
   int   command;

   command=address_eeprom;

   //i2c_start();
   //i2c_write(command);
   //i2c_write(0);
   //i2c_write(1);
   //i2c_start();
   //i2c_write(command+1);
   //data=i2c_read(0);
   //i2c_stop();
   return(data);
}

/******************************************************************************
// writes the current position to eeprom.  second line. 
******************************************************************************/
void write_ext_eeprom_POI_ACTUEL(int data)
{
   int  command;
   
//   output_bit(PIN_READ_WRITE,0);
   
   command=address_eeprom;
   
   //i2c_start();
   //i2c_write(command);
   //i2c_write(0);
   //i2c_write(1);
   //i2c_write(data);
//
   //i2c_stop();
 //
   //output_bit(PIN_READ_WRITE,1);
   //delay_ms(4);
}



