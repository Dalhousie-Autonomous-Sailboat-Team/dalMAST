/* sail_uart.c
 * Implementation of the UART driver module for the autonomous sailboat project.
 * Created on June 13, 2016.
 * Created by Thomas Gwynne-Timothy.
 */
#include "sail_uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sail_tasksinit.h"

#include <asf.h>
#include <string.h>

#include "sail_buffer.h"
#include "sail_debug.h"

//#define UART_RX_BUFFER_LENGTH		1024
#define UART_RX_BUFFER_LENGTH		512
//#define UART_TX_BUFFER_LENGTH		1024
#define UART_TX_BUFFER_LENGTH		512

// Buffers to hold receive and transmit data
static volatile uint8_t rx_buffers[UART_NUM_CHANNELS][UART_RX_BUFFER_LENGTH];
static volatile uint8_t tx_buffers[UART_NUM_CHANNELS][UART_TX_BUFFER_LENGTH];
// Buffer modules for the receiver and transmitter
static BUFF_Module rx_buff_modules[UART_NUM_CHANNELS];
static BUFF_Module tx_buff_modules[UART_NUM_CHANNELS];
// Temporary receive and transmit words
static uint16_t rx_words[UART_NUM_CHANNELS];
static uint16_t tx_words[UART_NUM_CHANNELS];

// Define array of UART modules
static struct usart_module uart_modules[UART_NUM_CHANNELS];

// Define constant arrays for the settings of each UART port

static uint32_t MUX1_baud_rates[] = {
	
};

static uint32_t MUX2_baud_rates[] = {
	
};

static uint32_t baud_rates[] = {	
	9600,	//MUX1 default
	9600,	//MUX2 default
	19200,	//windvane baud
	57600,	//vcom baud
	57600,	//beacon baud
	-1,		//spaceholder
	
	x,		//mppt 1 baud
	x,		//mppt 2 baud
	x,		//bms 1 baud
	x,		//bms 2 baud
	-1,		//spaceholder
	
	9600,	//radio baud
	9600,	//gps baud
	x,		//pixie baud
	57600,	//extra communication baud (arbitrary)
	-1		//spaceholder
};


#ifdef PCB

/*
	FYI:
	
	-Pin Mux Settings can only be set based on the table found at the link below
	-https://asf.microchip.com/docs/latest/saml21/html/asfdoc_sam0_sercom_usart_mux_settings.html
	-This table outlines the only way the TX and RX pins can be used on pads for SERCOM.
	-!!!Check this table before applying pin changes on schematic!!!

*/

static enum usart_signal_mux_settings mux_settings[] = {
	USART_RX_3_TX_2_XCK_3,	//MUX 1		SERCOM4 pads used
	USART_RX_3_TX_2_XCK_3,	//MUX 2		SERCOM3 pads used
	USART_RX_1_TX_0_XCK_1,	//Windvane	SERCOM0 pads used
	USART_RX_1_TX_0_XCK_1,	//VCOM		SERCOM5 pads used
	USART_RX_1_TX_0_XCK_1	//Beacon	SERCOM2 pads used
};



static uint32_t pinmux_pads[][UART_NUM_CHANNELS] = {
	{PINMUX_UNUSED, PINMUX_UNUSED, PINMUX_PB10D_SERCOM4_PAD2, PINMUX_PB11D_SERCOM4_PAD3},	//MUX 1		PINMUX DECLARATION
	{PINMUX_UNUSED, PINMUX_UNUSED, PINMUX_PA18D_SERCOM3_PAD2, PINMUX_PA19D_SERCOM3_PAD3},	//MUX 2		PINMUX DECLARATION
	{PINMUX_PA04D_SERCOM0_PAD0, PINMUX_PA05D_SERCOM0_PAD1, PINMUX_UNUSED, PINMUX_UNUSED},	//Windvane	PINMUX DECLARATION
	{PINMUX_PB02D_SERCOM5_PAD0, PINMUX_PB03D_SERCOM5_PAD1, PINMUX_UNUSED, PINMUX_UNUSED},	//VCOM		PINMUX DECLARATION
	{PINMUX_PA12C_SERCOM2_PAD0, PINMUX_PA13C_SERCOM2_PAD1, PINMUX_UNUSED, PINMUX_UNUSED}	//Beacon	PINMUX DECLARATION
};

