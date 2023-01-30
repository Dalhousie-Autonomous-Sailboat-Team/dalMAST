/* sail_adc.c
 * Implementation of the ADC driver module for the autonomous sailboat project.
 * Created on August 11, 2016.
 * Created by Thoms Gwynne-Timothy & Julia Sarty.
 */

#include "sail_adc.h"

#include <asf.h>
#include <stdbool.h>
#include <inttypes.h>

#include "sail_math.h"
#include "sail_debug.h"

// ADC instance
static struct adc_module adc;

// ADC pin mappings
static const enum adc_positive_input input_pins[ADC_NUM_CHANNELS] = {
	ADC_POSITIVE_INPUT_PIN0,
	ADC_POSITIVE_INPUT_PIN1
};

// Flag to indicate the module has been initialized
static bool init_flag = false;

enum status_code ADC_Init(void) {
	struct adc_config config_adc;

	adc_get_config_defaults(&config_adc);

	config_adc.gain_factor                = ADC_GAIN_FACTOR_DIV2;
	config_adc.reference                  = ADC_REFERENCE_INTVCC1;
	config_adc.resolution                 = ADC_RESOLUTION_12BIT;
	config_adc.positive_input             = ADC_POSITIVE_INPUT_PIN0;

    if (adc_init(&adc, ADC, &config_adc) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

    adc_enable(&adc);

	init_flag = true;
	
	return STATUS_OK;
}

enum status_code ADC_GetReading(ADC_ChannelID id, double *reading)
{
	// Return if the module hasn't been initialized yet
	if (!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	// Return if the channel id is invalid
	if (id >= ADC_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}

	// Return if the pointer is NULL
	if (reading == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Set the input pin
	adc_set_positive_input(&adc, input_pins[id]);
	
	// Start the ADC reading/conversion
    adc_start_conversion(&adc);
    uint16_t result;
	
	// Let the conversion complete
    do {
	    // Wait for conversion to be done and read out result
    } while (adc_read(&adc, &result) == STATUS_BUSY);
	
	// Return the output
	*reading = MATH_Map((double)result, 0.0, 4095.0, 0.0, 3.3);
	
	return STATUS_OK;
}

	