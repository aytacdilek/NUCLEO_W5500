/*******************************************************************************
 * @File:			stm32f10x_microsd.h
 * @Description:	The SPI SD Card application function
 * @Author:			Aytac Dilek
 * @Date:			03.16.2016
 ******************************************************************************
 * @Release Notes:
 ******************************************************************************/

#ifndef __STM32F10X_MICROSD_H
#define __STM32F10X_MICROSD_H

/* Includes *******************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"

#include <stdio.h>

/* Defines ********************************************************************/
#define MICROSD_USE_SPI1		1
//#define MICROSD_USE_SPI2		2
//#define PRINT_INFO  1

#if defined(MICROSD_USE_SPI1)
	#define MICROSD_MOSI_PORT		GPIOA					// D11
	#define MICROSD_MOSI_PIN		GPIO_Pin_7
	#define MICROSD_MOSI_CLOCK		RCC_APB2Periph_GPIOA

	#define MICROSD_MISO_PORT		GPIOA					// D12
	#define MICROSD_MISO_PIN		GPIO_Pin_6
	#define MICROSD_MISO_CLOCK		RCC_APB2Periph_GPIOA

	#define MICROSD_SCK_PORT		GPIOA					// D13
	#define MICROSD_SCK_PIN			GPIO_Pin_5
	#define MICROSD_SCK_CLOCK		RCC_APB2Periph_GPIOA

	#define MICROSD_CS_PORT			GPIOB					// D4
	#define MICROSD_CS_PIN			GPIO_Pin_5
	#define MICROSD_CS_CLOCK		RCC_APB2Periph_GPIOB

	#define MICROSD_CD_PORT			GPIOA
	#define MICROSD_CD_PIN			GPIO_Pin_3
	#define MICROSD_CD_CLOCK		RCC_APB2Periph_GPIOA

	#define MICROSD_SPI_PER			SPI1
	#define MICROSD_SPI_CLOCK		RCC_APB2Periph_SPI1
#elif defined(MICROSD_USE_SPI2)
	#define MICROSD_MOSI_PORT		GPIOB					// D11
	#define MICROSD_MOSI_PIN		GPIO_Pin_15
	#define MICROSD_MOSI_CLOCK		RCC_APB2Periph_GPIOB

	#define MICROSD_MISO_PORT		GPIOB					// D12
	#define MICROSD_MISO_PIN		GPIO_Pin_14
	#define MICROSD_MISO_CLOCK		RCC_APB2Periph_GPIOB

	#define MICROSD_SCK_PORT		GPIOB					// D13
	#define MICROSD_SCK_PIN			GPIO_Pin_13
	#define MICROSD_SCK_CLOCK		RCC_APB2Periph_GPIOB

	#define MICROSD_CS_PORT			GPIOB					// D4
	#define MICROSD_CS_PIN			GPIO_Pin_1
	#define MICROSD_CS_CLOCK		RCC_APB2Periph_GPIOB

	#define MICROSD_CD_PORT			GPIOA
	#define MICROSD_CD_PIN			GPIO_Pin_3
	#define MICROSD_CD_CLOCK		RCC_APB2Periph_GPIOA

	#define MICROSD_SPI_PER			SPI2
	#define MICROSD_SPI_CLOCK		RCC_APB1Periph_SPI2
#endif

#define CARDTYPE_MMC     	     0x00
#define CARDTYPE_SDV1      	     0x01
#define CARDTYPE_SDV2      	     0x02
#define CARDTYPE_SDV2HC    	     0x04

#define DUMMY_BYTE				 0xFF
#define MSD_BLOCKSIZE			 512

/* SD/MMC command list - SPI mode */
#define CMD0                     0       /* Reset */
#define CMD1                     1       /* Send Operator Condition - SEND_OP_COND */
#define CMD8                     8       /* Send Interface Condition - SEND_IF_COND	*/
#define CMD9                     9       /* Read CSD */
#define CMD10                    10      /* Read CID */
#define CMD12                    12      /* Stop data transmit */
#define CMD16                    16      /* Set block size, should return 0x00 */
#define CMD17                    17      /* Read single block */
#define CMD18                    18      /* Read multi block */
#define ACMD23                   23      /* Prepare erase N-blokcs before multi block write */
#define CMD24                    24      /* Write single block */
#define CMD25                    25      /* Write multi block */
#define ACMD41                   41      /* should return 0x00 */
#define CMD55                    55      /* should return 0x01 */
#define CMD58                    58      /* Read OCR */
#define CMD59                    59      /* CRC disable/enbale, should return 0x00 */



