/* main.c
 * Main function for the sailboat.
 * Initializes the system and then starts the freeRTOS tasks
 * Created on 
 * Created by
 */

#include "sail_ctrl.h"
#include "sail_debug.h"
#include "sail_tasksinit.h"
#include "sail_pwm.h"

int main(void)
{
	CTRL_InitSystem(); // Init -> DEBUG UART, RADIO, EERPROM
	#ifndef DEBUG
	CTRL_InitSensors(); // Initialize the WeatherStation
	#endif
	PWM_Init();
	startup(); //Enable WS - Init Motors - Get the first waypoint
	init_tasks();
}
