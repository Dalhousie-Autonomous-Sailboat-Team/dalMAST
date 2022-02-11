#ifndef MONITOR_RADIO_H
#define MONITOR_RADIO_H
#include "GPSSensor.h"
#include "gps_log.h"

void init_monitor_radio(void);
void transmit_radio(gps_gprmc *gprmc, gps_deci_signed *ds_robot, 
	gps_deci_signed *ds_bouee, int angleRobot, distance_deci_signed *distance, 
	int correctionCap, int dirVent, int servoVoile, int poi_actuel);


#endif