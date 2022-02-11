
#include <asf.h>

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "sail_eeprom.h"
#include "sail_i2c.h"
#include "sail_debug.h"

#define MISSION_ENTRY_COUNT		60
#define MISSION_LENGTH			EEPROM_ENTRY_OFFSET + EEPROM_ENTRY_LENGTH * MISSION_ENTRY_COUNT
#define WRITE_DATA_LENGTH		EEPROM_ENTRY_LENGTH
#define WRITE_BUFFER_LENGTH		EEPROM_ENTRY_LENGTH + 2

typedef union Mission {
	struct {
		uint16_t current_wp_idx;
		uint16_t wp_count;
		EEPROM_Entry entries[MISSION_ENTRY_COUNT];
	};
	uint8_t bytes[MISSION_LENGTH];
} Mission;

typedef union I2C_Buffer {
	struct {
		uint16_t addr;
		uint8_t data[WRITE_DATA_LENGTH];
		uint16_t reserved;
	};
	uint8_t bytes[WRITE_BUFFER_LENGTH];
} I2C_Buffer;

int main(void) {
	// Define loop indices
	unsigned int i, j;
	
	// Initialize SAMD20J18
	system_init();
	
	// Setup debug UART
	DEBUG_Init();
	

	DEBUG_Write("Initializing EEPROM ... ");
	
	// Stop if EEPROM initialization fails
	if (EEPROM_Init() != STATUS_OK) {
		DEBUG_Write("Failed!\r\n");
		return 0;
	}
	
	DEBUG_Write("Complete.\r\n");
	
	DEBUG_Write("Creating a mission ... ");
	
	// Create the mission
	Mission test_mission;
	test_mission.current_wp_idx = 0;
	test_mission.wp_count = 1;
	
	// Create entries
	uint8_t checksum;
	srand(1234);
	for (i = 0; i < MISSION_ENTRY_COUNT; i++) {
		test_mission.entries[i].wp.pos.lat = 40.0 + 10.0 * rand() / (double)RAND_MAX;
		test_mission.entries[i].wp.pos.lon = -60.0 + 10.0 * rand() / (double)RAND_MAX;
		test_mission.entries[i].wp.rad = 1000.0 + 1000.0 * rand() / (double)RAND_MAX;
		test_mission.entries[i].next_wp_idx = (uint16_t)((i + 1) % MISSION_ENTRY_COUNT);
		checksum = test_mission.entries[i].data[0];
		for (j = 1; j < 26; j++) {
			checksum ^= test_mission.entries[i].data[j];
		}
		test_mission.entries[i].checksum = checksum;
		test_mission.entries[i].reserved = 0x00;
	}
	
	DEBUG_Write("Complete.\r\n");
	
	DEBUG_Write("Writing mission to the EEPROM:\r\n");
	
	// Write the mission header to the EEPROM
	EEPROM_StartMissionConfig();
	for (i = 0; i < MISSION_ENTRY_COUNT; i++) {
		DEBUG_Write("Entry %2d ... ", i + 1);
		if (EEPROM_LoadWayPoint(&test_mission.entries[i]) != STATUS_OK) {
			DEBUG_Write("Failed!\r\n");
		}
		DEBUG_Write("Complete.\r\n");
	}
	EEPROM_EndMissionConfig();
	
	delay_s(1);
	
	// Loop through way points and compare to mission
	EEPROM_WayPoint wp;
	for (i = 0; i < MISSION_ENTRY_COUNT; i++) {
		DEBUG_Write("Getting current way point %2d ... ", i + 1);
		// Get the waypoint
		if (EEPROM_GetCurrentWayPoint(&wp) != STATUS_OK) {
			DEBUG_Write("Failed!\r\n");
			return 0;
		}
		DEBUG_Write("Complete!\r\n");
		// Compare the waypoint
		DEBUG_Write("lat: %4"PRIi16".%"PRIu32" deg  | %4"PRIi16".%"PRIu32" deg\r\n",
			(int16_t)wp.pos.lat, (uint32_t)(fmod(fabs(wp.pos.lat), 1.0)*1000000.0),
			(int16_t)test_mission.entries[i].wp.pos.lat, (uint32_t)(fmod(fabs(test_mission.entries[i].wp.pos.lat), 1.0)*1000000.0));
		DEBUG_Write("lon: %4"PRIi16".%"PRIu32" deg  | %4"PRIi16".%"PRIu32" deg\r\n",
			(int16_t)wp.pos.lon, (uint32_t)(fmod(fabs(wp.pos.lon), 1.0)*1000000.0),
			(int16_t)test_mission.entries[i].wp.pos.lon, (uint32_t)(fmod(fabs(test_mission.entries[i].wp.pos.lon), 1.0)*1000000.0));
		DEBUG_Write("rad: %4"PRIi16".%"PRIu32" m  | %4"PRIi16".%"PRIu32" m\r\n",
			(int16_t)wp.rad, (uint32_t)(fmod(fabs(wp.rad), 1.0)*1000000.0),
			(int16_t)test_mission.entries[i].wp.rad, (uint32_t)(fmod(fabs(test_mission.entries[i].wp.rad), 1.0)*1000000.0));
		// Move to the next way point
		DEBUG_Write("Completing way point ... ");
		if (EEPROM_CompleteCurrentWayPoint() != STATUS_OK) {
			DEBUG_Write("Failed!\r\n");
			return 0;
		}	
		DEBUG_Write("Complete.\r\n");	
	}
	
	DEBUG_Write("Test complete!\r\n");
	return 0;
}