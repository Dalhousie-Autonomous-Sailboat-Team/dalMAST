#include "sail_anglesensor.h"

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


/*
 * Constants from the AS5600 Driver
 * 
 */

#define AS5600_LIB_VERSION              (F("0.3.7"))

//  default addresses
const uint8_t AS5600_DEFAULT_ADDRESS    = 0x36;
const uint8_t AS5600L_DEFAULT_ADDRESS   = 0x40;
const uint8_t AS5600_SW_DIRECTION_PIN   = 255;

//  setDirection
const uint8_t AS5600_CLOCK_WISE         = 0;  //  LOW
const uint8_t AS5600_COUNTERCLOCK_WISE  = 1;  //  HIGH

//  0.087890625;
const float   AS5600_RAW_TO_DEGREES     = 360.0 / 4096;
const float   AS5600_DEGREES_TO_RAW     = 4096 / 360.0;
//  0.00153398078788564122971808758949;
const float   AS5600_RAW_TO_RADIANS     = M_PI * 2.0 / 4096;
//  4.06901041666666e-6
const float   AS5600_RAW_TO_RPM         = 60.0 / 4096;

//  getAngularSpeed
const uint8_t AS5600_MODE_DEGREES       = 0;
const uint8_t AS5600_MODE_RADIANS       = 1;
const uint8_t AS5600_MODE_RPM           = 2;

//  CONFIGURE CONSTANTS
//  check datasheet for details

//  setOutputMode
const uint8_t AS5600_OUTMODE_ANALOG_100 = 0;
const uint8_t AS5600_OUTMODE_ANALOG_90  = 1;
const uint8_t AS5600_OUTMODE_PWM        = 2;

//  setPowerMode
const uint8_t AS5600_POWERMODE_NOMINAL  = 0;
const uint8_t AS5600_POWERMODE_LOW1     = 1;
const uint8_t AS5600_POWERMODE_LOW2     = 2;
const uint8_t AS5600_POWERMODE_LOW3     = 3;

//  setPWMFrequency
const uint8_t AS5600_PWM_115            = 0;
const uint8_t AS5600_PWM_230            = 1;
const uint8_t AS5600_PWM_460            = 2;
const uint8_t AS5600_PWM_920            = 3;

//  setHysteresis
const uint8_t AS5600_HYST_OFF           = 0;
const uint8_t AS5600_HYST_LSB1          = 1;
const uint8_t AS5600_HYST_LSB2          = 2;
const uint8_t AS5600_HYST_LSB3          = 3;

//  setSlowFilter
const uint8_t AS5600_SLOW_FILT_16X      = 0;
const uint8_t AS5600_SLOW_FILT_8X       = 1;
const uint8_t AS5600_SLOW_FILT_4X       = 2;
const uint8_t AS5600_SLOW_FILT_2X       = 3;

//  setFastFilter
const uint8_t AS5600_FAST_FILT_NONE     = 0;
const uint8_t AS5600_FAST_FILT_LSB6     = 1;
const uint8_t AS5600_FAST_FILT_LSB7     = 2;
const uint8_t AS5600_FAST_FILT_LSB9     = 3;
const uint8_t AS5600_FAST_FILT_LSB18    = 4;
const uint8_t AS5600_FAST_FILT_LSB21    = 5;
const uint8_t AS5600_FAST_FILT_LSB24    = 6;
const uint8_t AS5600_FAST_FILT_LSB10    = 7;

//  setWatchDog
const uint8_t AS5600_WATCHDOG_OFF       = 0;
const uint8_t AS5600_WATCHDOG_ON        = 1;

//  CONFIGURATION REGISTERS
const uint8_t AS5600_ZMCO = 0x00;
const uint8_t AS5600_ZPOS = 0x01;   //  + 0x02
const uint8_t AS5600_MPOS = 0x03;   //  + 0x04
const uint8_t AS5600_MANG = 0x05;   //  + 0x06
const uint8_t AS5600_CONF = 0x07;   //  + 0x08

