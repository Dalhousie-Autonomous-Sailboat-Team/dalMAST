/* sail_eeprom.c
* Implementation for the EEPROM module.
* Created on June 23, 2016.
* Created by Thomas Gwynne-Timothy/Julia.
*/

#include "sail_EEPROM.h"

#include <asf.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "sail_i2c.h"
#include "sail_debug.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_tasksinit.h"

static union {
	struct {
		uint16_t addr;
		uint8_t data[EEPROM_ENTRY_LENGTH];
		uint16_t reserved;
	} fields;
	uint8_t bytes[EEPROM_ENTRY_LENGTH + 4];
} i2c_buffer;

// Static functions to utilize I2C driver
static enum status_code ReadBufferFromAddr(uint16_t addr, uint8_t *data, uint16_t data_length);
static enum status_code WriteBufferToAddr(uint16_t addr, uint8_t *data, uint16_t data_length);
static enum status_code ReadByteFromAddr(uint16_t addr, uint8_t *data);
static enum status_code WriteByteToAddr(uint16_t addr, uint8_t data);
static enum status_code ReadWordFromAddr(uint16_t addr, uint16_t *data);
static enum status_code WriteWordToAddr(uint16_t addr, uint16_t data);

static uint16_t Idx2Addr(uint16_t idx);

static enum EEPROM_States {
	EEPROM_UNINITIALIZED,
	EEPROM_READY,
	EEPROM_CONFIG,
	EEPROM_NUM_STATES
} state = EEPROM_UNINITIALIZED;

static uint16_t config_idx;

void test_kml_loader(void) {
	uint16_t count = 0, wp_idx = 2;
	EEPROM_WayPoint wp = {0};
	int i = 0;

	// test waypoint count
	EEPROM_GetWayPointCount(&count);
	DEBUG_Write("Waypoint count: %d\n", count);
	
	// test initial id
	ReadWordFromAddr(EEPROM_IDX_ADDR, &wp_idx);
	DEBUG_Write("Waypoint idx: %d\n", wp_idx);
	
	// test waypoints
	for (i = 0; i < count; i++) {
		EEPROM_GetCurrentWayPoint(&wp);
		ReadWordFromAddr(EEPROM_IDX_ADDR, &wp_idx);
		delay_ms(10);
		DEBUG_Write("Waypoint idx: %d\n", wp_idx);
		DEBUG_Write("way point: lat - %d lon - %d rad - %d\r\n", (int)(wp.pos.lat * 1000000.0), (int)(wp.pos.lon * 1000000.0), (int)(wp.rad));
		delay_ms(10);
		EEPROM_CompleteCurrentWayPoint();
	}
	
}

enum status_code EEPROM_Init(void)
{
	// Return if the EEPROM has already been initialized
	if (state != EEPROM_UNINITIALIZED) {
		return STATUS_ERR_ALREADY_INITIALIZED;
	}

	// Return if an error occurred while initializing the I2C driver
	
	switch (I2C_Init()) {
		case STATUS_OK:
		case STATUS_ERR_ALREADY_INITIALIZED:
			break;
		default:
			return STATUS_ERR_DENIED;
	}
	
	// Set the EEPROM module's state to READY
	state = EEPROM_READY;

	return STATUS_OK;
}


enum status_code EEPROM_GetCurrentWayPoint(EEPROM_WayPoint *wp)
{
	uint16_t wp_addr, wp_idx;
	uint8_t checksum;
	uint16_t i;
	EEPROM_Entry entry;
	
	// Return if the module is in an incorrect state
	if (state != EEPROM_READY) {
		DEBUG_Write("Not ready\n");
		return STATUS_ERR_DENIED;
	}

	// Return if provided a null pointer
	if (wp == NULL) {
		DEBUG_Write("Null pointer\n");
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Read way point index from EEPROM
	if (ReadWordFromAddr(EEPROM_IDX_ADDR, &wp_idx) != STATUS_OK) {
		DEBUG_Write("io error 1\n");
		return STATUS_ERR_IO;
	}

	// Give the EEPROM time before next action
	delay_ms(10);

	// Read way point from EEPROM
	wp_addr = Idx2Addr(wp_idx);
	if (ReadBufferFromAddr(wp_addr, entry.data, EEPROM_ENTRY_LENGTH) != STATUS_OK) {
		DEBUG_Write("io error 2\n");
		return STATUS_ERR_IO;
	}

	// Calculate the expected checksum
	checksum = entry.data[0];
	for (i = 1; i < EEPROM_CHECKSUM_IDX; i++) {
		checksum ^= entry.data[i];
	}

	// Return if the checksum doesn't match
	if (checksum != entry.checksum) {
		DEBUG_Write("Invalid checksum\n");
		DEBUG_Write("Checksums %u and %u do not match\n", checksum, entry.checksum);
		return STATUS_ERR_BAD_DATA;
	}
	
	// Return the way point
	*wp = entry.wp;
	DEBUG_Write("Returning the waypoint\n");

	return STATUS_OK;
}


enum status_code EEPROM_CompleteCurrentWayPoint(void)
{
	uint16_t wp_idx, wp_addr;
	uint8_t checksum;
	EEPROM_Entry entry;

