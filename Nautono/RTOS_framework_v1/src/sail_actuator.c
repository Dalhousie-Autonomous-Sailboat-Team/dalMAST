/*! \file sail_actuator.c
	\author Manav Sohi
	\brief Contains functions to get the %extension for linear actuator based on desired sail angle
*/
#include <stdbool.h>
#include <asf.h>

#include "sail_debug.h"
#include "sail_adc.h"
#include "sail_actuator.h"

#include "sail_tasksinit.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

// Actuator retraction pin
#define LIN_PWM1 PIN_PB16 
// Actuator extension pin
#define LIN_PWM2 PIN_PB17

#define LAC_OFF_STATE false
#define LAC_ON_STATE true

static void init_pins(void) {
	
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	// Set the Forward direction pin
	port_pin_set_config(LIN_PWM1, &config_port_pin);

	// Set the backward direction pin
	port_pin_set_config(LIN_PWM2, &config_port_pin);
}

static void TurnOff(void) {
	port_pin_set_output_level(LIN_PWM1, LAC_OFF_STATE);
	port_pin_set_output_level(LIN_PWM2, LAC_OFF_STATE);
}

static void ActuatorPotPos(double *data) {
	ADC_GetReading(ADC_SAIL, data);
}

static void LAC_forward(void) {
	port_pin_set_output_level(LIN_PWM1, LAC_OFF_STATE);
	port_pin_set_output_level(LIN_PWM2, LAC_ON_STATE);
}

static void LAC_backward(void) {
	port_pin_set_output_level(LIN_PWM1, LAC_ON_STATE);
	port_pin_set_output_level(LIN_PWM2, LAC_OFF_STATE);
}

void AC_init(void) {
	
	init_pins();
	
	ADC_Init(ADC_SAIL);
	
	TurnOff();
}

void LAC_set_pos(double pos) 
{
	double curr_pos = 0, prev_pos = 0;
	int count = 0;
	ActuatorPotPos(&curr_pos);
	
	while((curr_pos <= pos*0.98 || curr_pos >= pos*1.02) ) {
		
		if(curr_pos > pos) {
			LAC_backward();
		} else {
			LAC_forward();
		}
		ActuatorPotPos(&curr_pos);
		delay_ms(10);
		ActuatorPotPos(&curr_pos);
	}
	TurnOff();
}

#define TEST_ACTUATOR_DELAY_MS 1000

void Test_Actuator(void){

	TickType_t testDelay = pdMS_TO_TICKS(TEST_ACTUATOR_DELAY_MS);
	
	AC_init();
	
	double curr_pos = 0;

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Actuator >>>>>>>>>>\n\r");
		
		//LAC_set_pos(100);
		ActuatorPotPos(&curr_pos);
		DEBUG_Write("current pos: %d\r\n", (int)curr_pos);
	
		vTaskDelay(testDelay);
	}
}
