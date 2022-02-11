
#ifndef SAIL_MATH_H_
#define SAIL_MATH_H_

// Clamp a value between lo and hi
double MATH_Clamp(double val, double low, double high);

// Map a value from one range to another (linear interpolation)
double MATH_Map(double val, double in_low, double in_high, double out_low, double out_high);

// Force the angle into range [-180, 180)
double MATH_ForceAngleTo180(double angle);

// Force the angle into range [0, 360)
double MATH_ForceAngleTo360(double angle);

#endif /* SAIL_MATH_H_ */