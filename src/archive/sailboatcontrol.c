#include <asf.h>
#include "sailboatcontrol.h"
#include "rc_detection.h"
#include "voilier.h"
#include "GPSsensor.h"
#include "WindVaneDriver.h"
#include "HMC6343.h"
#include "gps_log.h"
#include "MonitorRadio.h"
#include "servo_pwm.h"

void autonomous_navigate(void);

#define RC_RUDDER_OUT PIN_PB06
#define RC_SAIL_OUT PIN_PB07
#define AUT_RUDDER_OUT PIN_PA13
#define AUT_SAIL_OUT PIN_PB03
#define AUT_RUDDER_FBK PIN_PA13
#define AUT_SAIL_FBK PIN_PB03

#define rayon 2500//1800 //250 //150 (1unité ~ 1dm)
#define precision_validation_angle 450  //cone de validation de la bouée !!desactivé!!


int angle_nord;
int cmpn;
int cmpv;
int cmpValidation;
int temp;
int vent;
	
gps_gprmc gprmc_main;				// GPS information as read by sensor
//	gps_deci_signed ds_main;			// lat, lon
//	gps_deci_marge marge_main;			// error lat, lon
distance_deci_signed distance_main;	// distance to next waypoint
	
gps_deci_signed ds_position;		// current position read from GPS
gps_deci_signed ds_poi;				// position read from eeprom
gps_deci_marge marge_poi;			// margin	as read from eeprom
	
int nb_poi, poi_actuel; // number of waypoints and current waypoint



/******************************************************************************
******************************************************************************/
void configure_rc_extint_channel(void)
{
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);

	// RC RUDDER_IN
	config_extint_chan.gpio_pin           = PIN_PA20A_EIC_EXTINT4;
	config_extint_chan.gpio_pin_mux       = MUX_PA20A_EIC_EXTINT4;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
	extint_chan_set_config(4, &config_extint_chan);
	
	// PA21 (SAIL_IN) is PIN_PA21A_EIC_EXTINT5
	config_extint_chan.gpio_pin           = PIN_PA21A_EIC_EXTINT5;
	config_extint_chan.gpio_pin_mux       = MUX_PA20A_EIC_EXTINT5;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
	extint_chan_set_config(5, &config_extint_chan);	
	// PB05, (RUDDER_FBK) PIN_PB05A_EIC_EXTINT5
	
	// PB30 (SAIL_FBK) // PIN_PB30A_EIC_EXTINT14
}

/******************************************************************************
******************************************************************************/
void configure_rc_extint_callbacks(void)
{
	extint_register_callback(extint_rc_detection_callback, 4, 
		EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(4, EXTINT_CALLBACK_TYPE_DETECT);
}

/******************************************************************************
******************************************************************************/
void configure_ports(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	// input ports
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(RC_RUDDER_IN, &config_port_pin);
	port_pin_set_config(RC_SAIL_IN, &config_port_pin);
	port_pin_set_config(AUT_RUDDER_FBK, &config_port_pin);
	port_pin_set_config(AUT_SAIL_FBK, &config_port_pin);		

	// output ports
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(RC_RUDDER_OUT, &config_port_pin);
	port_pin_set_config(RC_SAIL_OUT, &config_port_pin);
	port_pin_set_config(AUT_RUDDER_OUT, &config_port_pin);
	port_pin_set_config(AUT_SAIL_OUT, &config_port_pin);	
	//! [setup_6]
}

/******************************************************************************
******************************************************************************/
void extint_rc_detection_callback(void)
{
	bool pin_state = port_pin_get_input_level(RC_RUDDER_IN);
	port_pin_set_output_level(RC_RUDDER_OUT, pin_state);
}

/******************************************************************************
******************************************************************************/
void navigate(void)
{
	static bool first_rc_time = true;
	static bool first_aut_time = true;
	
	if(is_autonomous == false)
	{
		if(first_rc_time){
			// enable interrupts
			configure_rc_extint_channel();
			configure_rc_extint_callbacks();

			system_interrupt_enable_global();
			first_rc_time = false;
		}
	}
	else{
		first_rc_time = true;
		// disable interrupt
		
		// call Diriger to set pwm outputs
//		autonomous_navigate();
	}
}

/******************************************************************************
******************************************************************************/
void autonomous_navigate(void){
	if(get_gprmc(&gprmc_main)){
	
		//if we have arrived at destination, we re-initialize
		if(poi_actuel>=nb_poi){
			write_ext_eeprom_POI_ACTUEL(0);
			// delay before read
			// delay_ms(30);
			poi_actuel=read_ext_eeprom_POI_ACTUEL();
		}
	
		read_ext_eeprom_POI(poi_actuel, &ds_poi, &marge_poi);
	
		// the compass is read at every loop
		if(cmpn<1)
		cmpn++;
		else{
			angle_nord=boussole_i2c();
			cmpn=0;
		}

		// the wind is read at every 500 loops
		if(cmpv<100)
		cmpv++;
		else{
			vent = getGirouette(vent,angle_nord);
			cmpv=0;
		}

		// Convert raw GPS data into our desired format
		convert_gprmc_to_deci_signed(&gprmc_main, &ds_position);
		// distance calculus
		calcul_distance(&ds_position, &ds_poi, &distance_main);
	
		// Send data to the base computer
		transmit_radio(&gprmc_main, &ds_position, &ds_poi, angle_nord,
		&distance_main, 2/*correctionCap*/, vent, 3/*servoVoile*/, poi_actuel);


		// check if we're within the specified radius of the waypoint
		if(distance_main.distance < rayon){
			cmpValidation++; // if it's been verified 4 times
			if(cmpValidation > 4){
				temp = read_ext_eeprom_POI_ACTUEL();
				temp++;//increase the waypoint number --> go to the next waypoint
				write_ext_eeprom_POI_ACTUEL(temp);
			
				// add a delay
				//				delay_ms(20);

				poi_actuel = temp;
				read_ext_eeprom_POI(poi_actuel, &ds_poi, &marge_poi);
			}
		}
		else{
			cmpValidation=0;
		}
		Diriger(angle_nord, distance_main.angle, vent);
	}
	else{
		// send a gps error on the adeunis line
		//fputc(0x0d,pickit_stream);
		//fputc(0x0a,pickit_stream);
		//fprintf(pickit_stream,"            !!!!!!   GPS ERROR !!!!!!");
		//fputc(0x0d,pickit_stream);
		//fputc(0x0a,pickit_stream);
		//fputc(0x0d,pickit_stream);
		//fputc(0x0a,pickit_stream);
	}
}
/*************************************
initialize pwm and sensors
*************************************/
void init_navigation(void){
	angle_nord = 0;
	cmpn = 0;
	cmpv = 0;
	cmpValidation = 0;
	
	system_init();
	// init_comm();
	
	configure_ports(); // initialize pins
	
	// initialize servos and sensors
	ConfigServos();
	voilierInit();

	
	nb_poi = read_ext_eeprom_NB_POI();		// reads total number of waypoints
	poi_actuel=read_ext_eeprom_POI_ACTUEL();// reads current waypoint
}