static Sercom *const sercom_ptrs[] = {
	SERCOM4,
	SERCOM3,
	SERCOM0,
	SERCOM5,
	SERCOM2
};

#else
// Dev Board
static enum usart_signal_mux_settings mux_settings[] = {
	USART_RX_1_TX_0_XCK_1,
	USART_RX_1_TX_0_XCK_1,
	USART_RX_1_TX_0_XCK_1,
	USART_RX_3_TX_2_XCK_3
};

static uint32_t pinmux_pads[][UART_NUM_CHANNELS] = {
	{PINMUX_PA04D_SERCOM0_PAD0, PINMUX_PA05D_SERCOM0_PAD1, PINMUX_UNUSED, PINMUX_UNUSED},
	{PINMUX_PB08D_SERCOM4_PAD0, PINMUX_PB09D_SERCOM4_PAD1, PINMUX_UNUSED, PINMUX_UNUSED},
	{PINMUX_PB16C_SERCOM5_PAD0, PINMUX_PB17C_SERCOM5_PAD1, PINMUX_UNUSED, PINMUX_UNUSED},
	{PINMUX_UNUSED, PINMUX_UNUSED, PINMUX_PA24C_SERCOM3_PAD2, PINMUX_PA25C_SERCOM3_PAD3}
};

static Sercom *const sercom_ptrs[] = {
	SERCOM3,
	SERCOM4,
	SERCOM2,
	SERCOM5,
	SERCOM5
};
#endif

// Receiver states
typedef enum UART_RxStates {
	UART_RX_OFF,
	UART_RX_DISABLED,
	UART_RX_ENABLED
} UART_RxState;

// Default receiver states
static UART_RxState rx_states[UART_NUM_CHANNELS] = {
	UART_RX_OFF,
	UART_RX_OFF,
	UART_RX_OFF,
	UART_RX_OFF
};

// Transmitter states
typedef enum UART_TxStates {
	UART_TX_OFF,
	UART_TX_IDLE,
	UART_TX_BUSY
} UART_TxState;

// Default transmitter states
static UART_TxState tx_states[UART_NUM_CHANNELS] = {
	UART_TX_OFF,
	UART_TX_OFF,
	UART_TX_OFF,
	UART_TX_OFF
};

// Device specific callbacks
static void MUX1_RxCallback(struct usart_module * const usart_module);
static void MUX1_TxCallback(struct usart_module * const usart_module);
static void MUX2_RxCallback(struct usart_module * const usart_module);
static void MUX2_TxCallback(struct usart_module * const usart_module);
static void WIND_RxCallback(struct usart_module * const usart_module);
static void WIND_TxCallback(struct usart_module * const usart_module);
static void VCOM_RxCallback(struct usart_module * const usart_module);
static void VCOM_TxCallback(struct usart_module * const usart_module);
static void XEOS_RxCallback(struct usart_module * const usart_module);
static void XEOS_TxCallback(struct usart_module * const usart_module);

// Generic callbacks
static void UART_RxCallback(UART_ChannelID id);
static void UART_TxCallback(UART_ChannelID id);

// Callback pointer array
static usart_callback_t RxCallbacks[] = {
	MUX1_RxCallback,
	MUX2_RxCallback,
	WIND_RxCallback,
	VCOM_RxCallback,
	XEOS_RxCallback
};

// Callback pointer array
static usart_callback_t TxCallbacks[] = {
	MUX1_TxCallback,	
	MUX2_TxCallback,	
	WIND_TxCallback,
	VCOM_TxCallback,
	XEOS_TxCallback
};

enum UART_ChannelID UART_MUX_ChannelID(UART_ChannelID id){
	
	if(id > UART_NUM_CHANNELS && id < UART_MUX1_CHANNELS){
		return UART_MUX1;
	}else if(id > UART_MUX1_CHANNELS && id < UART_MUX2_CHANNELS){
		return UART_MUX2;
	}else{
		return id;
	}
}

