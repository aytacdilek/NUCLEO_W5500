/*******************************************************************************
 * @File:			stm32f10x_sd.c
 * @Author:			Aytac Dilek
 * @email:			aytacdilek@gmail.com
 * @Web:
 * @Date:			03.01.2018
 * @Description:	STM32F10x library for SD Memory
 * @Version:		1.0
 * @Platform:		NUCLEO-F103RB
 * @CPU:			STM32F103RB
 * @IDE:			CooCox CoIDE 1.7.8
 * @GCC:			7.2.1 2017q4
 * @Module:			spi
 * @Function:		Ethernet Controller
 * @Notes:			None
*******************************************************************************/

/* Includes *******************************************************************/
#include "stm32f10x_sd.h"

u8 SD_Type = 0; // SD card type



/* Internal Functions *********************************************************/
#ifdef STM32_SD_USE_DMA
// #warning "Information only: using DMA"
/*-----------------------------------------------------------------------*/
/* Transmit/Receive Block using DMA (Platform dependent. STM32 here)     */
/*-----------------------------------------------------------------------*/
static void stm32_dma_transfer(uint8_t receive, const BYTE *buff, uint16_t btr);
#endif


/*******************************************************************************
* Function:		sd_init
* Description:	Initialize SD card
* Input:		None
* Output:		None
* Return:		Return if successful, it will automatically set the speed to 18Mhz SPI
* 				0: NO_ERR
* 				1: TIME_OUT
* 				99: NO_CARD
* Attention:	None
*******************************************************************************/
uint8_t sd_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	/*!< SD CS/MOSI/MISO/CLK Pin GPIO Peripheral clock enable */
	RCC_APB1PeriphClockCmd(	SD_SPI_SCK_GPIO_CLK |
							SD_SPI_MISO_GPIO_CLK |
							SD_SPI_MOSI_GPIO_CLK |
							SD_SPI_CS_GPIO_CLK, ENABLE);
	/*!< SD_SPI Peripheral clock enable */
#if defined(SD_ENABLE_SPI1)
	/* Initialize GPIO Peripheral */
	RCC_APB2PeriphClockCmd(SD_SPI_CLOCK, ENABLE);
#elif defined(SD_ENABLE_SPI2)
	RCC_APB1PeriphClockCmd(SD_SPI_CLOCK, ENABLE);