//  CONFIGURATION BIT MASKS - byte level
const uint8_t AS5600_CONF_POWER_MODE    = 0x03;
const uint8_t AS5600_CONF_HYSTERESIS    = 0x0C;
const uint8_t AS5600_CONF_OUTPUT_MODE   = 0x30;
const uint8_t AS5600_CONF_PWM_FREQUENCY = 0xC0;
const uint8_t AS5600_CONF_SLOW_FILTER   = 0x03;
const uint8_t AS5600_CONF_FAST_FILTER   = 0x1C;
const uint8_t AS5600_CONF_WATCH_DOG     = 0x20;


//  UNKNOWN REGISTERS 0x09-0x0A

//  OUTPUT REGISTERS
const uint8_t AS5600_RAW_ANGLE = 0x0C;   //  + 0x0D
const uint8_t AS5600_ANGLE     = 0x0E;   //  + 0x0F

// I2C_ADDRESS REGISTERS (AS5600L)
const uint8_t AS5600_I2CADDR   = 0x20;
const uint8_t AS5600_I2CUPDT   = 0x21;

//  STATUS REGISTERS
const uint8_t AS5600_STATUS    = 0x0B;
const uint8_t AS5600_AGC       = 0x1A;
const uint8_t AS5600_MAGNITUDE = 0x1B;   //  + 0x1C
const uint8_t AS5600_BURN      = 0xFF;

//  STATUS BITS
const uint8_t AS5600_MAGNET_HIGH   = 0x08;
const uint8_t AS5600_MAGNET_LOW    = 0x10;
const uint8_t AS5600_MAGNET_DETECT = 0x20;


/************************************************************************/
/* Static Variables for AS5600                                          */
/************************************************************************/

static uint16_t _offset;
static uint8_t _directionPin;
static uint8_t _direction;
static uint32_t _lastMeasurement;
static int _lastAngle;

/*
 *  Functions for AS5600 
 * 
 */

static enum status_code WriteByte(uint8_t reg, uint8_t val);
static enum status_code WriteWord(uint8_t reg, uint16_t val);
static enum status_code ReadByte(uint8_t reg, uint8_t * data);
static enum status_code ReadWord(uint8_t reg, uint16_t * data);

static enum status_code WriteByte(uint8_t reg, uint8_t val)
{
   uint8_t buffer[2];
   buffer[1] = reg;
   buffer[2] = val;

   if(I2C_WriteBuffer(I2C_AS, buffer, 2, I2C_WRITE_NORMAL) != STATUS_OK) {
        return STATUS_ERR_DENIED;
   }
}

static enum status_code WriteWord(uint8_t reg, uint16_t val)
{
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = val & 0xFF;
    buffer[2] = (val >> 8) & 0xFF;

    if(I2C_WriteBuffer(I2C_AS, buffer, 3, I2C_WRITE_NORMAL) != STATUS_OK) {
        return STATUS_ERR_DENIED;
   }
}

static enum status_code ReadByte(uint8_t reg, uint8_t * data)
{
   if(data == NULL) {
    return STATUS_ERR_BAD_DATA;
   }

   if(I2C_WriteBuffer(I2C_AS, &reg, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
      return STATUS_ERR_DENIED;
   }

   if(I2C_ReadBuffer(I2C_AS, data, 1, I2C_READ_NORMAL) != STATUS_OK) {
      return STATUS_ERR_DENIED;
   }

   return STATUS_OK;
}

static enum status_code ReadWord(uint8_t reg, uint16_t *data)
{
   if(data == NULL) {
    return STATUS_ERR_BAD_DATA;
   }

   if(I2C_WriteBuffer(I2C_AS, &reg, 1, I2C_WRITE_NORMAL) != STATUS_OK) {
      return STATUS_ERR_DENIED;
   }

   if(I2C_ReadBuffer(I2C_AS, (uint8_t*)data, 2, I2C_READ_NORMAL) != STATUS_OK) {
      return STATUS_ERR_DENIED;
   }

