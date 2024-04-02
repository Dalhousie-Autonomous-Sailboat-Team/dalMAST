// TO DO:
// Run through code with Manav
// Ask what includes to use and any compiler flags to use

#include <stdlib.h>
#include <inttypes.h>
#include "sail_i2c.h"

typedef enum Reg_Names {
	CH1_BUSV,
	CH2_BUSV,
	CH3_BUSV,
	CH1_SHUNTV,
	CH2_SHUNTV,
	CH3_SHUNTV,
	NUM_REGISTERS
} Reg_Names;

float ReadVoltage(I2C_DeviceID address, int channel);
float ReadCurrent(I2C_DeviceID address, int channel);
void Test_INA(void);