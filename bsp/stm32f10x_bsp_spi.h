/*******************************************************************************
 * @File:			stm32f10x_bsp_spi.h
 * @Description:	None
 * @Author:			Aytac Dilek
 * @Date:			05.09.2016
 ******************************************************************************
 * @Release Notes:
 ******************************************************************************/

#ifndef __STM32F10X_BSP_SPI_H
#define __STM32F10X_BSP_SPI_H

/* Includes *******************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"



/* Defines ********************************************************************/
#define SPI1_ENABLE		1
//#define SPI2_ENABLE		2



/* Macro Definitions **********************************************************/



/* Enumerations ***************************************************************/



/* Structures *****************************************************************/



/* Global Variables ***********************************************************/



/* Global Functions ***********************************************************/
void spi_init(void);
uint8_t spi1_readWrite(uint8_t writedat);
uint8_t spi2_readWrite(uint8_t writedat);


#endif	// __STM32F10X_BSP_SPI_H