#endif
	/*!< Configure SD_SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MOSI_PIN;
	GPIO_Init(SD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SD_CS_PIN pin: SD CS pin */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SD_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

	SD_CS_DISABLE();

	/* Initialize SPI Peripheral */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SD_SPI_PORT, &SPI_InitStructure);

	/* SPI2 enable */
	SPI_Cmd(SD_SPI_PORT, ENABLE);


	u8 r1; // return value of SD card storage
	u16 retry; // used to count out
	u8 buff[6] = {0,0,0,0,0};
	u8 ret;

	sd_setSpiSpeed(SPI_SPEED_256);

	// start transmission
	ret = sd_readWriteByte(0xff);

	if (SD_Idle_Sta())
		return 1; 	// 1 set out to return to the idle mode failed

	//----------------- SD card is reset to the idle end of the -----------------
	// Get the SD card version
	SD_CS_ENABLE();

	r1 = SD_SendCommand_NoDeassert (8, 0x1aa, 0x87);

	// If the version information is the v1.0 version of the card, ie r1 = 0x05, then the following initialization
	if (r1 == 0x05)
	{
		// Set the card type to SDV1.0, if the latter is detected as the MMC card, and then modified to MMC
		SD_Type = SD_TYPE_V1;
		// If the V1.0 card, CMD8 instruction did not follow-up data
		// Set chip select high end of the second order
		SD_CS_ENABLE();

		// Multiple 8 CLK, so that the end of follow-up operation SD
		sd_readWriteByte (0xFF);

		//----------------- SD card, MMC card initialization -----------------
		// Initialize command CMD55 + ACMD41 Card
		// If there is response, that is the SD card, and the initialization is complete
		// No response, that is the MMC card, the corresponding additional initialization
		retry = 0;
		do
		{
			// Starting CMD55, should return 0x01; or wrong
			r1 = SD_SendCommand (CMD55, 0, 0);
			if (r1 == 0XFF) return r1; // if not 0xff, then sent to

			// Get the correct response, sent ACMD41, the return value should be 0x00, otherwise retry 200
			r1 = SD_SendCommand (ACMD41, 0, 0);

			retry++;
	    } while ((r1 != 0x00) && (retry <SD_MEMORY_RETRY_COUNT));

	    // Determine the correct response is the time-out or
	    // If response: is the SD card; did not respond: is the MMC card
	    //---------- MMC card for additional initialization started ------------
	    if (retry == SD_MEMORY_RETRY_COUNT)
	    {
			retry = 0;
			// Initialize command to send MMC card (not tested)
			do
			{
			    r1 = SD_SendCommand (1,0,0);
			    retry++;
			} while ((r1 != 0x00) && (retry <SD_MEMORY_RETRY_COUNT));

			if (retry == SD_MEMORY_RETRY_COUNT) return 1; // MMC card initialization timed out

			// Write card type
			SD_Type = SD_TYPE_MMC;
	    }

	    //---------- MMC card for additional initialization end ------------
	    // Set SPI high-speed mode
	    sd_setSpiSpeed(SPI_SPEED_4);
	    sd_readWriteByte (0xFF);

	    // Disable CRC check
	    r1 = SD_SendCommand (CMD59, 0, 0x95);
	    if (r1 != 0x00)
			return r1; // command error, returns r1

	    // Set Sector Size
	    r1 = SD_SendCommand (CMD16, 512, 0x95);
	    if (r1 != 0x00)
			return r1; // command error, returns r1
	    //----------------- SD card, MMC card initialization ended -----------------

	}
	// SD Card for the V1.0 version of the initialization is complete

	// Here is the card initialization V2.0
	// Which need to read the OCR data, with a diagnosis or SD2.0HC Card SD2.0
	else if (r1 == 0x01)
	{
	    // V2.0 card, CMD8 command returns 4 bytes of data, and then to skip the end of the command
	    buff[0] = sd_readWriteByte (0xFF); // should be 0x00
	    buff[1] = sd_readWriteByte (0xFF); // should be 0x00
	    buff[2] = sd_readWriteByte (0xFF); // should be 0x01
	    buff[3] = sd_readWriteByte (0xFF); // should be 0xAA

	    SD_CS_DISABLE();
	    sd_readWriteByte (0xFF); // the next 8 clocks
	    // Determine whether the card supports a voltage range of 2.7V-3.6V
	    // If (buff [2] == 0x01 && buff [3] == 0xAA) // do not judge, let the cards to support more
	    {
			retry = 0;
			// Initialize command CMD55 + ACMD41 Card
			do
			{
				r1 = SD_SendCommand (CMD55, 0, 0);
				if (r1 != 0x01) return r1;

				r1 = SD_SendCommand (ACMD41, 0x40000000, 0);
				if (retry> SD_MEMORY_RETRY_COUNT) return r1; // timeout return status r1
			} while (r1 != 0);

			// Initialize command to send to complete, the next access to OCR information
			// SD2.0 card identification since version
			//----------- -----------
			r1 = SD_SendCommand_NoDeassert (CMD58, 0, 0);
			if (r1 != 0x00)
			{
				SD_CS_DISABLE(); // release chip select signal SD
				return r1; // If the command does not return the correct response, direct withdrawal, return response
			} // Read OCR command is issued, followed by 4 bytes of OCR information

			buff [0] = sd_readWriteByte (0xFF);
			buff [1] = sd_readWriteByte (0xFF);
			buff [2] = sd_readWriteByte (0xFF);
			buff [3] = sd_readWriteByte (0xFF);

			// OCR to receive complete set of high chip select
			SD_CS_DISABLE();
			sd_readWriteByte (0xFF);

			// Check the received OCR in bit30-bit (CCS), was identified as SD2.0 SDHC
			// If the CCS = 1: SDHC CCS = 0: SD2.0
			if (buff [0] & 0x40)
				SD_Type = SD_TYPE_V2HC; // Check the CCS
			else
				SD_Type = SD_TYPE_V2;
			//----------- Identification card version of SD2.0 end -----------
			// Set SPI high-speed mode
			sd_setSpiSpeed(SPI_SPEED_4);
	    }
	}
	return r1;
}

/*******************************************************************************
* Function:		sd_setSpiSpeed
* Description:	SPI speed setting function
* Input:		SpeedSet:
* 				SPI_SPEED_2 2 frequency (SPI 36M @ sys 72M)
* 				SPI_SPEED_4 4 frequency (SPI 18M @ sys 72M)
* 				SPI_SPEED_8 8 frequency (SPI 9M @ sys 72M)
* 				SPI_SPEED_16 16 frequency (SPI 4.5M @ sys 72M)
* 				SPI_SPEED_256 256 frequency (SPI 281.25K @ sys 72M)
* Output:		None
* Return:		None
* Attention:	None
*******************************************************************************/
void sd_setSpiSpeed(uint8_t speed){
	SD_SPI_PORT->CR1 &= 0XFFC7; // Fsck = Fcpu/256
	switch (speed)
	{
		case SPI_SPEED_2: // Second division
			SD_SPI_PORT->CR1 |= 0<<3; // Fsck = Fpclk / 2 = 36Mhz
			break;
		case SPI_SPEED_4: // four-band
			SD_SPI_PORT-> CR1 |= 1<<3; // Fsck = Fpclk / 4 = 18Mhz
			break;
		case SPI_SPEED_8: // eighth of the frequency
			SD_SPI_PORT-> CR1 |= 2<<3; // Fsck = Fpclk / 8 = 9Mhz
			break;
		case SPI_SPEED_16: // sixteen frequency
			SD_SPI_PORT-> CR1 |= 3<<3; // Fsck = Fpclk/16 = 4.5Mhz
			break;
		case SPI_SPEED_256: // 256 frequency division
			SD_SPI_PORT-> CR1 |= 7<<3; // Fsck = Fpclk/16 = 281.25Khz
			break;
	}
	SD_SPI_PORT->CR1 |= 1<<6; // SPI devices enable
}


