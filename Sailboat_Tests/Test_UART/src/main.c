
#include <asf.h>
#include <inttypes.h>

#include "sail_debug.h"
#include "sail_uart.h"

#define BUFFER_LENGTH 256

void configure_tc(void);
void configure_tc_callbacks(void);
void tc_callback(struct tc_module *const module_inst);
struct tc_module tc_instance;

uint8_t buffer[BUFFER_LENGTH];

volatile uint32_t sum;

int main (void)
{
	system_init();
	
	DEBUG_Init();
	
	DEBUG_Write("Initializing the WIND UART ... ");
	if (UART_Init(UART_WIND) != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	// Configure timer
	configure_tc();
	configure_tc_callbacks();
	
	// Enable global interrupts
	system_interrupt_enable_global();	
	
	DEBUG_Write("Enabling wind UART ... ");
	if (UART_Enable(UART_WIND) != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	UART_TxString(UART_WIND, (uint8_t *)"$PLCJE,,,89,,\n\r");
	
	while (true) {
		if (UART_RxString(UART_WIND, buffer, BUFFER_LENGTH) == STATUS_VALID_DATA) {
			DEBUG_Write("%s - %"PRIu32"\r\n", (char *)buffer, sum);
		}
	}
}

void tc_callback(struct tc_module *const module_inst) {
	int i;
	sum = 0;
	for (i = 0; i < BUFFER_LENGTH; i++) {
		sum += buffer[i];
	}
}


void configure_tc(void) {
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);
	
	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
	config_tc.counter_8_bit.period = 10;

	tc_init(&tc_instance, TC3, &config_tc);
	tc_enable(&tc_instance);
}


void configure_tc_callbacks(void) {
	tc_register_callback(&tc_instance, tc_callback, TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_instance, TC_CALLBACK_OVERFLOW);
}