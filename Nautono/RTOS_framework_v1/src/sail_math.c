
#include "sail_math.h"

#include <math.h>

// Clamp the input between the high and low thresholds
double MATH_Clamp(double val, double low, double high)
{
	// If the value is below the lower threshold, clamp it
	if (val < low) {
		return low;
		// If the value is above the upper threshold, clamp it
	} else if (val > high) {
		return high;
		// Otherwise, return the value
	} else {
		return val;
	}
}


// Map val from one range to another via linear interpolation
double MATH_Map(double val, double in_low, double in_high, double out_low, double out_high)
{
	// Ensure there's no division by zero
	if (fabs(in_high - in_low) < 0.001) {
		return 0.0;
	}
	
	// Remove input offset
	double output = val - in_low;
	
	// Apply scaling
	output *= (out_high - out_low)/(in_high - in_low);
	
	// Add output offset
	output += out_low;
	
	return output;
}

// Force the angle into range (-180, 180]
float MATH_ForceAngleTo180(double angle)
{
	// Force the angle into range [0, 360)
	angle = MATH_ForceAngleTo360(angle);
	
	// Limit the angle to 180
	if (angle > 180.0) {
		angle -= 360.0;
	}
	
	return angle;
}


// Force the angle into range [0, 360)
float MATH_ForceAngleTo360(double angle)
{
	// Reduce the angle
	angle = fmod(angle, 360.0);
	
	// Force the angle to a positive value
	angle = fmod(angle + 360.0, 360.0);
	
	return angle;
}