/*******************************************************************************
* Function:		sd_readWriteByte
* Description:	Initializes the peripherals used by the W5500 driver.
* Input:		One Byte Data to Write
* Output:		None
* Return:		One Byte Data to Read
* Attention:	None
*******************************************************************************/
uint8_t sd_readWriteByte (uint8_t tx_data){
	u16 retry=0;

	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus (SD_SPI_PORT, SPI_I2S_FLAG_TXE) == RESET)
	{
		retry++;
		if(retry>400)
			return 0;
	}

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData (SD_SPI_PORT, tx_data);

	retry=0;

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus (SD_SPI_PORT, SPI_I2S_FLAG_RXNE) == RESET)
	{
		retry++;

		if(retry>400)
			return 0;
	}

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData (SD_SPI_PORT);
}


/*******************************************************************************
* Function:		getMountedMemorySize
* Description:	Initializes the peripherals used by the W5500 driver.
* Input:		One Byte Data to Write
* Output:		None
* Return:		One Byte Data to Read
* Attention:	None
*******************************************************************************/
FRESULT sd_getMountedMemorySize(uint8_t mount_ret, uint32_t * totalSize, uint32_t * availableSize)
{
	FATFS *fs;
	DWORD fre_clust, fre_sect, tot_sect;
	FRESULT res;

	/* Get volume information and free clusters of drive 1 */
	if(mount_ret == SPI_FLASHM)
		res = f_getfree("0:", &fre_clust, &fs);
	else if((mount_ret >= CARD_MMC) && (mount_ret <= CARD_SDHC))
		res = f_getfree("1:", &fre_clust, &fs);

	if (!res){
		/* Get total sectors and free sectors */
		tot_sect = (fs->n_fatent - 2) * fs->csize;
		fre_sect = fre_clust * fs->csize;

		/* Print the free space (assuming 512 bytes/sector) */
		//printf(" - Available memory size : %8ld / %8ld kB\r\n", fre_sect / 2, tot_sect / 2);

		*totalSize = tot_sect / 2;
		*availableSize = fre_sect / 2;
	}

	return res;
}

/*******************************************************************************
* Function:		getMountedMemorySize
* Description:	Initializes the peripherals used by the W5500 driver.
* Input:		One Byte Data to Write
* Output:		None
* Return:		One Byte Data to Read
* Attention:	None
*******************************************************************************/
uint64_t sd_getAvailableMemorySize(void){
	uint64_t size = 4096;

	return size;
}


// Wait for response to SD card
// Response: to get the feedback value
// Return value: 0, success has been the value of the response
// Otherwise, get the value of failure to respond
u8 SD_GetResponse (u8 Response)
{
	u16 Count = 0xFFF; // wait times

	while ((sd_readWriteByte (0XFF) != Response) && Count)
		Count--; // waiting to get an accurate response

	if (Count == 0)
		return MSD_RESPONSE_FAILURE; // get a response failure
	else
		return MSD_RESPONSE_NO_ERROR; // correct response
}


// Wait for write to complete the SD card
// Return value: 0 success;
// Other, error codes;
u8 SD_WaitDataReady (void)
{
	u8 r1 = MSD_DATA_OTHER_ERROR;
	u32 retry;
	retry = 0;
	do
	{
       	 r1 = sd_readWriteByte (0xFF) &0X1F; // read response

        	if (retry == 0xfffe)
			return 1;

		retry++;

		switch (r1)
		{
		case MSD_DATA_OK: // correct the data reception
			r1 = MSD_DATA_OK;
			break;
		case MSD_DATA_CRC_ERROR: // CRC checksum error
			return MSD_DATA_CRC_ERROR;
		case MSD_DATA_WRITE_ERROR: // Data write error
			return MSD_DATA_WRITE_ERROR;
		default: // Unknown error
			r1 = MSD_DATA_OTHER_ERROR;
		break;
		}
	} while (r1 == MSD_DATA_OTHER_ERROR); // data errors have been waiting for

	retry = 0;
	while (sd_readWriteByte (0XFF) == 0) // read data to 0, the data has not yet completed writing
	{
		retry++;

		// Delay_us (10); // SD card write takes a long time to wait
		if (retry >= 0XFFFFFFFE)
			return 0XFF; // Wait failed
	};
	return 0; // success
}



