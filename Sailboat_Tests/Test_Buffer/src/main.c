
#include <asf.h>

#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "sail_buffer.h"
#include "sail_debug.h"

#define BUFFER_LENGTH 2048

void configure_tc(void);
void configure_tc_callbacks(void);
void write_callback(struct tc_module *const module_inst);

struct tc_module tc_instance;

BUFF_Module module;
volatile uint8_t buffer[BUFFER_LENGTH];

uint8_t writeStr[] = "\r\nHello buffer 0\r\n\r\n";
uint8_t readStr[BUFFER_LENGTH];
uint16_t length;

int main(void) {
	system_init();
	
	// Initialize the debug UART
	DEBUG_Init();
	
	// Send a status message
	DEBUG_Write("Initializing buffer...\r\n");
	
	// Initialize the buffer
	if (BUFF_Init(&module, buffer, BUFFER_LENGTH) != STATUS_OK) {
		DEBUG_Write("Buffer initialization failed!\r\n");
		return 0;
	}
	
	// Send a status message
	DEBUG_Write("Buffer initialized.\r\n");
	
	// Configure timer
	configure_tc();
	configure_tc_callbacks();
	
	// Enable global interrupts
	system_interrupt_enable_global();
	
	delay_ms(500);
	
	// Enable global interrupts
	system_interrupt_disable_global();	
	
	while (1) {
		if (BUFF_ReadString(&module, readStr, BUFFER_LENGTH) == STATUS_VALID_DATA) {
			DEBUG_Write("%s\r\n", (char *)readStr);
		}
	}
}


void write_callback(struct tc_module *const module_inst) {
	writeStr[15] = ((writeStr[15] - 48 + 1) % 10) + 48;
	BUFF_WriteBuffer(&module, writeStr, strlen((char *)writeStr));
}


void configure_tc(void) {
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);
	
	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV64;
	config_tc.counter_8_bit.period = 10;

	tc_init(&tc_instance, TC3, &config_tc);
	tc_enable(&tc_instance);
}


void configure_tc_callbacks(void) {
	tc_register_callback(&tc_instance, write_callback, TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_instance, TC_CALLBACK_OVERFLOW);
}