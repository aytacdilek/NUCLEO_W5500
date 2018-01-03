/*******************************************************************************
 * @File:			stm32f10x_bsp_spi.h
 * @Description:	None
 * @Author:			Aytac Dilek
 * @Date:			05.09.2016
 ******************************************************************************
 * @Release Notes:
 ******************************************************************************/

#ifndef __STM32F10X_W5500_H
#define __STM32F10X_W5500_H

#include "stm32f10x_conf.h"
#include <stdio.h>
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "spiHandler.h"
#include "boardutil.h"
#include "wizchip_conf.h"




/* Defines ********************************************************************/
#define W5500_ENABLE_SPI1	1
#define W5500_ENABLE_SPI2	2
//#define STM32_WIZCHIP_USE_DMA

/* W5500 SPI Interface pins */
#if defined(W5500_ENABLE_SPI1)
	#define W5500_SPI_PORT					SPI1
	#define W5500_SPI_CLOCK					RCC_APB2Periph_SPI1

	#define W5500_SPI_SCK_PIN               GPIO_Pin_5                  /* PB.5 */
	#define W5500_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */
	#define W5500_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOA

	#define W5500_SPI_MISO_PIN              GPIO_Pin_6                  /* PB.6 */
	#define W5500_SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */
	#define W5500_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOA

	#define W5500_SPI_MOSI_PIN              GPIO_Pin_7                  /* PB.7 */
	#define W5500_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
	#define W5500_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOA

	#define W5500_SPI_CS_PIN              	GPIO_Pin_6                  /* PB.6 */
	#define W5500_SPI_CS_GPIO_PORT        	GPIOB                       /* GPIOB */
	#define W5500_SPI_CS_GPIO_CLK         	RCC_APB2Periph_GPIOB

	#define W5500_SPI_RST_PIN              	GPIO_Pin_9                  /* PA.9 */
	#define W5500_SPI_RST_GPIO_PORT        	GPIOA                       /* GPIOA */
	#define W5500_SPI_RST_GPIO_CLK         	RCC_APB2Periph_GPIOA

	#define W5500_SPI_INT_PIN              	GPIO_Pin_7                  /* PC.7 */
	#define W5500_SPI_INT_GPIO_PORT        	GPIOC                       /* GPIOC */
	#define W5500_SPI_INT_GPIO_CLK         	RCC_APB2Periph_GPIOC
#elif defined(W5500_ENABLE_SPI2)
	#define W5500_SPI_PORT               	SPI2
	#define W5500_SPI_CLOCK             	RCC_APB1Periph_SPI2

	#define W5500_SPI_SCK_PIN               GPIO_Pin_13                  /* PB.13 */
	#define W5500_SPI_SCK_GPIO_PORT         GPIOB                       /* GPIOB */
	#define W5500_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOB

	#define W5500_SPI_MISO_PIN              GPIO_Pin_14                  /* PB.14 */
	#define W5500_SPI_MISO_GPIO_PORT        GPIOB                       /* GPIOB */
	#define W5500_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOB

	#define W5500_SPI_MOSI_PIN              GPIO_Pin_15                  /* PB.15 */
	#define W5500_SPI_MOSI_GPIO_PORT        GPIOB                       /* GPIOB */
	#define W5500_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOB

	#define W5500_SPI_CS_PIN            	GPIO_Pin_6                  /* PB.6 */
	#define W5500_SPI_CS_GPIO_PORT       	GPIOB                       /* GPIOB */
	#define W5500_SPI_CS_GPIO_CLK        	RCC_APB2Periph_GPIOB

	#define W5500_SPI_RST_PIN              	GPIO_Pin_9                  /* PA.9 */
	#define W5500_SPI_RST_GPIO_PORT        	GPIOA                       /* GPIOA */
	#define W5500_SPI_RST_GPIO_CLK         	RCC_APB2Periph_GPIOA

	#define W5500_SPI_INT_PIN              	GPIO_Pin_7                  /* PC.7 */
	#define W5500_SPI_INT_GPIO_PORT        	GPIOC                       /* GPIOC */
	#define W5500_SPI_INT_GPIO_CLK         	RCC_APB2Periph_GPIOC
#endif


#define W5500_DUMMY_BYTE		0xFF

#ifdef STM32_WIZCHIP_USE_DMA
	#define DMA_Channel_SPI_WIZCHIP_RX    	DMA1_Channel4
	#define DMA_Channel_SPI_WIZCHIP_TX    	DMA1_Channel5
	#define DMA_FLAG_SPI_WIZCHIP_TC_RX    	DMA1_FLAG_TC4
	#define DMA_FLAG_SPI_WIZCHIP_TC_TX    	DMA1_FLAG_TC5
#endif


/* Macro Definitions **********************************************************/



/* Enumerations ***************************************************************/



/* Structures *****************************************************************/



/* Global Variables ***********************************************************/



/* Global Functions ***********************************************************/
void w5500_init(void);
void  wizchip_write(uint8_t wb);
uint8_t wizchip_read(void);




#endif	// __STM32F10X_W5500_H