// Send a command to the SD card
// Input: u8 cmd command
// U32 arg command arguments
// U8 crc crc checksum
// Return value: SD card, the response returned
u8 SD_SendCommand (u8 cmd, u32 arg, u8 crc)
{
	u8 r1;
	u16 Retry = 0;

	// Close the chip select
	SD_CS_DISABLE();

	sd_readWriteByte (0xff); // high-speed write command delay
	sd_readWriteByte (0xff);
	sd_readWriteByte (0xff);

	// Chip select low-end set, select the SD card
	SD_CS_ENABLE();

	// Send
	sd_readWriteByte (cmd | 0x40); // write command, respectively,
	sd_readWriteByte (arg>> 24);
	sd_readWriteByte (arg>> 16);
	sd_readWriteByte (arg>> 8);
	sd_readWriteByte (arg);
	sd_readWriteByte (crc);

	// Wait for a response, or time-out exit
	while ((r1 = sd_readWriteByte (0xFF)) == 0xFF)
	{
		Retry++;
		if (Retry> SD_MEMORY_RETRY_COUNT)
			break;
	}

	// Close the chip select
	SD_CS_DISABLE();

	// Additional 8 on the bus clock, so that SD card to complete the remaining work
	sd_readWriteByte (0xFF);

	// Return status values
	return r1;

}


// Send a command to the SD card (the end is yet to chip select, there came the follow-up data)
// Input: u8 cmd command
// U32 arg command arguments
// U8 crc crc checksum
// Return value: SD card, the response returned
u8 SD_SendCommand_NoDeassert (u8 cmd, u32 arg, u8 crc)
{
	u16 Retry = 0;
	u8 r1;

	sd_readWriteByte (0xff); // high-speed write command delay
	sd_readWriteByte (0xff);

	// chip select low-end set, select the SD card
	SD_CS_ENABLE();

	// Send
	sd_readWriteByte (cmd | 0x40); // write command, respectively,
	sd_readWriteByte (arg>> 24);
	sd_readWriteByte (arg>> 16);
	sd_readWriteByte (arg>> 8);
	sd_readWriteByte (arg);
	sd_readWriteByte (crc);

	// Wait for a response, or time-out exit
	while ((r1 = sd_readWriteByte (0xFF)) == 0xFF)
	{
	    Retry++;
	    if (Retry> SD_MEMORY_RETRY_COUNT)
			break;
	}

	// Return the response value
	return r1;
}



// Set the SD card into suspend mode
// Return value: 0, successfully set
// 1, setup failed
u8 SD_Idle_Sta (void)
{
	u16 i;
	u8 r1=0;
	u16 retry;

	for (i = 0; i <0xf00; i++);// pure delay, waiting for complete power-SD card

	// Should generate> 74 pulses, so that SD card to complete their initialization
	for (i = 0; i <10; i++) sd_readWriteByte (0xFF);

	//----------------- SD card is reset to the idle start -----------------
	// Loop continuously sending CMD0, until the SD card back 0x01, enter the IDLE state
	// Timeout then exit
	retry = 0;
	do
	{
		// Send CMD0, so that SD card into the IDLE state
		r1 = SD_SendCommand (CMD0, 0, 0x95);
		retry++;
	} while ((r1 != 0x01) && (retry <SD_MEMORY_RETRY_COUNT));

	// Exit the loop, check reason: Initialization successful? or try out?
	if (retry == SD_MEMORY_RETRY_COUNT) return 1; // fail

	return 0; // success
}


// Read back from the SD card in the specified length of data placed in a given position
// Input: u8 * data (read back the data storage memory> len)
// U16 len (data length)
// U8 release (whether to release the bus after transfer CS is set high 0: do not release 1: Release)
// Return value: 0: NO_ERR
// Other: Error Message
u8 SD_ReceiveData (u8 * data, u16 len, u8 release)
{
	// Start the first transfer
	SD_CS_ENABLE();
	if (SD_GetResponse (0xFE)) // wait for data sent back to the starting SD card token 0xFE
	{
		SD_CS_DISABLE();
		return 1;
	}

	while (len--) // Start receiving data
	{
		*data = sd_readWriteByte (0xFF);
		data++;
	}

	// Here are two pseudo-CRC (dummy CRC)
	sd_readWriteByte (0xFF);
	sd_readWriteByte (0xFF);
	if (release == RELEASE) // demand the release of the bus, the CS is set high
	{
		SD_CS_DISABLE(); // end of transmission
		sd_readWriteByte (0xFF);
	}
	return 0;
}

