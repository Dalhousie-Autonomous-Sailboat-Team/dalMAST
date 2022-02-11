/*
 * win_timer.c
 *
 *  Created on: Jun 21, 2016
 *      Author: JFB
 */

#include <windows.h>

DWORD start;

/******************************************************************************
returns the current tick count.
**************************************************************************** */
void init_timer()
{
	start = GetTickCount();
}


/******************************************************************************
This function returns the time elapsed.  Note that if there is an overflow on
the tick count, the time elapsed period will be .
 *****************************************************************************/
unsigned int GetTime_Elapsed()
{
	DWORD end = GetTickCount();

	return end - start;
}
