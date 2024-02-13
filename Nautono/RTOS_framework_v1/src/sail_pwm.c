// Unused code, left for learning purposes.


/* sail_pwm.c
 * Implementation of the PWM driver module for the autonomous sailboat project.
 * Created on Thomas Gwynne-Timothy.
 * Created by August 16, 2016.
 */

#include "sail_pwm.h"
#include "sail_debug.h"

//only 2 outputs of this software therefore ony 2 pins

#define PWM_MODULE				TC1
#define PWM_SAIL_OUT_PIN		PIN_PA10E_TC1_WO0
#define PWM_SAIL_OUT_MUX		MUX_PA10E_TC1_WO0

static struct tc_module pwm_timer;

// Function to convert angle to duty cycle
static uint8_t AngleToDutyCycle(uint8_t angle) {
    if (angle > PWM_MAX_ANGLE) angle = PWM_MAX_ANGLE; // Limiting the angle to the maximum 
    return (uint8_t)((float)angle / PWM_MAX_ANGLE * PWM_MAX_DUTY); // Converting the angle to duty cycle
}

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

	tc_init(&pwm_timer, PWM_MODULE, &config_tc);

	tc_enable(&pwm_timer);

	return STATUS_OK;
}


enum status_code PWM_SetDuty(PWM_ChannelID id, uint8_t duty)
{
	tc_set_compare_value(&pwm_timer, id, duty);	
	return STATUS_OK;
}

// Function to set PWM based on angle
enum status_code PWM_SetAngle(PWM_ChannelID id, uint8_t angle) {
    uint8_t duty = AngleToDutyCycle(angle); // Converting the angle to duty cycle
    return PWM_SetDuty(id, duty); // Set the duty cycle
}


enum status_code PWM_Disable(PWM_ChannelID id)
{
	tc_set_compare_value(&pwm_timer, id, 0);	
	return STATUS_OK;
}