enum status_code UART_Init(UART_ChannelID id) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Return if the ID is invalid
	if (MUXED_ID >= UART_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if the receive buffer module cannot be initialized
	if (BUFF_Init(&rx_buff_modules[MUXED_ID], &rx_buffers[MUXED_ID][0], UART_RX_BUFFER_LENGTH) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}	
	
	// Return if the transmit buffer module cannot be initialized
	if (BUFF_Init(&tx_buff_modules[MUXED_ID], &tx_buffers[MUXED_ID][0], UART_TX_BUFFER_LENGTH) != STATUS_OK) {
		return STATUS_ERR_DENIED;
	}	
	
	// Get config struct for UART
	struct usart_config uart_config;
	usart_get_config_defaults(&uart_config);
	
	// Select settings for specified UART
	uart_config.baudrate = baud_rates[id];
	uart_config.mux_setting = mux_settings[MUXED_ID];
	uart_config.pinmux_pad0 = pinmux_pads[MUXED_ID][0];
	uart_config.pinmux_pad1 = pinmux_pads[MUXED_ID][1];
	uart_config.pinmux_pad2 = pinmux_pads[MUXED_ID][2];
	uart_config.pinmux_pad3 = pinmux_pads[MUXED_ID][3];
	
	// Apply the settings to the UART module
	while (usart_init(&uart_modules[MUXED_ID], sercom_ptrs[MUXED_ID], &uart_config) != STATUS_OK);
	usart_enable(&uart_modules[MUXED_ID]);
	
	// Register and enable callbacks
	usart_register_callback(&uart_modules[MUXED_ID], RxCallbacks[MUXED_ID], USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&uart_modules[MUXED_ID], USART_CALLBACK_BUFFER_RECEIVED);
	
	usart_register_callback(&uart_modules[MUXED_ID], TxCallbacks[MUXED_ID], USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&uart_modules[MUXED_ID], USART_CALLBACK_BUFFER_TRANSMITTED);
		
	// Set the state
	rx_states[MUXED_ID] = UART_RX_DISABLED;
	tx_states[MUXED_ID] = UART_TX_IDLE;
	
	return STATUS_OK;	
}

enum status_code UART_Enable(UART_ChannelID id) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Return if the ID is invalid
	if (MUXED_ID >= UART_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if the receiver has not been initialized
	if (rx_states[MUXED_ID] == UART_RX_OFF) {
		return STATUS_ERR_NOT_INITIALIZED;
	}	
	
	// Return if the receiver has already been enabled
	if (rx_states[MUXED_ID] == UART_RX_ENABLED) {
		return STATUS_NO_CHANGE;
	}
	
	// Set the state
	rx_states[MUXED_ID] = UART_RX_ENABLED;
	
	// Start interrupt-driven receive
	usart_read_job(&uart_modules[MUXED_ID], &rx_words[MUXED_ID]);
	
	return STATUS_OK;	
}

enum status_code UART_Disable(UART_ChannelID id) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Return if the ID is invalid
	if (MUXED_ID >= UART_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if the receiver has not been initialized
	if (rx_states[MUXED_ID] == UART_RX_OFF) {
		return STATUS_ERR_NOT_INITIALIZED;
	}
	
	// Return if the receiver has already been disabled
	if (rx_states[MUXED_ID] == UART_RX_DISABLED) {
		return STATUS_NO_CHANGE;
	}

	// Set the state
	rx_states[MUXED_ID] = UART_RX_DISABLED;
	
	// Abort the job
	usart_abort_job(&uart_modules[MUXED_ID], USART_TRANSCEIVER_RX);
	
	return STATUS_OK;
}

enum status_code UART_TxString(UART_ChannelID id, uint8_t *data) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Return if the ID is invalid
	if (MUXED_ID >= UART_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Get the length of the provided string (must be null-terminated)
	uint16_t length = strlen((char *)data);
	
	// Write the data into the buffer 
	xSemaphoreTake(write_buffer_mutex[MUXED_ID], portMAX_DELAY);
	BUFF_WriteBuffer(&tx_buff_modules[MUXED_ID], data, length);
	xSemaphoreGive(write_buffer_mutex[MUXED_ID]);
	
	// If the transmitter is idle, manually trigger a new transmission
	if (tx_states[MUXED_ID] == UART_TX_IDLE) {
		tx_states[MUXED_ID] = UART_TX_BUSY;
		UART_TxCallback(MUXED_ID);
	}
	
	// Otherwise, return and let the callback send the new data

	return STATUS_OK;	
}