/* Macro Definitions **********************************************************/
#define _card_enable()      	GPIO_ResetBits(MICROSD_CS_PORT, MICROSD_CS_PIN)
#define _card_disable()     	GPIO_SetBits(MICROSD_CS_PORT, MICROSD_CS_PIN)
#define _card_power_on()
#define _card_insert()       	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)



/* Private typedef -----------------------------------------------------------*/
enum _CD_HOLD{
	HOLD = 0,
	RELEASE = 1,
};



/* Structures *****************************************************************/
typedef struct{               /* Card Specific Data */
	vu8  CSDStruct;            /* CSD structure */
	vu8  SysSpecVersion;       /* System specification version */
	vu8  Reserved1;            /* Reserved */
	vu8  TAAC;                 /* Data read access-time 1 */
	vu8  NSAC;                 /* Data read access-time 2 in CLK cycles */
	vu8  MaxBusClkFrec;        /* Max. bus clock frequency */
	vu16 CardComdClasses;      /* Card command classes */
	vu8  RdBlockLen;           /* Max. read data block length */
	vu8  PartBlockRead;        /* Partial blocks for read allowed */
	vu8  WrBlockMisalign;      /* Write block misalignment */
	vu8  RdBlockMisalign;      /* Read block misalignment */
	vu8  DSRImpl;              /* DSR implemented */
	vu8  Reserved2;            /* Reserved */
	vu32 DeviceSize;           /* Device Size */
	vu8  MaxRdCurrentVDDMin;   /* Max. read current @ VDD min */
	vu8  MaxRdCurrentVDDMax;   /* Max. read current @ VDD max */
	vu8  MaxWrCurrentVDDMin;   /* Max. write current @ VDD min */
	vu8  MaxWrCurrentVDDMax;   /* Max. write current @ VDD max */
	vu8  DeviceSizeMul;        /* Device size multiplier */
	vu8  EraseGrSize;          /* Erase group size */
	vu8  EraseGrMul;           /* Erase group size multiplier */
	vu8  WrProtectGrSize;      /* Write protect group size */
	vu8  WrProtectGrEnable;    /* Write protect group enable */
	vu8  ManDeflECC;           /* Manufacturer default ECC */
	vu8  WrSpeedFact;          /* Write speed factor */
	vu8  MaxWrBlockLen;        /* Max. write data block length */
	vu8  WriteBlockPaPartial;  /* Partial blocks for write allowed */
	vu8  Reserved3;            /* Reserded */
	vu8  ContentProtectAppli;  /* Content protection application */
	vu8  FileFormatGrouop;     /* File format group */
	vu8  CopyFlag;             /* Copy flag (OTP) */
	vu8  PermWrProtect;        /* Permanent write protection */
	vu8  TempWrProtect;        /* Temporary write protection */
	vu8  FileFormat;           /* File Format */
	vu8  ECC;                  /* ECC code */
	vu8  CSD_CRC;              /* CSD CRC */
	vu8  Reserved4;            /* always 1*/
}MSD_CSD;

typedef struct{				 /*Card Identification Data*/
	vu8  ManufacturerID;       /* ManufacturerID */
	vu16 OEM_AppliID;          /* OEM/Application ID */
	vu32 ProdName1;            /* Product Name part1 */
	vu8  ProdName2;            /* Product Name part2*/
	vu8  ProdRev;              /* Product Revision */
	vu32 ProdSN;               /* Product Serial Number */
	vu8  Reserved1;            /* Reserved1 */
	vu16 ManufactDate;         /* Manufacturing Date */
	vu8  CID_CRC;              /* CID CRC */
	vu8  Reserved2;            /* always 1 */
}MSD_CID;

typedef struct{
	MSD_CSD CSD;
	MSD_CID CID;
	u32 Capacity;              /* Card Capacity */
	u32 BlockSize;             /* Card Block Size */
	u16 RCA;
	u8 CardType;
	u32 SpaceTotal;            /* Total space size in file system */
	u32 SpaceFree;      	     /* Free space size in file system */
}MSD_CARDINFO, *PMSD_CARDINFO;


/* Private variables ---------------------------------------------------------*/
MSD_CARDINFO CardInfo;


/* Global Functions ***********************************************************/
int microsd_init(void);
int microsd_getCardInfo(PMSD_CARDINFO cardinfo);
int microsd_readSingleBlock(uint32_t sector, uint8_t *buffer);
int microsd_readMultiBlock(uint32_t sector, uint8_t *buffer, uint32_t NbrOfSector);
int microsd_writeSingleBlock(uint32_t sector, uc8 *buffer);
int microsd_writeMultiBlock(uint32_t sector, uc8 *buffer, uint32_t NbrOfSector);
void microsd_test(void);



#endif	// __STM32F10X_MICROSD_H
