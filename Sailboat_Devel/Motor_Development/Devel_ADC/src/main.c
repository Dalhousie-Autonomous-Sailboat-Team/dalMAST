/* main.c
 * Test bench for ADC driver
 * Created on August 15, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#include <asf.h>

#include "sail_adc.h"
#include "sail_debug.h"

#define SAIL_ON_OFF_PIN		PIN_PB15
#define SAIL_ON_STATE		true
#define DELAY_SECONDS		5

// Time since last reset
static uint64_t t_ms;
// Time step (resolution of RTC)
static uint16_t dt_ms = 200;
// RTC timer
struct rtc_module rtc_timer;

static void Loop(void);
static void StartTimer(void);

static void InitPin(void);
static void TurnOn(void);
static void TurnOff(void);

static double sail_reading, rudder_reading;

int main (void) {
	/**************** SETUP ****************/
	system_init();
	
	DEBUG_Init();
	
	DEBUG_Write("Testing ADC driver...\r\n");
	
	InitPin();
	
	TurnOn();
	
	// Initialize the ADC
	ADC_Init();

	// Start the timer
	StartTimer();
	
	// Loop and let interrupts take over
	while (true) {
		// Do nothing
	}	
}


static void Loop(void)
{
	// Update the system clock
	t_ms += dt_ms;
	
	// Read from the ADC
	ADC_GetReading(ADC_SAIL, &sail_reading);
	DEBUG_Write("sail:   %d\r\n", (int)(sail_reading * 1000.0));

	//ADC_GetReading(ADC_RUDDER, &rudder_reading);
	//DEBUG_Write("rudder: %d\r\n", (int)(rudder_reading * 1000.0));
}

static void StartTimer(void)
{
	struct rtc_count_config config_rtc_count;
	rtc_count_get_config_defaults(&config_rtc_count);

	config_rtc_count.prescaler           = RTC_COUNT_PRESCALER_DIV_2;
	config_rtc_count.mode                = RTC_COUNT_MODE_16BIT;
	config_rtc_count.continuously_update = true;

	rtc_count_init(&rtc_timer, RTC, &config_rtc_count);
	rtc_count_enable(&rtc_timer);
	
	// Register and enable the new callback
	rtc_count_register_callback(&rtc_timer, Loop, RTC_COUNT_CALLBACK_OVERFLOW);
	rtc_count_enable_callback(&rtc_timer, RTC_COUNT_CALLBACK_OVERFLOW);
	
	// Set the period
	rtc_count_set_period(&rtc_timer, dt_ms);
}

static void InitPin(void) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT_WTH_READBACK;
	
	port_pin_set_config(SAIL_ON_OFF_PIN, &config_port_pin);
}


static void TurnOn(void) {
	port_pin_set_output_level(SAIL_ON_OFF_PIN, SAIL_ON_STATE);
}


static void TurnOff(void) {
	port_pin_set_output_level(SAIL_ON_OFF_PIN, !SAIL_ON_STATE);
}