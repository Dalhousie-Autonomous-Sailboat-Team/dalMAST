#ifndef SAILBOATCONTROL_H
#define SAILBOATCONTROL_H

#include "rc_detection.h"

void configure_rc_extint_channel(void);
void configure_rc_extint_callbacks(void);
void extint_rc_detection_callback(void);
void navigate(void);
void init_navigation(void);

#endif

