/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>
#include <string.h>

enum UART_ChannelIDs	{
	GPSMod,
	WINDMod,
	RADIOMod,
	DEBUGMod
};

const char GPSGreeting[] = {"GPS Uart"};
const char WindGreeting[] = {"Wind Uart"};
const char DebugGreeting[] = {"Debug Uart"};
const char RadioGreeting[] = {"Radio Uart"};
	
	
// Function Prototypes

void RxCallBack(const struct usart_module *const usart_module);
void TxCallBack(const struct usart_module *const usart_module);
void ConfigUARTs(void);
bool CheckIt(uint8_t,volatile uint8_t*);

// Instantiate Module Instances
struct usart_module GPSModule;	// Config'd for SERCOM0
struct usart_module WINDModule;	// Config'd for SERCOM4
struct usart_module RADIOModule;// Config'd for SERCOM1
struct usart_module DBGModule; ;// Config'd for SERCOM6

// Define Queues and flags
#define BUFFER_LENGTH   128

volatile uint8_t GPSRxBuffer[BUFFER_LENGTH];
volatile uint8_t GPSTxBuffer[BUFFER_LENGTH];
volatile uint8_t GPSReturn[BUFFER_LENGTH];
volatile uint8_t RxFlagGPS, TxFlagGPS;

volatile uint8_t DBGRxBuffer[BUFFER_LENGTH];
volatile uint8_t DBGReturn[BUFFER_LENGTH];
volatile uint8_t DBGTxBuffer[BUFFER_LENGTH];
volatile uint8_t RxFlagDBG, TxFlagDBG;

volatile uint8_t WINDRxBuffer[BUFFER_LENGTH];
volatile uint8_t WINDTxBuffer[BUFFER_LENGTH];
volatile uint8_t WINDReturn[BUFFER_LENGTH];
volatile uint8_t RxFlagWIND, TxFlagWIND;

volatile uint8_t RADIORxBuffer[BUFFER_LENGTH];
volatile uint8_t RADIOTxBuffer[BUFFER_LENGTH];
volatile uint8_t RADIOReturn[BUFFER_LENGTH];
volatile uint8_t RxFlagRADIO, TxFlagRADIO;


// *********************************************************
//
// *************   Callback Functions **********************
// To Do Calculate checksum
// *********************************************************

void RxCallBack(const struct usart_module *const mod) {
	volatile int Check, tmp;
	volatile uint8_t r, Sum;
	volatile uint8_t  *Buffptr, *RxBuffptr, *Flag;
	// Determine Queues by Module ID.
	switch(mod->ID) {
		case GPSMod:
			RxBuffptr = GPSReturn;
			Flag = &RxFlagGPS;
			break;
		case WINDMod:
			RxBuffptr = WINDReturn;
			Flag = &RxFlagWIND;
			break;
		case RADIOMod:
			RxBuffptr = RADIOReturn;
			Flag = &RxFlagRADIO;
			break;
		default:	// DEBUGModule:
			RxBuffptr = DBGReturn;
			Flag = &RxFlagDBG;
			break;
		
		}
	r = Sum = 0;
	Buffptr = mod->StartOfRXBuffer;
	while(*Buffptr++ != '$') {
		if(++r == mod->RXBufferLength)
		break;
		}
	if(r < mod->RXBufferLength) {
		while(*Buffptr != '*') {
			Sum ^= *Buffptr;
			*RxBuffptr++ = *Buffptr++;
			if(++r == mod->RXBufferLength)
			break;
			}
		
		if(r < mod->RXBufferLength) {
			Buffptr++;
			tmp=*Buffptr++;
			Check = tmp - (tmp > 64 ? 55 : 48);
			Check *=  16;
			tmp=*Buffptr;
			Check += tmp - (tmp > 64 ? 55 : 48);

			if(Check == Sum) {
				*RxBuffptr++ = '\0';
				*Flag = 1;
				}
			}
		}
}




void TxCallBack(const struct usart_module *const mod) {
	volatile uint8_t  *Flag;
	switch(mod->ID) {
		case GPSMod:
			Flag = &TxFlagGPS;
			break;
		case WINDMod:
			Flag = &TxFlagWIND;
			break;
		case RADIOMod:
			Flag = &TxFlagRADIO;
			break;
		default:	// DEBUGMod:
			Flag = &TxFlagDBG;
			break;
		}
	*Flag = 1;
}


