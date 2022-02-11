
#include <asf.h>
#include "rc_detection.h"
#include "sailboatcontrol.h"
#include "boat_clock.h"


// This is a comment



/******************************************************************************
******************************************************************************/
int init_comm(void)
{
//	init_i2c();			// EEPROM and compass interface
	
	// UART interfaces
//	init_gps_link();		
//	init_windvane_link();
//	init_monitor_radio();
//	init_pwm();
	

	
	return 0;
}

/******************************************************************************
******************************************************************************/
int main (void)
{
	init_navigation();		// initialize servos and sensors
	configure_rc_pins();	// configure the pins for the RC
	init_control_mode();	// set mode to autonomous
	init_boatclock();		// 
			
	while(1)
	{
		detect_remote();	// poll the RC to check if there is a signal
		navigate();			// sail!
	}
	return 0;
}
