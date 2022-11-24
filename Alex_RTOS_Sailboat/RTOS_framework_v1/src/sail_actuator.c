/* sail_actuator.c
 * Implementation of the actuator controller for the autonomous sailboat project.
 * Created by Manav Sohi.
 * Created by Oct 28, 2022.
 */

#include <asf.h>

#include "sail_pwm.h"
#include "sail_actuator.h"
#include "sail_debug.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "sail_tasksinit.h"

// Pins to control the linear actuator
// #define PWM_ACTUATOR_OUT_PIN	PIN_PB09F_TC4_WO1
// #define PWM_ACTUATOR_OUT_MUX	MUX_PB09F_TC4_WO1

#define TEST_ACTUATOR_DELAY_MS 1000

// static void LA_init_pins(void);
// enum status_code LA_init(void);

void Test_Actuator(void){
	// Delay Setup
	TickType_t testDelay = pdMS_TO_TICKS(TEST_ACTUATOR_DELAY_MS);
	// Task Loop
	
	uint8_t actuator_duty[9] = {0, 25, 50, 75, 100, 75, 50, 25, 0};
	int idx = 0;
	
	PWM_Init(void);
	
	while(1){
	
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
	
		running_task = eUpdateCourse; // don't worry about this for now (for watchdog)
		
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Actuator >>>>>>>>>>\n\r");
		
		// Start Actuator test here
		
		// The duty of the PWM signal sets the % extension of actuator.
		idx = idx >= 9 ? 0 : idx;
		DEBUG_Write("\n\r idx: %d && Duty: %d%% \n\r", idx, actuator_duty[idx]);
		PWM_SetDuty(PWM_SAIL, actuator_duty[idx]); 
		idx++;
		
		
		// End of task loop
		vTaskDelay(testDelay);	
	}
}

/*
enum status_code LA_init(void) {
	
	// Initialize the control pins
	LA_init_pins();
	
	return STATUS_OK;		
}

static void LA_pos(int pos_percent) {
	// pos range from 0 to 100% for duty cycle 
	
	
}

static void LA_init_pins(void) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
		
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	// set the pwm pin for Linear actuator
	port_pin_set_config(LA_PWM_PIN, &config_port_pin);
}
*/