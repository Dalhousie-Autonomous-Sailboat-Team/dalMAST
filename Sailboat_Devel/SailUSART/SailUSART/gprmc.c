//ensembles des fonctions de traitement de la trame GPRMC


//#define gps_stream 1
//#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=gps_stream,ERRORS)


///////////////////////////////////////////////////
//
//       definition des différents types
//
///////////////////////////////////////////////////
#include <math.h>
#include "gprmc.h"
// #include "configuration.h"

#define PI 3.14159265

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

void convert_ensieta_to_signed(gps_ensieta *en,gps_deci_signed *ds){
   gps_deci_unsigned *du;
   convert_ensieta_to_unsigned(en,du);
   convert_deci_unsigned_to_signed(du,ds);
}

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

//#separate
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

//#separate
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

//#separate
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

// #separate
char compare_ensieta_lon(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       longitude puis latitude si necessaire
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

// #separate
char compare_ensieta_lat(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       latitude puis longitude si necessaire
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

// #separate
char compare_ensieta_lat_seul(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       latitude puis longitude si necessaire
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

// #separate
char compare_ensieta_lon_seul(gps_ensieta *en1,gps_ensieta *en2){                         // en1 <= en2 ?       latitude puis longitude si necessaire
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



// #separate
int compare_deci_signed_lat_seuil(gps_deci_signed *ds_lu,
gps_deci_signed *ds_mem,short seuil){       //        ds_mem-seuil <= ds_lu <= ds_mem+seuil ?       latitude puis longitude si necessaire
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


//#separate
long traduction_ascii_to_num(int taille){
   int i;
   long val;
   
   val=fgetc(gps_stream)-48;
   for(i=1;i<taille;i++){
      val=(val*10)+fgetc(gps_stream)-48;
   }
   return val;
}

//#separate
/*int1 get_gprmc(gps_gprmc *gps){
   int temp1,h,m,s;
   long temp32;
   gps_ok=0;
   while(!attente_trame()){}

   if(fgetc(gps_stream)!=','){
      //gps->valid=0;
      //goto end;
      }//,            heure
   h=fgetc(gps_stream)-48;
   h=h*10+fgetc(gps_stream)-48;
   m=fgetc(gps_stream)-48;
   m=m*10+fgetc(gps_stream)-48;
   s=fgetc(gps_stream)-48;
   s=s*10+fgetc(gps_stream)-48;
   
   gps->time_h=h;
   gps->time_mn=m;
   gps->time_s=s;
   
   
   //time_h    =  traduction_ascii_to_num(2);
   //time_mn   =  traduction_ascii_to_num(2);
   //time_s    =  traduction_ascii_to_num(2);
   if(temp_gps!=46){
      //gps->valid=0;
      //goto end;
      }                           //.
   gps->time_ds   =  traduction_ascii_to_num(3);
   fgetc(gps_stream);                           //,            validité
   if(fgetc(gps_stream)=='A')                     
      gps->error=0;
   else
      gps->error=1;
   fgetc(gps_stream);                           //,            latitude
   gps->lat       =  traduction_ascii_to_num(2);
   gps->lat_mn    =  traduction_ascii_to_num(2);
   fgetc(gps_stream);                           //.
   gps->lat_mn_dec=  traduction_ascii_to_num(4);
   fgetc(gps_stream);                           //,
   if(fgetc(gps_stream)=='N')                     
      gps->nord=1;
   else
      gps->nord=0;
   fgetc(gps_stream);                           //,            longitude
   gps->lon       =  traduction_ascii_to_num(3);
   gps->lon_mn    =  traduction_ascii_to_num(2);
   fgetc(gps_stream);                           //.
   gps->lon_mn_dec=  traduction_ascii_to_num(4);
   fgetc(gps_stream);                           //,
   if(fgetc(gps_stream)=='E')                     
      gps->est=1;
   else
      gps->est=0;
   fgetc(gps_stream);                           //,            vitesse
   temp1=fgetc(gps_stream);
   gps->speed=0;
      while (temp1!='.'){
         gps->speed=gps->speed*10+temp1-48;
         temp1=fgetc(gps_stream);
      }
   
   //gps->speed     =  traduction_ascii_to_num(3);
   //fgetc(gps_stream);                           //.
   gps->speed_dec =  fgetc(gps_stream);
   fgetc(gps_stream);                           //,            course
   temp1=fgetc(gps_stream);
   gps->course=0;
      while (temp1!='.'){
         gps->course=gps->course*10+temp1-48;
         temp1=fgetc(gps_stream);
      }
   //gps->course     =  traduction_ascii_to_num(3);
   //fgetc(gps_stream);                           //.
   gps->course_dec =  fgetc(gps_stream);
   fgetc(gps_stream);                           //,            date
   gps->date_d    =  traduction_ascii_to_num(2);
   gps->date_m    =  traduction_ascii_to_num(2);
   gps->date_y    =  traduction_ascii_to_num(2);
   fgetc(gps_stream);                           //,            compas
   gps->compas    =  traduction_ascii_to_num(3);
   fgetc(gps_stream);                           //,
   if(fgetc(gps_stream)=='E')                     
      gps->compas_est=1;
   else
      gps->compas_est=0;
   
   end : return;
}*/

char get_gprmc(gps_gprmc *gps){
   int error=0;
   char car,h,m,s,la,laM,lo,loM,N,E,vit,vit_dc,day,mon,year,course_dc,compas_E;
   short lodM,ladM,ds,course,compas;

   while(!attente_trame()){}

   if(fgetc(gps_stream)!=',')
      error=2;
   h=fgetc(gps_stream)-48;
   h=h*10+fgetc(gps_stream)-48;
   m=fgetc(gps_stream)-48;
   m=m*10+fgetc(gps_stream)-48;
   s=fgetc(gps_stream)-48;
   s=s*10+fgetc(gps_stream)-48;
   if(fgetc(gps_stream)!=46)
      error=3;
   ds=fgetc(gps_stream)-48;
   ds=ds*10+fgetc(gps_stream)-48;
   ds=ds*10+fgetc(gps_stream)-48;
   
   if(fgetc(gps_stream)!=',')    //precision pour les secondes
      error=4;
   
   if(fgetc(gps_stream)!='A')                     
      error=5;
   if(fgetc(gps_stream)!=',')
      error=6;                           //,            latitude
                  
   la=fgetc(gps_stream)-48;
   la=la*10+fgetc(gps_stream)-48;
   laM=fgetc(gps_stream)-48;
   laM=laM*10+fgetc(gps_stream)-48;
   if(fgetc(gps_stream)!=46)
      error=7;
   ladM=fgetc(gps_stream)-48;
   ladM=ladM*10+fgetc(gps_stream)-48;
   ladM=ladM*10+fgetc(gps_stream)-48;
   ladM=ladM*10+fgetc(gps_stream)-48;
                  
   if(fgetc(gps_stream)!=',')
      error=8;
   N=fgetc(gps_stream);
   if(fgetc(gps_stream)!=',')
      error=9;
                  
   lo=fgetc(gps_stream)-48;
   lo=lo*10+fgetc(gps_stream)-48;
   lo=lo*10+fgetc(gps_stream)-48;
   loM=fgetc(gps_stream)-48;
   loM=loM*10+fgetc(gps_stream)-48;
   if(fgetc(gps_stream)!=46)
      error=10;
   lodM=fgetc(gps_stream)-48;
   lodM=lodM*10+fgetc(gps_stream)-48;
   lodM=lodM*10+fgetc(gps_stream)-48;
   lodM=lodM*10+fgetc(gps_stream)-48;
                  
   if(fgetc(gps_stream)!=',')
      error=11;
   E=fgetc(gps_stream);
   if(fgetc(gps_stream)!=',')
      error=12;
   
   
   //vitesse
   vit=0;             
   car=fgetc(gps_stream);
   while (car!=46){
      vit=vit*10+car-48;
      car=fgetc(gps_stream);
   }
   vit_dc=0;
   car=fgetc(gps_stream);
   while (car!=','){
      vit_dc=vit_dc*10+car-48;
      car=fgetc(gps_stream);
   }
   
   
   //Course
   car=fgetc(gps_stream);
   course=0;
   while (car!=46){
      course=course*10+car-48;
      car=fgetc(gps_stream);
   }
   car=fgetc(gps_stream);
   course_dc=0;
   while (car!=','){
      course_dc=course_dc*10+car-48;
      car=fgetc(gps_stream);
   }
   
   
   //date
   day=fgetc(gps_stream)-48;
   day=day*10+fgetc(gps_stream)-48;
   mon=fgetc(gps_stream)-48;
   mon=mon*10+fgetc(gps_stream)-48;
   year=fgetc(gps_stream)-48;
   year=year*10+fgetc(gps_stream)-48;
   
   
   //compas
   if(fgetc(gps_stream)!=',')
      error=14;
   car=fgetc(gps_stream);
   compas=0;
   while (car!=','){
      compas=compas*10+car-48;
      car=fgetc(gps_stream);
   }
   compas_E=0;
   compas_E=fgetc(gps_stream);
                  
   //affectation à *GPG
   gps->time_h=h;
   gps->time_mn=m;
   gps->time_s=s;
   gps->time_ds=ds;
   gps->lat=la;
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


//attente trame gprmc

//#separate
char attente_trame(){
   while(fgetc(gps_stream)!='$'){}
      if(fgetc(gps_stream)=='G'){
         if(fgetc(gps_stream)=='P'){
            if(fgetc(gps_stream)=='R'){
               if(fgetc(gps_stream)=='M'){
                  if(fgetc(gps_stream)=='C'){
                     return 1;
                  }
               }
            }
         }
      }
      return 0;
}


//macro d'affichage
/*
#separate
void affiche(){
   lcd_putc("\f");
   lcd_gotoxy(1,1);
   if (comp<25){
      printf(lcd_putc,"%02u/%02u/%02u",day,mon,year);
      printf(lcd_putc," %3ukm/h",vit1);
      lcd_gotoxy(5,2);
      printf(lcd_putc,"%02uh%02um%02u",h,m,s);
      }
   else{
      printf(lcd_putc,"  %3u%c %02u'%04lu",la,N,laM,ladM);
      lcd_gotoxy(1,2);
      printf(lcd_putc,"  %3u%c %02u'%04lu",lo,E,loM,lodM);
   }
}*/




//calcul de la distance entre le radar et la voiture

void calcul_distance(gps_deci_signed *ds_dep, gps_deci_signed *ds_arr, distance_deci_signed *distance_calcul)
{
   float deg_float,tempFloat,distance_carre,latrad, coslat,lat;
   long dif_lat,dif_lon;
   int temp;
   char lon_d_sup, lat_d_sup;
   
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

/*
int1 presence_poi(distance_deci_signed *distance_calcul,gps_deci_marge *marge){
   if(distance_calcul->delta_lon<marge->delta_lon)
      if(distance_calcul->delta_lat<marge->delta_lat)
         return 1;
   return 0;
}*/



//traitement de la trame gprmc
/*
#separate
void trame_gprmc(){
      car=getchar();
      h=getchar()-48;
      h=h*10+getchar()-48;
      m=getchar()-48;
      m=m*10+getchar()-48;
      s=getchar()-48;
      s=s*10+getchar()-48;
      
      while(getchar()!=','){}    //precision pour les secondes
            
      valid=getchar();   //validité
                     
      getchar();
                     
      la=getchar()-48;
      la=la*10+getchar()-48;
      laM=getchar()-48;
      laM=laM*10+getchar()-48;
      getchar();
      ladM=getchar()-48;
      ladM=ladM*10+getchar()-48;
      ladM=ladM*10+getchar()-48;
      ladM=ladM*10+getchar()-48;
                     
      getchar();
      N=getchar();
      getchar();
                     
      lo=getchar()-48;
      lo=lo*10+getchar()-48;
      lo=lo*10+getchar()-48;
      loM=getchar()-48;
      loM=loM*10+getchar()-48;
      car=getchar();
      lodM=getchar()-48;
      lodM=lodM*10+getchar()-48;
      lodM=lodM*10+getchar()-48;
      lodM=lodM*10+getchar()-48;
                     
      getchar();
      E=getchar();
      getchar();
      
      
      //vitesse
                   
      car=getchar();
      while (car!='.'){
         vit1=vit1*10+car-48;
         car=getchar();
      }
      if (getchar()>53)
         vit1=vit1+1; 
      vit1=vit1*185;
      vit1=vit1/100;
      
      while (getchar()!=','){}
      while (getchar()!=','){}
      
      day=getchar()-48;
      day=day*10+getchar()-48;
      mon=getchar()-48;
      mon=mon*10+getchar()-48;
      year=getchar()-48;
      year=year*10+getchar()-48;
                     
      affiche();
                     
      ladM=ladM/100;
      lodM=lodM/100;
                     
      if(valid=='A')
         calcul_coord();
      else{                   //trame non valide
         led_off(vert);
         led_on(rouge);
      }

}

*/

