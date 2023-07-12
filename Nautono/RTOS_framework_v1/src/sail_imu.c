/*
 * sail_imu.c
 *
 * Created: 2023-06-20 6:48:42 PM
 *  Author: Manav Sohi
 */ 


#include "sail_imu.h"

#include <asf.h>

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "sail_debug.h"
#include "sail_i2c.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sail_ctrl.h"
#include "sail_tasksinit.h"

/** BNO055 ID **/
#define BNO055_ID (0xA0)

/** Offsets registers **/
#define NUM_BNO055_OFFSET_REGISTERS (22)

#define BNO055_SAMPLERATE_DELAY_MS (100)

/** Operation mode settings **/
typedef enum {
	OPERATION_MODE_CONFIG = 0X00,
	OPERATION_MODE_ACCONLY = 0X01,
	OPERATION_MODE_MAGONLY = 0X02,
	OPERATION_MODE_GYRONLY = 0X03,
	OPERATION_MODE_ACCMAG = 0X04,
	OPERATION_MODE_ACCGYRO = 0X05,
	OPERATION_MODE_MAGGYRO = 0X06,
	OPERATION_MODE_AMG = 0X07,
	OPERATION_MODE_IMUPLUS = 0X08,
	OPERATION_MODE_COMPASS = 0X09,
	OPERATION_MODE_M4G = 0X0A,
	OPERATION_MODE_NDOF_FMC_OFF = 0X0B,
	OPERATION_MODE_NDOF = 0X0C
} adafruit_bno055_opmode_t;