// Get the SD card CID information, including manufacturer information
// Input: u8 * cid_data (CID stored in the memory, at least 16Byte)
// Return value: 0: NO_ERR
// 1: TIME_OUT
// Other: Error Message
u8 SD_GetCID (u8 * cid_data)
{
	u8 r1;

	// Send CMD10 command, read CID
	r1 = SD_SendCommand(CMD10,0,0xff);

	if (r1 != 0x00)
		return r1; // do not return the correct answer, then exit, error

	SD_ReceiveData (cid_data, 16, RELEASE); // 16 bytes of data received
	return 0;
}

// Get the SD card, CSD, including the capacity and speed of information
// Input: u8 * cid_data (CID stored in the memory, at least 16Byte)
// Return value: 0: NO_ERR
// 1: TIME_OUT
// Other: Error Message
u8 SD_GetCSD (u8 *csd_data)
{
	u8 r1;

	r1 = SD_SendCommand(CMD9, 0, 0xff); // send CMD9 command, read CSD

	if (r1)
		return r1; // do not return the correct answer, then exit, error

	SD_ReceiveData (csd_data, 16, RELEASE); // 16 bytes of data received

	return 0;
}

// Get the SD card capacity (bytes)
// Return value: 0: take capacity error
// Other: SD card capacity (bytes)
u32 SD_GetCapacity (void)
{
	u8 csd [16];
	u32 Capacity;
	u8 r1;
	u16 i;
	u16 temp;
	// Get CSD information, if during the error, return 0
	if (SD_GetCSD (csd) != 0) return 0;

	// If the SDHC card, calculated in accordance with the following
	if ((csd [0] & 0xC0) == 0x40)
	{
		Capacity = ((u32) csd [8]) <<8;
		Capacity += (u32) csd [9] +1;
		Capacity = (Capacity) * 1024; // get number of sectors
		Capacity *= 512; // get number of bytes
	}
	else
	{
		//int j;

		//for(j=0;j<16;j++)
		//	usart1_transmit_string_format("%x\r\n", csd[j]);

		i = csd [6] &0x03;
		//usart1_transmit_string_format("\r\ni=%x", i);
		i <<= 8;
		//usart1_transmit_string_format("\r\ni=%x", i);
		i += csd [7];
		//usart1_transmit_string_format("\r\ni=%x", i);
		i <<= 2;
		//usart1_transmit_string_format("\r\ni=%x", i);
		i += ((csd [8] & 0xc0)>> 6);
		//usart1_transmit_string_format("\r\ni=%x", i);

		// C_SIZE_MULT
		r1 = csd [9] &0x03;
		r1 <<= 1;
		r1 += ((csd [10] & 0x80)>> 7);
		r1 += 2; // BLOCKNR
		temp = 1;

		while (r1)
		{
			temp *= 2;
			r1--;
		}

		Capacity = ((u32) (i +1 ))*(( u32) temp);

		//usart1_transmit_string_format("\r\ntemp=%d,  Capacity=%d, i=%d\r\n", temp, Capacity, i);

		// READ_BL_LEN
		i = csd [5] &0x0f;

		// BLOCK_LEN
		temp = 1;

		while (i)
		{
			temp *= 2;
			i--;
		}

		//usart1_transmit_string_format("\r\\ntemp=%d,  Capacity=%d Bytes\r\n", temp, Capacity);

		// The final result
		Capacity *= (u32) temp; // in bytes
	}
	return (u32) Capacity;
}

// Read a block SD card
// Input: u32 sector to take the address (sector value, non-physical address)
// U8 * buffer data storage location (size at least 512byte)
// Return value: 0: Success
// Other: failure
u8 SD_ReadSingleBlock (u32 sector, u8 * buffer)
{
	u8 r1;

	// Set to high-speed mode
	// bsp_set_spi2_speed(SPI_SPEED_4);

	// If it is not SDHC, given that the sector address, convert it into a byte address
	if (SD_Type != SD_TYPE_V2HC)
	{
	    sector = sector <<9;
	}

	r1 = SD_SendCommand (CMD17, sector, 0); // read command

	if (r1 != 0x00) return r1;

	r1 = SD_ReceiveData (buffer, 512, RELEASE);
	if (r1 != 0)
		return r1; // read data error!
	else
		return 0;
}


//////////////////////////// Function of the following two required
/////////// USB reader //////////////
// Define block size SD card
#define BLOCK_SIZE 512

