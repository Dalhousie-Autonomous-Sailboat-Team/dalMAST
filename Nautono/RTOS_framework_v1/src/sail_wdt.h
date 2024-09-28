/*
 * sail_wdt.h
 *
 * Created: 2024-09-16 8:43:06 PM
 *  Author: Nader Hdeib
 */ 

#include <asf.h>
#include <status_codes.h>
#include "sail_debug.h"

#define INT_WDT_SLEEP_PERIOD	120000	// milliseconds
#define EXT_WDT_SLEEP_PERIOD	1000	// milliseconds
#define EXT_WDT_PIN		PIN_PA28

// Internal watchdog init
void intWDT_Init(void);


// External watchdog init
void extWDT_Init(void);


// Internal watchdog task
void intWDT_Task(void);


// External watchdog task
void extWDT_Task(void);


// Reset internal watchdog timer
void intWDT_Kick(void);


// Reset external watchdog timer
void extWDT_Kick(void);