/** BNO055 Registers **/
typedef enum {
	/* Page id register definition */
	BNO055_PAGE_ID_ADDR = 0X07,

	/* PAGE0 REGISTER DEFINITION START*/
	BNO055_CHIP_ID_ADDR = 0x00,
	BNO055_ACCEL_REV_ID_ADDR = 0x01,
	BNO055_MAG_REV_ID_ADDR = 0x02,
	BNO055_GYRO_REV_ID_ADDR = 0x03,
	BNO055_SW_REV_ID_LSB_ADDR = 0x04,
	BNO055_SW_REV_ID_MSB_ADDR = 0x05,
	BNO055_BL_REV_ID_ADDR = 0X06,

	/* Accel data register */
	BNO055_ACCEL_DATA_X_LSB_ADDR = 0X08,
	BNO055_ACCEL_DATA_X_MSB_ADDR = 0X09,
	BNO055_ACCEL_DATA_Y_LSB_ADDR = 0X0A,
	BNO055_ACCEL_DATA_Y_MSB_ADDR = 0X0B,
	BNO055_ACCEL_DATA_Z_LSB_ADDR = 0X0C,
	BNO055_ACCEL_DATA_Z_MSB_ADDR = 0X0D,

	/* Mag data register */
	BNO055_MAG_DATA_X_LSB_ADDR = 0X0E,
	BNO055_MAG_DATA_X_MSB_ADDR = 0X0F,
	BNO055_MAG_DATA_Y_LSB_ADDR = 0X10,
	BNO055_MAG_DATA_Y_MSB_ADDR = 0X11,
	BNO055_MAG_DATA_Z_LSB_ADDR = 0X12,
	BNO055_MAG_DATA_Z_MSB_ADDR = 0X13,

	/* Gyro data registers */
	BNO055_GYRO_DATA_X_LSB_ADDR = 0X14,
	BNO055_GYRO_DATA_X_MSB_ADDR = 0X15,
	BNO055_GYRO_DATA_Y_LSB_ADDR = 0X16,
	BNO055_GYRO_DATA_Y_MSB_ADDR = 0X17,
	BNO055_GYRO_DATA_Z_LSB_ADDR = 0X18,
	BNO055_GYRO_DATA_Z_MSB_ADDR = 0X19,

	/* Euler data registers */
	BNO055_EULER_H_LSB_ADDR = 0X1A,
	BNO055_EULER_H_MSB_ADDR = 0X1B,
	BNO055_EULER_R_LSB_ADDR = 0X1C,
	BNO055_EULER_R_MSB_ADDR = 0X1D,
	BNO055_EULER_P_LSB_ADDR = 0X1E,
	BNO055_EULER_P_MSB_ADDR = 0X1F,

	/* Quaternion data registers */
	BNO055_QUATERNION_DATA_W_LSB_ADDR = 0X20,
	BNO055_QUATERNION_DATA_W_MSB_ADDR = 0X21,
	BNO055_QUATERNION_DATA_X_LSB_ADDR = 0X22,
	BNO055_QUATERNION_DATA_X_MSB_ADDR = 0X23,
	BNO055_QUATERNION_DATA_Y_LSB_ADDR = 0X24,
	BNO055_QUATERNION_DATA_Y_MSB_ADDR = 0X25,
	BNO055_QUATERNION_DATA_Z_LSB_ADDR = 0X26,
	BNO055_QUATERNION_DATA_Z_MSB_ADDR = 0X27,

	/* Linear acceleration data registers */
	BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR = 0X28,
	BNO055_LINEAR_ACCEL_DATA_X_MSB_ADDR = 0X29,
	BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR = 0X2A,
	BNO055_LINEAR_ACCEL_DATA_Y_MSB_ADDR = 0X2B,
	BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR = 0X2C,
	BNO055_LINEAR_ACCEL_DATA_Z_MSB_ADDR = 0X2D,

	/* Gravity data registers */
	BNO055_GRAVITY_DATA_X_LSB_ADDR = 0X2E,
	BNO055_GRAVITY_DATA_X_MSB_ADDR = 0X2F,
	BNO055_GRAVITY_DATA_Y_LSB_ADDR = 0X30,
	BNO055_GRAVITY_DATA_Y_MSB_ADDR = 0X31,
	BNO055_GRAVITY_DATA_Z_LSB_ADDR = 0X32,
	BNO055_GRAVITY_DATA_Z_MSB_ADDR = 0X33,

	/* Temperature data register */
	BNO055_TEMP_ADDR = 0X34,

	/* Status registers */
	BNO055_CALIB_STAT_ADDR = 0X35,
	BNO055_SELFTEST_RESULT_ADDR = 0X36,
	BNO055_INTR_STAT_ADDR = 0X37,

	BNO055_SYS_CLK_STAT_ADDR = 0X38,
	BNO055_SYS_STAT_ADDR = 0X39,
	BNO055_SYS_ERR_ADDR = 0X3A,

	/* Unit selection register */
	BNO055_UNIT_SEL_ADDR = 0X3B,

	/* Mode registers */
	BNO055_OPR_MODE_ADDR = 0X3D,
	BNO055_PWR_MODE_ADDR = 0X3E,

	BNO055_SYS_TRIGGER_ADDR = 0X3F,
	BNO055_TEMP_SOURCE_ADDR = 0X40,

	/* Axis remap registers */
	BNO055_AXIS_MAP_CONFIG_ADDR = 0X41,
	BNO055_AXIS_MAP_SIGN_ADDR = 0X42,

	/* SIC registers */
	BNO055_SIC_MATRIX_0_LSB_ADDR = 0X43,
	BNO055_SIC_MATRIX_0_MSB_ADDR = 0X44,
	BNO055_SIC_MATRIX_1_LSB_ADDR = 0X45,
	BNO055_SIC_MATRIX_1_MSB_ADDR = 0X46,
	BNO055_SIC_MATRIX_2_LSB_ADDR = 0X47,
	BNO055_SIC_MATRIX_2_MSB_ADDR = 0X48,
	BNO055_SIC_MATRIX_3_LSB_ADDR = 0X49,
	BNO055_SIC_MATRIX_3_MSB_ADDR = 0X4A,
	BNO055_SIC_MATRIX_4_LSB_ADDR = 0X4B,
	BNO055_SIC_MATRIX_4_MSB_ADDR = 0X4C,
	BNO055_SIC_MATRIX_5_LSB_ADDR = 0X4D,
	BNO055_SIC_MATRIX_5_MSB_ADDR = 0X4E,
	BNO055_SIC_MATRIX_6_LSB_ADDR = 0X4F,
	BNO055_SIC_MATRIX_6_MSB_ADDR = 0X50,
	BNO055_SIC_MATRIX_7_LSB_ADDR = 0X51,
	BNO055_SIC_MATRIX_7_MSB_ADDR = 0X52,
	BNO055_SIC_MATRIX_8_LSB_ADDR = 0X53,
	BNO055_SIC_MATRIX_8_MSB_ADDR = 0X54,

	/* Accelerometer Offset registers */
	ACCEL_OFFSET_X_LSB_ADDR = 0X55,
	ACCEL_OFFSET_X_MSB_ADDR = 0X56,
	ACCEL_OFFSET_Y_LSB_ADDR = 0X57,
	ACCEL_OFFSET_Y_MSB_ADDR = 0X58,
	ACCEL_OFFSET_Z_LSB_ADDR = 0X59,
	ACCEL_OFFSET_Z_MSB_ADDR = 0X5A,

	/* Magnetometer Offset registers */
	MAG_OFFSET_X_LSB_ADDR = 0X5B,
	MAG_OFFSET_X_MSB_ADDR = 0X5C,
	MAG_OFFSET_Y_LSB_ADDR = 0X5D,
	MAG_OFFSET_Y_MSB_ADDR = 0X5E,
	MAG_OFFSET_Z_LSB_ADDR = 0X5F,
	MAG_OFFSET_Z_MSB_ADDR = 0X60,

	/* Gyroscope Offset register s*/
	GYRO_OFFSET_X_LSB_ADDR = 0X61,
	GYRO_OFFSET_X_MSB_ADDR = 0X62,
	GYRO_OFFSET_Y_LSB_ADDR = 0X63,
	GYRO_OFFSET_Y_MSB_ADDR = 0X64,
	GYRO_OFFSET_Z_LSB_ADDR = 0X65,
	GYRO_OFFSET_Z_MSB_ADDR = 0X66,

	/* Radius registers */
	ACCEL_RADIUS_LSB_ADDR = 0X67,
	ACCEL_RADIUS_MSB_ADDR = 0X68,
	MAG_RADIUS_LSB_ADDR = 0X69,
	MAG_RADIUS_MSB_ADDR = 0X6A
} adafruit_bno055_reg_t;

