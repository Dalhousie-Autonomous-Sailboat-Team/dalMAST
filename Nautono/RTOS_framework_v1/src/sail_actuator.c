/*! \file sail_actuator.c
	\author Manav Sohi
	\brief Contains functions to get the %extension for linear actuator based on desired sail angle
*/

#include "sail_actuator.h"
#include "sail_debug.h"
#include "sail_tasksinit.h"
#include "sail_pwm.h"
#include "sail_adc.h"
#include <stdbool.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#define SAIL_ANGLE_MIN 25
#define SAIL_ANGLE_MAX 115
#define ACTUATOR_EXT_MIN 0
#define ACTUATOR_EXT_MAX 255

static void getExtnesion(float sail_angle, float * extension) {
	*extension = ((sail_angle - SAIL_ANGLE_MIN) * (ACTUATOR_EXT_MAX - ACTUATOR_EXT_MIN) / (SAIL_ANGLE_MAX - SAIL_ANGLE_MIN)) + ACTUATOR_EXT_MIN;
}

static void SetExtension(uint8_t extension) {
	PWM_SetDuty(PWM_SAIL, extension);
	return STATUS_OK;
}

enum status_code setActuator(float sail_angle) {
	
	float extension = 0;
	getExtnesion(sail_angle, &extension);
	/*uint8_t _extension = (uint8_t)extension;*/
	SetExtension((uint8_t)extension);
	
	return STATUS_OK;
}

/* <<For testing>>*/

// Backward
#define LIN_PWM1 PIN_PB16 
// Forward
#define LIN_PWM2 PIN_PB17

#define LAC_OFF_STATE false
#define LAC_ON_STATE true

#define TEST_ACTUATOR_DELAY_MS 1000



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

static void pot_pos(double *data) {
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
	double curr_pos = 0;
	pot_pos(&curr_pos);
	
	//void (*mov_func)(void);
	//
	//mov_func = *LAC_forward;
	//
	//if(curr_pos > pos) {
		//mov_func = *LAC_backward;
	//}
	
	DEBUG_Write("Setting LAC to pos: %d\r\n", (int)pos);
	
	while(curr_pos <= pos*0.98 || curr_pos >= pos*1.02) {
		
		if(curr_pos > pos) {
			LAC_backward();
		} else {
			LAC_forward();
		}
		//TurnOff();
		pot_pos(&curr_pos);
		delay_ms(10);
		pot_pos(&curr_pos);
		//DEBUG_Write("curr pos: %d\r\n", (int)curr_pos);
	}
	TurnOff();
	//pot_pos(&curr_pos);
	
	DEBUG_Write("Reached pos: %d\r\n", (int)curr_pos);
	DEBUG_Write("<<< Done >>>\r\n");
}

void Test_Actuator(void){

	TickType_t testDelay = pdMS_TO_TICKS(TEST_ACTUATOR_DELAY_MS);
	//PWM_Init();
	
	AC_init();
	
	double curr_pos = 0;

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Actuator >>>>>>>>>>\n\r");
		
		
		LAC_set_pos(100);
		pot_pos(&curr_pos);
		DEBUG_Write("curr pos: %d\r\n", (int)curr_pos);
	
		vTaskDelay(testDelay);
	}
}
