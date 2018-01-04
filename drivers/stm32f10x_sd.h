/*******************************************************************************
 * @File:			stm32f10x_sd.h
 * @Description:	SD driver file
 * @Author:			Aytac Dilek
 * @Date:			02.01.2018
 ******************************************************************************
 * @Release Notes:
 ******************************************************************************/

#ifndef __STM32F10X_SD_H
#define __STM32F10X_SD_H

/* Includes *******************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "stm32f10x_bsp_delay.h"

#include "ff.h"
#include "diskio.h"

#include "dataflash.h"

#include <stdio.h> // for debugging


/* Defines ********************************************************************/
#define SD_ENABLE_SPI1			1
#define SD_ENABLE_SPI2			2

#if defined(SD_ENABLE_SPI1)
	#define SD_SPI_PORT					SPI1
	#define SD_SPI_CLOCK				RCC_APB2Periph_SPI1

	#define SD_SPI_SCK_PIN				GPIO_Pin_5                  /* PA.5 */
	#define SD_SPI_SCK_GPIO_PORT		GPIOA                       /* GPIOA */
	#define SD_SPI_SCK_GPIO_CLK			RCC_APB2Periph_GPIOA

	#define SD_SPI_MISO_PIN				GPIO_Pin_6                  /* PA.6 */
	#define SD_SPI_MISO_GPIO_PORT		GPIOA                       /* GPIOA */
	#define SD_SPI_MISO_GPIO_CLK		RCC_APB2Periph_GPIOA

	#define SD_SPI_MOSI_PIN				GPIO_Pin_7                  /* PA.7 */
	#define SD_SPI_MOSI_GPIO_PORT		GPIOA                       /* GPIOA */
	#define SD_SPI_MOSI_GPIO_CLK		RCC_APB2Periph_GPIOA

	#define SD_SPI_CS_PIN				GPIO_Pin_5                  /* PB.6 */
	#define SD_SPI_CS_GPIO_PORT			GPIOB                       /* GPIOB */
	#define SD_SPI_CS_GPIO_CLK			RCC_APB2Periph_GPIOB
#elif defined(SD_ENABLE_SPI2)
	#define SD_SPI_PORT					SPI2
	#define SD_SPI_CLOCK				RCC_APB1Periph_SPI2

	#define SD_SPI_SCK_PIN				GPIO_Pin_13                  /* PA.5 */
	#define SD_SPI_SCK_GPIO_PORT		GPIOB                       /* GPIOA */
	#define SD_SPI_SCK_GPIO_CLK			RCC_APB2Periph_GPIOB

	#define SD_SPI_MISO_PIN				GPIO_Pin_14                  /* PA.6 */
	#define SD_SPI_MISO_GPIO_PORT		GPIOB                       /* GPIOA */
	#define SD_SPI_MISO_GPIO_CLK		RCC_APB2Periph_GPIOB

	#define SD_SPI_MOSI_PIN				GPIO_Pin_15                  /* PA.7 */
	#define SD_SPI_MOSI_GPIO_PORT		GPIOB                       /* GPIOA */
	#define SD_SPI_MOSI_GPIO_CLK		RCC_APB2Periph_GPIOB

	#define SD_SPI_CS_PIN				GPIO_Pin_5                  /* PB.6 */
	#define SD_SPI_CS_GPIO_PORT			GPIOB                       /* GPIOB */
	#define SD_SPI_CS_GPIO_CLK			RCC_APB2Periph_GPIOB
#endif

#define SPI_SPEED_2   0
#define SPI_SPEED_4   1
#define SPI_SPEED_8   2
#define SPI_SPEED_16  3
#define SPI_SPEED_256 4

// for vs1003b
#define SPI_SPEED_LOW   0
#define SPI_SPEED_HIGH  1

//#define STM32_SD_USE_DMA
#define _FS_DEBUG_


#ifdef STM32_SD_USE_DMA
#define DMA_Channel_SPI_SD_RX    DMA1_Channel4
#define DMA_Channel_SPI_SD_TX    DMA1_Channel5
#define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC4
#define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC5
//#define GPIO_SPI_SD              GPIOB
//#define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_13
//#define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_14
//#define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_15
//#define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB1PeriphClockCmd
//#define RCC_APBPeriph_SPI_SD     RCC_APB1Periph_SPI2
#endif

#define GO_IDLE_STATE			0
#define SEND_OP_COND			1
#define SEND_IF_COND			8
#define SET_BLOCKLEN			16
#define READ_BLOCK				17
#define WRITE_BLOCK				24
#define APP_CMD					55
#define READ_OCR				58
#define SD_SEND_OP_COND			(0x40|41)

// SD data transmission whether to release the bus after the end of the macro definition
#define NO_RELEASE 0
#define RELEASE 1

