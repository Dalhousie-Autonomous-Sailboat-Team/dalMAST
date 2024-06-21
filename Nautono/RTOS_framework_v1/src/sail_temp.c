/*
 * sail_temp.c
 *
 * Created: 04-May-24 2:47:33 PM
 *  Author: Ahmed Khairallah
 */ 
#include "sail_temp.h"
#include "sail_i2c.h"
#include "sail_debug.h"
#include "sail_uart.h"
#include "FreeRTOS.h"

/*****************************************************************/
/* CONSTANTS                                                     */
/*****************************************************************/

static const uint8_t CTRL_HUM_ADDR   = 0xF2;
static const uint8_t CTRL_MEAS_ADDR  = 0xF4;
static const uint8_t CONFIG_ADDR     = 0xF5;
static const uint8_t PRESS_ADDR      = 0xF7;
static const uint8_t TEMP_ADDR       = 0xFA;
static const uint8_t HUM_ADDR        = 0xFD;
static const uint8_t TEMP_DIG_ADDR   = 0x88;
static const uint8_t PRESS_DIG_ADDR  = 0x8E;
static const uint8_t HUM_DIG_ADDR1   = 0xA1;
static const uint8_t HUM_DIG_ADDR2   = 0xE1;
static const uint8_t ID_ADDR         = 0xD0;

static const uint8_t TEMP_DIG_LENGTH         = 6;
static const uint8_t PRESS_DIG_LENGTH        = 18;
static const uint8_t HUM_DIG_ADDR1_LENGTH    = 1;
static const uint8_t HUM_DIG_ADDR2_LENGTH    = 7;
static const uint8_t DIG_LENGTH              = 32;
static const uint8_t SENSOR_DATA_LENGTH      = 8;

static enum status_code I2C_readLen(I2C_DeviceID device_id, uint8_t addr, uint8_t *data,  uint8_t len)
{
	// Return if the pointer is NULL
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	/* Prepare buffer with the EEPROM address to read from */
	uint8_t buffer = addr;
	