// Write MSD / SD data
// PBuffer: Data Storage
// ReadAddr: writing the first address
// NumByteToRead: number of bytes to write
// Return value: 0, write to complete
// Otherwise, write failure
u8 MSD_WriteBuffer (u8 * pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
	u32 i, NbrOfBlock = 0, Offset = 0;
	u32 sector;
	u8 r1;

	NbrOfBlock = NumByteToWrite / BLOCK_SIZE; // get the number of blocks to be written
	SD_CS_ENABLE();

	while (NbrOfBlock--) // write a sector
	{
		sector = WriteAddr + Offset;
		if (SD_Type == SD_TYPE_V2HC) sector >>= 9; // perform the reverse operation and common action

		r1 = SD_SendCommand_NoDeassert (CMD24, sector, 0xff); // write command
		if (r1)
		{
			SD_CS_DISABLE();
			return 1; // response is not correct, a direct return
		}

		sd_readWriteByte (0xFE); // put initial token 0xFE

		// Put data in a sector
		for (i = 0; i <512; i++)
			sd_readWriteByte (*pBuffer++);

		// Send the dummy CRC Byte 2
		sd_readWriteByte (0xff);
		sd_readWriteByte (0xff);

		if (SD_WaitDataReady ())// SD card data is written to wait for the completion
		{
			SD_CS_DISABLE();
			return 2;
		}

		Offset += 512;
	}

	// Write completed, set to 1 chip select
	SD_CS_DISABLE();
	sd_readWriteByte (0xff);

	return 0;

}
// Read the MSD / SD data
// PBuffer: Data Storage
// ReadAddr: Read the first address
// NumByteToRead: number of bytes to read
// Return value: 0, read out the complete
// Others, read failure
u8 MSD_ReadBuffer (u8 * pBuffer, u32 ReadAddr, u32 NumByteToRead)
{
	u32 NbrOfBlock = 0, Offset = 0;
	u32 sector = 0;
	u8 r1 = 0;

	NbrOfBlock = NumByteToRead / BLOCK_SIZE;

	SD_CS_ENABLE();

	while (NbrOfBlock--)
	{
		sector = ReadAddr + Offset;
		if (SD_Type == SD_TYPE_V2HC)
			sector >>= 9; // perform the reverse operation and common action

		r1 = SD_SendCommand_NoDeassert (CMD17, sector, 0xff); // read command
		if (r1) // command to send an error
		{
			SD_CS_DISABLE();
			return r1;
		}

		r1 = SD_ReceiveData (pBuffer, 512, RELEASE);
		if (r1) // reading error
		{
			SD_CS_DISABLE();
			return r1;
		}

		pBuffer += 512;
		Offset += 512;
	}

	SD_CS_DISABLE();
	sd_readWriteByte (0xff);

	return 0;
}
//////////////////////////////////////////////////////////////////////////
// Write SD card, a block (not actually tested)
// Input: u32 sector sector address (sector value, non-physical address)
// U8 * buffer data storage location (size at least 512byte)
// Return value: 0: Success
// Other: failure
u8 SD_WriteSingleBlock (u32 sector, const u8 * data)
{
	u8 r1;
	u16 i;
	u16 retry;

	// Set to high-speed mode
	// SPIx_SetSpeed(SPI_SPEED_HIGH);

	// If it is not SDHC, given that the sector address, convert it into a byte address
	if (SD_Type != SD_TYPE_V2HC)
	{
		sector = sector <<9;
	}

	r1 = SD_SendCommand (CMD24, sector, 0x00);
	if (r1 != 0x00)
	{
		return r1; // response is not correct, a direct return
	}

	// Start preparing data
	SD_CS_ENABLE();
	// First put three empty data, SD card ready to wait
	sd_readWriteByte (0xff);
	sd_readWriteByte (0xff);
	sd_readWriteByte (0xff);
	// Put initial token 0xFE
	sd_readWriteByte (0xFE);

	// Put data in a sector
	for (i = 0; i <512; i++)
	{
		sd_readWriteByte (* data++);
	}

	// Send a dummy CRC Byte 2
	sd_readWriteByte (0xff);
	sd_readWriteByte (0xff);

	// Wait for answer SD Card
	r1 = sd_readWriteByte (0xff);
	if ((r1 & 0x1F) != 0x05)
	{
	    SD_CS_DISABLE();
	    return r1;
	}

	// Wait operation is completed
	retry = 0;
	while ( !sd_readWriteByte (0xff))
	{
	    retry++;
	    if (retry> 0xfffe) // if not done for a long time to write, error exit
	    {
	        SD_CS_DISABLE();
	        return 1; // write timeout return 1
	    }
	}
	// Write completed, set to 1 chip select
	SD_CS_DISABLE();
	sd_readWriteByte (0xff);

	return 0;
}

