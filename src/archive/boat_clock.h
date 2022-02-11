#ifndef BOAT_CLOCK_H
#define BOAT_CLOCK_H

void SysTick_Handler(void);
void init_boatclock(void);
int get_boatclock(void);
int get_boatdeltaclock(int startclock);

#endif