#include <asf.h>
#include "WindVaneDriver.h"


/******************************************************************************
******************************************************************************/
static void usart_callback(struct usart_module *const module)
{		
	//// copy it into a FIFO
	//for(int i = 0; i < STRING_LEN; i++)
		//QueuePut(rx_string[i], &QueueGPS);
}

/******************************************************************************
******************************************************************************/
void init_windvane_link(void)
{

	//usart_get_config_defaults(&usart_config);
	//usart_config.mux_setting = EXT1_UART_SERCOM_MUX_SETTING;
	//usart_config.pinmux_pad0 = PINMUX_PB08D_SERCOM4_PAD0; // used for TX
	//usart_config.pinmux_pad1 = PINMUX_PB09D_SERCOM4_PAD1; // 
	//usart_config.pinmux_pad2 = PINMUX_UNUSED;
	//usart_config.pinmux_pad3 = PINMUX_UNUSED;
	//usart_config.baudrate    = GPS_SPEED;
	///* Apply configuration */
	//usart_init(&usart_module, SERCOM0, &usart_config);
	///* Enable USART */
	//usart_enable(&usart_module);
//
	///* Configure TX USART */
///*	usart_get_config_defaults(&usart_tx_config);
	//usart_tx_config.mux_setting = CONF_TX_USART_SERCOM_MUX;
	//usart_tx_config.pinmux_pad0 = CONF_TX_USART_PINMUX_PAD0;
	//usart_tx_config.pinmux_pad1 = CONF_TX_USART_PINMUX_PAD1;
	//usart_tx_config.pinmux_pad2 = CONF_TX_USART_PINMUX_PAD2;
	//usart_tx_config.pinmux_pad3 = CONF_TX_USART_PINMUX_PAD3;
	//usart_tx_config.baudrate    = TEST_USART_SPEED;
	//// Apply configuration 
	//usart_init(&usart_tx_module, CONF_TX_USART, &usart_tx_config);
	//// Enable USART 
	//usart_enable(&usart_tx_module);*/
	//
	//usart_register_callback(&usart_module, usart_callback,
		//USART_CALLBACK_BUFFER_RECEIVED);
	//usart_enable_callback(&usart_module, 
		//USART_CALLBACK_BUFFER_RECEIVED);
//
	//transfer_complete = false;
	//
	//QueueInit(&QueueGPS);
}

/******************************************************************************
******************************************************************************/
void receive_windvane_usart(void)
{
//	volatile uint8_t tx_string[TEST_STRING_LEN] = TEST_STRING;
//	usart_read_buffer_job(&usart_module, (uint8_t*)rx_string, STRING_LEN);


}

// renvoie l'angle de direction du vent absolu (ie : par rapport au nord)
int getGirouette(int ancien, int nord){
   //int angle,tmp = 0;
   //int c1,c2,c3,c4,relatif;
   //int temp32=0;
   //
   //while(temp32<300){
      //if(kbhit(girouette_stream)){
         //tmp++;
      //}
         //restart_wdt();
         //delay_ms(1);
      //temp32++;  
      //if(tmp>3)
      //temp32=-1;
   //}
   //if(tmp>3){
      //while(fgetc(girouette_stream)!='M'){}
         //if(fgetc(girouette_stream)=='W'){
            //if(fgetc(girouette_stream)=='V'){
                    //
               //fgetc(girouette_stream);
               //angle = 0;
               //c1 = fgetc(girouette_stream)-48;
               //c2 = fgetc(girouette_stream)-48;
               //c3 = fgetc(girouette_stream)-48;
               //fgetc(girouette_stream);
               //c4 = fgetc(girouette_stream)-48;
               //fgetc(girouette_stream);
               //relatif = fgetc(girouette_stream);
               //
               //
               //angle=c1;
               //angle = angle*10+c2;
               //angle = angle*10+c3;
               //angle = angle*10+c4;
               //
               //if(relatif=='R')
                  //if(angle<3601){
                     //angle += girouetteCor;
                     //angle += nord;
                     //return angle%3600;
                  //}
            //}
         //}
         //return ancien;
   //}
   //else{//si jamais la girouette plante
      //set_adc_channel(0);//lire la valeur du potentiomètre
      //delay_us(10);
      //temp32=read_adc();
      //temp32*=3600;
      //temp32/=256;
      //return temp32;
            //
   //}
}
