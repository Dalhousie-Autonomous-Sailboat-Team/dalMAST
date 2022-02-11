
#include <asf.h>
#include <math.h>

#include "sail_debug.h"
#include "sail_motor.h"

#define NUM_ANGLES 9
const double rudder_list[NUM_ANGLES] = {
	-45.0,
	  0.0,
	+45.0,
	-45.0,
	  0.0,
	+45.0,
	-45.0,
	  0.0,
	+45.0
};

const double sail_list[NUM_ANGLES] = {
	10.0,
	20.0,
	30.0,
	40.0,
	45.0,
	50.0,
	55.0,
	60.0,
	65.0
};


int main (void)
{
	system_init();

	DEBUG_Init();

	DEBUG_Write("Initializing the motors...\r\n");

	MOTOR_Init();

	double angle;

	for (int i = 0; i < NUM_ANGLES; i = (i + 1) % NUM_ANGLES) {
		DEBUG_Write("Setting rudder to %d deg\r\n", (int)rudder_list[i]);
		MOTOR_SetRudder(rudder_list[i]);
		DEBUG_Write("Setting boom to %d deg\r\n", (int)sail_list[i]);
		MOTOR_SetSail(sail_list[i]);
		delay_ms(10000);
	}
}
