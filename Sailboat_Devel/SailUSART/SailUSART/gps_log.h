#ifndef GPS_LOG_H
#define GPS_LOG_H

///////////////////////////////////////////////////////////////////////////
////   Library for a 24XX1025 serial EEPROM                            ////
////                                                                   ////
////   init_ext_eeprom();    Call before the other functions are used  ////
////                                                                   ////
////   write_ext_eeprom(a, d);  Write the byte d to the address a      ////
////                                                                   ////
////   d = read_ext_eeprom(a);   Read the byte d from the address a    ////
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

//#define EEPROM_SDA  PIN_b4
//#define EEPROM_SCL  PIN_b6
//#define PIN_READ_WRITE PIN_C0
#define address_eeprom 0xa4
#define EEPROM_ADDRESS long
#define EEPROM_SIZE   0x1FFFF
//#define PIN_READ_WRITE pin_c3
//#use i2c(master,Fast, sda=EEPROM_SDA, scl=EEPROM_SCL)


/////////////////////////////
////                     ////
//// Function Prototypes ////
////                     ////
/////////////////////////////


typedef struct gps_deci_signed_           //malloc(8)
{
   long lon; 
   long lat; 
}gps_deci_signed;

typedef struct gps_deci_marge_           //malloc(8)
{
   long delta_lon; 
   long delta_lat; 
}gps_deci_marge;

void init_ext_eeprom(void);
/*
void write_ext_eeprom(int32 address, BYTE data);
This function writes a byte of data to the specified address in the EEPROM
Param: EEPROM_ADDRESS : The address where data is to be written (32 bit)
Param data: The byte of data to be written
Returns: none
*/

void write_ext_eeprom(EEPROM_ADDRESS address, char data);
/*
BYTE read_ext_eeprom(EEPROM_ADDRESS address)
This will read a byte of data from the EEPROM
Param:  EEPROM_ADDRESS : The read address of the EEPROM
Returns: Data byte
*/

// void init_ext_eeprom(void);
// void write_ext_eeprom(long address, char data);
char read_ext_eeprom(long address);
void write_ext_eeprom_NB_REC(int data);
void write_ext_eeprom_POI_ACTUEL(int data);
int read_ext_eeprom_NB_POI(void);
int read_ext_eeprom_POI_ACTUEL(void);
char read_ext_eeprom_POI(long num,gps_deci_signed *ds,gps_deci_marge *marge);

void decale_haut(int add_rec_deb_decal);
// void decale_bas_simple(int add_a_combler);
// void decale_bas(int add_a_combler,char periode);

#endif