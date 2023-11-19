/*
 * sail_anglesensor.h
 *
 * Created: 2023-07-18 5:11:04 PM
 *  Author: manav
 */ 

#include <inttypes.h>

void Test_AS(void);
extern void AS_init(uint8_t directionPin);
extern enum status_code readAngle(uint16_t *data);
extern enum status_code rawAngle(uint16_t *data);