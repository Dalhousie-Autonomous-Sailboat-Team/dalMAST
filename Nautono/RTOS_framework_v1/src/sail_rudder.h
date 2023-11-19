/* sail_rudder.h
 * Modified by: Manav Sohi
 * Date: October 29, 2023
 * Updates: Modified to work with new rudder controller.
 */

#ifndef SAIL_RUDDER_H_
#define SAIL_RUDDER_H_

#include <asf.h>

/* RUDDER_Init
 * Initialize rudder
 * Status:
 *   STATUS_OK - Rudder initialization was successful
 */
enum status_code RUDDER_Init(void);

void RudderPotPos(double * data);
extern void RudderSetPos(double pos) ;

void Test_Rudder(void);

#endif