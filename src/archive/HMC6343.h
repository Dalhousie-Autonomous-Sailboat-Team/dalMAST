#ifndef HMC6343_H
#define HMC6343_H

//#use i2c(Master, sda=PIN_C4, scl=PIN_C3, FORCE_HW) 

#define HMC6343_I2C_WRITE_ADDRESS 0x32 
#define HMC6343_I2C_READ_ADDRESS 0x33 
#define HMC6343_GET_DATA_COMMAND 0x50 

//#define angle_boussole_boitier
//#define angle_cone
//#define angle_precision
                                //{0,300,600,900,1200,1500,1800,2100,2400,2700,3000,3300,3600}
//short const table_corection[13] = {0,300,600,900,1200,1500,1800,2100,2400,2700,3000,3300,3600};

short correction_boussole(short angleBoussole);
short boussole_i2c(void);
int cone_direction(short robot_nord,short direction_nord);
short moyenne(short val1,short val2);
short HMC6352_moyenne(short mHMC6352_moyenne_6);

#endif