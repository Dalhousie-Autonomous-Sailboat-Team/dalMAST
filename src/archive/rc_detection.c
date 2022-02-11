#include <asf.h>
#include "rc_detection.h"
#include "boat_clock.h"

int delta_count;
int start_count;

const int max_delta_count = 10;



//! [setup]
void configure_rc_pins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(RC_RUDDER_IN, &config_port_pin);
	port_pin_set_config(RC_SAIL_IN, &config_port_pin);	

//	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	//! [setup_5]
	//! [setup_6]
//	port_pin_set_config(LED_0_PIN, &config_port_pin);
	//! [setup_6]
}

/*************************************************
set the mode to autonomous
*************************************************/
void init_control_mode(void){
	is_autonomous = true;
}

/*************************************************
check if there is a pulse from the RC 
(check only on RC_RUDDER)
*************************************************/
void detect_remote(void)
{
	if (is_autonomous == true){
		if(port_pin_get_input_level(RC_RUDDER_IN) == false)
		{
			is_autonomous = false;
			start_count = get_boatclock();
		}
	}
	else{
		// if there is a new pulse
		if(port_pin_get_input_level(RC_RUDDER_IN) == true)
		{
			start_count = get_boatclock(); // reset the counter
		}
		else // update the clock tick count
		{
			// check if timeout has expired 
			delta_count = get_boatdeltaclock(start_count);
			if(delta_count > max_delta_count)
				is_autonomous = true;
		}
	}
}

