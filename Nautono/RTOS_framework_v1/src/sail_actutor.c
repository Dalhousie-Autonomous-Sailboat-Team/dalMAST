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

#define TEST_ACTUATOR_DELAY_MS 10000

/*! \var uint16_t sail_angle_min
	\brief The angle of the sail when linear actuator is at 0 extension. Measured from the x-axis.
*/
static const uint16_t sail_angle_min = 25;
static const uint16_t sail_angle_max = 115;
static const uint8_t angle_increment = 5;

/*! \var uint32_t ActuatorPresets
	\brief Preset extension values for the linear actuator, measured by the Mechanical team.
*/
static const uint32_t ActuatorPresets[18] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 0, 0};

/*! \fn uint8_t GetExtensionIndex(uint16_t angle)
	\brief Gets the index of actuator presets based on \a angle. 
	\param angle the desired angle for the small sail.
*/
extern uint8_t GetExtensionIndex(uint16_t angle) {
	return (angle-sail_angle_min)/angle_increment;	
}
/*! \fn uint16_t GetActuatorExtension(uint8_t index)
	\brief returns the value stored at \a index in ActuatorPresets
	\param index The index of values in ActuatorPresets.
*/
extern uint16_t GetActuatorExtension(uint8_t index) {
	return ActuatorPresets[index];
}

/* <<For testing>>*/

void Test_Actuator(void){

	TickType_t testDelay = pdMS_TO_TICKS(TEST_ACTUATOR_DELAY_MS);
	
	uint8_t angle = 29;
	uint8_t index = 0;
	uint16_t extension = 0;

	while(angle < sail_angle_max){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Actuator >>>>>>>>>>\n\r");
		
		
		index = GetExtensionIndex(angle);
		extension = GetActuatorExtension(index);
		
		DEBUG_Write("\n\r angle: %d && Duty: %d%% \n\r", angle, extension);
		
		PWM_SetDuty(PWM_SAIL, extension);
		
		angle += 8;
		
		vTaskDelay(testDelay);
	}
}
