/*******************************************************************************
 * @File:			stm32f10x_bsp_delay.h
 * @Description:	Delay Functions
 * @Author:			Aytac Dilek
 * @Date:			04.18.2016
 ******************************************************************************
 * @Release Notes:
 ******************************************************************************/

#ifndef __STM32F10X_BSP_DELAY_H
#define __STM32F10X_BSP_DELAY_H

/* Includes *******************************************************************/
#include "stm32f10x.h"


/* Defines ********************************************************************/
#define DELAY_ENABLE_LOOP
//#define DELAY_ENABLE_SYSTICK

#define DELAY_TICK_FREQUENCY_US 1000000   /* = 1MHZ -> microseconds delay */
#define DELAY_TICK_FREQUENCY_MS 1000      /* = 1kHZ -> milliseconds delay */


/* Macros *********************************************************************/



/* Enumarations ***************************************************************/



/* Global Variables ***********************************************************/
__IO uint32_t TimingDelay; // __IO -- volatile


/* Global Functions ***********************************************************/
void delay_init(void);
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
uint32_t millis(void);


#endif	// __STM32F10X_BSP_DELAY_H
