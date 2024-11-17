#include "sail_debug.h"
#include "sail_tasksinit.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "sail_ina.h"
#include "sail_i2c.h"

static uint8_t reg[NUM_REGISTERS] = {
	0x2,
	0x4,
	0x6,
	0x1,
	0x3,
	0x5
};

static enum status_code ReadWord(I2C_DeviceID ina, uint8_t register_address, uint16_t *data){
	if(data == NULL) {
		return STATUS_ERR_BAD_DATA;
	}

	if(I2C_WriteBuffer(ina, &register_address, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	if(I2C_ReadBuffer(ina, (uint8_t*)data, 2, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}

	return STATUS_OK;
}

float ReadVoltage(I2C_DeviceID ina, int channel) {
	float voltage_V = 0.0;
	uint8_t reggie;
	uint16_t val_raw = 0;

	switch (channel) {
		case 1:
		reggie = reg[CH1_BUSV];
		break;
		case 2:
		reggie = reg[CH2_BUSV];
		break;
		case 3:
		reggie = reg[CH3_BUSV];
		break;
	}

	ReadWord(ina, reggie, &val_raw);
	
	val_raw = (val_raw << 8) | (val_raw >> 8);
	
	//DEBUG_Write("val_raw: %d\r\n", val_raw);


	voltage_V = val_raw / 1000.0;

	return voltage_V;
}

static int32_t getShuntVoltage(I2C_DeviceID ina, int channel){
	int32_t res;
	uint8_t reggie;
	uint16_t val_raw = 0;

	switch (channel) {
		case 1:
		reggie = reg[CH1_SHUNTV];
		break;
		case 2:
		reggie = reg[CH2_SHUNTV];
		break;
		case 3:
		reggie = reg[CH3_SHUNTV];
		break;
	}

	ReadWord(ina, reggie, &val_raw);

	// 1 Least Significant Bit = 40uV
	res = (int32_t)(val_raw >> 3) * 40;

	return res;
}

float ReadCurrent(I2C_DeviceID ina, int channel) {
	float shunt_uV = 0;
	float current_A  = 0;
	uint32_t shuntRes = 100;

	shunt_uV  = (float)getShuntVoltage(ina, channel);
	current_A = (float)((shunt_uV / 1000.0) / shuntRes); // in the void INA3221::begin(TwoWire *theWire) of the original function they are set to 10
	return current_A;
}

#define TEST_INA_DELAY_MS 1000
#define VOLTAGE_MEASUREMENT
#define CURRENT_MEASUREMENT

void Test_INA(void){
	TickType_t testDelay = pdMS_TO_TICKS(TEST_INA_DELAY_MS);

	while(1){
#ifdef VOLTAGE_MEASUREMENT
		DEBUG_Write("############### Reading Voltages from INA's ###############\r\n");
		DEBUG_Write("INA 1 channel 1 (3.3V bus) voltage:			>%d<\r\n", (int)(ReadVoltage(I2C_INA1, 1)*1000));
		DEBUG_Write("INA 1 channel 2 (12 V bus) voltage:			>%d<\r\n", (int)(ReadVoltage(I2C_INA1, 2)*1000));
		DEBUG_Write("INA 1 channel 3 (disconnected) voltage:			>%d<\r\n", (int)(ReadVoltage(I2C_INA1, 3)*1000));
		
		DEBUG_Write("INA 2 channel 1 (rudder motor) voltage:			>%d<\r\n", (int)(ReadVoltage(I2C_INA2, 1)*1000));
		DEBUG_Write("INA 2 channel 2 (linear actuator) voltage:		>%d<\r\n", (int)(ReadVoltage(I2C_INA2, 2)*1000));
		DEBUG_Write("INA 2 channel 3 (beacon) voltage:			>%d<\r\n", (int)(ReadVoltage(I2C_INA2, 3)*1000));
#endif /* VOLTAGE_MEASUREMENT */
#ifdef CURRENT_MEASUREMENT
		DEBUG_Write("############### Reading Currents from INA's ###############\r\n");
		DEBUG_Write("INA 1 channel 1 (3.3V bus) current:			>%d<\r\n", (int)(ReadCurrent(I2C_INA1, 1)*1000));
		DEBUG_Write("INA 1 channel 2 (12 V bus) current:			>%d<\r\n", (int)(ReadCurrent(I2C_INA1, 2)*1000));
		DEBUG_Write("INA 1 channel 3 (disconnected) current:			>%d<\r\n", (int)(ReadCurrent(I2C_INA1, 3)*1000));
		
		DEBUG_Write("INA 2 channel 1 (rudder motor) voltage:			>%d<\r\n", (int)(ReadCurrent(I2C_INA2, 1)*1000));
		DEBUG_Write("INA 2 channel 2 (linear actuator) voltage:		>%d<\r\n", (int)(ReadCurrent(I2C_INA2, 2)*1000));
		DEBUG_Write("INA 2 channel 3 (beacon) voltage:			>%d<\r\n", (int)(ReadCurrent(I2C_INA2, 3)*1000));
#endif /* CURRENT_MEASUREMENT */
		vTaskDelay(testDelay);
	}
}