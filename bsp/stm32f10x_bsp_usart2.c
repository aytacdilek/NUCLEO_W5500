/*
 * File			:	stm32f10x_bsp_usart2.c
 * Description	:	STM32F10x library for HTU21 Digital Humidity and Temperature Sensor
 * Datum		:	2015.03.20
 * Version		:	1.0
 * Author		:	Aytac Dilek
 * email		:	aytacdilek@gmail.com
 * Web			:
 * Platform		:	OPEN103Z-B
 * CPU			:	STM32F103ZET6
 * IDE			:	CooCox CoIDE 1.7.7
 * GCC			:	4.8 2014q2
 * Module		:	HMC5883L
 * Function		:	Compassing
 */


/* Includes */
#include "stm32f10x_bsp_usart2.h"





/*******************************************************************************
* Function Name  : uart2_init
* Description    : Initialize USART2: enable interrupt on reception of a character
* Input          : None
* Output         : None
* Return         : None
* Attention		 : Internal Function
*******************************************************************************/
void usart2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* Enable clock for GPIOA, USART2, AFIO */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* GPIOA PIN9 alternative function Tx */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* GPIOA PIN10 alternative function Rx */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Enable USART2 */
	USART_Cmd(USART2, ENABLE);

	USART_InitStructure.USART_BaudRate = USART2_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	/* Configure USART2 */
	USART_Init(USART2, &USART_InitStructure);

	/* Enable RXNE interrupt */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	/* Enable USART2 global interrupt */
	NVIC_EnableIRQ(USART2_IRQn);
}



/*******************************************************************************
* Function Name  : usart2_sendByte
* Description    : Initialize USART2: enable interrupt on reception of a character
* Input          : None
* Output         : None
* Return         : None
* Attention		 : Internal Function
*******************************************************************************/
void usart2_sendByte(uint8_t data)
{
	// Wait until old bytes sent
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, data);
}


/*******************************************************************************
* Function Name  : usart2_sendByte
* Description    : Send a string via UART
* Input          : None
* Output         : None
* Return         : None
* Attention		 : Internal Function
*******************************************************************************/
void usart2_sendString(char *ptr)
{
	while(*ptr != 0xff){
		usart2_sendByte(*ptr);
		ptr++;
	}
}

//--------------------------------------------------------------
// receive a string via UART
// (the reception is dealt with by interrupt)
// this function must be polled cyclically
// Return Value :
//  -> if not received 		= 	RX_EMPTY
//  -> if string received 	= 	RX_READY -> String is in * ptr
//  -> if buffer full     	= 	RX_FULL
//--------------------------------------------------------------
uint8_t* usart2_readString(void)
{
	return USART2_Buffer.rx_buffer;
}



/*******************************************************************************
* Function Name  : usart2_sendByte
* Description    : Store the received character in the buffer
* Input          : None
* Output         : None
* Return         : None
* Attention		 : Internal Function
*******************************************************************************/
void usart2_clearBuffer(void)
{
	uint8_t i;

	for (i = 0; i < USART2_BUFFER_SIZE; ++i) {
		USART2_Buffer.rx_buffer[i] = 0;
	}
	USART2_Buffer.buffer_head = 0;
}



/*******************************************************************************
* Function Name  : usart2_sendByte
* Description    : Store the received character in the buffer
* Input          : None
* Output         : None
* Return         : None
* Attention		 : Internal Function
*******************************************************************************/
void usart2_receiveString(uint8_t value)
{
	if(USART2_Buffer.buffer_head < USART2_BUFFER_SIZE){
		USART2_Buffer.rx_buffer[USART2_Buffer.buffer_head] = value;
		USART2_Buffer.buffer_head++;
	}
}



/*******************************************************************************
* Function Name  : usart2_sendByte
* Description    : USART2-Interrupt
* Input          : None
* Output         : None
* Return         : None
* Attention		 : Internal Function
*******************************************************************************/
void USART2_IRQHandler(void) {
	uint8_t value;

	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
		// if a byte in the receive buffer is
		value = USART_ReceiveData(USART2);
		// save byte
		usart2_receiveString(value);
	}
}