/** BNO055 power settings */
typedef enum {
	POWER_MODE_NORMAL = 0X00,
	POWER_MODE_LOWPOWER = 0X01,
	POWER_MODE_SUSPEND = 0X02
} adafruit_bno055_powermode_t;

/** Remap settings **/
typedef enum {
	REMAP_CONFIG_P0 = 0x21,
	REMAP_CONFIG_P1 = 0x24, // default
	REMAP_CONFIG_P2 = 0x24,
	REMAP_CONFIG_P3 = 0x21,
	REMAP_CONFIG_P4 = 0x24,
	REMAP_CONFIG_P5 = 0x21,
	REMAP_CONFIG_P6 = 0x21,
	REMAP_CONFIG_P7 = 0x24
} adafruit_bno055_axis_remap_config_t;

/** Remap Signs **/
typedef enum {
	REMAP_SIGN_P0 = 0x04,
	REMAP_SIGN_P1 = 0x00, // default
	REMAP_SIGN_P2 = 0x06,
	REMAP_SIGN_P3 = 0x02,
	REMAP_SIGN_P4 = 0x03,
	REMAP_SIGN_P5 = 0x01,
	REMAP_SIGN_P6 = 0x07,
	REMAP_SIGN_P7 = 0x05
} adafruit_bno055_axis_remap_sign_t;

/** A structure to represent revisions **/
typedef struct {
	uint8_t accel_rev; /**< acceleration rev */
	uint8_t mag_rev;   /**< magnetometer rev */
	uint8_t gyro_rev;  /**< gyroscope rev */
	uint16_t sw_rev;   /**< SW rev */
	uint8_t bl_rev;    /**< boot loader rev */
} adafruit_bno055_rev_info_t;

/** A structure to represent offsets **/
typedef struct {
  int16_t accel_offset_x; /**< x acceleration offset */
  int16_t accel_offset_y; /**< y acceleration offset */
  int16_t accel_offset_z; /**< z acceleration offset */

  int16_t mag_offset_x; /**< x magnetometer offset */
  int16_t mag_offset_y; /**< y magnetometer offset */
  int16_t mag_offset_z; /**< z magnetometer offset */

  int16_t gyro_offset_x; /**< x gyroscrope offset */
  int16_t gyro_offset_y; /**< y gyroscrope offset */
  int16_t gyro_offset_z; /**< z gyroscrope offset */

  int16_t accel_radius; /**< acceleration radius */

  int16_t mag_radius; /**< magnetometer radius */
} adafruit_bno055_offsets_t;


/* Static function for IMU */
static enum status_code ReadEEPROM(uint8_t addr, uint8_t *data, uint8_t len);
static enum status_code WriteEEPROM(uint8_t addr, uint8_t data);


static bool init_flag = false;
static adafruit_bno055_opmode_t _mode = OPERATION_MODE_NDOF;
static bool calibrate_flag = false;