	// Return if the module is in an incorrect state
	if (state != EEPROM_READY) {
		return STATUS_ERR_DENIED;
	}

	// Read way point index from EEPROM
	if (ReadWordFromAddr(EEPROM_IDX_ADDR, &wp_idx) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	// Give the EEPROM time before next action
	delay_ms(10);	

	// Read way point from EEPROM
	wp_addr = Idx2Addr(wp_idx);
	if (ReadBufferFromAddr(wp_addr, entry.data, EEPROM_ENTRY_LENGTH) != STATUS_OK) { 
		return STATUS_ERR_IO;
	}

	// Calculate the checksum
	checksum = entry.data[0];
	int i;
	for (i = 1; i < EEPROM_CHECKSUM_IDX; i++) {
		checksum ^= entry.data[i];
	}

	// Return if the checksum doesn't match
	if (checksum != entry.checksum) {
		return STATUS_ERR_BAD_DATA;
	}
	
	// Give the EEPROM time before next action
	delay_ms(10);

	// Write new way point index to the EEPROM
	if (WriteWordToAddr(EEPROM_IDX_ADDR, entry.next_wp_idx) != STATUS_OK) {
		return STATUS_ERR_IO;
	}

	return STATUS_OK;
}


enum status_code EEPROM_StartMissionConfig(void)
{
	// Return if the module is in an incorrect state
	if (state != EEPROM_READY) {
		return STATUS_ERR_DENIED;
	}
	
	// Set the state to CONFIG
	state = EEPROM_CONFIG;
	
	// Initialize the index 
	config_idx = 0;
	
	return STATUS_OK;
}


enum status_code EEPROM_LoadWayPoint(EEPROM_Entry *entry)
{
	// Return if the module is in an incorrect state
	if (state != EEPROM_CONFIG) {
		return STATUS_ERR_DENIED;
	}	
	
	// Return if the pointer is null
	if (entry == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Compute the address of the way point entry to be written
	uint16_t entry_addr = Idx2Addr(config_idx);
	
	// Compute the checksum
	uint8_t checksum = entry->data[0];
	int i;
	for (i = 1; i < EEPROM_CHECKSUM_IDX; i++) {
		checksum ^= entry->data[i];
	}
	
	// Add the checksum
	entry->checksum = checksum;
	
	delay_ms(10);
	
	// Write the data
	
	if (WriteBufferToAddr(entry_addr, entry->data, EEPROM_ENTRY_LENGTH) != STATUS_OK) {
		return STATUS_ERR_IO;
	}
	
	// Increment the index
	config_idx++;
	
	return STATUS_OK;
}


enum status_code EEPROM_EndMissionConfig(void)
{
	
	// Return if the module is in an incorrect state
	if (state != EEPROM_CONFIG) {
		return STATUS_ERR_DENIED;
	}
	
	// Write the starting index to the EEPROM
	WriteWordToAddr(EEPROM_IDX_ADDR, 0);
	
	delay_ms(10);
	
	// Write the entry count to the EEPROM
	WriteWordToAddr(EEPROM_COUNT_ADDR, config_idx);
		
	// Exit configuration state
	state = EEPROM_READY;
	
	test_kml_loader(); //used to test kml_to_waypoint_uploader processing file
	
	return STATUS_OK;
}


enum status_code EEPROM_GetWayPointCount(uint16_t *wp_count)
{
	// Return if the EEPROM module has not been initialized
	if (state == EEPROM_UNINITIALIZED) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if a null pointer is provided
	if (wp_count == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Read the way point count
	if (ReadWordFromAddr(EEPROM_COUNT_ADDR, wp_count) != STATUS_OK) {
		return STATUS_ERR_IO;
	}

	return STATUS_OK;
}



/*************** STATIC FUNCTIONS ***************/

static enum status_code WriteBufferToAddr(uint16_t addr, uint8_t *data, uint16_t data_length)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Return if the provided data won't fit in the internal buffer
	if (data_length > EEPROM_ENTRY_LENGTH) {
		return STATUS_ERR_NO_MEMORY;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Load the data into the write module
	memcpy(i2c_buffer.fields.data, data, data_length);

	// Repeat until the data is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the data
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, data_length + 2, I2C_WRITE_NORMAL)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	return STATUS_OK;
}


static enum status_code ReadBufferFromAddr(uint16_t addr, uint8_t *data, uint16_t data_length)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);

	// Repeat until the address is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the address
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 2, I2C_WRITE_NO_STOP)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	// Request a read from the device
	if (I2C_ReadBuffer(I2C_EEPROM, data, data_length, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


static enum status_code WriteByteToAddr(uint16_t addr, uint8_t data)
{
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Load the data into the write module
	i2c_buffer.fields.data[0] = data;
	
	// Repeat until the data is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the data
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 3, I2C_WRITE_NORMAL)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	return STATUS_OK;
}


static enum status_code ReadByteFromAddr(uint16_t addr, uint8_t *data)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);

