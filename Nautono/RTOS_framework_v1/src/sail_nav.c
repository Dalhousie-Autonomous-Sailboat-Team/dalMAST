/* sail_nav.c
 * Implementation of the navigation controller.
 * Created on August 3, 2016.
 * Created by Thomas Gwynne-Timothy.
 *
 * Updated on July 27 2020.
 * Updated by Dylan Hubble
 * rebuilt navigation logic as well as some of the preset angles
 *
 * Updated on May 23 2024
 * Updated by Luca Salvatore
 * Added logic for limiting how far the boat can tack
 * Removed cloth sail logic
 *
 */

#include "sail_nav.h"

#include <math.h>
#include <stdbool.h>

#include "sail_debug.h"
#include "sail_math.h"
#include <machine\fastmath.h>

#define R_EARTH             6371008.0

//want to reconfirm that this is the max
#define MAX_RUDDER_DEG           45.0

//<<<<<<< HEAD
//will remove the majority of these as they are unused
//=======
//>>>>>>> fc041ddd22a8619f3d34256a382816f79f263f49
#define DEAD_ANGLE_DEG           25.0
// #define CLOSE_HAULED_ANGLE_DEG   60.0
// #define CLOSE_REACH_ANGLE_DEG    75.0
// #define BEAM_REACH_ANGLE_DEG     90.0
// #define BROAD_REACH_ANGLE_DEG   135.0
#define RUNNING_ANGLE_DEG       170.0
#define OVERSHOOT_ANGLE           5.0


// #define CLOSE_HAULED_SET_DEG     10.0
// #define CLOSE_REACH_SET_DEG      30.0
// #define BEAM_REACH_SET_DEG       40.0
// #define BROAD_REACH_SET_DEG      50.0
// #define RUNNING_SET_DEG          60.0

#define BOUNDING_BOX_WIDTH	 0.2      // Degrees of lat/lon, 0.2 = 22.2km
#define TACKING_BOX_WIDTH	 0.1      // 11.1km
#define BOUNDING_BOX_BACK_SPACE  0.025    // 3km

// Functions to compute the distance and bearing between two GPS positions
static float GetBearing(GPS_Reading from, GPS_Reading to);
static double GetDistance(GPS_Reading from, GPS_Reading to);
// calculate if we're in the tacking area
static int CoordinateInBox(GPS_Reading gps, GPS_Reading box[4]);
double triangleArea(GPS_Reading trianglePoints[3]);
enum status_code createTackingBox(GPS_Reading tacking_boundary[4], float *bearing, GPS_Reading wp, GPS_Reading gps);

