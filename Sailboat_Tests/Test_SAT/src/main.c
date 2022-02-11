
#include <asf.h>

#include "sail_uart.h"

#define XEOS_ON_OFF_PIN		PIN_PA20
#define XEOS_ON_STATE		false

static void InitPin(void);
static void TurnOn(void);
static void TurnOff(void);

int main (void)
{
	system_init();

	UART_Init(UART_RADIO);
	UART_Init(UART_XEOS);

	UART_TxString(UART_RADIO, (uint8_t *)"Testing satellite module...\r\n");

	// Turn on the XEOS
	InitPin();
	TurnOn();

	// Wait a bit
	delay_ms(5000);

	uint8_t data[256];

	// Send data to the XEOS
	UART_TxString(UART_XEOS, (uint8_t *)"$$sendSBD\r\n");
	UART_TxString(UART_XEOS, (uint8_t *)"Hello\r\n");
	UART_TxString(UART_XEOS, (uint8_t *)"$finished\r\n");

	// Check if the XEOS responded...
	while (true) {
		if (UART_RxString(UART_XEOS, data, 256) == STATUS_VALID_DATA) {
			UART_TxString(UART_RADIO, data);
		}
		delay_ms(1000);
	}
}

static void InitPin(void)
{
    struct port_config config_port_pin;
    port_get_config_defaults(&config_port_pin);
    
    config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
    
    port_pin_set_config(XEOS_ON_OFF_PIN, &config_port_pin);
}


static void TurnOn(void)
{
	port_pin_set_output_level(XEOS_ON_OFF_PIN, XEOS_ON_STATE);
}


static void TurnOff(void)
{
	port_pin_set_output_level(XEOS_ON_OFF_PIN, !XEOS_ON_STATE);
}

