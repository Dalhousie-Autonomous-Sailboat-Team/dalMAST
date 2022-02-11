
#include <math.h>
#include "voilier.h"
#include "servo_pwm.h"
// #include "configuration.h"

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



/******************************************************************************
******************************************************************************/
char voilierInit(void){
   while(!(voileNeutre() && directionNeutre())){		//while sail and direction functions are not being called, do nothing (or wait)
		// TODO wait for 100 msec
//			delay_ms(100);
	   }//
	// TODO wait for 100 msec
	// delay_ms(500);
   return 1;											// return true
}

/******************************************************************************
******************************************************************************/
char tourneD(int val,char droite){						//turn right, takes int 'val' and the bool 'right'
   int temp;
   char sens;
   sens = droite + sensDirection;						//right or left
   if(sens){											//if sens is true (yes to right)
      temp = neutreDirection+val/2;						//temp is equal to servo's neutral position plus rudder angle divided by 2 (next line indicates the possibility of negative angles)
      if(temp<neutreDirection)							//if temp is smaller thatn the servo's neutral position, temp is equal to 255
         temp = 255;
      if(temp>maxDirection)								//if temp is greater than the max direction (230), than temp is equal to the max direction
         temp=maxDirection;
      return set_servo(direction,temp);					//return set_servo with direction (1) and temp
   }
   else{												//if sens is false (yes to left)
      temp = neutreDirection-val/2;						//temp is equal to servo's neutral position minus val divided by 2
      if(temp>neutreDirection)							//if temp is greater than servo's neutral position, temp is equal to 0
         temp = 0;
      if(temp<minDirection)								//if temp is small than the min direction (50)
         temp=minDirection;								//temp is equal to min direction
      return set_servo(direction,temp);					// return set_servo with direction (1) and temp
   }
}

/******************************************************************************
******************************************************************************/
char borderVoile(int val){							//sail on border with the argument 'val' (sent later from the diriger function)
   int temp,val1;
   long temp32;
   
   //val1 = -val;
   val1=val;										//putting val into the tempory val1
   temp32 = maxVoile-minVoile;						//temp32 is equal to max sail minus min sail, defined above as 220 and 80 (220-80 = 140)
   temp32*=val1;									//temp32 is equal to temp32 multiplied with val1
   temp32/=256;										//temp32 is equal to temp32 divided by 256
   temp32 += minVoile;								//temp32 is equal to temp32 plus min sail (80)
   if(temp32>255)									//if temp32 is greater than 255, temp32 = 255
      temp32=255;
   temp=temp32;   									//temp is equal to temp32
   /*if(!sensVoile) // ! sensVoile = 1 ??? !
      temp=-temp;*/
   
   return set_servo(voile,temp);                    //return set_servo with voile (voile = 2) and temp (derived from val given by Diriger)
   
}

/******************************************************************************
******************************************************************************/
char directionNeutre(void){
   return set_servo(direction,neutreDirection);			//returns the call to motor function rudder angle and the neutral rudder angle of 130
}

/******************************************************************************
******************************************************************************/
char voileNeutre(void){
   return set_servo(voile, neutreVoile);				//returns the call to motor function sail angle and the neutral sail angle of 200
}


/******************************************************************************
******************************************************************************/
void tourneDroite(int angle_bat_cap,char droite){							//receives boat angle and bool 'port'
/*   if(angle_bat_cap<angle_filtre0){
       directionNeutre();
   }
   if(angle_bat_cap>angle_filtre0 && angle_bat_cap<angle_filtre1){
      tourneD(50,droite);
   }
   if(angle_bat_cap>angle_filtre1 && angle_bat_cap<angle_filtre2){
      tourneD(100,droite);
   }
   if(angle_bat_cap>angle_filtre2 && angle_bat_cap<angle_filtre3){
      tourneD(170,droite);
   }
   if(angle_bat_cap>angle_filtre3){
      tourneD(250,droite);
   }
*/

   
	float comsafr;
	int comsafrent;
  
	comsafr=angle_bat_cap*0.1;
	comsafrent=(int)floor(comsafr);
	tourneD(comsafrent,droite);												//converts rudder angle and gives it to tourneD
   
}