   return STATUS_OK;
}

static enum status_code setDirection(uint8_t direction)
{
	_direction = direction;
	if (_directionPin != AS5600_SW_DIRECTION_PIN)
	{
		port_pin_set_output_level(_directionPin, _direction);
	}
	
	return STATUS_OK;
}

static enum status_code readStatus(uint8_t * data)
{
	if(data == NULL) {
		return STATUS_ERR_BAD_DATA;
	}
	
	if(ReadByte(AS5600_STATUS, data) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}
	
	return STATUS_OK;
}

static enum status_code detectMagnet()
{
	uint8_t data = 0;
	readStatus(data);
	return (data & AS5600_MAGNET_DETECT) > 1;
}

static void initPins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	port_pin_set_config(_directionPin, &config_port_pin);	
}

void AS_init(uint8_t directionPin)
{
	_directionPin = directionPin;	
	if (_directionPin != AS5600_SW_DIRECTION_PIN) 
	{
		initPins();
	}
	setDirection(AS5600_CLOCK_WISE);
}

enum status_code rawAngle(uint16_t *data)
{
	ReadWord(AS5600_RAW_ANGLE, data);
	
	if (_offset > 0) *data = (*data + _offset) & 0x0FFF;

	if ((_directionPin == AS5600_SW_DIRECTION_PIN) &&
	(_direction == AS5600_COUNTERCLOCK_WISE))
	{
		*data = (4096 - *data) & 0x0FFF;
	}
	
	return STATUS_OK;	
}

enum status_code readAngle(uint16_t *data)
{
	rawAngle(data);
	uint8_t msb = *data & 0xff;
	uint8_t lsb = (*data >> 8) & 0xff;
	*data = msb << 8 | lsb;
	
	//rawAngle(&raw_angle);
	*data = (*data)*AS5600_RAW_TO_DEGREES;
	
	return STATUS_OK;
}

static void getTicks(uint16_t * data) {
	*data = xTaskGetTickCount();
}

enum status_code getAngularSpeed(uint8_t mode, float *data)
{
	uint32_t now = 0;
	getTicks(&now);
	
	int	angle = 0;
	readAngle(&angle);
	
	uint32_t deltaT  = now - _lastMeasurement;
	int      deltaA  = angle - _lastAngle;

	//  assumption is that there is no more than 180? rotation
	//  between two consecutive measurements.
	//  => at least two measurements per rotation (preferred 4).
	if (deltaA >  2048) deltaA -= 4096;
	if (deltaA < -2048) deltaA += 4096;
	*data   = (deltaA * 1e6) / deltaT;

	//  remember last time & angle
	_lastMeasurement = now;
	_lastAngle       = angle;

	//  return radians, RPM or degrees.
	if (mode == AS5600_MODE_RADIANS)
	{
		*data * AS5600_RAW_TO_RADIANS;
	}
	if (mode == AS5600_MODE_RPM)
	{
		*data * AS5600_RAW_TO_RPM;
	}
	//  default return degrees
	*data * AS5600_RAW_TO_DEGREES;
	
	return STATUS_OK;
}

#define TEST_AS_DELAY_MS 1000

void Test_AS(void){
	
	TickType_t testDelay = pdMS_TO_TICKS(TEST_AS_DELAY_MS);
	
	uint16_t raw_angle = 0;
	uint16_t ticks = 0;
	
	// Need direction pin: 
	AS_init(PIN_PA08);
	
	enum status_code rc;
	
	while(1){
		taskENTER_CRITICAL();
		watchdog_counter |= 0x20;
		taskEXIT_CRITICAL();
		running_task = eUpdateCourse;

		DEBUG_Write_Unprotected("\n\r<<<<<<<<<<< Testing AS >>>>>>>>>>\n\r");
		readAngle(&raw_angle);
		//rawAngle(&raw_angle);
		DEBUG_Write("raw angle: %d\r\n", raw_angle);
		
		rc = detectMagnet();
		DEBUG_Write("rc: %d\r\n");
		
		vTaskDelay(testDelay);
	}
}