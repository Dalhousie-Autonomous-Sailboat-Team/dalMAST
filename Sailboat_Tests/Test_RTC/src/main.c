
#include <asf.h>
#include <stdint.h>
#include <inttypes.h>

#include "sail_debug.h"

void rtc_overflow_callback(void);
void configure_rtc_count(void);
void configure_rtc_callbacks(void);

struct rtc_module rtc_instance;

void rtc_overflow_callback(void)
{
	DEBUG_Write("OVERFLOW\r\n");
}


void configure_rtc_count(void)
{
	struct rtc_count_config config_rtc_count;
	rtc_count_get_config_defaults(&config_rtc_count);

	config_rtc_count.prescaler           = RTC_COUNT_PRESCALER_DIV_1;
	config_rtc_count.mode                = RTC_COUNT_MODE_16BIT;
	config_rtc_count.continuously_update = true;

	rtc_count_init(&rtc_instance, RTC, &config_rtc_count);

	rtc_count_enable(&rtc_instance);

}

void configure_rtc_callbacks(void)
{
	rtc_count_register_callback(&rtc_instance, rtc_overflow_callback, RTC_COUNT_CALLBACK_OVERFLOW);
	rtc_count_enable_callback(&rtc_instance, RTC_COUNT_CALLBACK_OVERFLOW);
}


int main(void)
{

	system_init();
	
	DEBUG_Init();

	configure_rtc_count();

	configure_rtc_callbacks();

	rtc_count_set_period(&rtc_instance, 1000);

	while (true) {
		// Do nothing
	}
}
