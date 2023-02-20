/* sail_motor.c
 * Implementation of the motor controller for the autonomous sailboat project.
 * Created on Thomas Gwynne-Timothy.
 * Created by August 16, 2016.
 */

#include "sail_motor.h"

#include <math.h>
#include <stdbool.h>

#include "sail_adc.h"
#include "sail_pwm.h"
#include "sail_debug.h"
#include "sail_math.h"

#define MOTOR_POWER_PIN			PIN_PB15
#define MOTOR_ON_STATE			true

#define MOTOR_SAIL_DIR_PIN		PIN_PB06
#define MOTOR_SAIL_CW_STATE		true

#define MOTOR_RUDDER_DIR_PIN	PIN_PB07
#define MOTOR_RUDDER_CW_STATE	true

typedef enum MOTOR_Directions {
	MOTOR_CW,
	MOTOR_CCW
} MOTOR_Direction;

// Define the angle range for each motor
static const double winch_min_deg       =   0.0;
static const double winch_max_deg       = 360.0;
static const double winch_neutral_deg   = 100.0;
static const double winch_threshold_deg =   2.0;

static const double shaft_min_deg       =   0.0;
static const double shaft_max_deg       =  90.0;
static const double shaft_neutral_deg   =  45.0;
static const double shaft_threshold_deg =   1.0;

static const double shaft_motor_min_speed = 100.0;
static const double shaft_motor_max_speed = PWM_MAX_DUTY;
static const double winch_motor_min_speed = 150.0;
static const double winch_motor_max_speed = PWM_MAX_DUTY;
static const double error_min_deg       =   5.0;
static const double error_max_deg       =  20.0;

static uint16_t winch_threshold_count   =   0;
static uint16_t shaft_threshold_count   =   0;
static const uint16_t required_count    =   2;

static bool sail_on   = false;
static bool rudder_on = false;

// Define target angles for the sail winch and rudder shaft
static double target_winch_deg;
static double target_shaft_deg;

static double last_winch_deg;
static double last_shaft_deg;

// Define a timer to control the feedback loop
static struct tc_module timer;
static Tc *const timer_hw = TC0;

// Utility function to initialize and start timer
static void InitTimer(void);

// Callback to control sail/winch angle
static void WinchControlCallback(struct tc_module *const module_inst);
static void ShaftControlCallback(struct tc_module *const module_inst);

// Function to map sail boom angle to winch angle 
static double SailMap(double sail_deg);
// Function to map winch pot. voltage to winch angle
static double WinchMap(double winch_volt);

// Function to map rudder angle to shaft angle
static double RudderMap(double rudder_deg);
// Function to map rudder angle (relative to boat) to shaft angle
static double ShaftMap(double shaft_volt);

// Function to compute motor speed from error
static uint8_t SpeedControl(double error_deg);

// Function to initialize the power and direction pins
static void InitPins(void);
// Function to turn on the specified motor
static void TurnOn(MOTOR_ChannelID id);
// Function to turn off the specified motor
static void TurnOff(MOTOR_ChannelID id);
// Function to turn set the specified motor clockwise
static void SetDirection(MOTOR_ChannelID id, MOTOR_Direction dir);

enum status_code MOTOR_Init(void)
{
	// TODO Error checking here

	// Initialize the control pins
	InitPins();

	// Initialize the ADC
	ADC_Init();

	// Initialize the PWM
	PWM_Init();

	// Initialize the target angles
	target_winch_deg = winch_neutral_deg;
	target_shaft_deg = shaft_neutral_deg;

	// Set the last angles to something bogus - far away from any possible angles
	last_winch_deg = winch_min_deg - 100.0;
	last_shaft_deg = shaft_min_deg - 100.0;
	
	// Turn on the timer
	InitTimer();
	
	return STATUS_OK; 
}


enum status_code MOTOR_SetSail(double sail_deg)
{
	target_winch_deg = MATH_Clamp(SailMap(sail_deg), winch_min_deg, winch_max_deg);

	// Return if the angle hasn't changed significantly
	if (fabs(target_winch_deg - last_winch_deg) < winch_threshold_deg) {
		last_winch_deg = target_winch_deg;
		return STATUS_NO_CHANGE;
	}