// SD Card Type Definition
#define SD_TYPE_MMC 1
#define SD_TYPE_V1 2
#define SD_TYPE_V2 3
#define SD_TYPE_V2HC 4

// SD Card instruction sheet
#define CMD0 0 // Card Reset
#define CMD1 1
#define CMD9 9 // command 9, CSD data read
#define CMD10 10 // Command 10, read CID data
#define CMD12 12 // command 12, to stop data transmission
#define CMD16 16 // Command 16, set SectorSize should return 0x00
#define CMD17 17 // Command 17, read sector
#define CMD18 18 // Command 18, read Multi sector
#define ACMD23 23 // Command 23, set the multi-sector erase writing in advance of a block N
#define CMD24 24 // Command 24, write sector
#define CMD25 25 // Command 25, write Multi sector
#define ACMD41 41 // command to 41, should return 0x00
#define CMD55 55 // command to 55, should return 0x01
#define CMD58 58 // Command 58, read OCR information
#define CMD59 59 // command to 59, enables / disables the CRC, should return 0x00

// Write data to respond to the word meaning
#define MSD_DATA_OK 0x05
#define MSD_DATA_CRC_ERROR 0x0B
#define MSD_DATA_WRITE_ERROR 0x0D
#define MSD_DATA_OTHER_ERROR 0xFF

// SD card labeled word response
#define MSD_RESPONSE_NO_ERROR 0x00
#define MSD_IN_IDLE_STATE 0x01
#define MSD_ERASE_RESET 0x02
#define MSD_ILLEGAL_COMMAND 0x04
#define MSD_COM_CRC_ERROR 0x08
#define MSD_ERASE_SEQUENCE_ERROR 0x10
#define MSD_ADDRESS_ERROR 0x20
#define MSD_PARAMETER_ERROR 0x40
#define MSD_RESPONSE_FAILURE 0xFF

#define SD_MEMORY_RETRY_COUNT 255//400


/* Macro Definitions **********************************************************/
// SD Card Selection; CS control
#define SD_CS_ENABLE()     		GPIO_ResetBits(SD_SPI_CS_GPIO_PORT, SD_SPI_CS_PIN)
#define SD_CS_DISABLE()     	GPIO_SetBits(SD_SPI_CS_GPIO_PORT, SD_SPI_CS_PIN)


/* Enumerations ***************************************************************/
typedef enum {
	NO_CARD,
	CARD_MMC,
	CARD_SD,
	CARD_SD2,
	CARD_SDHC,
	SPI_FLASHM
} MemoryType_Enum;




/* Structures *****************************************************************/



/* Global Variables ***********************************************************/
FATFS Fatfs[1];
extern u8 SD_Type; // SD card type
extern int g_mkfs_done;
extern int g_sdcard_done;


/* Global Functions ***********************************************************/
MemoryType_Enum mmc_mount(void);
uint8_t flash_mount();

/* Physical Level Functions */
uint8_t sd_init(void);
void sd_setSpiSpeed(uint8_t speed);
uint8_t sd_readWriteByte(uint8_t tx_data);

/* SD Level Functions */
FRESULT sd_getMountedMemorySize(uint8_t mount_ret, uint32_t * totalSize, uint32_t * availableSize);


// Function state area
void bsp_set_spi1_speed_mmcsd(u16 prescaler);
u8 SD_WaitReady (void); // SD card ready to wait
u8 SD_SendCommand (u8 cmd, u32 arg, u8 crc); // SD card to send a command
u8 SD_SendCommand_NoDeassert (u8 cmd, u32 arg, u8 crc);
u8 SD_Idle_Sta (void); // set the SD card into suspend mode

u8 SD_ReceiveData (u8 * data, u16 len, u8 release); // SD card reader data
u8 SD_GetCID (u8 * cid_data); // reading SD card CID
u8 SD_GetCSD (u8 * csd_data); // reading SD card CSD
u32 SD_GetCapacity (void); // check SD card capacity

// USB SD card reader operation function
u8 MSD_WriteBuffer (u8 * pBuffer, u32 WriteAddr, u32 NumByteToWrite);
u8 MSD_ReadBuffer (u8 * pBuffer, u32 ReadAddr, u32 NumByteToRead);

u8 SD_ReadSingleBlock (u32 sector, u8 * buffer); // read a sector
u8 SD_WriteSingleBlock (u32 sector, const u8 * buffer); // write a sector
u8 SD_ReadMultiBlock (u32 sector, u8 * buffer, u8 count); // read multiple sector
u8 SD_WriteMultiBlock (u32 sector, const u8 * data, u8 count); // write multiple sector
u8 SD_Read_Bytes (unsigned long address, unsigned char * buf, unsigned int offset, unsigned int bytes); // read a byte
u8 SD_WaitReady(void);

#endif	// __STM32F10X_SD_H
