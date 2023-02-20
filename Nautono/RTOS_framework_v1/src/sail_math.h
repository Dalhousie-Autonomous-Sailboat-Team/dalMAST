
#ifndef SAIL_MATH_H_
#define SAIL_MATH_H_


/* MATH_Clamp
 * Clamp the input between the high and low thresholds
 * Input:
 *	 val - Input value
 *	 low - Minimum
 *	 high - Maximum
 * Return:
 *	 val - Processed value
 */ 
double MATH_Clamp(double val, double low, double high);


/* MATH_Map
 * Map a value from one range to another (linear interpolation)
 * Input:
 *	 val - Input Value
 *	 in_low - Input value minimum
 *	 in_high - Input value maximum
 *	 out_low - Expected output value minimum
 *	 out_high - Expected output value maximum
 * Return:
	 output - Processed value
 */ 
double MATH_Map(double val, double in_low, double in_high, double out_low, double out_high);


/* MATH_ForceAngleTo180
 * Force the angle into range [-180, 180)
 * Input:
 *	 angle - Input angle value
 * Output:
 *	 angle - Processed angle value
 */ 
float MATH_ForceAngleTo180(double angle);


/* MATH_ForceAngleTo360
 * Force the angle into range [0, 360)
 * Input:
 *	 angle - Input angle value
 * Output:
 *	 angle - Processed angle value
 */ 
float MATH_ForceAngleTo360(double angle);

#endif /* SAIL_MATH_H_ */