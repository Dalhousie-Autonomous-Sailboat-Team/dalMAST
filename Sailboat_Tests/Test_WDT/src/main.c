
#include <asf.h>
#include "sail_debug.h"

void watchdog_early_warning_callback(void);
void configure_wdt(void);
void configure_wdt_callbacks(void);

void watchdog_early_warning_callback(void)
{
	DEBUG_Write("Early warning callback!\r\n");
}

void configure_wdt(void)
{
	struct wdt_conf config_wdt;

	wdt_get_config_defaults(&config_wdt);

	config_wdt.always_on            = false;
	config_wdt.clock_source         = GCLK_GENERATOR_4;
	config_wdt.timeout_period       = WDT_PERIOD_4096CLK;
	config_wdt.early_warning_period = WDT_PERIOD_2048CLK;

	wdt_set_config(&config_wdt);
}

void configure_wdt_callbacks(void)
{
	wdt_register_callback(watchdog_early_warning_callback, WDT_CALLBACK_EARLY_WARNING);
	wdt_enable_callback(WDT_CALLBACK_EARLY_WARNING);
}

int main(void)
{
	system_init();

	DEBUG_Init();

	DEBUG_Write("Testing watchdog timer...\r\n");

	configure_wdt();
	configure_wdt_callbacks();

	system_interrupt_enable_global();

	while (true) {
		delay_s(3);
		wdt_reset_count();
	}
}
