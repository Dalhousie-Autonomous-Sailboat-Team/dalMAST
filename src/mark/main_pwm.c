
/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>


#define TOP 39999	// PWM frequency = 20mS with div4 prescale.

// TC Instances

struct tc_module Sail;
struct tc_module Rudder;

void ConfigServos(uint16_t);


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


int main(void)
{
	system_init();
	uint16_t SailWidth, RudderWidth; 
	SailWidth = RudderWidth = 500;
	ConfigServos(1);

	// Insert application code here, after the board has been initialized.

	// This skeleton code simply sets the LED to the state of the button.
	while (1) {
		// Is button pressed?
		if (port_pin_get_input_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) { // Increase pulse width with each button push
			// Yes, so turn LED on.
			port_pin_set_output_level(LED_0_PIN, LED_0_ACTIVE);
			SailWidth += 500;
			RudderWidth += 500;
			if(SailWidth >= TOP)
				SailWidth = RudderWidth = 500;
			tc_set_compare_value(&Sail, TC_COMPARE_CAPTURE_CHANNEL_1, SailWidth);
			tc_set_compare_value(&Rudder, TC_COMPARE_CAPTURE_CHANNEL_1, RudderWidth);
			
			while(port_pin_get_input_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE); // Wait for button release
			
			} 
		else {
			// No, so turn LED off.
			port_pin_set_output_level(LED_0_PIN, !LED_0_ACTIVE);
		}
	}
}
