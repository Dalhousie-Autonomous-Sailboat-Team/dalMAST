
#include <asf.h>
#include <stdint.h>

#include "sail_nmea.h"
#include "sail_debug.h"

#define BUFFER_LENGTH		128
#define GPS_ON_OFF_PIN		PIN_PA02
#define GPS_ON_STATE		false

static void GPS_InitPin(void);
static void GPS_TurnOn(void);
static void GPS_TurnOff(void);

uint8_t message[] = "PMTK314,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";

int main (void) {
	system_init();
	
	// Initialize the debug port
	DEBUG_Init();
	
	GPS_InitPin();
	GPS_TurnOff();
	
	// Write a greeting
	DEBUG_Write("Testing NMEA module...\r\n");
	
	// Initialize the GPS channel
	NMEA_ChannelID channel = NMEA_GPS;
	
	DEBUG_Write("Initializing NMEA module...\r\n");
	NMEA_Init(channel);
	DEBUG_Write("NMEA module initialized.\r\n");
	
	uint8_t buffer[BUFFER_LENGTH];
	
	DEBUG_Write("Enabling NMEA module...\r\n");
	system_interrupt_enable_global();
	NMEA_Enable(channel);
	DEBUG_Write("NMEA module enabled.\r\n");
	
	DEBUG_Write("Activating GPS.\r\n");
	GPS_TurnOn();
	
	delay_ms(2000);
	
	// Wait for the first startup string - "PGACK,103"
	while (NMEA_RxString(channel, buffer, BUFFER_LENGTH) != STATUS_VALID_DATA) {
		// Do nothing
	}
	DEBUG_Write("1/5 - %s\r\n", (char *)buffer);
	
	// Wait for the second startup string - "PGACK,105"
	while (NMEA_RxString(channel, buffer, BUFFER_LENGTH) != STATUS_VALID_DATA) {
		// Do nothing
	}
	DEBUG_Write("2/5 - %s\r\n", (char *)buffer);	
	
	// Wait for the third startup string - "PMTK011,MTKGPS"
	while (NMEA_RxString(channel, buffer, BUFFER_LENGTH) != STATUS_VALID_DATA) {
		// Do nothing
	}
	DEBUG_Write("3/5 - %s\r\n", (char *)buffer);	
	
	// Wait for the fourth startup string - "PMTK010,001"
	while (NMEA_RxString(channel, buffer, BUFFER_LENGTH) != STATUS_VALID_DATA) {
		// Do nothing
	}
	DEBUG_Write("4/5 - %s\r\n", (char *)buffer);	
	
	// Wait for the fifth startup string - "PMTK010,002"
	while (NMEA_RxString(channel, buffer, BUFFER_LENGTH) != STATUS_VALID_DATA) {
		// Do nothing
	}
	DEBUG_Write("5/5 - %s\r\n", (char *)buffer);	
	
	NMEA_TxString(channel, message);
	
	// Wait for the fifth startup string - "PMTK010,002"
	while (NMEA_RxString(channel, buffer, BUFFER_LENGTH) != STATUS_VALID_DATA) {
		// Do nothing
	}
	DEBUG_Write("ACK - %s\r\n", (char *)buffer);	
	
	while(1) {
		if (NMEA_RxString(channel, buffer, BUFFER_LENGTH) == STATUS_VALID_DATA) {
			DEBUG_Write("%s\r\n", (char *)buffer);
		}
	}
}


void GPS_InitPin(void) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT_WTH_READBACK;
	
	port_pin_set_config(GPS_ON_OFF_PIN, &config_port_pin);
}


void GPS_TurnOn(void) {
	port_pin_set_output_level(GPS_ON_OFF_PIN, GPS_ON_STATE);
}


void GPS_TurnOff(void) {
	port_pin_set_output_level(GPS_ON_OFF_PIN, !GPS_ON_STATE);
}