// **************   UART configuration *******************
// For each channel the default configuration is modified
// with the desired baud rate and Pin-MUX setting.  The
// The Module is initialized and a channel ID is assigned.
// Call backs are then registered and enabled.
// *******************************************************

void ConfigUARTs(void) {
	struct usart_config Uart;

	usart_get_config_defaults(&Uart);
	
	Uart.baudrate    = 9600;
	Uart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	Uart.pinmux_pad0 = PINMUX_UNUSED;
	Uart.pinmux_pad1 = PINMUX_UNUSED;
	Uart.pinmux_pad2 = PINMUX_PA24C_SERCOM3_PAD2;
	Uart.pinmux_pad3 = PINMUX_PA25C_SERCOM3_PAD3;


	while (usart_init(&DBGModule, EDBG_CDC_MODULE, &Uart) != STATUS_OK) {};
	usart_enable(&DBGModule);
	DBGModule.ID =  DEBUGMod;



//  ********************** Wind Vane UART ************************
//  Windvane on SERCOM4 Module	<<Tx On PB08>> <<Rx on PB09>>


	Uart.baudrate    = 9600;
	Uart.mux_setting = USART_RX_1_TX_0_XCK_1;
	Uart.pinmux_pad0 = PINMUX_PB08D_SERCOM4_PAD0;
	Uart.pinmux_pad1 = PINMUX_PB09D_SERCOM4_PAD1;
	Uart.pinmux_pad2 = PINMUX_UNUSED;
	Uart.pinmux_pad3 = PINMUX_UNUSED;
	

	while (usart_init(&WINDModule, SERCOM4, &Uart) != STATUS_OK) {};
	usart_enable(&WINDModule);
	WINDModule.ID = WINDMod;


//  ******************** GPS UART ************************
//  GPS on SERCOM0 Module	<<Tx On PA04>> <<Rx on PA05>>

	Uart.baudrate    = 9600;
	Uart.mux_setting = USART_RX_1_TX_0_XCK_1;
	Uart.pinmux_pad0 = PINMUX_PA04D_SERCOM0_PAD0;
	Uart.pinmux_pad1 = PINMUX_PA05D_SERCOM0_PAD1;

	while (usart_init(&GPSModule, SERCOM0, &Uart) != STATUS_OK) {};
	usart_enable(&GPSModule);
	GPSModule.ID = GPSMod;
	

//  ********************* Radio UART ********************
//  Radio on SERCOM1 Module	<<Tx On PA16>> <<Rx on PA17>>

	
	Uart.baudrate    = 9600;
	Uart.mux_setting = USART_RX_1_TX_0_XCK_1;
	Uart.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0;
	Uart.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1;

	while (usart_init(&RADIOModule, SERCOM1, &Uart) != STATUS_OK) {};
	usart_enable(&RADIOModule);
	RADIOModule.ID = RADIOMod;
	
	// Register and enable Callbacks.
	usart_register_callback(&GPSModule, TxCallBack, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&GPSModule, RxCallBack, USART_CALLBACK_BUFFER_RECEIVED);
	usart_register_callback(&DBGModule, TxCallBack, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&DBGModule, RxCallBack, USART_CALLBACK_BUFFER_RECEIVED);
	usart_register_callback(&WINDModule, TxCallBack, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&WINDModule, RxCallBack, USART_CALLBACK_BUFFER_RECEIVED);
	usart_register_callback(&RADIOModule, TxCallBack, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&RADIOModule, RxCallBack, USART_CALLBACK_BUFFER_RECEIVED);

	usart_enable_callback(&GPSModule, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&GPSModule, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&DBGModule, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&DBGModule, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&WINDModule, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&WINDModule, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&RADIOModule, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&RADIOModule, USART_CALLBACK_BUFFER_RECEIVED);
}

