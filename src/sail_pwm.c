/* sail_pwm.c
 * Implementation of the PWM driver module for the autonomous sailboat project.
 * Created on Thomas Gwynne-Timothy.
 * Created by August 16, 2016.
 */

#include "sail_pwm.h"
#include "sail_debug.h"

#define PWM_MODULE				TC4
#define PWM_SAIL_OUT_PIN		PIN_PB13E_TC4_WO1
#define PWM_SAIL_OUT_MUX		MUX_PB13E_TC4_WO1
#define PWM_RUDDER_OUT_PIN		PIN_PB12E_TC4_WO0
#define PWM_RUDDER_OUT_MUX		MUX_PB12E_TC4_WO0

static struct tc_module pwm_timer;

enum status_code PWM_Init(void)
{
	// Get the default configuration
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);

	// Set the counter size
	config_tc.clock_source    = GCLK_GENERATOR_3;
	config_tc.counter_size    = TC_COUNTER_SIZE_8BIT;
	config_tc.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc.waveform_invert_output = TC_WAVEFORM_INVERT_CC0_MODE | TC_WAVEFORM_INVERT_CC1_MODE;
	config_tc.counter_8_bit.period = PWM_MAX_DUTY;

	// Setup sail channel
	config_tc.counter_8_bit.compare_capture_channel[PWM_SAIL] = 0;
	config_tc.pwm_channel[PWM_SAIL].enabled = true;
	config_tc.pwm_channel[PWM_SAIL].pin_out = PWM_SAIL_OUT_PIN;
	config_tc.pwm_channel[PWM_SAIL].pin_mux = PWM_SAIL_OUT_MUX;

	// Setup rudder channel
	config_tc.counter_8_bit.compare_capture_channel[PWM_RUDDER] = 0;
	config_tc.pwm_channel[PWM_RUDDER].enabled = true;
	config_tc.pwm_channel[PWM_RUDDER].pin_out = PWM_RUDDER_OUT_PIN;
	config_tc.pwm_channel[PWM_RUDDER].pin_mux = PWM_RUDDER_OUT_MUX;

	tc_init(&pwm_timer, PWM_MODULE, &config_tc);

	tc_enable(&pwm_timer);

	return STATUS_OK;
}


enum status_code PWM_SetDuty(PWM_ChannelID id, uint8_t duty)
{
	tc_set_compare_value(&pwm_timer, id, duty);	
	return STATUS_OK;
}


enum status_code PWM_Disable(PWM_ChannelID id)
{
	tc_set_compare_value(&pwm_timer, id, 0);	
	return STATUS_OK;
}


