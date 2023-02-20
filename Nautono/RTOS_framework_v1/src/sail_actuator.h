/*! \file sail_actuator.h
    \brief Contains definitions for functions used to get extension of actuator 
*/

#ifndef _SAIL_ACTUATOR_H
#define _SAIL_ACTUATOR_H

#include <asf.h>

extern uint8_t GetExtensionIndex(uint16_t angle);
extern uint16_t GetActuatorExtension(uint8_t index);
void Test_Actuator(void);

#endif