int main (void) {
	system_init();
	RxFlagGPS = TxFlagGPS = RxFlagWIND = TxFlagWIND = 0;
	RxFlagDBG = TxFlagDBG = 0, RxFlagRADIO = TxFlagRADIO = 0;	
	bool LastLEDState;

	ConfigUARTs();
	system_interrupt_enable_global();

	LastLEDState = LED_0_ACTIVE;

	strcpy((char*) GPSTxBuffer, GPSGreeting);
	strcpy((char*) WINDTxBuffer, WindGreeting);
	strcpy((char*) DBGTxBuffer, DebugGreeting);
	strcpy((char*) RADIOTxBuffer, RadioGreeting);
		
	usart_write_buffer_job(&GPSModule, (uint8_t*) GPSTxBuffer, sizeof(GPSTxBuffer));
	usart_write_buffer_job(&WINDModule, (uint8_t*) WINDTxBuffer, sizeof(WINDTxBuffer));
	usart_write_buffer_job(&DBGModule, (uint8_t*) DBGTxBuffer, sizeof(DBGTxBuffer));
	usart_write_buffer_job(&RADIOModule, (uint8_t*) RADIOTxBuffer, sizeof(RADIOTxBuffer));
	
	port_pin_set_output_level(LED_0_PIN, LastLEDState);

	usart_read_buffer_job(&DBGModule,(uint8_t *)GPSRxBuffer, BUFFER_LENGTH);
	usart_read_buffer_job(&WINDModule,(uint8_t *)GPSRxBuffer, BUFFER_LENGTH);
	usart_read_buffer_job(&GPSModule,(uint8_t *)GPSRxBuffer, BUFFER_LENGTH);
	usart_read_buffer_job(&RADIOModule,(uint8_t *)GPSRxBuffer, BUFFER_LENGTH);
	
	while (true) {
		if(RxFlagGPS == 1){
			usart_write_buffer_job(&DBGModule, (uint8_t*) GPSReturn, sizeof(GPSReturn));
			if (LastLEDState == LED_0_ACTIVE)
			// LED Off, so turn LED on.
			LastLEDState = !LED_0_ACTIVE;
			else {	// No, then turn it off.
				LastLEDState = LED_0_ACTIVE;
				}
			port_pin_set_output_level(LED_0_PIN, LastLEDState);

			RxFlagGPS = 0;	// Do it again
			}
		if(RxFlagWIND == 1){
			usart_write_buffer_job(&DBGModule, (uint8_t*) WINDReturn, sizeof(WINDReturn));
			if (LastLEDState == LED_0_ACTIVE)
			// LED Off, so turn LED on.
			LastLEDState = !LED_0_ACTIVE;
			else {	// No, then turn it off.
				LastLEDState = LED_0_ACTIVE;
				}
			port_pin_set_output_level(LED_0_PIN, LastLEDState);

			RxFlagWIND = 0;	// Do it again
			}
		if(RxFlagRADIO == 1){
			usart_write_buffer_job(&DBGModule, (uint8_t*) RADIOReturn, sizeof(RADIOReturn));
			if (LastLEDState == LED_0_ACTIVE)
			// LED Off, so turn LED on.
			LastLEDState = !LED_0_ACTIVE;
			else {	// No, then turn it off.
				LastLEDState = LED_0_ACTIVE;
				}
			port_pin_set_output_level(LED_0_PIN, LastLEDState);

			RxFlagRADIO = 0;	// Do it again
			}
		if(RxFlagDBG == 1){
			strcpy((char*) GPSTxBuffer, (char*) DBGReturn);
			strcpy((char*) WINDTxBuffer, (char*) DBGReturn);
			strcpy((char*) RADIOTxBuffer, (char*) DBGReturn);
			usart_write_buffer_job(&GPSModule, (uint8_t*) GPSTxBuffer, sizeof(GPSTxBuffer));
			usart_write_buffer_job(&WINDModule, (uint8_t*) WINDTxBuffer, sizeof(WINDTxBuffer));
			usart_write_buffer_job(&RADIOModule, (uint8_t*) RADIOTxBuffer, sizeof(RADIOTxBuffer));
			if (LastLEDState == LED_0_ACTIVE)
			// LED Off, so turn LED on.
			LastLEDState = !LED_0_ACTIVE;
			else {	// No, then turn it off.
				LastLEDState = LED_0_ACTIVE;
				}
			port_pin_set_output_level(LED_0_PIN, LastLEDState);

			RxFlagDBG = 0;	// Do it again
			}
		if(TxFlagGPS == 1)
			TxFlagGPS = 0;
		if(TxFlagDBG == 1)
			TxFlagDBG = 0;
		if(TxFlagWIND == 1)
			TxFlagWIND = 0;

		}
}
