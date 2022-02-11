#include <asf.h>
#include "SailboatTimers.h"

#define PWM_MODULE	EXT1_PWM_MODULE
#define PWM_RUDDER_PIN		EXT1_PWM_0_PIN
#define PWM_RUDDER_MUX		EXT1_PWM_0_MUX
#define PWM_SAIL_PIN		EXT1_PWM_1_PIN
#define PWM_SAIL_MUX		EXT1_PWM_1_MUX

int sail_mag = 0;
int rudder_mag = 0;

void configure_tc(void);
void configure_tc_callbacks(void);
void tc_callback_to_change_rudder_dc(struct tc_module *const module_inst);
void tc_callback_to_change_sail_dc(struct tc_module *const module_inst);

struct tc_module tc_instance;

/******************************************************************************
******************************************************************************/
void tc_callback_to_change_rudder_dc(struct tc_module *const module_inst)
{
	uint16_t rudder_width = 4096 + 4096*rudder_mag/256;

	//rudder_width += 2;
	//if (rudder_width == (2*4096))
		//rudder_width = 4096;
	
	tc_set_compare_value(module_inst, TC_COMPARE_CAPTURE_CHANNEL_0, rudder_width);
}

/******************************************************************************
******************************************************************************/
void tc_callback_to_change_sail_dc(struct tc_module *const module_inst)
{
	uint16_t sail_width = 4096 + 4096*sail_mag/256;

	// i += 2;
	// if (i == (2*4096))
	//		i = 4096;
	
	tc_set_compare_value(module_inst, TC_COMPARE_CAPTURE_CHANNEL_1, sail_width);
}



/******************************************************************************
******************************************************************************/
void configure_tc(void)
{
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);

	config_tc.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV2;
	config_tc.counter_16_bit.compare_capture_channel[0] = 0xFFFF;

	// rudder servo
	config_tc.pwm_channel[0].enabled = true;
	config_tc.pwm_channel[0].pin_out = PWM_RUDDER_PIN;
	config_tc.pwm_channel[0].pin_mux = PWM_RUDDER_MUX;
	
	// sail servo
	config_tc.pwm_channel[1].enabled = true;
	config_tc.pwm_channel[1].pin_out = PWM_SAIL_PIN;
	config_tc.pwm_channel[1].pin_mux = PWM_SAIL_MUX;


	tc_init(&tc_instance, PWM_MODULE, &config_tc);
	tc_enable(&tc_instance);
}

/******************************************************************************
******************************************************************************/
void configure_tc_callbacks(void)
{
	// register callback to change the duty cycle of the rudder
	tc_register_callback(&tc_instance, tc_callback_to_change_rudder_dc,
		TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
	
	// register callback to change the duty cycle of the rudder
	tc_register_callback(&tc_instance, tc_callback_to_change_sail_dc,
	TC_CALLBACK_CC_CHANNEL1);
	tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
	
}

/******************************************************************************
******************************************************************************/
int set_rudder_mag(int mag)
{
	rudder_mag = mag;	
	return 0;	
}

/******************************************************************************
******************************************************************************/
int set_sail_mag(int mag)
{
	sail_mag = mag;
	return 0;
}

/******************************************************************************
******************************************************************************/
void init_pwm(void)
{
	// this generates a 1 sec tick from the 8 MHz clock. 
	SysTick_Config(system_gclk_gen_get_hz(GCLK_GENERATOR_0));
	
	configure_tc();
	configure_tc_callbacks();

	system_interrupt_enable_global();
	sail_mag = 127;
	rudder_mag = 127;
}


