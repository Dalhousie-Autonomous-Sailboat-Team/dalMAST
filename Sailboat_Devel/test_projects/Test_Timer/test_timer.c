/*
 ============================================================================
 Name        : test_timer.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "win_timer.h"





int main(void) {

	int refresh_rate = 1000; // let's update a value every 4 seconds
	int current_time, last_time;
	int max_count = refresh_rate*16;

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	fflush(stdout);

	// set the initial time
	init_timer();

	// confirm the current time
	current_time = GetTime_Elapsed();
	last_time = current_time; // initialize the previous time to the current time


	// while we havent reached the maximum elapsed time
	while(GetTime_Elapsed() < max_count)
	{
		current_time = GetTime_Elapsed();
		if(current_time > last_time + refresh_rate)
		{
			last_time = current_time;
			puts("Heart beat.");
			fflush(stdout);
		}
	}

	puts("!!!Bye Bye World!!!"); /* prints !!!Bye Bye World!!! */


	return EXIT_SUCCESS;
}