	// Write the command to the compass
	if (I2C_WriteBuffer(device_id, &buffer, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
		DEBUG_Write("write in readlen Error\r\n");
		return STATUS_ERR_DENIED;
	}
	
	// Read the data back from the compass
	bool read_complete = false;
	uint8_t i=0, sz = 0;
	while(len && !read_complete) {
		if(len>2) {
			sz = 2;
			} else {
			sz = len;
		}
		
		switch(I2C_ReadBuffer(device_id, &data[i], sz, I2C_READ_NORMAL))  {
			case STATUS_OK:
			read_complete = true;
			i+=len;
			len -= sz;
			break;
			// Continue the loop if the write could not be completed because the device was busy
			case STATUS_BUSY:
			read_complete = false;
			i+=len;
			len -= sz;
			//i++;len--;
			break;
			// Return if an error occurred while writing to the I2C line
			default:
			return STATUS_ERR_DENIED;
		}
	}
	
	return STATUS_OK;
}


static float CalculatePressure(int32_t raw, int32_t t_fine, enum PresUnit unit){
	  // Code based on calibration algorthim provided by Bosch.
	  int64_t var1, var2, pressure;
	  float final;

	  uint16_t dig_P1 = (m_dig[7]   << 8) | m_dig[6];
	  int16_t   dig_P2 = (m_dig[9]   << 8) | m_dig[8];
	  int16_t   dig_P3 = (m_dig[11] << 8) | m_dig[10];
	  int16_t   dig_P4 = (m_dig[13] << 8) | m_dig[12];
	  int16_t   dig_P5 = (m_dig[15] << 8) | m_dig[14];
	  int16_t   dig_P6 = (m_dig[17] << 8) | m_dig[16];
	  int16_t   dig_P7 = (m_dig[19] << 8) | m_dig[18];
	  int16_t   dig_P8 = (m_dig[21] << 8) | m_dig[20];
	  int16_t   dig_P9 = (m_dig[23] << 8) | m_dig[22];

	  var1 = (int64_t)t_fine - 128000;
	  var2 = var1 * var1 * (int64_t)dig_P6;
	  var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
	  var2 = var2 + (((int64_t)dig_P4) << 35);
	  var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
	  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
	  if (var1 == 0) { return NAN; }                                                         // Don't divide by zero.
	  pressure   = 1048576 - raw;
	  pressure = (((pressure << 31) - var2) * 3125)/var1;
	  var1 = (((int64_t)dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
	  var2 = (((int64_t)dig_P8) * pressure) >> 19;
	  pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

	  final = ((uint32_t)pressure)/256.0;

	  // Conversion units courtesy of www.endmemo.com.
	  switch(unit){
		  case PresUnit_hPa: /* hPa */
		  final /= 100.0;
		  break;
		  case PresUnit_inHg: /* inHg */
		  final /= 3386.3752577878;          /* final pa * 1inHg/3386.3752577878Pa */
		  break;
		  case PresUnit_atm: /* atm */
		  final /= 101324.99766353; /* final pa * 1 atm/101324.99766353Pa */
		  break;
		  case PresUnit_bar: /* bar */
		  final /= 100000.0;               /* final pa * 1 bar/100kPa */
		  break;
		  case PresUnit_torr: /* torr */
		  final /= 133.32236534674;            /* final pa * 1 torr/133.32236534674Pa */
		  break;
		  case PresUnit_psi: /* psi */
		  final /= 6894.744825494;   /* final pa * 1psi/6894.744825494Pa */
		  break;
		  default: /* Pa (case: 0) */
		  break;
	  }
	  return final;
}

static float CalculateTemperature(int32_t raw, int32_t* t_fine, enum TempUnit unit){
	   // Code based on calibration algorthim provided by Bosch.
	   int32_t var1, var2, final;
	   uint16_t dig_T1 = (m_dig[1] << 8) | m_dig[0];
	   int16_t   dig_T2 = (m_dig[3] << 8) | m_dig[2];
	   int16_t   dig_T3 = (m_dig[5] << 8) | m_dig[4];
	   var1 = ((((raw >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
	   var2 = (((((raw >> 4) - ((int32_t)dig_T1)) * ((raw >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	  *t_fine = var1 + var2;
	   final = (*t_fine * 5 + 128) >> 8;
	   return unit == TempUnit_Celsius ? final/100.0 : final/100.0*9.0/5.0 + 32.0;
}

static float CalculateHumidity(int32_t raw, int32_t t_fine){
	   // Code based on calibration algorthim provided by Bosch.
	   int32_t var1;
	   uint8_t   dig_H1 =   m_dig[24];
	   int16_t dig_H2 = (m_dig[26] << 8) | m_dig[25];
	   uint8_t   dig_H3 =   m_dig[27];
	   int16_t dig_H4 = (m_dig[28] << 4) | (0x0F & m_dig[29]);
	   int16_t dig_H5 = (m_dig[30] << 4) | ((m_dig[29] >> 4) & 0x0F);
	   int8_t   dig_H6 =   m_dig[31];

	   var1 = (t_fine - ((int32_t)76800));
	   var1 = (((((raw << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * var1)) +
	   ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)dig_H6)) >> 10) * (((var1 *
	   ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	   ((int32_t)dig_H2) + 8192) >> 14));
	   var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
	   var1 = (var1 < 0 ? 0 : var1);
	   var1 = (var1 > 419430400 ? 419430400 : var1);
	   return ((uint32_t)(var1 >> 12))/1024.0;
}

static int BME280_ReadTrim(){
	uint8_t ord = 0;
	int success =  1;//for true

	
	// Temp. Dig
	success &= I2C_readLen(I2C_TEMP1, TEMP_DIG_ADDR, &m_dig[ord], TEMP_DIG_LENGTH);
	ord += TEMP_DIG_LENGTH;
	DEBUG_Write("read temperature\r\n");

	// Pressure Dig
	success &= I2C_readLen(I2C_TEMP1, PRESS_DIG_ADDR, &m_dig[ord], PRESS_DIG_LENGTH);
	ord += PRESS_DIG_LENGTH;

	// Humidity Dig 1
	success &= I2C_readLen(I2C_TEMP1, HUM_DIG_ADDR1_LENGTH, &m_dig[ord], HUM_DIG_ADDR1_LENGTH);
	ord += HUM_DIG_ADDR1_LENGTH;

	// Humidity Dig 2
	success &= I2C_readLen(I2C_TEMP1, HUM_DIG_ADDR2_LENGTH, &m_dig[ord], HUM_DIG_ADDR2_LENGTH);
	ord += HUM_DIG_ADDR2_LENGTH;
	return success && ord == DIG_LENGTH;
}




int BME280_read(float* pressure, float* temp, float* humidity, enum TempUnit tempUnit, enum PresUnit presUnit){
	if(BME280_ReadTrim() != BME_SUCCESS){
		return BME_ERROR;
	}
	
	int32_t data[8];
	int32_t t_fine;
	uint32_t rawPressure = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
	uint32_t rawTemp = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
	uint32_t rawHumidity = (data[6] << 8) | data[7];
	*temp = CalculateTemperature(rawTemp, &t_fine, tempUnit);
	*pressure = CalculatePressure(rawPressure, t_fine, presUnit);
	*humidity = CalculateHumidity(rawHumidity, t_fine);
	return BME_SUCCESS;
}

void TestTemperatureSensor(){
	TickType_t testDelay = pdMS_TO_TICKS(6000 / portTICK_RATE_MS);
	UART_Init(UART_VCOM);
	
	char buffer[80] = {0};
//#ifdef DEFAULT_TEMPERATURE
	enum TempUnit tempUnit = TempUnit_Celsius;
//#endif

//#ifdef DEFAULT_PRESSURE
	enum PresUnit presUnit = PresUnit_hPa;
//#endif
float temp = 0.0, pres = 0.0, humidity = 0.0;

	while(1){
		BME280_read(&pres, &temp, &humidity, tempUnit, presUnit);
		sprintf(buffer, "Temp: %.2f Humidity: %.2f Press %.2f\r\n", temp, humidity, pres);
		DEBUG_Write("%s", buffer);
	}
	
}