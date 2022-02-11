
#include "sail_ctrl.h"
#include "sail_debug.h"

int main(void)
{
	CTRL_InitSystem();
	DEBUG_Write("Initializing sensors...\r\n");
	CTRL_InitSensors();
	DEBUG_Write("Entering main loop...\r\n");
	CTRL_RunLoop();
}