/******************************************************************************
******************************************************************************/
void Diriger(int angleBateau,int angleCap,int angleVent){ // , char angle_mort
   int A,vent,Cap,vent_bat,vent_cap; 
   char Cap_Tribord,Vent_Babord;
   

   ///////////////////////////// IGNORE THIS IS USELESS //////
   if(angleBateau> angleVent){
        vent_bat=angleBateau-angleVent;				//if angle of boat is greater than wind angle, vent_bat (relative wind direction to boat) is boatangle - windangle
   }
   else{
      vent_bat=angleVent-angleBateau;				//else vent_bat  (relative wind direction to boat) is windangle - boat angle
   }
   if(vent_bat>1800){								//if vent_bat(relative wind direction to boat) is greater than 180 degrees, than vant_bat=3600-vent_bat
      vent_bat=3600-vent_bat;
   }
   
   /////////////////// THIS CALCULATES RELATIVE WIND DIRECTION TO DESIRED  -- USED FOR SETTING SAIL POSITION [0, 1800]  //////////////////////////
     if(angleCap> angleVent){						//if desired angle is greater than wind angle, that vent_cap(relative wind direction to desired) desiredangle - windangle
        vent_cap=angleCap-angleVent;
   }
   else{
      vent_cap=angleVent-angleCap;					//else vent_cap  (relative wind direction to desired) is windangle - desiredangle
   }
   if(vent_cap>1800){
      vent_cap=3600-vent_cap;						//if vent_cap(relative wind direction to desired) is greater than 180 degrees, than vant_capt=3600-vent_cap
   }
   
   /////////////////////// THIS CALCULATES RELATIVE WIND DIRECTION TO DESIRED FOR TACKING AND RUDDER [0, 1800]. with startboard or port info  //////////////////////
   if(angleCap> angleVent){							//if desired angle is greater than wind angle, then vent(relative wind direction to desired) desiredangle - windangle
        vent=angleCap-angleVent;
        Vent_Babord=1;								//starboard = one (true?)
   }
   else{
      vent=angleVent-angleCap;						//else vent  (relative wind direction to desired) is windangle - desiredangle
      Vent_Babord=0;								// starboard = zero (false?)
   }
   if(vent>1800){					        		//if vent (relative wind direction to desired) is greater than 180 degrees, than vant_capt=3600-vent_cap
      vent=3600-vent;
      Vent_Babord++;								//starboard + one (?)
   }
   
   ////////////////// IF IN THE DEAD ZONE THIS EFFECTIVELY TACKS THE BOAT //////////////////////

   if(vent<angle_mort){								//if vent (relative wind direction to desired) is smaller than dead angle (-45 to +45 relative to incoming wind)
	   vent=angle_mort;								//then vent (relative wind direction to desired) is equal to the dead angle (maybe 45 degrees NOT DEFINED ANYWHERE)
	   if(Vent_Babord){								//if starboard is set to true, then Cap (?) is equal to windangle + dead angle (maybe 45 degrees), then modulus with 360 degrees
		   Cap=angleVent+angle_mort;
		   Cap=Cap%3600;
	   }
	   else{										// if starboard is set to false,
		   Cap=angleVent-angle_mort;				// then Cap is equal to wind direction minus the dead angle (maybe 45 degrees), then modulus with 360 degrees
		   Cap=Cap%3600;
	   }
	   //         Cap=angleVent+angle_mort;
	   //       Cap=Cap%3600;
   }
   else{											// if vent(relative wind direction to desired) is greater than dead angle (-45 to 45 degrees), then Cap is set to angleCap (desired angle)
	   Cap=angleCap;
   }
   // this part of the algorithm is attempting to place the boat in the "close haul" position, effectively tacking the boat. The close heal position hugs either side of the
   // dead zone for efficient travel when wind is in front of the boat
   
   //////////////////////////////////////////
   if(Cap> angleBateau){							// if Cap (set above) is greater than the boat angle, then A is equal to Cap minus boat angle
        A=Cap-angleBateau;
        Cap_Tribord=1;								// set port to true
   }
   else{
      A=angleBateau-Cap;							// if Cap is smaller than the boat angle, then A is equal to boat angle minus Cap
      Cap_Tribord=0;								// set port to false
   }
   if(A>1800){										// if A is greater that 180 degrees, than A is equal to 360 degrees minus the original A
      A=3600-A;
      Cap_Tribord++;								// port plus one
   }
   /////////////////////////////////////////
      tourneDroite(A,Cap_Tribord); 					// Calls function turn right (towards port) function, giving it A and true or false port value
      	  	  	  	  	  	  	  	  	  			// I'm pretty sure that this is a rudder function
 
   ///////////// SAIL CONTROL -- INDEPENDANT OF DEAD ZONE ///////////////
   
   if(vent_cap<750){								// if vent (relative wind direction to desired) is smaller than 75 degrees, call function sail on border and give it 220 (unit?)
      borderVoile(220);//reglage border				// close reach adjustment
   }
   if(vent_cap<1100 && vent_cap>750){				//if vent (relative wind direction to desired) is between 75 and 110 degrees, call function sail on border and give it 150
      borderVoile(150);//reglage travers			// beam reach adjustment
   }
   if(vent_cap>1100){								// if vent (relative wind direction to desired) is greater than 110, call function sail on border and give it 70
      borderVoile(70);//reglage grand largue 		//broad reach adjustment
   }
}
