#include "rc_detection.h"

void configure_extint_channel(void);
void configure_extint_callbacks(void);
void extint_detection_callback(void);

//! [setup]
void configure_extint_channel(void)
{
	//! [setup_1]
	struct extint_chan_conf config_extint_chan;
	//! [setup_1]
	//! [setup_2]
	extint_chan_get_config_defaults(&config_extint_chan);
	//! [setup_2]

	//! [setup_3]
	config_extint_chan.gpio_pin           = BUTTON_0_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = BUTTON_0_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
	//! [setup_3]
	//! [setup_4]
	extint_chan_set_config(BUTTON_0_EIC_LINE, &config_extint_chan);
	//! [setup_4]
}

void configure_extint_callbacks(void)
{
	//! [setup_5]
	extint_register_callback(extint_detection_callback,
	BUTTON_0_EIC_LINE,
	EXTINT_CALLBACK_TYPE_DETECT);
	//! [setup_5]
	//! [setup_6]
	extint_chan_enable_callback(BUTTON_0_EIC_LINE,
	EXTINT_CALLBACK_TYPE_DETECT);
	//! [setup_6]
}

//! [setup_7]
void extint_detection_callback(void)
{
	bool pin_state = port_pin_get_input_level(BUTTON_0_PIN);
	port_pin_set_output_level(LED_0_PIN, pin_state);
}

void navigate()
{
	if(is_autonomous == false)
	{
		// enable interrupts
		configure_extint_channel();
		configure_extint_callbacks();

		system_interrupt_enable_global();		
	}
	else{
		// disable interrupt
		
		// call Diriger to set pwm outputs
		autonomous_navigate();
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