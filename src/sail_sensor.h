/*
 * sail_sensor.h
 *
 *  Created on: Jul 8, 2016
 *      Author: JULIA SARTY
 */

#ifndef SAIL_SENSOR_H_
#define SAIL_SENSOR_H_
/* SENS_GPSnav
 * Returns final GPS data
 * Inputs/Outputs
 *   direction - pointer to data structure for output
 * Status:
 *	 STATUS_OK - command was valid and sent successfully and data was received successfully
 *	 STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code SENS_GPSnav(GPS_Direction *direction);
/* SENS_WINDnav
 * Calls upon SENS_WindGetAverage to compute and average of past readings, 
 * then returns final WIND data
 * Inputs/Outputs
 *   av - pointer to data structure for output, an average of the past readings
 * Status:
 *	 STATUS_OK - command was valid and sent successfully and data was received successfully
 *	 STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code SENS_WINDnav(WIND_Info *av);
/* SENS_COMPnav
 * Returns final WIND data
 * Inputs/Outputs
 *   info - pointer to data structure for output
 * Status:
 *	 STATUS_OK - command was valid and sent successfully and data was received successfully
 *	 STATUS_ERR_DENIED - a driver error has occurred
 */
enum status_code SENS_COMPnav(COMP_HeadingData *info);
#endif /* SAIL_SENSOR_H_ */
