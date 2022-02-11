#include "configuration.h"

void init_main(){
  gprmc_main=malloc(28);
   ds_main=malloc(8);
   marge_main=malloc(8);
   distance_main=malloc(14);
   
   setup_wdt(WDT_ON);
   setup_adc_ports(AN0|VSS_VDD);
   setup_adc(ADC_CLOCK_DIV_4);
   setup_timer_1(T1_DISABLED);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   setup_timer_2(T2_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);
   setup_vref(FALSE);
   UTRDIS = 1;                      //permet de desactiver completement l'USB
   
   
   // TODO: USER CODE!!
   init_ext_eeprom();
   restart_wdt();
   lcd_init();  
}

void beep(int num){
   int temp,temp2;
   for(temp2=0;temp2<num;temp2++){
      for(temp=0;temp<255;temp++){
         output_bit(pin_a5,1);
         delay_us(50);
         output_bit(pin_a5,0);
         delay_us(200);
      }
      delay_ms(200);
      restart_wdt();
   }
}

void main()
{  
   int i, cmpn, cmpv,nb_poi,poi_actuel,cmpValidation,temp;
   int16 angle_nord,vent;
   gps_deci_signed *ds_position,*ds_poi;
   gps_deci_marge *marge_poi;
   
   int attente_val=0;
   int attente[3]={0b01111100,0b00101111,'-'};
   ds_position=malloc(8);
   ds_poi=malloc(8);
   marge_poi=malloc(8);
   i=0;
   temp=0;
   angle_nord=0;
   cmpv=0;
   cmpn=0;
   init_main();
   
   
   lcd_gotoxy(2,1);
   printf(lcd_putc,"Test Automate");
   fprintf(pickit_stream,"Test Automate");
   fputc(0x0a,pickit_stream);
   fputc(0x0d,pickit_stream);
  
   beep(10);
   output_bit(led2,voilierInit());
   nb_poi=read_ext_eeprom_NB_POI();//2eme ligne 9et10eme chiffre  
   poi_actuel=read_ext_eeprom_POI_ACTUEL();//2eme ligne 11 et 12 eme chiffre
   //write_ext_eeprom_POI_ACTUEL(0);met le trajet à zéro
 
   while(true){
      restart_wdt();//Watch dog
      if(get_gprmc(gprmc_main)){
         restart_wdt();//Watch dog
           
     
     //si fin alors remise à zéro
         if(poi_actuel>=nb_poi){//->we arrived to the destination!!
            write_ext_eeprom_POI_ACTUEL(0);
            delay_ms(30);
            poi_actuel=read_ext_eeprom_POI_ACTUEL();
         }
         
         restart_wdt();
         read_ext_eeprom_POI(poi_actuel, ds_poi, marge_poi);
         
         
         restart_wdt();
         if(cmpn<2){//tous les 20 tours on vérifie la direction du nord
         cmpn++;
         }
         else{
            angle_nord=boussole_i2c();//compass data acquisition
            cmpn=0;
         }
      
         if(cmpv<30){//tous les 140 BOUCLES on vérifie la direction du vent
            cmpv++;
         }
         else{
            vent = getGirouette(vent,angle_nord);
            cmpv=0;
         }

         restart_wdt();
         //aquisition des données et calculs
         convert_gprmc_to_deci_signed(gprmc_main, ds_position);//Convert raw GPS data into our desired format  
         calcul_distance(ds_position,ds_poi,distance_main);//distance calculus
         
         restart_wdt();//Watch dog
      
 
       
       
      lcd_init();
    
//        lcd_gotoxy(1,1);
//       printf(lcd_putc,"lat:%5lu",ds_poi->lat);
//         lcd_gotoxy(1,2);
//         printf(lcd_putc,"lon:%5lu",ds_poi->lon);
          
    
       
       
       
      lcd_gotoxy(1,1);
       printf(lcd_putc,"DiG:%5lu",distance_main->distance);
       lcd_gotoxy(1,2);
       printf(lcd_putc,"AnG:%4lu Nd:%4lu",distance_main->angle,angle_nord);
     lcd_gotoxy(12,1);
       printf(lcd_putc,"BO:%u",poi_actuel);
       restart_wdt(); 
         
         restart_wdt();
         //HF communication module : Send data to the base computer
         adeunis (gprmc_main, ds_position, ds_poi, angle_nord, distance_main, 2/*correctionCap*/, vent, 3/*servoVoile*/, poi_actuel);



         if(distance_main->distance<(rayon)){
                  cmpValidation++;
                  if(cmpValidation>4){
                     temp = read_ext_eeprom_POI_ACTUEL();
                     temp++;//increase the waypoint number --> go to the next waypoint
                     write_ext_eeprom_POI_ACTUEL(temp);
                     delay_ms(20);
                     beep(20);
                     poi_actuel=temp;
                     read_ext_eeprom_POI(poi_actuel, ds_poi, marge_poi);
                  }
            }
            else{
               cmpValidation=0;
            }   
            Diriger(angle_nord,distance_main->angle,vent, angle_mort);
            restart_wdt();

      }
      else{
         lcd_init();
         
            fputc(0x0d,pickit_stream);
            fputc(0x0a,pickit_stream);
            fprintf(pickit_stream,"            !!!!!!   GPS ERROR !!!!!!");
            fputc(0x0d,pickit_stream);
            fputc(0x0a,pickit_stream);
            fputc(0x0d,pickit_stream);
            fputc(0x0a,pickit_stream);
   
         printf(lcd_putc,"  error GPS %c",attente[attente_val++]);
         attente_val%=3;
         i++;
         if(i==255)
            reset_cpu();
      }
      restart_wdt();
      
   }              
   
   free(distance_main);
   free(ds_position);
   free(ds_poi);
   free(marge_poi);
   free(marge_main);
   free(ds_main);
   free(gprmc_main);
}
