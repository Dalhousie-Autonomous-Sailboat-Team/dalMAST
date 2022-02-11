#ifndef RC_DETECTION_H
#define RC_DETECTION_H

#define RC_RUDDER_IN PIN_PA20
#define RC_SAIL_IN PIN_PA21

bool is_autonomous;

void configure_rc_pins(void);
void init_control_mode(void);
void detect_remote(void);

#endif

