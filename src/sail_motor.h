/* sail_motor.h
 * Header file for the motor controller for the autonomous sailboat project.
 * Created on Thomas Gwynne-Timothy.
 * Created by August 16, 2016.
 */

#ifndef SAIL_MOTOR_H_
#define SAIL_MOTOR_H_

#include <asf.h>

typedef enum MOTOR_ChannelIDs {
	MOTOR_SAIL,
	MOTOR_RUDDER,
	MOTOR_NUM_CHANNELS
} MOTOR_ChannelID;

enum status_code MOTOR_Init(void);
enum status_code MOTOR_SetSail(double angle);
enum status_code MOTOR_SetRudder(double angle);

#endif // SAIL_MOTOR_H_