enum status_code UART_TxString_Unprotected(UART_ChannelID id, uint8_t *data) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Return if the ID is invalid
	if (MUXED_ID >= UART_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Get the length of the provided string (must be null-terminated)
	uint16_t length = strlen((char *)data);
	
	// Write the data into the buffer
	BUFF_WriteBufferUnprotected(&tx_buff_modules[MUXED_ID], data, length);
	
	// If the transmitter is idle, manually trigger a new transmission
	if (tx_states[MUXED_ID] == UART_TX_IDLE) {
		tx_states[MUXED_ID] = UART_TX_BUSY;
		UART_TxCallback(MUXED_ID);
	}
	
	// Otherwise, return and let the callback send the new data
	//UART_TxCallback(id);
	return STATUS_OK;
}

enum status_code UART_RxString(UART_ChannelID id, uint8_t *data, uint16_t length) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Return if the ID is invalid
	if (MUXED_ID >= UART_NUM_CHANNELS) {
		return STATUS_ERR_INVALID_ARG;
	}
	
	// Return if a null pointer is provided
	if (data == NULL) {
		return STATUS_ERR_BAD_ADDRESS;
	}
	
	// Try to get a string from the buffer
	switch (BUFF_ReadString(&rx_buff_modules[MUXED_ID], data, length)) {
		case STATUS_VALID_DATA:
			return STATUS_VALID_DATA;
		case STATUS_NO_CHANGE:
			return STATUS_NO_CHANGE;
		case STATUS_ERR_NO_MEMORY:
			return STATUS_ERR_NO_MEMORY;
		default:
			return STATUS_ERR_DENIED;
	}
}


// **** Receive callbacks ******************************************************************

void MUX1_RxCallback(struct usart_module * const usart_module){
	UART_RxCallback(UART_MUX1);
}

void MUX2_RxCallback(struct usart_module * const usart_module){
	UART_RxCallback(UART_MUX2);
}

void WIND_RxCallback(struct usart_module * const usart_module){
	UART_RxCallback(UART_WIND);
}

void VCOM_RxCallback(struct usart_module * const usart_module){
	UART_RxCallback(UART_VCOM);
}

void XEOS_RxCallback(struct usart_module * const usart_module){
	UART_RxCallback(UART_XEOS);
}

// **** Transmit callbacks ******************************************************************

void MUX1_TxCallback(struct usart_module * const usart_module){
	UART_TxCallback(UART_MUX1);
}

void MUX2_TxCallback(struct usart_module * const usart_module){
	UART_TxCallback(UART_MUX2);
}

void WIND_TxCallback(struct usart_module * const usart_module){
	UART_TxCallback(UART_WIND);
}

void VCOM_TxCallback(struct usart_module * const usart_module){
	UART_TxCallback(UART_VCOM);
}

void XEOS_TxCallback(struct usart_module * const usart_module){
	UART_TxCallback(UART_XEOS);
}

// **** Generic callbacks ******************************************************************

void UART_RxCallback(UART_ChannelID id) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Write the byte to the buffer
	BUFF_WriteByte(&rx_buff_modules[MUXED_ID], (uint8_t)(0xff & rx_words[MUXED_ID]));
	
	// If the receiver is enabled, keep receiving
	if (rx_states[MUXED_ID] == UART_RX_ENABLED) {
		usart_read_job(&uart_modules[MUXED_ID], &rx_words[MUXED_ID]);
	}
}

void UART_TxCallback(UART_ChannelID id) {
	enum UART_ChannelID MUXED_ID = UART_MUX_ChannelID(id);
	
	// Get the length of the buffer
	uint16_t length;
	BUFF_GetLength(&tx_buff_modules[MUXED_ID], &length);
	
	// Return if there is no data in the buffer
	if (length == 0) {
		tx_states[MUXED_ID] = UART_TX_IDLE;
		return;
	}
	
	// Get a window of data from the buffer
	uint8_t temp;
	BUFF_ReadByte(&tx_buff_modules[MUXED_ID], &temp);
	tx_words[MUXED_ID] = temp;
	
	// Start a transmit job to send the window of data
	usart_write_job(&uart_modules[MUXED_ID], &tx_words[MUXED_ID]);
}