	// Enable the motor if it's currently off
	if (!sail_on) {
		// Enable callback
		tc_enable_callback(&timer, TC_CALLBACK_CC_CHANNEL0);
		// Turn on the motor
		TurnOn(MOTOR_SAIL);
	}

	last_winch_deg = target_winch_deg;
	
	return STATUS_OK;
}


enum status_code MOTOR_SetRudder(double rudder_deg)
{
	target_shaft_deg = MATH_Clamp(RudderMap(rudder_deg), shaft_min_deg, shaft_max_deg);

	// Return if the angle hasn't changed significantly
	if (fabs(target_shaft_deg - last_shaft_deg) < shaft_threshold_deg) {
		last_shaft_deg = target_shaft_deg;
		return STATUS_NO_CHANGE;
	}

	// Enable the motor if it's currently off
	if (!rudder_on) {
		// Enable callback
		tc_enable_callback(&timer, TC_CALLBACK_CC_CHANNEL1);
		// Turn on the motor
		TurnOn(MOTOR_RUDDER);
	}

	last_shaft_deg = target_shaft_deg;
	
	return STATUS_OK;
}


static void InitTimer(void)
{
	// Get the default configuration
	struct tc_config timer_config;
	tc_get_config_defaults(&timer_config);

	// Set the counter size
	timer_config.clock_source         = GCLK_GENERATOR_4;
	timer_config.counter_size         = TC_COUNTER_SIZE_8BIT;
	timer_config.clock_prescaler      = TC_CLOCK_PRESCALER_DIV1;
	timer_config.counter_8_bit.period = 10;
	timer_config.counter_8_bit.compare_capture_channel[0] = 2;
	timer_config.counter_8_bit.compare_capture_channel[1] = 7;

	tc_init(&timer, timer_hw, &timer_config);

	tc_enable(&timer);

	tc_register_callback(&timer, WinchControlCallback, TC_CALLBACK_CC_CHANNEL0);
	tc_register_callback(&timer, ShaftControlCallback, TC_CALLBACK_CC_CHANNEL1);
}


static void WinchControlCallback(struct tc_module *const module_inst)
{
	double winch_volt;
	
	// Read the voltage at each potentiometer
	ADC_GetReading(ADC_SAIL, &winch_volt);
	
	// Get the corresponding angles
	double winch_deg = WinchMap(winch_volt);

	// Compare the angles
	double winch_error_deg = target_winch_deg - winch_deg;

	// Stop if the error is less than the threshold
	if (fabs(winch_error_deg) < winch_threshold_deg) {
		// Increment the counter
		winch_threshold_count++;
		// Stop if the counter limit has been reached
		if (winch_threshold_count >= required_count) {
			// Turn off PWM
			PWM_Disable(PWM_SAIL);
			// Turn off the motor
			TurnOff(MOTOR_SAIL);
			// Disable the callback
			tc_disable_callback(&timer, TC_CALLBACK_CC_CHANNEL0);
			return;
		}
	} else {
		// Reset the counter
		winch_threshold_count = 0;
	}
	
	// Map the error to a speed
	double speed = MATH_Map(fabs(winch_error_deg), error_min_deg, error_max_deg, winch_motor_min_speed, winch_motor_max_speed);	
	speed = MATH_Clamp(speed, winch_motor_min_speed, winch_motor_max_speed);
	
	// Update the motor
	PWM_SetDuty(PWM_SAIL, speed);
	SetDirection(MOTOR_SAIL, (winch_error_deg >= 0.0 ? MOTOR_CW : MOTOR_CCW));
}


