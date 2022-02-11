
#include <asf.h>

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "sail_debug.h"

#define TC_COUNT 8

void configure_tc(void);
void configure_tc_callbacks(void);

void tc_callback_0(struct tc_module *const module_inst);
void tc_callback_1(struct tc_module *const module_inst);
void tc_callback_2(struct tc_module *const module_inst);
void tc_callback_3(struct tc_module *const module_inst);
void tc_callback_4(struct tc_module *const module_inst);
void tc_callback_5(struct tc_module *const module_inst);
void tc_callback_6(struct tc_module *const module_inst);
void tc_callback_7(struct tc_module *const module_inst);

// Array of interrupts periods, in milliseconds (these can be set, according to the restrictions in the comments below)
static uint16_t interrupt_periods_ms[] = {
	200,	// Clock resolution		(must be between 1		and 250)
	5000,	// Data log period		(must be between 1,000	and 250,000)
	5000,	// GPS read period		(must be between 1,000	and 250,000)
	10000,	// Wind read period		(must be between 1,000	and 250,000)
	200,	// Compass read period	(must be between 100	and 25,000)
	200,	// Rudder set period	(must be between 100	and 25,000)
	10000,	// Sail set period		(must be between 1,000	and 250,000)
	2000	// Clock resolution		(must be between 1,000	and 250,000)
};



// Clock assignments for each timer (these are based on the range of periods for each type of interrupt)
static enum gclk_generator timer_clocks[] = {
	GCLK_GENERATOR_2,
	GCLK_GENERATOR_1,
	GCLK_GENERATOR_1,
	GCLK_GENERATOR_1,
	GCLK_GENERATOR_3,
	GCLK_GENERATOR_3,
	GCLK_GENERATOR_1,
	GCLK_GENERATOR_1
};

// Clock prescaler assignments for each timer (these are based on the range of periods for each type of interrupt)
static enum tc_clock_prescaler timer_prescalers[] = {
	TC_CLOCK_PRESCALER_DIV1,
	TC_CLOCK_PRESCALER_DIV256,
	TC_CLOCK_PRESCALER_DIV256,
	TC_CLOCK_PRESCALER_DIV256,
	TC_CLOCK_PRESCALER_DIV16,
	TC_CLOCK_PRESCALER_DIV16,
	TC_CLOCK_PRESCALER_DIV256,
	TC_CLOCK_PRESCALER_DIV256
};

// Clock periods for each timer (these are a function of the timer_clock and timer_prescaler, which come in pairs)
static uint16_t clock_periods_ms[] = {
	1,		// Clock ticks every 1ms
	1000,	// Clocks tick every 1000ms
	1000,
	1000,
	100,	// Clocks tick every 100ms
	100,
	1000,
	1000
};

tc_callback_t tc_callbacks[8] = {
	tc_callback_0,
	tc_callback_1,
	tc_callback_2,
	tc_callback_3,
	tc_callback_4,
	tc_callback_5,
	tc_callback_6,
	tc_callback_7			
};

Tc *const hw_instances[8] = {
	TC0,
	TC1,
	TC2,
	TC3,
	TC4,
	TC5,
	TC6,
	TC7
};

uint32_t clocks_ms[8] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

struct tc_module tc_instances[8];

int main(void) {
	system_init();
	
	// Initialize the debug UART
	DEBUG_Init();
	
	
	// Configure timer
	configure_tc();
	configure_tc_callbacks();	
	
	// Enable global interrupts
	system_interrupt_enable_global();	
	
	// Enter a sleep mode
	system_set_sleepmode(SYSTEM_SLEEPMODE_IDLE_2);
	system_sleep();
}


void tc_callback_0(struct tc_module *const module_inst)
{
	clocks_ms[0] += interrupt_periods_ms[0];
	DEBUG_Write("t[0]=%"PRIu32"ms\r\n", clocks_ms[0]);
}

void tc_callback_1(struct tc_module *const module_inst)
{
	clocks_ms[1] += interrupt_periods_ms[1];
	DEBUG_Write("t[1]=%"PRIu32" ms\r\n", clocks_ms[1]);
}

void tc_callback_2(struct tc_module *const module_inst)
{
	clocks_ms[2] += interrupt_periods_ms[2];
	DEBUG_Write("t[2]=%"PRIu32" ms\r\n", clocks_ms[2]);
}

void tc_callback_3(struct tc_module *const module_inst)
{
	clocks_ms[3] += interrupt_periods_ms[3];
	DEBUG_Write("t[3]=%"PRIu32" ms\r\n", clocks_ms[3]);
}

void tc_callback_4(struct tc_module *const module_inst)
{
	clocks_ms[4] += interrupt_periods_ms[4];
	DEBUG_Write("t[4]=%"PRIu32" ms\r\n", clocks_ms[4]);
}

void tc_callback_5(struct tc_module *const module_inst)
{
	clocks_ms[5] += interrupt_periods_ms[5];
	DEBUG_Write("t[5]=%"PRIu32" ms\r\n", clocks_ms[5]);
}

void tc_callback_6(struct tc_module *const module_inst)
{
	clocks_ms[6] += interrupt_periods_ms[6];
	DEBUG_Write("t[6]=%"PRIu32" ms\r\n", clocks_ms[6]);
}

void tc_callback_7(struct tc_module *const module_inst)
{
	clocks_ms[7] += interrupt_periods_ms[7];
	DEBUG_Write("t[7]=%"PRIu32" ms\r\n", clocks_ms[7]);
}


void configure_tc(void) {
	struct tc_config config_tc;
	int i;
	for (i = 0; i < TC_COUNT; i++) {
		tc_get_config_defaults(&config_tc);
		
		config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
		config_tc.clock_source = timer_clocks[i];
		config_tc.clock_prescaler = timer_prescalers[i];
		config_tc.counter_8_bit.period = interrupt_periods_ms[i]/clock_periods_ms[i];

		tc_init(&tc_instances[i], hw_instances[i], &config_tc);
		tc_enable(&tc_instances[i]);
	}
}


void configure_tc_callbacks(void) {
	int i;
	for (i = 0; i < TC_COUNT; i++) {
		tc_register_callback(&tc_instances[i], tc_callbacks[i], TC_CALLBACK_OVERFLOW);
		tc_enable_callback(&tc_instances[i], TC_CALLBACK_OVERFLOW);
	}
}