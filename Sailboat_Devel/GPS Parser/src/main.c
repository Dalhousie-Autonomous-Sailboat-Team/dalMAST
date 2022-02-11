/*
 * main.c
 *
 *  Created on: Jun 9, 2016
 *      Author: UW-Stream
 */


#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define BUFFER_LENGTH 64

typedef struct MyStruct {
	uint8_t myInt;
	float myFloatArray[2];
} MyStruct;

int main (void) {
	// Create a test input string
	char str[BUFFER_LENGTH] = "123, 42.5, 0.001\0";

	// Create a struct to store the data from the string
	MyStruct myStruct;

	// Create variable to record number of scanned items and ...
	// load data from string to struct
	int readCnt = sscanf(str, "%"SCNu8",%f,%f", &(myStruct.myInt), &(myStruct.myFloatArray[0]), &(myStruct.myFloatArray[1]));

	// Check the output
	printf("# of items read: %d\n", readCnt);
	printf("Data: %"PRIu8", %f, %f\n", myStruct.myInt, myStruct.myFloatArray[0], myStruct.myFloatArray[1]);

	return 0;
}
