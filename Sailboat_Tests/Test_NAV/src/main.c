
#include <asf.h>

#include "sail_debug.h"
#include "sail_nav.h"
#include "sail_types.h"

#define NUM_WAY_POINTS 18

const GPS_Reading wp_list[NUM_WAY_POINTS] = {
	{ 44.0          , -59.5 },
	{ 44.1710100717 , -59.5301536896 },
	{ 44.3213938048 , -59.6169777784 },
	{ 44.4330127019 , -59.75 },
	{ 44.4924038765 , -59.9131759112 },
	{ 44.4924038765 , -60.0868240888 },
	{ 44.4330127019 , -60.25 },
	{ 44.3213938048 , -60.3830222216 },
	{ 44.1710100717 , -60.4698463104 },
	{ 44.0          , -60.5 },
	{ 43.8289899283 , -60.4698463104 },
	{ 43.6786061952 , -60.3830222216 },
	{ 43.5669872981 , -60.25 },
	{ 43.5075961235 , -60.0868240888 },
	{ 43.5075961235 , -59.9131759112 },
	{ 43.5669872981 , -59.75 },
	{ 43.6786061952 , -59.6169777784 },
	{ 43.8289899283 , -59.5301536896 },
};

const GPS_Reading coords = {44.0, -60.0};

const WIND_Reading wind = {270.0, 5};

const COMP_HeadingData comp = {90.0, 0.0, 0.0};

int main (void)
{
	system_init();

	DEBUG_Init();

	DEBUG_Write("Testing navigation algorithm.\r\n");
	
	DEBUG_Write("All angles are in thousandths of a degree.\r\n");

	double course, bearing, sail_deg, rudder_deg;
	for (int i = 0; i < NUM_WAY_POINTS; i++) {
		NAV_GetBearing(wp_list[i], coords, &bearing);
		NAV_UpdateCourse(wp_list[i], coords, wind, comp.heading, &course, &sail_deg);
		NAV_CalculateRudderPosition(course, comp.heading, &rudder_deg);
		
		DEBUG_Write("-- way point #%02d -----------------------------------------\r\n", i + 1);
		DEBUG_Write("    lat: %7d    lon: %7d\r\n", (int)(wp_list[i].lat*1000.0), (int)(wp_list[i].lon*1000.0));
		DEBUG_Write("bearing: %7d course: %7d\r\n", (int)(bearing*1000.0), (int)(course*1000.0));
		DEBUG_Write(" rudder: %7d   sail: %7d\r\n", (int)(rudder_deg*1000.0), (int)(sail_deg*1000.0));
		
		// Give the buffer time to print the data
		delay_ms(200);
	}


}
