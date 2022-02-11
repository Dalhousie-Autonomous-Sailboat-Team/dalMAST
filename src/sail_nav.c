/* sail_nav.c
 * Implementation of the navigation controller.
 * Created on August 3, 2016.
 * Created by Thomas Gwynne-Timothy.
 */

#include "sail_nav.h"

#include <math.h>
#include <stdbool.h>

#include "sail_debug.h"
#include "sail_math.h"

#define R_EARTH             6371008.0

#define MAX_RUDDER_DEG           45.0

#define DEAD_ANGLE_DEG           45.0
#define CLOSE_HAULED_ANGLE_DEG   60.0
#define CLOSE_REACH_ANGLE_DEG    75.0
#define BEAM_REACH_ANGLE_DEG     90.0
#define BROAD_REACH_ANGLE_DEG   135.0
#define RUNNING_ANGLE_DEG       170.0

#define CLOSE_HAULED_SET_DEG     10.0
#define CLOSE_REACH_SET_DEG      30.0
#define BEAM_REACH_SET_DEG       40.0
#define BROAD_REACH_SET_DEG      50.0
#define RUNNING_SET_DEG          60.0

// Functions to compute the distance and bearing between two GPS positions
static double GetBearing(GPS_Reading from, GPS_Reading to);
static double GetDistance(GPS_Reading from, GPS_Reading to);


enum status_code NAV_UpdateCourse(GPS_Reading wp, GPS_Reading gps, WIND_Reading wind, double heading, double *course, double *sail_angle)
{
	// Return if any of the pointers are NULL
	if (course == NULL || sail_angle == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Compute the bearing from the current position to the way point location
	double bearing = GetBearing(gps, wp);

	// Get the wind angle relative to magnetic north
	double wind_wrt_north = MATH_ForceAngleTo180(wind.angle + heading);
	
	// Express the bearing as an angle with respect to (wrt) the wind direction
	double bearing_wrt_wind = MATH_ForceAngleTo180(bearing - wind_wrt_north);
	
	// Check if the bearing is in the dead zone
	if (fabs(bearing_wrt_wind) < DEAD_ANGLE_DEG) {
		// Set the course to the closest arm of the dead zone
		if (bearing_wrt_wind >= 0.0) {
			*course = MATH_ForceAngleTo180(wind_wrt_north + DEAD_ANGLE_DEG);
		} else {
			*course = MATH_ForceAngleTo180(wind_wrt_north - DEAD_ANGLE_DEG);
		}
	// Check if the bearing is in the jibe zone
	} else if (fabs(bearing_wrt_wind) > RUNNING_ANGLE_DEG) {
		// Set the course to the closest arm of the jibe zone
		if (bearing_wrt_wind >= 0.0) {
			*course = MATH_ForceAngleTo180(wind_wrt_north + RUNNING_ANGLE_DEG);
		} else {
			*course = MATH_ForceAngleTo180(wind_wrt_north - RUNNING_ANGLE_DEG);
		}
	} else {
		// Otherwise, head straight for the way point
		*course = bearing;
	}
	
	// Express the course as an angle with respect to (wrt) the wind direction
	double course_wrt_wind = fabs(MATH_ForceAngleTo180(*course - wind_wrt_north));
	
	// Use the relative course to select a sail setting
	// Close hauled
	if (course_wrt_wind < CLOSE_HAULED_ANGLE_DEG) {
		*sail_angle = CLOSE_HAULED_SET_DEG;
	// Close reaching
	} else if (course_wrt_wind < CLOSE_REACH_ANGLE_DEG) {
		*sail_angle = CLOSE_REACH_SET_DEG;
	// Beam reaching
	} else if (course_wrt_wind < BEAM_REACH_ANGLE_DEG) {
		*sail_angle = BEAM_REACH_SET_DEG;
	// Broad reaching
	} else if (course_wrt_wind < BROAD_REACH_ANGLE_DEG) {
		*sail_angle = BROAD_REACH_SET_DEG;
	// Running
	} else {
		*sail_angle = RUNNING_SET_DEG;
	}
	
	return STATUS_OK;
}


enum status_code NAV_CalculateRudderPosition(double course, double heading, double *rudder_angle)
{
	// Return if any of the pointers are NULL
	if (rudder_angle == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Compute the rudder angle
	*rudder_angle = -0.5 * MATH_ForceAngleTo180(heading - course);
	
	// Limit the rudder angle
	*rudder_angle = MATH_Clamp(*rudder_angle, -MAX_RUDDER_DEG, MAX_RUDDER_DEG);

	return STATUS_OK;
}

enum status_code NAV_GetDistance(GPS_Reading wp, GPS_Reading gps, double *distance)
{
	// Compute the distance
	*distance = GetDistance(gps, wp);
	
	return STATUS_OK;
}

enum status_code NAV_GetBearing(GPS_Reading wp, GPS_Reading gps, double *bearing)
{
	// Compute the bearing
	*bearing = GetBearing(gps, wp);
	
	return STATUS_OK;
}

/**** NAVIGATION UTILITIES ********************************************************/

static double GetBearing(GPS_Reading from, GPS_Reading to)
{
	// Convert the inputs
	double lat1 = from.lat*M_PI/180.0;	// Convert current frame to radians
	double lat2 = to.lat*M_PI/180.0;	// Convert waypoint to radians
	double lon1 = from.lon*M_PI/180.0;	// Convert current frame to radians
	double lon2 = to.lon*M_PI/180.0;	// Convert waypoint to radians
	double delta_lon = lon2 - lon1;		// Delta lon in radians
	
	// Compute the bearing
	double y = sin(delta_lon)*cos(lat2);
	double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(delta_lon);
	double bearing = atan2(y, x);
	
	// Convert the bearing to degrees
	bearing *= (180.0/M_PI);
	
	return MATH_ForceAngleTo180(bearing);
}


static double GetDistance(GPS_Reading from, GPS_Reading to)
{
	// Convert the inputs
	double lat1 = from.lat*M_PI/180.0;	// Convert current frame to radians
	double lat2 = to.lat*M_PI/180.0;		// Convert waypoint to radians
	double lon1 = from.lon*M_PI/180.0;	// Convert current frame to radians
	double lon2 = to.lon*M_PI/180.0;		// Convert waypoint to radians
	double delta_lat = lat1 - lat2;		// Delta lat in radians
	double delta_lon = lon2 - lon1;		// Delta lon in radians

	// Use the haversine formula to compute the distance
	double a = sin(delta_lat/2)*sin(delta_lat/2) + cos(lat1)*cos(lat2)*sin(delta_lon/2)*sin(delta_lon/2);
	double y = sqrt(a);
	double x = sqrt(1-a);
	double b = 2 * atan2(y, x);
	return R_EARTH * b;
}








