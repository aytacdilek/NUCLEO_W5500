/*******************************************************************************
* File  		:	stm32f10x_bsp_usart2.h
* Description	: 	STM32F10x library for HMC5883L 3-Axis Digital Compass
* Author		: 	Aytac Dilek
* Note			: 	GNU General Public License, version 3 (GPL-3.0)
*******************************************************************************/

#ifndef __STM32F10X_BSP_USART2_H
#define __STM32F10X_BSP_USART2_H



/* Includes *******************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

#include "stdbool.h"







/* Defines ********************************************************************/
#define		UART2_RCC_CLOCK		RCC_APB2Periph_USART2
#define 	UART2_GPIO_CLOCK	RCC_APB2Periph_GPIOA

#define 	USART2_BAUDRATE		115200


#define 	USART2_BUFFER_SIZE		100

#define		DEBUG_ENABLE_USART



/* Enumarations ***************************************************************/




/* Structures *****************************************************************/
typedef struct{
	uint8_t rx_buffer[USART2_BUFFER_SIZE];
	uint8_t buffer_head;
}USART2_Buffer_TypeDef;



/* Global Variables ***********************************************************/
USART2_Buffer_TypeDef USART2_Buffer;



/* Global Functions ***********************************************************/
void usart2_init(void);
void usart2_sendByte(uint8_t data);
void usart2_sendString(char *ptr);
void usart2_clearBuffer(void);
uint8_t* usart2_readString(void);




//--------------------------------------------------------------
#endif // __STM32F10X_BSP_USART2_H
