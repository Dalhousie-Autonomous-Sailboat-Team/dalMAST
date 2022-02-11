
#ifndef SAIL_CTRL_H_
#define SAIL_CTRL_H_

#include <asf.h>
#include <status_codes.h>

typedef enum Sensor_Types {
	SENSOR_GPS,
	SENSOR_WIND,
	SENSOR_COMP,
	SENSOR_COUNT
} Sensor_Type;


/* CTRL_InitSystem
 * Initialize the sail boat controller.
 *
 */
enum status_code CTRL_InitSystem(void);

/* CTRL_InitSensors
 * Initialize each sensor.
 *
 */
enum status_code CTRL_InitSensors(void);

enum status_code CTRL_RunLoop(void);




#endif /* SAIL_CTRL_H_ */ 