/*!
@breif
This is the source file for the implementation of Nantono's linear actuator control
@author

*/

#include <asf.h>

#include "sail_actuator.h"
#include "sail_debug.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "sail_tasksinit.h"


#define TEST_ACTUATOR_DELAY_MS 1000

void Test_Actuator(void){
	// Delay Setup
	TickType_t testDelay = pdMS_TO_TICKS(TEST_ACTUATOR_DELAY_MS);
	// Task Loop
	while(1){
	
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
	
		running_task = eUpdateCourse; // don't worry about this for now (for watchdog)
		
		DEBUG_Write("\n\r<<<<<<<<<<< Testing Actuator >>>>>>>>>>\n\r");
		
		// Start Actuator test here
		
		// End of task loop
		vTaskDelay(testDelay);
		
	}
	
	
}