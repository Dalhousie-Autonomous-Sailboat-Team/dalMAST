/* main.c
 * Test bench for PWM driver
 * Created on _______.
 * Created by _______.
 */

#include <asf.h>

#define SAIL_ON_OFF_PIN			PIN_PB15
#define SAIL_LEFT_RIGHT_PIN		PIN_PB07
#define SAIL_ON_STATE			true
#define SAIL_LEFT_STATE			true

static void InitPin(void);
static void TurnOn(void);
static void TurnOff(void);
static void TurnLeft(void);
static void TurnRight(void);

#include "sail_pwm.h"
#include "sail_debug.h"

int main (void) {
	/**************** SETUP ****************/
	system_init();
	
	DEBUG_Init();
	
	DEBUG_Write("Testing PWM driver...\r\n");

	InitPin();
		
	TurnOn();

	PWM_Init();
	
	uint8_t sail_duty, rudder_duty;

	sail_duty = 10;
	PWM_SetDuty(PWM_SAIL, sail_duty);
	
	/**************** LOOP *****************/
	while (true) {
		//TurnRight();
		delay_ms(500);
		//TurnLeft();
		delay_ms(500);
	}
}


static void InitPin(void) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	port_pin_set_config(SAIL_ON_OFF_PIN, &config_port_pin);
	//port_pin_set_config(SAIL_LEFT_RIGHT_PIN, &config_port_pin);
}


static void TurnOn(void) {
	port_pin_set_output_level(SAIL_ON_OFF_PIN, SAIL_ON_STATE);
}


static void TurnOff(void) {
	port_pin_set_output_level(SAIL_ON_OFF_PIN, !SAIL_ON_STATE);
}

static void TurnLeft(void) {
	port_pin_set_output_level(SAIL_LEFT_RIGHT_PIN, SAIL_LEFT_STATE);
}


static void TurnRight(void) {
	port_pin_set_output_level(SAIL_LEFT_RIGHT_PIN, !SAIL_LEFT_STATE);
}