static void ShaftControlCallback(struct tc_module *const module_inst)
{
	double shaft_volt;
	
	// Read the voltage at each potentiometer
	if (ADC_GetReading(ADC_RUDDER, &shaft_volt) != STATUS_OK) {
		DEBUG_Write("Error\r\n");
	}
	
	// Get the corresponding angles
	double shaft_deg = ShaftMap(shaft_volt);

	// Compare the angles
	double shaft_error_deg = target_shaft_deg - shaft_deg;

	// Stop if the error is less than the threshold
	if (fabs(shaft_error_deg) < shaft_threshold_deg) {
		// Increment the counter
		shaft_threshold_count++;
		if (shaft_threshold_count >= required_count) {
			// Turn off PWM
			PWM_Disable(PWM_RUDDER);
			// Turn off the motor
			TurnOff(MOTOR_RUDDER);
			// Disable the callback
			tc_disable_callback(&timer, TC_CALLBACK_CC_CHANNEL1);
			return;
		}
	} else {
		// Reset the counter
		shaft_threshold_count = 0;
	}
	
	// Map the error to a speed
	double speed = MATH_Map(fabs(shaft_error_deg), error_min_deg, error_max_deg, shaft_motor_min_speed, shaft_motor_max_speed);
	speed = MATH_Clamp(speed, shaft_motor_min_speed, shaft_motor_max_speed);	
	
	// Update the motor
	PWM_SetDuty(PWM_RUDDER, speed);
	SetDirection(MOTOR_RUDDER, (shaft_error_deg >= 0.0 ? MOTOR_CCW : MOTOR_CW));
}


// Function to map sail boom angle to winch angle
static double SailMap(double sail_deg)
{
	// Ensure input is positive
	sail_deg = fabs(sail_deg);
	
	return MATH_Map(sail_deg, 10.0, 65.0, 30.0, 360.0);
}

// Function to map winch pot. voltage to winch angle
static double WinchMap(double winch_volt)
{
	return MATH_Map(winch_volt, 2.42, 1.30, winch_min_deg, winch_max_deg);
}

// Function to map rudder angle to shaft angle
static double RudderMap(double rudder_deg)
{
	return rudder_deg + 45.0;
}

// Function to map shaft pot. voltage to shaft angle
static double ShaftMap(double shaft_volt)
{
	return MATH_Map(shaft_volt, 1.10, 2.25, shaft_min_deg, shaft_max_deg);
}

// Function to compute motor speed from error
static uint8_t SpeedControl(double error_deg)
{
	// Map the error to a speed
	double speed = MATH_Map(fabs(error_deg), error_min_deg, error_max_deg, shaft_motor_min_speed, shaft_motor_max_speed);

	// Clamp the motor speed 
	return MATH_Clamp(speed, shaft_motor_min_speed, shaft_motor_max_speed);
}

// Function to initialize the power and direction pins for the specified motor
static void InitPins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	// Set the power pin (shared)
	port_pin_set_config(MOTOR_POWER_PIN, &config_port_pin);

	// Set the sail direction pin
	port_pin_set_config(MOTOR_SAIL_DIR_PIN, &config_port_pin);

	// Set the rudder direction pin
	port_pin_set_config(MOTOR_RUDDER_DIR_PIN, &config_port_pin);
}



// Function to turn on the specified motor
static void TurnOn(MOTOR_ChannelID id)
{
	// Set the flag
	switch(id) {
		case MOTOR_SAIL: 
			sail_on = true;
			break;
		case MOTOR_RUDDER:
			rudder_on = true;
			break;
		default:
			return;
	}

	// Set the pin
	port_pin_set_output_level(MOTOR_POWER_PIN, MOTOR_ON_STATE);
}



// Function to turn off the specified motor
static void TurnOff(MOTOR_ChannelID id)
{
	// Clear the flag
	switch(id) {
		case MOTOR_SAIL:
			sail_on = false;
			break;
		case MOTOR_RUDDER:
			rudder_on = false;
			break;
		default:
			return;
	}

	// Set the pin if both motors should be off
	if (!sail_on && !rudder_on) {
		port_pin_set_output_level(MOTOR_POWER_PIN, !MOTOR_ON_STATE);
	}
}



// Function to turn set the direction of the specified motor
static void SetDirection(MOTOR_ChannelID id, MOTOR_Direction dir)
{
	switch(id) {
		case MOTOR_SAIL:
			port_pin_set_output_level(MOTOR_SAIL_DIR_PIN, (dir == MOTOR_CW ? MOTOR_SAIL_CW_STATE : !MOTOR_SAIL_CW_STATE));
			//DEBUG_Write("%s\r\n", (dir == MOTOR_CW ? "CW" : "CCW"));
			break;
		case MOTOR_RUDDER:
			port_pin_set_output_level(MOTOR_RUDDER_DIR_PIN, (dir == MOTOR_CW ? MOTOR_RUDDER_CW_STATE : !MOTOR_RUDDER_CW_STATE));
			break;
		default:
			break;
	}
}
