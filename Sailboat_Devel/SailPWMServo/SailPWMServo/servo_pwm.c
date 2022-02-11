#include "servo_rs232.h"

/****************************
configure counters to output 20 msec servo. 
****************************/
void ConfigServos(uint16_t Width)
{
	struct tc_config ServoConfig;

	tc_get_config_defaults(&ServoConfig);

	ServoConfig.wave_generation = TC_WAVE_GENERATION_MATCH_PWM;
	ServoConfig.counter_16_bit.compare_capture_channel[TC_COMPARE_CAPTURE_CHANNEL_0] = (TOP);
	ServoConfig.counter_16_bit.compare_capture_channel[TC_COMPARE_CAPTURE_CHANNEL_1] = (Width);
	ServoConfig.clock_prescaler =  TC_CLOCK_PRESCALER_DIV4;
	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].enabled = true;
	
	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].pin_out = PIN_PB03F_TC6_WO1;
	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].pin_mux = MUX_PB03F_TC6_WO1;
	
	tc_init(&Sail, TC6, &ServoConfig);  // Using TC6 for Sail Winch.

	//  Could use TC4
	//	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].pin_out = PIN_PA23F_TC4_WO1;
	//	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].pin_mux = MUX_PA23F_TC4_WO1;
	//	tc_init(&Rudder, TC4, &ServoConfig);
	
	
	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].pin_out = PIN_PA13E_TC2_WO1;
	ServoConfig.pwm_channel[TC_COMPARE_CAPTURE_CHANNEL_1].pin_mux = MUX_PA13E_TC2_WO1;
	
	tc_init(&Rudder, TC2, &ServoConfig);	// Using TC2 for Rudder

	tc_enable(&Sail);
	tc_enable(&Rudder);
}

/***************
***************/
int convert_val_to_pulse_width(int val)
{
	return (int)((double)(7.8125*val))+1999;
}

/******************************************************************************
set automated servo output
******************************************************************************/
int set_servo(int num_servo,int val){           //attention, servo à partir de 1 !!!! : Warning: the servo numbers begins with 1 !!!!
   
   	SailWidth += WIDTH_INC;
   	RudderWidth += WIDTH_INC;
   	if(num_servo == 1){
	   	SailWidth = convert_val_to_pulse_width(val);
   		tc_set_compare_value(&Sail, TC_COMPARE_CAPTURE_CHANNEL_1, SailWidth);		   
	   }
	else{ 
		RudderWidth = convert_val_to_pulse_width(val);
   		tc_set_compare_value(&Rudder, TC_COMPARE_CAPTURE_CHANNEL_1, RudderWidth);		
	}  
   return 0;
}


