/*
 * sail_bme680.c
 *
 * Created: 10/19/2024 1:34:38 PM
 *  Author: gkeut
 */ 
#include "sail_debug.h"
#include "sail_bme680.h"

#include "sail_tasksinit.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

float get_temp(void){
	float temp;
	uint32_t temp_adc;
	uint8_t buffer[3];
	uint8_t par_t11;
	uint8_t par_t12;
	uint8_t par_t1;
	uint8_t par_t21;
	uint8_t par_t22;
	uint8_t par_t2;
	uint8_t par_t3;
	uint8_t par11_buffer = 0xE9;
	uint8_t par12_buffer = 0xEA;
	uint8_t par21_buffer = 0x8A;
	uint8_t par22_buffer = 0x8B;
	uint8_t par3_buffer = 0x8C;
	uint8_t write_buffer = 0x22;
	double var1, var2, t_fine;
	
	//initiate the conversation with bme680
	I2C_WriteBuffer(I2C_BME, &write_buffer, 1, I2C_WRITE_NORMAL);
	I2C_ReadBuffer(I2C_BME, buffer, 3, I2C_READ_NORMAL);
	
	I2C_WriteBuffer(I2C_BME, &par11_buffer, 1, I2C_WRITE_NORMAL);
	I2C_ReadBuffer(I2C_BME, &par_t11, 1, I2C_READ_NORMAL);
	I2C_WriteBuffer(I2C_BME, &par12_buffer, 1, I2C_WRITE_NORMAL);
	I2C_ReadBuffer(I2C_BME, &par_t12, 1, I2C_READ_NORMAL);
	
	I2C_WriteBuffer(I2C_BME, &par21_buffer, 1, I2C_WRITE_NORMAL);
	I2C_ReadBuffer(I2C_BME, par_t21, 1, I2C_READ_NORMAL);
	I2C_WriteBuffer(I2C_BME, &par22_buffer, 1, I2C_WRITE_NORMAL);
	I2C_ReadBuffer(I2C_BME, par_t22, 1, I2C_READ_NORMAL);
	
	I2C_WriteBuffer(I2C_BME, &par3_buffer, 1, I2C_WRITE_NORMAL);
	I2C_ReadBuffer(I2C_BME, &par_t3, 1, I2C_READ_NORMAL);
	
	temp_adc = ((uint32_t)(buffer[0] << 12) | (uint32_t)(buffer[1] << 4) | ((uint32_t)buffer[2] >> 4) );
	par_t1 = ((uint16_t)(par_t12 << 8) | (uint16_t)(par_t11));
	par_t2 = ((uint16_t)(par_t22 << 8) | (uint16_t)(par_t21));
	
	
	var1 = (((double)temp_adc / 16384.0) - ((double)par_t1/1024.0)) * (double)par_t2;
	var2 = ((((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0)) * (((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0))) * ((double)par_t3 * 16.0);
	t_fine = var1 + var2;
	temp = t_fine / 5120.0;
	
	return temp;
}

void debug_BME680(void){
	float temp;
	
	TickType_t testDelay = pdMS_TO_TICKS(1000);
	
	while(1){
		temp = get_temp();
		DEBUG_Write("temp is %d\r\n",(int)(temp * 1000));
		vTaskDelay(testDelay);
	}
	
}