/*
 * sail_beacon.h
<<<<<<< HEAD:Alex_RTOS_Sailboat/RTOS_framework_v1/src/sail_beacon.h
 * this is the header file for the stream 211 SDB beacon
 * Created: 2022-06-03 6:45:03 PM
 *  Author: Matthew Cockburn
 */ 
// sail boat libraries 
#include "status_codes.h"
// beacon types and defs

enum beacon_msg_type{
	beacon_AT_msg,
	beacon_ERROR_msg,
	beacon_msg_type_total
};

// Function Prototypes 
void DataLogTask(void);
enum status_code beacon_status(void);
enum status_code signal_strength(void);
enum status_code tx_sbd_msg(void);
enum status_code rx_sbd_msg(void);
enum status_code gather_data(void);
=======
 *
 * Created: 9/27/2023 2:59:46 PM
 *  Author: manav
 */ 

void Test_Beacon(void);
>>>>>>> PCB_Test:Nautono/RTOS_framework_v1/src/sail_beacon.h
