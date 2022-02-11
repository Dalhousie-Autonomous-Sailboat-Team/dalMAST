
#include <asf.h>

#include "sail_radio.h"
#include "sail_debug.h"
#include "sail_gps.h"
#include "sail_eeprom.h"
#include "sail_comp.h"

int main (void)
{
	RADIO_GenericMsg rx_msg, tx_msg;
	GPS_Reading gps_data;
	EEPROM_WayPoint wp_data;
	COMP_Reading comp_reading;
	uint8_t gps_period, wp_period, comp_period;
	
	system_init();
	
	DEBUG_Init();
	
	/* Test 0: Initialization
	 * Check if the module can be initialized and enabled.
	 */
	
	DEBUG_Write("Initializing radio module ... ");
	if (RADIO_Init() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");

	DEBUG_Write("Enabling radio receiver ... ");
	if (RADIO_Enable() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	DEBUG_Write("Complete!\r\n");
	
	/* Test 1: Sensor
	 * Check if the module can send log data at different periods.
	 */
	gps_data.lat = 40.123456;
	gps_data.lon = -65.987654;
	wp_data.pos.lat = 89.999999;
	wp_data.pos.lon = -179.999999;
	wp_data.rad = 50;
	comp_reading.type = COMP_HEADING;
	comp_reading.data.heading.heading = 97.5;
	comp_reading.data.heading.pitch = -1.3;
	comp_reading.data.heading.roll = 34.1;

	gps_period = 3;
	wp_period = 4;
	comp_period = 5;
	
	int i;
	for (i = 0; i < 60; i++) {
		// Check if it's time to send the GPS data
		if (i % gps_period == 0) {
			tx_msg.type = RADIO_GPS;
			tx_msg.fields.gps.data = gps_data;
			RADIO_TxMsg(&tx_msg);
		}
		
		// Check if it's time to send the EEPROM data
		if (i % wp_period == 0) {
			tx_msg.type = RADIO_WAY_POINT;
			tx_msg.fields.wp.idx = i;
			tx_msg.fields.wp.data = wp_data;
			tx_msg.fields.wp.next_idx = i + 1;
			RADIO_TxMsg(&tx_msg);
		}
		
		// Check if it's time to send the compass data
		if (i % comp_period == 0) {
			tx_msg.type = RADIO_COMP;
			tx_msg.fields.comp.data = comp_reading;
			RADIO_TxMsg(&tx_msg);
		}
		
		// Wait for approximately 1 second
		delay_s(1);
	}
	
	/* Test 2: Acknowledgment
	 * Check if module can receive and acknowledge messages
	 */	
	tx_msg.type = RADIO_ACK;
	tx_msg.fields.ack.status = RADIO_STATUS_SUCCESS;
	for (i = 0; i < 5; i++) {
		// Wait until a message arrives
		while (RADIO_RxMsg(&rx_msg) != STATUS_OK) {
			DEBUG_Write("Looking for a message...\r\n");
			delay_ms(200);
		}
		DEBUG_Write("Found a message!\r\n");
		// Send an acknowledgment
		RADIO_TxMsg(&tx_msg);
	}
	
	
	return 0;
}