// Read more than one SD card block (actually tested)
// Input: u32 sector sector address (sector value, non-physical address)
// U8 * buffer data storage location (size at least 512byte)
// U8 count one continuous block read count
// Return value: 0: Success
// Other: failure
u8 SD_ReadMultiBlock (u32 sector, u8 * buffer, u8 count)
{
    u8 r1;

	// SPIx_SetSpeed(SPI_SPEED_HIGH); // set to high-speed mode

	// If it is not SDHC, the sector addresses turn into a byte address
	if (SD_Type != SD_TYPE_V2HC)
		sector = sector <<9;

	// SD_WaitDataReady ();
	// Read multi-block commands issued
	r1 = SD_SendCommand (CMD18, sector, 0); // read command
	if (r1 != 0x00) return r1;

	do // Start receiving data
	{
	    if (SD_ReceiveData (buffer, 512, NO_RELEASE) != 0x00) break;
	    buffer += 512;
	} while (--count);

	// All the transmission is completed, send the stop command
	SD_SendCommand (CMD12, 0, 0);

	// Release bus
	SD_CS_DISABLE();
	sd_readWriteByte (0xFF);

	if (count != 0)
		return count; // end if not passed, return the remaining number of
	else
		return 0;

}
// Write SD card, N-block (not actually tested)
// Input: u32 sector sector address (sector value, non-physical address)
// U8 * buffer data storage location (size at least 512byte)
// U8 count the number of write block
// Return value: 0: Success
// Other: failure
u8 SD_WriteMultiBlock (u32 sector, const u8 * data, u8 count)
{
	u8 r1;
	u16 i;

	// SPIx_SetSpeed (SPI_SPEED_HIGH); // set to high-speed mode

	if (SD_Type != SD_TYPE_V2HC)
		sector = sector <<9; // If it is not SDHC, given that the sector address, convert it to byte address
	if (SD_Type != SD_TYPE_MMC)
		r1 = SD_SendCommand (ACMD23, count, 0x00); // If the target card is not the MMC card, enable the command to enable pre-erase ACMD23

	r1 = SD_SendCommand (CMD25, sector, 0x00); // send multi-block write command
	if (r1 != 0x00)
		return r1; // response is not correct, a direct return

	SD_CS_ENABLE(); // start preparing data

	sd_readWriteByte (0xff); // first put three empty data, SD card ready to wait
	sd_readWriteByte (0xff);
	//-------- The following is written by N-loop part of the sector

	do
	{
		// Put the start token that is a multi-block write 0xFC
		sd_readWriteByte (0xFC);
		// Put data in a sector
		for (i = 0; i <512; i++)
		{
			sd_readWriteByte (* data++);
		}
		// Send a dummy CRC Byte 2
		sd_readWriteByte (0xff);
		sd_readWriteByte (0xff);

		// Wait for answer SD Card
		r1 = sd_readWriteByte (0xff);
		if ((r1 & 0x1F) != 0x05)
		{
			SD_CS_DISABLE(); // If the response is an error, then exit with error code
			return r1;
		}
		// Wait for write to complete the SD card
		if (SD_WaitDataReady () == 1)
		{
			SD_CS_DISABLE(); // wait for write to complete the SD card out, exit error
			return 1;
		}
	} while (--count); // completion of the sector data transfer

	// Send end of the transmission token 0xFD
	r1 = sd_readWriteByte (0xFD);
	if (r1 == 0x00)
	{
		count = 0xfe;
	}

	if (SD_WaitDataReady ()) // wait for ready
	{
		SD_CS_DISABLE();
		return 1;
	}
	// Write completed, set to 1 chip select
	SD_CS_DISABLE();
	sd_readWriteByte (0xff);

	return count; // return count value, if finished then count = 0, otherwise, count = 1

}
// In the specified sector, began to read out bytes from the offset byte
// Input: u32 sector sector address (sector value, non-physical address)
// U8 * buf data storage addresses (size <= 512byte)
// U16 offset offset inside the sector
// U16 bytes number of bytes to read
// Return value: 0: Success
// Other: failure
u8 SD_Read_Bytes (unsigned long address, unsigned char * buf, unsigned int offset, unsigned int bytes)
{
	u8 r1; u16 i = 0;
	r1 = SD_SendCommand (CMD17, address <<9,0); // send the Read Sector command

	if (r1)
		return r1; // response is not correct, a direct return

	SD_CS_ENABLE(); // select the SD card

	if (SD_GetResponse (0xFE)) // wait for data sent back to the starting SD card token 0xFE
	{
		SD_CS_DISABLE(); // close the SD card
		return 1; // read failure
	}

	for (i = 0; i <offset; i++) sd_readWriteByte (0xff); // skip the offset bits
	for (; i <offset + bytes; i++) * buf++= sd_readWriteByte (0xff); // read the useful data
	for (; i <512; i++) sd_readWriteByte (0xff); // read the remaining bytes

	sd_readWriteByte (0xff); // send the pseudo-code CRC
	sd_readWriteByte (0xff);

	SD_CS_DISABLE(); // close the SD card

	return 0;
}

u8 SD_WaitReady(void)
{
    u8 r1;
    u16 retry;
    retry = 0;
    do
    {
        r1 = sd_readWriteByte(0xFF);
        if(retry==0xfffe)
        {
            return 1;
        }
    }while(r1!=0xFF);

    return 0;
}








