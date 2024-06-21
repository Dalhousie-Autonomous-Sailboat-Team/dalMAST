/*
 * sail_temp.h
 *
 * Created: 04-May-24 2:47:54 PM
 *  Author: Ahmed Khairallah
 */ 


#ifndef SAIL_TEMP_H_
#define SAIL_TEMP_H_

#include <stdint.h>
/*****************************************************************/
/* VARIABLES                                                     */
/*****************************************************************/

uint8_t m_dig[32];

int m_initialized;

enum TempUnit
{
	TempUnit_Celsius,
	TempUnit_Fahrenheit
};

enum PresUnit
{
	PresUnit_Pa,
	PresUnit_hPa,
	PresUnit_inHg,
	PresUnit_atm,
	PresUnit_bar,
	PresUnit_torr,
	PresUnit_psi
};

enum {
	BME_ERROR,
	BME_SUCCESS
};
int BME280_read(float* pressure, float* temp, float* humidity, enum TempUnit tempUnit, enum PresUnit presUnit);

void TestTemperatureSensor();

#ifndef _HUGE_ENUF
	#define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VAL   ((double)INFINITY)
#define HUGE_VALF  ((float)INFINITY)
#define HUGE_VALL  ((long double)INFINITY)
// This operation creates a negative NAN adding a - to make it positive
#define NAN        (-(float)(INFINITY * 0.0F))


#endif /* SAIL_TEMP_H_ */