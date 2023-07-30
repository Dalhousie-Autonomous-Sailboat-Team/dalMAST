/*! \file sail_actuator.c
	\author Manav Sohi
	\brief Contains functions to get the %extension for linear actuator based on desired sail angle
*/

#include "sail_actuator.h"
#include "sail_debug.h"
#include "sail_tasksinit.h"
#include "sail_pwm.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#define TEST_ACTUATOR_DELAY_MS 1000

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
	uint8_t _extension = (uint8_t)extension;
	SetExtension(_extension);
	
	return STATUS_OK;
}

/* <<For testing>>*/

void Test_Actuator(void){

	TickType_t testDelay = pdMS_TO_TICKS(TEST_ACTUATOR_DELAY_MS);
	
	uint8_t angle = 29;
	uint8_t index = 0;
	uint16_t extension = 0;

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Actuator >>>>>>>>>>\n\r");
		
		
		
		vTaskDelay(testDelay);
	}
}
