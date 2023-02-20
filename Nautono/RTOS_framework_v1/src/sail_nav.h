/* sail_nav.h
 * Header file for the navigation controller.
 * Created on August 3, 2016.
 * Created by Thomas Gwynne-Timothy.
 */


#ifndef SAIL_NAV_H_
#define SAIL_NAV_H_

#include <status_codes.h>

#include "sail_types.h"


/* NAV_UpdateCourse
 * Calculate the target course.
 * This function should be called at the same frequency as NAV_CalculateSailPosition.
 * The course calculated by this function should be used as input to the NAV_CalculateRudderPosition
 * function, which should be called more frequently.
 *
 * Inputs:
 *   wp:      the current way point's position
 *   gps:     the most recent GPS reading
 *   wind:    the most recent wind vane reading
 *   heading: the average heading
 *
 * Inputs/Outputs:
 *   course:     pointer to the target course, which will be updated when the function returns
 *   sail_angle: pointer to the required sail angle for the given course
 *
 */
enum status_code NAV_UpdateCourse(GPS_Reading wp, GPS_Reading gps, WIND_Reading wind, float heading, float *course, float *sail_angle);


/* NAV_CalculateRudderPosition
 * Calculate the rudder angle to maintain the course.
 * This function should be called frequently to keep the boat following the desired course.
 *
 * Inputs:
 *   course:  the boat's current course, calculated by NAV_CalculateCourse
 *   heading: the boat's current heading, from the most recent compass reading
 *
 * Inputs/Outputs:
 *   rudder_angle: the rudder angle required to maintain course
 *
 */
enum status_code NAV_CalculateRudderPosition(float course, float heading, float *rudder_angle);


/* NAV_GetDistance
 * Calculate the distance from GPS location to waypoint
 *
 * Inputs:
 *   wp:  the next waypoint
 *   gps: the boat's current location, from GPS reading
 *
 * Inputs/Outputs:
 *   distance: the calculated distance from GPS location to waypoint
 */
enum status_code NAV_GetDistance(GPS_Reading wp, GPS_Reading gps, double *distance);


/* NAV_GetBearing
 * Math calculation to get bearing
 *
 * Inputs:
 *   wp:  the next waypoint
 *   gps: the boat's current location, from GPS reading
 *
 * Inputs/Outputs:
 *   bearing: the calculated bearing from GPS location to waypoint
 */
enum status_code NAV_GetBearing(GPS_Reading wp, GPS_Reading gps, double *bearing);

#endif /* SAIL_NAV_H_ */
