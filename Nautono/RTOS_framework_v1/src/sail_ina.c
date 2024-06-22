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

static enum status_code ReadWord(I2C_DeviceID ina, uint8_t reg, uint16_t *data){
	if(data == NULL) {
		return STATUS_ERR_BAD_DATA;
	}

	if(I2C_WriteBuffer(ina, &reg, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
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
	
	DEBUG_Write("val_raw: %d\r\n", val_raw);


	voltage_V = (float)( val_raw / 1000.0);

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
	int32_t shunt_uV = 0;
	float current_A  = 0;
	int32_t shuntRes = 10;

	shunt_uV  = getShuntVoltage(ina, channel);
	current_A = shunt_uV / 1000.0 / shuntRes; // in the void INA3221::begin(TwoWire *theWire) of the original function they are set to 10
	return current_A;
}

#define TEST_INA_DELAY_MS 10000

void Test_INA(void){
	TickType_t testDelay = pdMS_TO_TICKS(TEST_INA_DELAY_MS);

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
					
		DEBUG_Write("The INA is 1 and channel is 1 and voltage is %d \r\n", (int)ReadVoltage(I2C_INA1, 1));
		DEBUG_Write("The INA is 1 and channel is 2 and voltage is %d \r\n", (int)ReadVoltage(I2C_INA1, 2));
		DEBUG_Write("The INA is 1 and channel is 3 and voltage is %d \r\n\r\n", (int)ReadVoltage(I2C_INA1, 3));
		
		DEBUG_Write("The INA is 2 and channel is 1 and voltage is %d \r\n", (int)ReadVoltage(I2C_INA2, 1));
		DEBUG_Write("The INA is 2 and channel is 2 and voltage is %d \r\n", (int)ReadVoltage(I2C_INA2, 2));
		DEBUG_Write("The INA is 2 and channel is 3 and voltage is %d \r\n\r\n", (int)ReadVoltage(I2C_INA2, 3));
		
		vTaskDelay(testDelay);
	}
}