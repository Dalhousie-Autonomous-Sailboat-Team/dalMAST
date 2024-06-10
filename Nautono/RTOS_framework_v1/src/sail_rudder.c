/* sail_motor.c
 * Implementation of the motor controller for the autonomous sailboat project.
 * Created on Thomas Gwynne-Timothy.
 * Created by August 16, 2016.
 */

#include "sail_rudder.h"

#include <stdbool.h>

#include "sail_adc.h"
#include "sail_debug.h"

#include "sail_tasksinit.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

//#define MOTOR_POWER_PIN			PIN_PA03
#define MOTOR_CCW_POSITIVE_PIN	PIN_PB08
#define MOTOR_CW_POSITIVE_PIN	PIN_PB09
//#define MOTOR_ON_STATE			true

//#define MOTOR_SAIL_DIR_PIN		PIN_PB06
//#define MOTOR_SAIL_CW_STATE		true

//#define MOTOR_RUDDER_DIR_PIN	PIN_PB04
//#define MOTOR_RUDDER_CW_STATE	true

typedef enum MOTOR_Directions {
	MOTOR_CW,
	MOTOR_CCW
} MOTOR_Direction;

bool rudder_initialzed = false; 

extern uint16_t rudder_deg;

// Function to initialize the power and direction pins
static void InitPins(void);
// Function to turn on the specified motor
static void TurnOn(void);
// Function to turn off the specified motor
static void TurnOff(void);
// Function to turn set the specified motor clockwise
static void SetDirection(MOTOR_Direction dir);
// Function to get the pot position for rudder.
void RudderPotPos(double * data);

// Function to initialize the power and direction pins for the specified motor
static void InitPins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	// Set the power pin
	port_pin_set_config(MOTOR_CCW_POSITIVE_PIN, &config_port_pin);

	// Set the rudder direction pin
	port_pin_set_config(MOTOR_CW_POSITIVE_PIN, &config_port_pin);
}

enum status_code RUDDER_Init(void)
{
	// TODO Error checking here

	// Initialize the control pins
	InitPins();

	// Initialize the ADC
	ADC_Init(ADC_RUDDER);
	
	rudder_initialzed = true;
	
	return STATUS_OK; 
}

// Function to turn on the specified motor
static void TurnOn(void)
{
	// Set digital output high.
	//port_pin_set_output_level(MOTOR_POWER_PIN, MOTOR_ON_STATE);
}

//	Function to turn clockwise
static void TurnCW(void){
	port_pin_set_output_level(MOTOR_CW_POSITIVE_PIN, true);
	port_pin_set_output_level(MOTOR_CCW_POSITIVE_PIN, false);
	DEBUG_Write("Turning CW.\r\n");
}

//	Function to turn clockwise
static void TurnCCW(void){
	port_pin_set_output_level(MOTOR_CW_POSITIVE_PIN, false);
	port_pin_set_output_level(MOTOR_CCW_POSITIVE_PIN, true);
	DEBUG_Write("Turning CCW.\r\n");
}

// Function to turn off the specified motor
static void TurnOff(void)
{
	// Set the digital output low.
	port_pin_set_output_level(MOTOR_CW_POSITIVE_PIN, false);
	port_pin_set_output_level(MOTOR_CCW_POSITIVE_PIN, false);
}


// Function to turn set the direction of the specified motor
static void SetDirection(MOTOR_Direction dir)
{
	//port_pin_set_output_level(MOTOR_RUDDER_DIR_PIN, (dir == MOTOR_CW ? !MOTOR_RUDDER_CW_STATE : MOTOR_RUDDER_CW_STATE));
}

void RudderPotPos(double * data) {
	ADC_GetReading(ADC_RUDDER, data);
}

void RudderSetPos(double pos) 
{
	
	if(rudder_initialzed == false) {
		RUDDER_Init();
	}
	
	double curr_pos = 0, prev_pose = 0;
	int count = 0;
	RudderPotPos(&curr_pos);
	
	MOTOR_Direction dir = MOTOR_CW;
	
	if(curr_pos > pos) 
	{
		dir = MOTOR_CCW;
	}
	
	//SetDirection(dir);
	
	DEBUG_Write("Setting Rudder to POS: %d\r\n", (uint)pos);
	
	while((curr_pos <= pos*0.95 || curr_pos >= pos*1.05)) 
	{
		//TurnOn();
		if(dir == MOTOR_CCW)
		{
			TurnCCW();
		}
		else
		{
			TurnCW();
		}
		RudderPotPos(&curr_pos);
		//DEBUG_Write("RUDDER POS: %d\r\n", (uint)curr_pos);
	}
	
	TurnOff();
	
	RudderPotPos(&curr_pos);
	rudder_deg = (uint16_t)curr_pos;
	
	DEBUG_Write("Final Rudder POS: %d\r\n", (int)curr_pos);

}

#define TEST_RUDDER_DELAY_MS 1000

void Test_Rudder(void){ 

	TickType_t testDelay = pdMS_TO_TICKS(TEST_RUDDER_DELAY_MS);
	
	RUDDER_Init();
	
	double pos = 0;
	int int_pos = 0;

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		DEBUG_Write("\n\r<<<<<<<<<<< Testing POT >>>>>>>>>>\n\r");
		
		RudderPotPos(&pos);
		
		RudderSetPos(105);
		RudderSetPos(30);
		
		
		int_pos = pos;
		DEBUG_Write("POT reading: %d\r\n", int_pos);
		
		//RudderSetPos(250);
		
		vTaskDelay(testDelay);
	}
}