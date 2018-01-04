/*******************************************************************************
 * @File:			stm32f10x_bsp_spi.c
 * @Author:			Aytac Dilek
 * @email:			aytacdilek@gmail.com
 * @Web:
 * @Date:			03.16.2016
 * @Description:	STM32F10x library for
 * @Version:		1.0
 * @Platform:		NUCLEO-F103RB
 * @CPU:			STM32F103RB
 * @IDE:			CooCox CoIDE 1.7.8
 * @GCC:			4.8 2014q2
 * @Module:			spi
 * @Function:		Read shunt, current and power readings
 * @Notes:			None
 * @Copyright:		Copyright (C) Aytac Dilek, 2016
 * 					This program is free software: you can redistribute it
 * 					and/or modify it under the terms of the GNU General Public
 * 					License as published by the Free Software Foundation, either
 * 					version 3 of the License, or any later version.
 *
 * 					This program is distributed in the hope that it will be
 * 					useful, but WITHOUT ANY WARRANTY; without even the implied
 * 					warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * 					PURPOSE.  See the GNU General Public License for more
 * 					details.
 *
 * 					You should have received a copy of the GNU General Public
 * 					License along with this program.  If not,
 * 					see <http://www.gnu.org/licenses/>.
*******************************************************************************/

/* Includes *******************************************************************/
#include "stm32f10x_bsp_spi.h"



/* Internal Functions *********************************************************/



/* Global Functions ***********************************************************/

/*******************************************************************************
* Function:		spi_init
* Description:	Initialize ...
* Input:		None
* Output:		None
* Return:		None
* Attention:	None
*******************************************************************************/
void spi_init(void){
#if defined(SPI1_ENABLE)
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO Clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure MOSI Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure MISO Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure SCK Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* Configure CS Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, GPIO_Pin_12);

	SPI_InitTypeDef  SPI_InitStructure;

	/* Enable SPI2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* SPI2 configuration */
	/* Data bytes on both MOSI and MISO lines are sent with the MSB first. Data on both MOSI
	and MISO lines must be stable on the rising edge of the clock and can be changed on the
	falling edge. Data is provided by the MFRC522 on the falling clock edge and is stable
	during the rising clock edge. */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 72Mhz / 8 = 9Mhz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI2  */
	SPI_Cmd(SPI1, ENABLE);
#endif

#if defined(SPI2_ENABLE)
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO Clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure MOSI Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure MISO Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure SCK Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/* Configure CS Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_12);

	SPI_InitTypeDef  SPI_InitStructure;

	/* Enable SPI2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* SPI2 configuration */
	/* Data bytes on both MOSI and MISO lines are sent with the MSB first. Data on both MOSI
	and MISO lines must be stable on the rising edge of the clock and can be changed on the
	falling edge. Data is provided by the MFRC522 on the falling clock edge and is stable
	during the rising clock edge. */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 72Mhz / 8 = 9Mhz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI2  */
	SPI_Cmd(SPI2, ENABLE);
#endif
}



uint8_t spi1_readWrite(uint8_t writedat){
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI1, writedat);

	/* Wait to receive a byte */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}



uint8_t spi2_readWrite(uint8_t writedat){
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI2, writedat);

	/* Wait to receive a byte */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI2);
}
