#include <asf.h>
#include "boat_clock.h"

int boat_counter;

#define MAX_COUNTER 2048

void SysTick_Handler(void)
{
	boat_counter++;
	if(boat_counter == MAX_COUNTER)
		boat_counter = 0;
}

void init_boatclock(void){

	/*Configure system tick to generate 10 msec interrupts*/
	SysTick_Config(system_gclk_gen_get_hz(GCLK_GENERATOR_0)/100);
	
	boat_counter = 0;
}

int get_boatclock(void)
{
	return boat_counter;
}

int get_boatdeltaclock(int startclock)
{
	int delta = boat_counter - startclock;
	if (delta < 0)
		delta = MAX_COUNTER - startclock + boat_counter;
	return delta;
	
}