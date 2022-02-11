
#include <asf.h>
#include <system_interrupt.h>
#include "i2c_link.h"

void i2c_write_complete_callback(
		struct i2c_master_module *const module);
void configure_i2c(void);
void configure_i2c_callbacks(void);

//! [packet_data]
#define DATA_LENGTH 8

static uint8_t wr_buffer[DATA_LENGTH] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

static uint8_t wr_buffer_reversed[DATA_LENGTH] = {
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

static uint8_t rd_buffer[DATA_LENGTH];

#define SLAVE_ADDRESS 0x12

struct i2c_master_packet wr_packet;
struct i2c_master_packet rd_packet;

struct i2c_master_module i2c_master_instance;

/******************************************************************************
******************************************************************************/
void i2c_write_complete_callback(
		struct i2c_master_module *const module)
{

	i2c_master_read_packet_job(&i2c_master_instance,&rd_packet);
}

/******************************************************************************
******************************************************************************/
void configure_i2c(void)
{
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);


	/* Change buffer timeout to something longer */
	config_i2c_master.buffer_timeout = 65535;


	/* Initialize and enable device with config */
	while(i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master)     \
			!= STATUS_OK);
	i2c_master_enable(&i2c_master_instance);
}

/******************************************************************************
******************************************************************************/
void configure_i2c_callbacks(void)
{
	i2c_master_register_callback(&i2c_master_instance, i2c_write_complete_callback,
			I2C_MASTER_CALLBACK_WRITE_COMPLETE);

	i2c_master_enable_callback(&i2c_master_instance,
			I2C_MASTER_CALLBACK_WRITE_COMPLETE);
}


/******************************************************************************
******************************************************************************/
int init_i2c(void)
{
	system_init();
	
	configure_i2c();
	configure_i2c_callbacks();
	return 0;
}


/******************************************************************************
******************************************************************************/
int transmit(void)
{
	wr_packet.address     = SLAVE_ADDRESS;
	wr_packet.data_length = DATA_LENGTH;
	wr_packet.data        = wr_buffer;

	rd_packet.address     = SLAVE_ADDRESS;
	rd_packet.data_length = DATA_LENGTH;
	rd_packet.data        = rd_buffer;



	if (!port_pin_get_input_level(BUTTON_0_PIN)) {
		/* Send every other packet with reversed data */

		if (wr_packet.data[0] == 0x00) {
			wr_packet.data = &wr_buffer_reversed[0];
		} else {
			wr_packet.data = &wr_buffer[0];
		}
		i2c_master_write_packet_job(&i2c_master_instance, &wr_packet);

	}
	return 0;	
}
