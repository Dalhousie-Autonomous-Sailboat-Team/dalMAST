/* sail_eeprom.h
 * Header file for the EEPROM module manages waypoint access.
 * The data stored in the EEPROM has the following format:
 * 
 * byte |       0        |       1        |        2       |        3       |
 * -----+----------------+----------------+----------------+----------------+
 *  0   |       waypoint index (i)        |     number of waypoints (n)     |
 * -----+---------------------------------+---------------------------------+
 *  4   |                             reserved                              |
 * -----+-------------------------------------------------------------------+
 *  8   |                             reserved                              |
 * -----+-------------------------------------------------------------------+
 *  12  |                             reserved                              |
 * -----+-------------------------------------------------------------------+
 *  16  | [waypoint #0]       longitude (last 4 bytes)                      |
 * -----+-------------------------------------------------------------------+
 *  20  | [waypoint #0]       latitude (first 4 bytes)                      |
 * -----+-------------------------------------------------------------------+
 *  24  | [waypoint #0]       latitude (last 4 bytes)                       |
 * -----+-------------------------------------------------------------------+
 *  28  | [waypoint #0]       longitude (first 4 bytes)                     |
 * -----+-------------------------------------------------------------------+
 *  32  | [waypoint #0]       longitude (last 4 bytes)                      |
 * -----+-------------------------------------------------------------------+
 *  36  | [waypoint #0]       radius (first 4 bytes)                        |
 * -----+-------------------------------------------------------------------+
 *  40  | [waypoint #0]       radius (last 4 bytes)                         |
 * -----+-------------------------------------------------------------------+
 *  44  |      next way point index       |   checksum     |    reserved    |
 * -----+-------------------------------------------------------------------+ 
 *   .  |                             ...                                   |
 * -----+-------------------------------------------------------------------+
 *   .  | [waypoint #n-1]     latitude (first 4 bytes)                      |
 * -----+-------------------------------------------------------------------+
 *   .  | [waypoint #n-1]     latitude (last 4 bytes)                       |
 * -----+-------------------------------------------------------------------+
 *   .  | [waypoint #n-1]     longitude (first 4 bytes)                     |
 * -----+-------------------------------------------------------------------+
 *   .  | [waypoint #n-1]     longitude (last 4 bytes)                      |
 * -----+-------------------------------------------------------------------+
 *   .  | [waypoint #n-1]     radius (first 4 bytes)                        |
 * -----+-------------------------------------------------------------------+
 *   .  | [waypoint #n-1]     radius (last 4 bytes)                         |
 * -----+----------------+--------------------------------------------------+
 *   .  |      next way point index       |   checksum     |    reserved    |
 * -----+----------------+--------------------------------------------------+ 
 * 
 * Created on June 28, 2016.
 * Created by Thomas Gwynne-Timothy.
 */


#ifndef SAIL_EEPROM_H_
#define SAIL_EEPROM_H_

#include <status_codes.h>
#include <stdint.h>

#include "sail_types.h"

#define EEPROM_START_OFFSET		0	// must be a multiple of 128 to ensure page alignment
#define EEPROM_ENTRY_LENGTH		32	// 32 = 128/4 : 4 entries per page
#define EEPROM_ENTRY_OFFSET		32	// offset from start offset to first entry
#define EEPROM_IDX_ADDR			EEPROM_START_OFFSET + 0
#define EEPROM_COUNT_ADDR		EEPROM_START_OFFSET + 2
#define EEPROM_CHECKSUM_IDX		26	// index of checksum within an entry

typedef union EEPROM_Entry {
	struct {
		EEPROM_WayPoint wp;
		uint16_t next_wp_idx;
		uint8_t checksum;
		uint8_t reserved;
	};
	uint8_t data[EEPROM_ENTRY_LENGTH];
} EEPROM_Entry;

/* EEPROM_Init
 * Initialize the EEPROM.
 * Status:
 *   STATUS_OK - initialization was successful
 *   STATUS_ERR_DENIED - a driver error has occurred 
 *   STATUS_ERR_BAD_DATA - the EEPROM data was not formatted correctly
 *   STATUS_ERR_ALREADY_INITIALIZED - the module should only be initialized once
 */
enum status_code EEPROM_Init(void);


/* EEPROM_GetCurrentWayPoint
 * Get the current way point.
 * Inputs/Outputs:
 *   wp - pointer to an EEPROM_WayPoint structure to store current way point
 * Status:
 *   STATUS_OK - way point was returned successfully
 *   STATUS_ERR_DENIED - a driver error has occurred  
 *   STATUS_ERR_BAD_ADDRESS - an invalid pointer was provided
 *   STATUS_ERR_NOT_INITIALIZED - the EEPROM hasn't been initialized yet
 */
enum status_code EEPROM_GetCurrentWayPoint(EEPROM_WayPoint *wp);


/* EEPROM_CompleteCurrentWayPoint
 * Complete the current way point and set the next way point as active.
 * Status:
 *   STATUS_OK - EEPROM index was changed successfully
 *   STATUS_SUSPEND - no more way points are available, suspend navigation
 *   STATUS_ERR_DENIED - a driver error has occurred  
 *   STATUS_ERR_NOT_INITIALIZED - the EEPROM hasn't been initialized yet
 */ 
enum status_code EEPROM_CompleteCurrentWayPoint(void);


// TODO Add documentation here
enum status_code EEPROM_StartMissionConfig(void);
enum status_code EEPROM_LoadWayPoint(EEPROM_Entry *entry);
enum status_code EEPROM_EndMissionConfig(void);

enum status_code EEPROM_GetWayPointCount(uint16_t *wp_count);

#endif /* SAIL_EEPROM_H_ */