enum status_code NAV_UpdateCourse(GPS_Reading wp, GPS_Reading gps, WIND_Reading wind, float heading, float *course, GPS_Reading tackingBox[4], int *sailingMode, int *changingTack)
{
	// Return if any of the pointers are NULL
	if (course == NULL || sail_angle == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	// Compute the bearing from the current position to the way point location
	float bearing = GetBearing(gps, wp, tackingBox);

	// Get the wind angle relative to magnetic north
	float wind_wrt_north = MATH_ForceAngleTo180(wind.angle + heading);

	// Express the bearing as an angle with respect to (wrt) the wind direction
	float bearing_wrt_wind = MATH_ForceAngleTo180(bearing - wind_wrt_north);

	// Check if we need to switch from direct sailing to tacking
	if (fabs(bearing_wrt_wind) < DEAD_ANGLE_DEG && *sailingMode == 0) {
		// Set the course to the closest arm of the dead zone
		if (bearing_wrt_wind >= 0.0) {
			*course = MATH_ForceAngleTo180(wind_wrt_north + DEAD_ANGLE_DEG);
			*sailingMode = 1;
		} else {
			*course = MATH_ForceAngleTo180(wind_wrt_north - DEAD_ANGLE_DEG);
			*sailingMode = -1;
		}
		//if running on Dead angle zone go as close as possible with no overshoot
		// double course_wrt_wind = fabs(MATH_ForceAngleTo180(*course - wind_wrt_north));

		// Use the relative course to select a sail setting
		// *sail_angle = course_wrt_wind;

		return createTackingBox(tackingBox, bearing, wp, gps); // if we start to tack, create the bounding box

	// Check if need to go from direct sailing to jibin
	} else if (fabs(bearing_wrt_wind) > RUNNING_ANGLE_DEG && *sailingMode == 0) {
		// Set the course to the closest arm of the jibe zone
		if (bearing_wrt_wind >= 0.0) {
			*course = MATH_ForceAngleTo180(wind_wrt_north + RUNNING_ANGLE_DEG);
			*sailingMode = 1;
		} else {
			*course = MATH_ForceAngleTo180(wind_wrt_north - RUNNING_ANGLE_DEG);
			*sailingMode = -1;
		}
		//if running on edge go as close as possible no over shoot amount
		// double course_wrt_wind = fabs(MATH_ForceAngleTo180(*course - wind_wrt_north));

		// Use the relative course to select a sail setting
		// *sail_angle = course_wrt_wind;

		return createTackingBox(tackingBox, bearing, wp, gps);

	// if currently tacking
	} else if(fabs(bearing_wrt_wind) < DEAD_ANGLE_DEG && *sailingMode != 0){
		// check if inside tacking box
		if(CoordinateInBox(gps, tackingBox)){
			*changingTack = 0;

		} else if (!*changingTack){ // if we aren't in the process of changing tack
			*course = MATH_ForceAngleTo180(wind_wrt_north - DEAD_ANGLE_DEG * 2 * (*sailingMode));
			*sailingMode = -1 * (*sailingMode);
			*changingTack = 1;
		}

        // if currently jibin
	} else if(fabs(bearing_wrt_wind) < RUNNING_ANGLE_DEG && *sailingMode != 0){

		// check if outside of tacking box
		if(CoordinateInBox(gps, tackingBox)){
			*changingTack = 0;

		} else if (!*changingTack){
			*course = MATH_ForceAngleTo180(wind_wrt_north - RUNNING_ANGLE_DEG * 2 * (*sailingMode));
			*sailingMode = -1 * (*sailingMode);
			*changingTack = 1;
		}

	}else {
		// Otherwise, head straight for the way point
		*course = bearing;
		*changingTack = 0;
		*sailingMode = 0;

		// Express the course as an angle with respect to (wrt) the wind direction
		// double course_wrt_wind = fabs(MATH_ForceAngleTo180(*course - wind_wrt_north));

		// Use the relative course to select a sail setting
		// *sail_angle = course_wrt_wind;

		//linear function
		//makes it so that if boat is perpendicular to wind maximum overshooting is applied
		//linearly decreases based on difference of boat angle to where it is going
		// double overshoot_amount = fabs(((RUNNING_ANGLE_DEG-90)-fabs(*sail_angle-90))*(OVERSHOOT_ANGLE/(RUNNING_ANGLE_DEG-90)));

		// *sail_angle = *sail_angle-overshoot_amount;
	}



	return STATUS_OK;
}


enum status_code NAV_CalculateRudderPosition(float course, float heading, float *rudder_angle)
{
	//will be rewritten currently very dumb logic

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
// prob unused
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

static float GetBearing(GPS_Reading from, GPS_Reading to)
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


// not used?
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

// https://stackoverflow.com/questions/17136084/checking-if-a-point-is-inside-a-rotated-rectangle
static int CoordinateInBox(GPS_Reading gps, GPS_Reading box[4]){

	GPS_Reading triangleA[3] = {box[0], gps, box[3]};
	GPS_Reading triangleB[3] = {box[3], gps, box[2]};
	GPS_Reading triangleC[3] = {box[2], gps, box[1]};
	GPS_Reading triangleD[3] = {box[0], gps, box[1]};

	double sumOfTriangleAreas = triangleArea(triangleA) + triangleArea(triangleB) + triangleArea(triangleC) + triangleArea(triangleD);
	double rectArea = sqrt((box[0].lat - box[1].lat) * (box[0].lat - box[1].lat) + (box[0].lon - box[1].lon) * (box[0].lon - box[1].lon)) * sqrt((box[0].lat - box[2].lat) * (box[0].lat - box[2].lat) + (box[0].lon - box[2].lon) * (box[0].lon - box[2].lon));

	return sumOfTriangleAreas <= rectArea + 0.0000000001; // I added an extra little bit to account for weirdness in the rounding.

}

double triangleArea(GPS_Reading trianglePoints[3]){
	return fabs((trianglePoints[1].lon * trianglePoints[0].lat - trianglePoints[0].lon * trianglePoints[1].lat) + (trianglePoints[2].lon * trianglePoints[1].lat - trianglePoints[2].lat * trianglePoints[1].lon) + (trianglePoints[0].lon * trianglePoints[2].lat - trianglePoints[0].lat * trianglePoints[2].lon)) / 2;
}

enum status_code createTackingBox(GPS_Reading tacking_boundary[4], float *bearing, GPS_Reading wp, GPS_Reading gps){
        // Calculate the boxes to sail within
	tacking_boundary[0].lon = TACKING_BOX_WIDTH * cos((*bearing + 90.0) * M_PI / 180.0) + gps.lon - cos(*bearing * M_PI / 180.0) * BOUNDING_BOX_BACK_SPACE;
	tacking_boundary[0].lat = TACKING_BOX_WIDTH * sin((*bearing + 90.0) * M_PI / 180.0) + gps.lat - sin(*bearing * M_PI / 180.0) * BOUNDING_BOX_BACK_SPACE;

	tacking_boundary[1].lon = TACKING_BOX_WIDTH * cos((*bearing + 90.0) * M_PI / 180.0) + wp.lon;
	tacking_boundary[1].lat = TACKING_BOX_WIDTH * sin((*bearing + 90.0) * M_PI / 180.0) + wp.lat;

	tacking_boundary[2].lon = TACKING_BOX_WIDTH * cos((*bearing - 90.0) * M_PI / 180.0) + gps.lon - cos(*bearing * M_PI / 180.0) * BOUNDING_BOX_BACK_SPACE;
	tacking_boundary[2].lat = TACKING_BOX_WIDTH * sin((*bearing - 90.0) * M_PI / 180.0) + gps.lat - sin(*bearing * M_PI / 180.0) * BOUNDING_BOX_BACK_SPACE

	tacking_boundary[3].lon = TACKING_BOX_WIDTH * cos((*bearing - 90.0) * M_PI / 180.0) + wp.lon;
	tacking_boundary[3].lat = TACKING_BOX_WIDTH * sin((*bearing - 90.0) * M_PI / 180.0) + wp.lat;

	return STATUS_OK;
}


