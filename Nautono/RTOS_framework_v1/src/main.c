/* main.c
 * Main function for the sailboat.
 * Initializes the system and then starts the freeRTOS tasks
 * 
 * 
 */


#include "sail_ctrl.h"
#include "sail_debug.h"
#include "sail_tasksinit.h"

int main(void)
{
	CTRL_InitSystem(); // Init -> DEBUG UART, RADIO, EERPROM
<<<<<<< HEAD:Alex_RTOS_Sailboat/RTOS_framework_v1/src/main.c
	#if 0
	CTRL_InitSensors(); // Initialize the WeatherStation
	startup(); //Enable WS - Init Motors - Get the first waypoint
	#endif
=======
	//CTRL_InitSensors(); // Initialize the WeatherStation
	startup();
>>>>>>> PCB_Test:Nautono/RTOS_framework_v1/src/main.c
	init_tasks();
}