	// Repeat until the address is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the address
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 2, I2C_WRITE_NO_STOP)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	// Request a read from the device
	if (I2C_ReadBuffer(I2C_EEPROM, data, 1, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


static enum status_code WriteWordToAddr(uint16_t addr, uint16_t data)
{
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Load the data into the write module
	i2c_buffer.fields.data[0] = 0xff & data;
	i2c_buffer.fields.data[1] = 0xff & (data >> 8);

	// Repeat until the data is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the data
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 4, I2C_WRITE_NORMAL)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	return STATUS_OK;
}


static enum status_code ReadWordFromAddr(uint16_t addr, uint16_t *data)
{
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Swap endianness of address
	i2c_buffer.fields.addr = (addr >> 8) | (addr << 8);
	
	// Repeat until the address is sent successfully or an error occurs
	bool write_complete = false;
	while (!write_complete) {
		// Send the address
		switch (I2C_WriteBuffer(I2C_EEPROM, i2c_buffer.bytes, 2, I2C_WRITE_NO_STOP)) {
			// Exit the loop if the write was completed successfully
			case STATUS_OK:
			write_complete = true;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			write_complete = false;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}

	// Request a read from the device
	if (I2C_ReadBuffer(I2C_EEPROM, (uint8_t *)data, 2, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}


static uint16_t Idx2Addr(uint16_t idx)
{
	return EEPROM_START_OFFSET + EEPROM_ENTRY_OFFSET + EEPROM_ENTRY_LENGTH *idx;
}

#define MISSION_ENTRY_COUNT		40
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

#define TEST_EEPROM_DELAY_MS 1000

#include "sail_anglesensor.h"

void Test_EEPROM(void)
{
	TickType_t testDelay = pdMS_TO_TICKS(TEST_EEPROM_DELAY_MS);
	
	bool test = true;
	
	unsigned int i, j;
	
	uint8_t bno_id = 0;
	uint8_t reg_addr = 0x34;
	
	uint16_t raw_angle;
	
	
	
	uint8_t mode_buffer[2] = {0X3D, 0X01};
	I2C_WriteBuffer(I2C_IMU1, mode_buffer, 2, I2C_WRITE_NORMAL);

	while(test){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		/*
		DEBUG_Write("Initializing EEPROM ... ");

		// Stop if EEPROM initialization fails
		if (EEPROM_Init() != STATUS_OK) {
			DEBUG_Write("Failed!\r\n");
			return 0;
		}
		
		DEBUG_Write("Complete.\r\n");
		*/
		#ifdef TEST
		DEBUG_Write("Creating a mission ... \r\n");
		
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
		
		enum status_code code = STATUS_OK;
		
		for (i = 0; i < MISSION_ENTRY_COUNT; i++) {
			DEBUG_Write_Unprotected("Entry %2d ... ", i + 1);
			//if (EEPROM_LoadWayPoint(&test_mission.entries[i]) != STATUS_OK) {
				
			code = EEPROM_LoadWayPoint(&test_mission.entries[i]);
			if (code != STATUS_OK) {
				DEBUG_Write_Unprotected("Failed, code: %d!\r\n", code);
			}
			
			DEBUG_Write_Unprotected("Complete.\r\n");
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
		#endif
		
		#ifndef TEST2
		DEBUG_Write("<<<<<<<<<<<<<<<<< Testing EEPROM >>>>>>>>>>>>>>>> \r\n");
		
		uint8_t buffer[2] = {0x02, 0x10};
			
		//I2C_WriteBuffer(I2C_EEPROM, buffer, 2, I2C_WRITE_NORMAL);
		
		//WriteByteToAddr(32, 0x20);
		
		uint8_t addr = 0x02;
		uint8_t data = 0;
		
		//delay_ms(10);
		
		ReadByteFromAddr(32, &data);
		
		//I2C_WriteBuffer(I2C_EEPROM, &addr, 1, I2C_WRITE_NORMAL);
		
		//I2C_ReadBuffer(I2C_EEPROM, &data, 1, I2C_READ_NORMAL);
		
		DEBUG_Write("Data: %d\r\n", data);
		/*
		DEBUG_Write_Unprotected("\n\r<<<<<<<<<<< Testing IMU >>>>>>>>>>\n\r");
		
		I2C_WriteBuffer(I2C_IMU1, &reg_addr, 1, I2C_WRITE_NORMAL);
		I2C_ReadBuffer(I2C_IMU1, &bno_id, 1, I2C_READ_NORMAL);
		DEBUG_Write("Bno temp: %d\r\n", bno_id);
		
		DEBUG_Write_Unprotected("\n\r<<<<<<<<<<< Testing AS >>>>>>>>>>\n\r");
		
		rawAngle(&raw_angle);
		
		DEBUG_Write_Unprotected("Raw Angle: %d\r\n", raw_angle);
		*/
		#endif
		
		vTaskDelay(testDelay);
	}
	
}
