#include "HMC6343.h"
// #include "configuration.h"

#define angle_boussole_boitier    3530          //2690            //850
#define angle_cone_ok 0
#define angle_precision 200
//{0,300,600,900,1200,1500,1800,2100,2400,2700,3000,3300,3600};
//short const table_corection[13] = {0,320,550,820,1140,1420,1780,2170,2480,2110,3040,3360,3600};
//short const table_corection[13] = {0,440,740,1040,1310,1500,1760,2000,2270,2530,2810,3200,3600};
short const table_corection[13] = {0,305,555,800,1060,1340,1650,1960,2290,2640,2990,3320,3625};


short correction_boussole(short angleBoussole){
   short angleReel,angleD;
   long pasComp, pasTh,temp32;
   int i=0;
   
   while(table_corection[i]<=angleBoussole){
      i++;
      if(i>12)
         return -1;
   }
   angleD = angleBoussole + 3600-table_corection[i-1];
   angleD%=3600;
   pasComp = table_corection[i]-table_corection[i-1];
   pasTh=300;
   temp32 = angleD;
   temp32*=pasTh;
   temp32+=150;
   temp32/=pasComp;
   temp32+= 300*(i-1);
   
   angleReel=temp32;
   
   return angleReel%3600;
}

//renvoie l'angle en 1/10 de deg par rapport au Nord, sens des aiguille d'une montre
//#separate
short boussole_i2c(void)
{ 
   char lsb; 
   char msb;
   long angle;
   
   // TODO: request a read
   // i2c_start(); 
   // i2c_write(HMC6343_I2C_WRITE_ADDRESS); 
   // i2c_write(HMC6343_GET_DATA_COMMAND); 
   // i2c_stop(); 

	// TODO: read   
   // i2c_start(); 
   // i2c_write(HMC6343_I2C_READ_ADDRESS); 
   // msb = i2c_read(); 
   // lsb = i2c_read(0); 
   // i2c_stop();
    
   angle = (short)lsb | ((short)msb << 8);
   if(angle<=3600){
      angle+=angle_boussole_boitier;
      angle = angle % 3600;
      return(correction_boussole(angle));
   }
   else return -1;
   } 



//=================================== 
//heading = HMC6352_read_heading(); 
//=================================== 