MemoryType_Enum mmc_mount(void)
{
	FRESULT res;
    u8 state;

#if !defined(F_SPI_FLASH_ONLY)
	state = sd_init();
#if defined(_FS_DEBUG_)
    printf("SD_Init:%d\r\n", state);
#endif
	if(state == STA_NODISK)
	{
		return NO_CARD;
	}
	else if(state != 0)
	{
		return NO_CARD;
	}
	else
	{
		res = f_mount(&Fatfs[0], "1:", 0);
#if defined(_FS_DEBUG_)
	    printf("f_mount:%d\r\n", res);
#endif
		g_sdcard_done = 1;

		return SD_Type;
	}
#endif
	return NO_CARD;//0
}

uint8_t flash_mount()
{
#if 0
	if(mmc_init())
	{
		f_mount(0, &ff);
		return card_type;
	}
#else
	FRESULT res;

	DataFlash_Init();

	//disk_initialize(1);

	res = f_mount(&Fatfs[0],"0:",0);
#if defined(_FS_DEBUG_)
    printf("f_mount:%d\r\n", res);
#endif

#if defined(F_SPI_FLASH)
    if(check_spiflash_flag() == 1)
        g_mkfs_done = 1;
    else
        g_mkfs_done = 0;

	res = f_mkfs("0:",0,512);

#if defined(_FS_DEBUG_)
    printf("f_mkfs:%d %d\r\n", res, g_mkfs_done);
#endif
    if(check_spiflash_flag() == 1)
    	save_spiflash_flag();
    g_mkfs_done = 1;
#endif

	return SPI_FLASHM;
#endif
}

//*****************************************************************************

#ifdef STM32_SD_USE_DMA
/*-----------------------------------------------------------------------*/
/* Transmit/Receive Block using DMA (Platform dependent. STM32 here)     */
/*-----------------------------------------------------------------------*/
static void stm32_dma_transfer(
	uint8_t receive,		/* FALSE(0) for buff->SPI, TRUE(1) for SPI->buff               */
	const BYTE *buff,		/* receive TRUE  : 512 byte data block to be transmitted
						   	receive FALSE : Data buffer to store received data    */
	uint16_t btr 			/* receive TRUE  : Byte count (must be multiple of 2)
						   	receive FALSE : Byte count (must be 512)              */
)
{
	DMA_InitTypeDef DMA_InitStructure;
	WORD rw_workbyte[] = { 0xffff };

	/* shared DMA configuration values */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (DWORD)(&(SPI_SD->DR));
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_BufferSize = btr;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_DeInit(DMA_Channel_SPI_SD_RX);
	DMA_DeInit(DMA_Channel_SPI_SD_TX);

	if ( receive ) {

		/* DMA1 channel2 configuration SPI1 RX ---------------------------------------------*/
		/* DMA1 channel4 configuration SPI2 RX ---------------------------------------------*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)buff;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_Init(DMA_Channel_SPI_SD_RX, &DMA_InitStructure);

		/* DMA1 channel3 configuration SPI1 TX ---------------------------------------------*/
		/* DMA1 channel5 configuration SPI2 TX ---------------------------------------------*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)rw_workbyte;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		DMA_Init(DMA_Channel_SPI_SD_TX, &DMA_InitStructure);

	} else {

#if _FS_READONLY == 0
		/* DMA1 channel2 configuration SPI1 RX ---------------------------------------------*/
		/* DMA1 channel4 configuration SPI2 RX ---------------------------------------------*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)rw_workbyte;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		DMA_Init(DMA_Channel_SPI_SD_RX, &DMA_InitStructure);

		/* DMA1 channel3 configuration SPI1 TX ---------------------------------------------*/
		/* DMA1 channel5 configuration SPI2 TX ---------------------------------------------*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)buff;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_Init(DMA_Channel_SPI_SD_TX, &DMA_InitStructure);
#endif

	}

	/* Enable DMA RX Channel */
	DMA_Cmd(DMA_Channel_SPI_SD_RX, ENABLE);
	/* Enable DMA TX Channel */
	DMA_Cmd(DMA_Channel_SPI_SD_TX, ENABLE);

	/* Enable SPI TX/RX request */
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

	/* Wait until DMA1_Channel 3 Transfer Complete */
	/// not needed: while (DMA_GetFlagStatus(DMA_FLAG_SPI_SD_TC_TX) == RESET) { ; }
	/* Wait until DMA1_Channel 2 Receive Complete */
	while (DMA_GetFlagStatus(DMA_FLAG_SPI_SD_TC_RX) == RESET) { ; }
	// same w/o function-call:
	// while ( ( ( DMA1->ISR ) & DMA_FLAG_SPI_SD_TC_RX ) == RESET ) { ; }

	/* Disable DMA RX Channel */
	DMA_Cmd(DMA_Channel_SPI_SD_RX, DISABLE);
	/* Disable DMA TX Channel */
	DMA_Cmd(DMA_Channel_SPI_SD_TX, DISABLE);

	/* Disable SPI RX/TX request */
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);
}
#endif /* STM32_SD_USE_DMA */