/*!
 *	
 *
*/

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
	
	// Read from IMU and get it's slave address:
	if (ReadEEPROM(0x00, &slave_addr, 1) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Return if an incorrect slave address is provided
	if (slave_addr != BNO055_ID) {
		delay_ms(1000); //wait for boot
		if (ReadEEPROM(0x00, &slave_addr, 1) != STATUS_OK) {
			return STATUS_ERR_DENIED;
		}
		if (slave_addr != BNO055_ID) {
			return STATUS_ERR_DENIED;
		}
	}
	
	/* Set mode of BNO055 */
	_mode = OPERATION_MODE_NDOF;
	WriteEEPROM(BNO055_OPR_MODE_ADDR, _mode);
	//I2C_WriteBuffer(I2C_IMU, &_mode, 1, I2C_WRITE_NORMAL);
	
	/* Reset */
	WriteEEPROM(BNO055_SYS_TRIGGER_ADDR, 0x20);
	
	delay_ms(30);
	ReadEEPROM(BNO055_CHIP_ID_ADDR, slave_addr, 1);
	while (slave_addr != BNO055_ID) {
	ReadEEPROM(BNO055_CHIP_ID_ADDR, slave_addr, 1); 
		delay_ms(10);
	}
	delay_ms(50);

	/* Set to normal power mode */
	WriteEEPROM(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
	delay_ms(10);

	WriteEEPROM(BNO055_PAGE_ID_ADDR, 0);
	 
	WriteEEPROM(BNO055_SYS_TRIGGER_ADDR, 0x0);
	delay_ms(10);
	
	/* Set the operating mode */
	//I2C_WriteBuffer(I2C_IMU, &_mode, 1, I2C_WRITE_NORMAL);
	WriteEEPROM(BNO055_OPR_MODE_ADDR, _mode);
	delay_ms(20);
	
	init_flag = true;
	
	return STATUS_OK;
}

//static void getCalibration(CalibOffset * Calib) {
static void getCalibration(uint8_t *sys, uint8_t *gyro, uint8_t *accel, uint8_t *mag) {

	uint8_t calData = 0;

	ReadEEPROM(BNO055_CALIB_STAT_ADDR, &calData, 1);

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

// static enum status_code IMU_GetReading(COMP_ReadingType type, COMP_Reading *reading)
// {

// }

static enum status_code ReadEEPROM(uint8_t addr, uint8_t *data,  uint8_t len)
{
	// Return if the pointer is NULL
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	/* Prepare buffer with the EEPROM address to read from */
	uint8_t buffer = addr;
	
	// Write the command to the compass
	if (I2C_WriteBuffer(I2C_COMPASS, &buffer, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	// Give the compass time to prepare the data (see data sheet)
	delay_ms(10);
	
	// Read the data back from the compass
	if (I2C_ReadBuffer(I2C_COMPASS, data, len, I2C_READ_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}	
	
	return STATUS_OK;		
}

static enum status_code WriteEEPROM(uint8_t addr, uint8_t data)
{
	/* Prepare buffer with the following fields:
	 * 2. EEPROM address to write to, and 
	 * 3. Data to be written
	 */
	uint8_t buffer[2];
	buffer[0] = addr;
	buffer[1] = data;	
	
	// Write to the compass
	if (I2C_WriteBuffer(I2C_COMPASS, buffer, 2, I2C_WRITE_NORMAL) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;	
}


#define TEST_IMU_DELAY_MS 1000

void Test_IMU(void){
	
	TickType_t testDelay = pdMS_TO_TICKS(TEST_IMU_DELAY_MS);
	#ifdef TEST
	if(bno055_init() != STATUS_OK){
		DEBUG_Write("\n\r<<<<< Failed IMU initialization >>>>>n\r");
	}
	#endif
	
	uint8_t bno_id = 0;
	
	uint8_t mode_buffer[2] = {BNO055_OPR_MODE_ADDR, OPERATION_MODE_ACCONLY};
		
	uint8_t reg_addr = BNO055_TEMP_ADDR;
	//i2c_send_stop();
	I2C_WriteBuffer(I2C_IMU, mode_buffer, 2, I2C_WRITE_NORMAL);

	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;
		#ifdef TEST
		DEBUG_Write("\n\r<<<<<<<<<<< Testing IMU >>>>>>>>>>\n\r");
		
		WriteEEPROM(BNO055_OPR_MODE_ADDR, OPERATION_MODE_NDOF);
		
		//uint8_t calibData;
		
		//ReadEEPROM(ACCEL_OFFSET_X_LSB_ADDR, &calibData, NUM_BNO055_OFFSET_REGISTERS);
		ReadEEPROM(0x00, &bno_id, 1);
		
		DEBUG_Write("Bno ID: %d\r\n", bno_id);
		#endif
		
		I2C_WriteBuffer(I2C_IMU, &reg_addr, 1, I2C_WRITE_NORMAL);
		I2C_ReadBuffer(I2C_IMU, &bno_id, 1, I2C_READ_NORMAL);
		DEBUG_Write("Bno ID: %d\r\n", bno_id);
		//uint8_t buffer[2] = {0x00, 0xAA};
		//I2C_WriteBuffer(I2C_IMU, buffer, 2, I2C_WRITE_NORMAL);
		
		vTaskDelay(testDelay);
	}
}