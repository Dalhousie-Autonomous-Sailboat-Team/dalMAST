/*
 * sail_imu.c
 *
 * Created: 2023-06-20 6:48:42 PM
 *  Author: Manav Sohi
 */ 


#include "sail_imu.h"

#include <asf.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "sail_debug.h"
#include "sail_i2c.h"
#include "sail_types.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_ctrl.h"
#include "sail_tasksinit.h"

static bool init_flag = false;
static adafruit_bno055_opmode_t _mode = OPERATION_MODE_NDOF;
static bool calibrate_flag = false;

/* static function for IMU */
static enum status_code read8(uint8_t addr, uint8_t *data);
static enum status_code readLen(uint8_t addr, uint8_t *data, uint8_t len);
static enum status_code write8(uint8_t addr, uint8_t data);

static enum status_code read8(uint8_t addr, uint8_t *data)
{
	// Return if the pointer is NULL
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	/* Prepare buffer with the EEPROM address to read from */
	uint8_t buffer = addr;
	
	// Write the command to the compass
	if (I2C_WriteBuffer(I2C_IMU, &buffer, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Read the data back from the compass
	if (I2C_ReadBuffer(I2C_IMU, data, 1, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;
}

static enum status_code readLen(uint8_t addr, uint8_t *data,  uint8_t len)
{
	// Return if the pointer is NULL
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	/* Prepare buffer with the EEPROM address to read from */
	uint8_t buffer = addr;
	
	// Write the command to the compass
	if (I2C_WriteBuffer(I2C_IMU, &buffer, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Give the compass time to prepare the data (see data sheet)
	delay_ms(10);
	
	// Read the data back from the compass
	if (I2C_ReadBuffer(I2C_IMU, data, len, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;
}

static enum status_code write8(uint8_t addr, uint8_t data)
{
	/* Prepare buffer with the following fields:
	 * 1. IMU EEPROM address to write to, and 
	 * 2. Data to be written
	 */
	uint8_t buffer[2];
	buffer[0] = addr;
	buffer[1] = data;	
	
	// Write to the compass
	if (I2C_WriteBuffer(I2C_IMU, buffer, 2, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;	
}

//static void getCalibration(CalibOffset * Calib) {
static void getCalibration(uint8_t *sys, uint8_t *gyro, uint8_t *accel, uint8_t *mag) {

	uint8_t calData = 0;

	read8(BNO055_CALIB_STAT_ADDR, &calData);

	if (sys != NULL) {
		*sys = (calData >> 6) & 0x03;
	}
	if (gyro != NULL) {
		*gyro = (calData >> 4) & 0x03;
	}
	if (accel != NULL) {
		*accel = (calData >> 2) & 0x03;
	}
	if (mag != NULL) {
		*mag = calData & 0x03;
	}
}

static bool isFullyCalibrated() {

	// CalibOffset Calib;
	uint8_t system, gyro, accel, mag;
	//getCalibration(&Calib);
	getCalibration(&system, &gyro, &accel, &mag);

	switch (_mode) {
	case OPERATION_MODE_ACCONLY:
		return (accel == 3);
	case OPERATION_MODE_MAGONLY:
		return (mag == 3);
	case OPERATION_MODE_GYRONLY:
	case OPERATION_MODE_M4G: /* No magnetometer calibration required. */
		return (gyro == 3);
	case OPERATION_MODE_ACCMAG:
	case OPERATION_MODE_COMPASS:
		return (accel == 3 && mag == 3);
	case OPERATION_MODE_ACCGYRO:
	case OPERATION_MODE_IMUPLUS:
		return (accel == 3 && gyro == 3);
	case OPERATION_MODE_MAGGYRO:
		return (mag == 3 && gyro == 3);
	default:
		return (system == 3 && gyro == 3 && accel == 3 && mag == 3);
	}
}

static enum status_code IMU_calibrate(void)
{
	if(!init_flag) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	if(calibrate_flag) {
		return STATUS_NO_CHANGE;
	}

	while(!isFullyCalibrated())
	{
		DEBUG_Write("\n\r<<<<<<<<<<< Calibrating IMU >>>>>>>>>>\n\r");
		delay_ms(BNO055_SAMPLERATE_DELAY_MS);
	}

	calibrate_flag = true;

	return STATUS_OK;
}

static void setMode(adafruit_bno055_opmode_t mode) {
	_mode = mode;
	write8(BNO055_OPR_MODE_ADDR, _mode);
	delay_ms(30);
}

static void setExtCrystalUse(bool usextal) {
	adafruit_bno055_opmode_t modeback = _mode;

	/* Switch to config mode (just in case since this is the default) */
	setMode(OPERATION_MODE_CONFIG);
	delay_ms(25);
	write8(BNO055_PAGE_ID_ADDR, 0);
	if (usextal) {
		write8(BNO055_SYS_TRIGGER_ADDR, 0x80);
		} else {
		write8(BNO055_SYS_TRIGGER_ADDR, 0x00);
	}
	delay_ms(10);
	/* Set the requested operating mode (see section 3.3) */
	setMode(modeback);
	delay_ms(20);
}

bool getSensorOffsets(adafruit_bno055_offsets_t * offsets_type) {
	if (!isFullyCalibrated()){
		return false;
	}
	adafruit_bno055_opmode_t lastMode = _mode;
	setMode(OPERATION_MODE_CONFIG);
	delay_ms(25);

	/* Accel offset range depends on the G-range:
		+/-2g  = +/- 2000 mg
		+/-4g  = +/- 4000 mg
		+/-8g  = +/- 8000 mg
		+/-1§g = +/- 16000 mg */
	uint8_t msb = 0, lsb = 0;
	read8(ACCEL_OFFSET_X_MSB_ADDR, &msb);
	read8(ACCEL_OFFSET_X_LSB_ADDR, &lsb);
	offsets_type->accel_offset_x = msb << 8 | lsb;
	
	read8(ACCEL_OFFSET_Y_MSB_ADDR, &msb);
	read8(ACCEL_OFFSET_Y_LSB_ADDR, &lsb);
	offsets_type->accel_offset_y = msb << 8 | lsb;
	
	read8(ACCEL_OFFSET_Z_MSB_ADDR, &msb);
	read8(ACCEL_OFFSET_Z_LSB_ADDR, &lsb);			
	offsets_type->accel_offset_z = msb << 8 | lsb;

	/* Magnetometer offset range = +/- 6400 LSB where 1uT = 16 LSB */
	
	read8(MAG_OFFSET_X_MSB_ADDR, &msb);
	read8(MAG_OFFSET_X_LSB_ADDR, &lsb);
	offsets_type->mag_offset_x = ( msb << 8) | (lsb);
	
	read8(MAG_OFFSET_Y_MSB_ADDR, &msb);
	read8(MAG_OFFSET_Y_LSB_ADDR, &lsb);
	offsets_type->mag_offset_y = (msb << 8) | (lsb);
	
	read8(MAG_OFFSET_Z_MSB_ADDR, &msb);
	read8(MAG_OFFSET_Z_LSB_ADDR, &lsb);
	offsets_type->mag_offset_z = (msb << 8) | (lsb);

	/* Gyro offset range depends on the DPS range:
		2000 dps = +/- 32000 LSB
		1000 dps = +/- 16000 LSB
		500 dps = +/- 8000 LSB
		250 dps = +/- 4000 LSB
		125 dps = +/- 2000 LSB
		... where 1 DPS = 16 LSB */
	
	read8(GYRO_OFFSET_X_MSB_ADDR, &msb);
	read8(GYRO_OFFSET_X_LSB_ADDR, &lsb);
	offsets_type->gyro_offset_x = (msb << 8) | (lsb);
	
	read8(GYRO_OFFSET_Y_MSB_ADDR, &msb);
	read8(GYRO_OFFSET_Y_LSB_ADDR, &lsb);
	offsets_type->gyro_offset_y = (msb << 8) | (lsb);
	
	read8(GYRO_OFFSET_Z_MSB_ADDR, &msb);
	read8(GYRO_OFFSET_Z_LSB_ADDR, &lsb);
	offsets_type->gyro_offset_z = (msb << 8) | (lsb);

	/* Accelerometer radius = +/- 1000 LSB */
	read8(ACCEL_RADIUS_MSB_ADDR, &msb);
	read8(ACCEL_RADIUS_LSB_ADDR, &lsb);
	offsets_type->accel_radius = (msb << 8) | (lsb);

	/* Magnetometer radius = +/- 960 LSB */
	read8(MAG_RADIUS_MSB_ADDR, &msb);
	read8(MAG_RADIUS_LSB_ADDR, &lsb);
	offsets_type->mag_radius = (msb << 8) | (lsb);

	setMode(lastMode);
	return true;
}

static void setSensorOffsets(const uint8_t *calibData) {
  adafruit_bno055_opmode_t lastMode = _mode;
  setMode(OPERATION_MODE_CONFIG);
  delay_ms(25);

  /* Note: Configuration will take place only when user writes to the last
     byte of each config data pair (ex. ACCEL_OFFSET_Z_MSB_ADDR, etc.).
     Therefore the last byte must be written whenever the user wants to
     changes the configuration. */

  /* A writeLen() would make this much cleaner */
  write8(ACCEL_OFFSET_X_LSB_ADDR, calibData[0]);
  write8(ACCEL_OFFSET_X_MSB_ADDR, calibData[1]);
  write8(ACCEL_OFFSET_Y_LSB_ADDR, calibData[2]);
  write8(ACCEL_OFFSET_Y_MSB_ADDR, calibData[3]);
  write8(ACCEL_OFFSET_Z_LSB_ADDR, calibData[4]);
  write8(ACCEL_OFFSET_Z_MSB_ADDR, calibData[5]);

  write8(MAG_OFFSET_X_LSB_ADDR, calibData[6]);
  write8(MAG_OFFSET_X_MSB_ADDR, calibData[7]);
  write8(MAG_OFFSET_Y_LSB_ADDR, calibData[8]);
  write8(MAG_OFFSET_Y_MSB_ADDR, calibData[9]);
  write8(MAG_OFFSET_Z_LSB_ADDR, calibData[10]);
  write8(MAG_OFFSET_Z_MSB_ADDR, calibData[11]);

  write8(GYRO_OFFSET_X_LSB_ADDR, calibData[12]);
  write8(GYRO_OFFSET_X_MSB_ADDR, calibData[13]);
  write8(GYRO_OFFSET_Y_LSB_ADDR, calibData[14]);
  write8(GYRO_OFFSET_Y_MSB_ADDR, calibData[15]);
  write8(GYRO_OFFSET_Z_LSB_ADDR, calibData[16]);
  write8(GYRO_OFFSET_Z_MSB_ADDR, calibData[17]);

  write8(ACCEL_RADIUS_LSB_ADDR, calibData[18]);
  write8(ACCEL_RADIUS_MSB_ADDR, calibData[19]);

  write8(MAG_RADIUS_LSB_ADDR, calibData[20]);
  write8(MAG_RADIUS_MSB_ADDR, calibData[21]);

  setMode(lastMode);
}

static void getVector(adafruit_vector_type_t vector_type, float *data) {
  uint8_t buffer[6];
  memset(buffer, 0, 6*sizeof(uint8_t));

  int16_t x, y, z;
  x = y = z = 0;

  /* Read vector data (6 bytes) */
  readLen((adafruit_bno055_reg_t)vector_type, buffer, 6);

  x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
  y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
  z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

  /*!
   * Convert the value to an appropriate range (section 3.6.4)
   * and assign the value to the Vector type
   */
  switch (vector_type) {
  case VECTOR_MAGNETOMETER:
    /* 1uT = 16 LSB */
    data[0] = ((double)x) / 16.0;
    data[1] = ((double)y) / 16.0;
    data[2] = ((double)z) / 16.0;
    break;
  case VECTOR_GYROSCOPE:
    /* 1dps = 16 LSB */
    data[0] = ((double)x) / 16.0;
    data[1] = ((double)y) / 16.0;
    data[2] = ((double)z) / 16.0;
    break;
  case VECTOR_EULER:
    /* 1 degree = 16 LSB */
    data[0] = ((double)x) / 16.0;
    data[1] = ((double)y) / 16.0;
    data[2] = ((double)z) / 16.0;
    break;
  case VECTOR_ACCELEROMETER:
    /* 1m/s^2 = 100 LSB */
    data[0] = ((double)x) / 100.0;
    data[1] = ((double)y) / 100.0;
    data[2] = ((double)z) / 100.0;
    break;
  case VECTOR_LINEARACCEL:
    /* 1m/s^2 = 100 LSB */
    data[0] = ((double)x) / 100.0;
    data[1] = ((double)y) / 100.0;
    data[2] = ((double)z) / 100.0;
    break;
  case VECTOR_GRAVITY:
    /* 1m/s^2 = 100 LSB */
    data[0] = ((double)x) / 100.0;
    data[1] = ((double)y) / 100.0;
    data[2] = ((double)z) / 100.0;
    break;
  }
}

enum status_code getHeading(COMP_Reading *reading) {
	/* Clear the reading */
	memset(reading, 0, sizeof(sensors_orientation_t));
	
	reading->type = COMP_HEADING;

	/* Get a Euler angle sample for orientation */
	float euler[3];
	getVector(VECTOR_EULER, euler);
	reading->data.heading.heading = euler[0];
	reading->data.heading.roll = euler[1];
	reading->data.heading.pitch = euler[2];
	
	return STATUS_OK;
}

/* Function for IMU */

enum status_code bno055_init(void)
{
	if(init_flag)
	{
		return STATUS_ERR_ALREADY_INITIALIZED;
	}
	
	// Return if an error occurred while initializing the I2C driver
	switch (I2C_Init()) {
		case STATUS_OK:
		case STATUS_ERR_ALREADY_INITIALIZED:
		break;
		default:
		return STATUS_ERR_DENIED;
	}
	
	delay_ms(500);
	
	uint8_t slave_addr;
	setMode(OPERATION_MODE_NDOF);
	// Read from IMU and get it's slave address:
	if (read8(BNO055_CHIP_ID_ADDR, &slave_addr) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Return if an incorrect slave address is provided
	if (slave_addr != BNO055_ID) {
		delay_ms(1000); //wait for boot
		if (read8(BNO055_CHIP_ID_ADDR, &slave_addr) != STATUS_OK) {
			return STATUS_ERR_DENIED;
		}
		if (slave_addr != BNO055_ID) {
			return STATUS_ERR_DENIED;
		}
	}
	
	/* Set mode of BNO055 */
	_mode = OPERATION_MODE_NDOF;
	write8(BNO055_OPR_MODE_ADDR, _mode);
	//I2C_WriteBuffer(I2C_IMU, &_mode, 1, I2C_WRITE_NORMAL);
	
	/* Reset */
	write8(BNO055_SYS_TRIGGER_ADDR, 0x20);
	
	delay_ms(30);
	read8(BNO055_CHIP_ID_ADDR, slave_addr);
	while (slave_addr != BNO055_ID) {
		read8(BNO055_CHIP_ID_ADDR, slave_addr);
		delay_ms(10);
	}
	delay_ms(50);

	/* Set to normal power mode */
	write8(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
	delay_ms(10);

	write8(BNO055_PAGE_ID_ADDR, 0);
	
	write8(BNO055_SYS_TRIGGER_ADDR, 0x0);
	delay_ms(10);
	
	/* Set the operating mode */
	write8(BNO055_OPR_MODE_ADDR, _mode);
	delay_ms(20);
	
	init_flag = true;
	
	return STATUS_OK;
}

#define TEST_IMU_DELAY_MS 1000

void Test_IMU(void){
	
	TickType_t testDelay = pdMS_TO_TICKS(TEST_IMU_DELAY_MS);

	//if(bno055_init() != STATUS_OK){
		//DEBUG_Write("\n\r<<<<< Failed IMU initialization >>>>>n\r");
	//}


	COMP_Reading reading;
	int heading, roll, pitch;
	//
	//uint8_t bno_id = 0;
	//uint8_t reg_addr = BNO055_CHIP_ID_ADDR;
	
	uint8_t reg_addr = BNO055_TEMP_ADDR;
	
	uint8_t mode_buffer[2] = {BNO055_OPR_MODE_ADDR, OPERATION_MODE_ACCONLY};
	I2C_WriteBuffer(I2C_IMU, mode_buffer, 2, I2C_WRITE_NORMAL);
	
	uint8_t temp;
	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		
		DEBUG_Write("<<<<<<<<<<< Testing IMU >>>>>>>>>>\n\r");
		//getHeading(&reading);
		//
		//heading = reading.data.heading.heading;
		//roll = reading.data.heading.roll;
		//pitch = reading.data.heading.pitch;
		//
		//DEBUG_Write("The magnetic heading is: %d\r\n", heading);
		//DEBUG_Write("The roll is: %d\r\n", roll);
		//DEBUG_Write("The pitch is: %d\r\n", pitch);
		
		//I2C_WriteBuffer(I2C_IMU, &reg_addr, 1, I2C_WRITE_NORMAL);
		//I2C_ReadBuffer(I2C_IMU, &bno_id, 1, I2C_READ_NORMAL);
		//DEBUG_Write("Bno ID: %02X\r\n", bno_id);
		
		//read8(0x34, &temp);
		I2C_WriteBuffer(I2C_IMU, &reg_addr, 1, I2C_WRITE_NORMAL);
		I2C_ReadBuffer(I2C_IMU, &temp, 1, I2C_READ_NORMAL);
		DEBUG_Write("Temp: %d\r\n", temp);
		
		vTaskDelay(testDelay);
	}
}