/*******************************************************************************
 * @File:			stm32f10x_microsd.c
 * @Author:			Aytac Dilek
 * @email:			aytacdilek@gmail.com
 * @Web:
 * @Date:			04.16.2016
 * @Description:	The SPI SD Card application function
 * @Version:		1.0
 * @Platform:		OPEN103Z-B
 * @CPU:			STM32F103ZET6
 * @IDE:			CooCox CoIDE 1.7.8
 * @GCC:			4.8 2014q2
 * @Module:			MicroSD
 * @Function:		Read micro SD card
 * @Notes:			None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_sd.h"



/* Internal Functions *********************************************************/
void microsd_gpioInit(void);
void microsd_spiHighSpeed(uint8_t b_high);

int _spi_read_write(uint8_t data);
int _send_command(uint8_t cmd, uint32_t arg, uint8_t crc);
int _send_command_hold(uint8_t cmd, uint32_t arg, uint8_t crc);
int _read_buffer(uint8_t *buff, uint16_t len, uint8_t release);



/*******************************************************************************
* Function Name  : MSD_SPI_Configuration
* Description    : SD Card SPI Configuration
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void microsd_gpioInit(void){
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable Pin Clocks */
	RCC_APB2PeriphClockCmd(	MICROSD_MOSI_CLOCK |
							MICROSD_MISO_CLOCK |
							MICROSD_SCK_CLOCK |
							MICROSD_CS_CLOCK |
							MICROSD_CD_CLOCK, ENABLE);

	/* SPI_MOSI  */
	GPIO_InitStructure.GPIO_Pin = MICROSD_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(MICROSD_MOSI_PORT, &GPIO_InitStructure);

	/* SPI1_MISO */
	GPIO_InitStructure.GPIO_Pin = MICROSD_MISO_PIN;
	GPIO_Init(MICROSD_MISO_PORT, &GPIO_InitStructure);

	/* SPI1_SCK */
	GPIO_InitStructure.GPIO_Pin = MICROSD_SCK_PIN;
	GPIO_Init(MICROSD_SCK_PORT, &GPIO_InitStructure);

	/* SD_CS */
	GPIO_InitStructure.GPIO_Pin = MICROSD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(MICROSD_CS_PORT, &GPIO_InitStructure);

	/* SD_CD */
	GPIO_InitStructure.GPIO_Pin = MICROSD_CD_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(MICROSD_CD_PORT, &GPIO_InitStructure);

	_card_disable();

#if defined(MICROSD_USE_SPI1)
	RCC_APB2PeriphClockCmd(MICROSD_SPI_CLOCK, ENABLE);
#elif defined(MICROSD_USE_SPI2)
	RCC_APB1PeriphClockCmd(MICROSD_SPI_CLOCK, ENABLE);
#endif

	microsd_spiHighSpeed(0);

	SPI_Cmd(MICROSD_SPI_PER, ENABLE);
}



/*******************************************************************************
* Function Name  : MSD_SPIHighSpeed
* Description    : SD Card Speed Set
* Input          : - b_high: 1 = 18MHz, 0 = 281.25Hz
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void microsd_spiHighSpeed(uint8_t b_high){
	SPI_InitTypeDef SPI_InitStructure;

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	/* Speed select */
	if(b_high == 0){
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	}
	else{
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	}

	SPI_Init(MICROSD_SPI_PER, &SPI_InitStructure);
}

/*******************************************************************************
* Function Name  : microsd_init
* Description    : SD Card initializtion
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int microsd_init(void){
	uint8_t r1;
	uint8_t buff[6] = {0};
	uint16_t retry;

	microsd_gpioInit();

	/* Check , if no card insert */
//    if( _card_insert() )
//	{
//#ifdef PRINT_INFO
//		printf("There is no card detected! \r\n");
//#endif
//	  /* FATFS error flag */
//      return -1;
//	}

	/* Power on and delay some times */
	for(retry=0; retry<0x100; retry++){
		_card_power_on();
	}

	/* Start send 74 clocks at least */
	for(retry=0; retry<10; retry++){
		_spi_read_write(DUMMY_BYTE);
	}

	/* Start send CMD0 till return 0x01 means in IDLE state */
	for(retry=0; retry<0xFFF; retry++){
		r1 = _send_command(CMD0, 0, 0x95);
		if(r1 == 0x01){
			retry = 0;
			break;
		}
	}

	/* Timeout return */
	if(retry == 0xFFF){
#ifdef PRINT_INFO
		printf("Reset card into IDLE state failed!\r\n");
#endif
		return 1;
	}

	/* Get the card type, version */
	r1 = _send_command_hold(CMD8, 0x1AA, 0x87);

	/* r1=0x05 -> V1.0 */
	if(r1 == 0x05){
	  CardInfo.CardType = CARDTYPE_SDV1;

	  /* End of CMD8, chip disable and dummy byte */
	  _card_disable();
	  _spi_read_write(DUMMY_BYTE);

	  /* SD1.0/MMC start initialize */
	  /* Send CMD55+ACMD41, No-response is a MMC card, otherwise is a SD1.0 card */
	  for(retry=0; retry<0xFFF; retry++){
	     r1 = _send_command(CMD55, 0, 0);			/* should be return 0x01 */
		 if(r1 != 0x01){
#ifdef PRINT_INFO
			printf("Send CMD55 should return 0x01, response=0x%02x\r\n", r1);
#endif
			return r1;
		 }

		 r1 = _send_command(ACMD41, 0, 0);			/* should be return 0x00 */
		 if(r1 == 0x00)
		 {
			retry = 0;
			break;
		 }
	  }

	  /* MMC card initialize start */
	  if(retry == 0xFFF)
	  {
		 for(retry=0; retry<0xFFF; retry++)
	     {
			 r1 = _send_command(CMD1, 0, 0);		/* should be return 0x00 */
			 if(r1 == 0x00)
			 {
				retry = 0;
				break;
			 }
		 }

		 /* Timeout return */
		 if(retry == 0xFFF)
		 {
#ifdef PRINT_INFO
			printf("Send CMD1 should return 0x00, response=0x%02x\r\n", r1);
#endif
			return 2;
		 }

		CardInfo.CardType = CARDTYPE_MMC;
#ifdef PRINT_INFO
		printf("Card Type                     : MMC\r\n");
#endif
	  }
		/* SD1.0 card detected, print information */
#ifdef PRINT_INFO
	  else
	  {
		 printf("Card Type                     : SD V1\r\n");
	  }
#endif

	  /* Set spi speed high */
	  microsd_spiHighSpeed(1);

	  /* CRC disable */
	  r1 = _send_command(CMD59, 0, 0x01);
	  if(r1 != 0x00)
	  {
#ifdef PRINT_INFO
		  printf("Send CMD59 should return 0x00, response=0x%02x\r\n", r1);
#endif
		  return r1;		/* response error, return r1 */
	  }

	  /* Set the block size */
	  r1 = _send_command(CMD16, MSD_BLOCKSIZE, 0xFF);
	  if(r1 != 0x00)
	  {
#ifdef PRINT_INFO
		  printf("Send CMD16 should return 0x00, response=0x%02x\r\n", r1);
#endif
		  return r1;		/* response error, return r1 */
	  }
   }

   /* r1=0x01 -> V2.x, read OCR register, check version */
   else if(r1 == 0x01)
   {
	 /* 4Bytes returned after CMD8 sent	*/
	 buff[0] = _spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[1] = _spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[2] = _spi_read_write(DUMMY_BYTE);				/* should be 0x01 */
	 buff[3] = _spi_read_write(DUMMY_BYTE);				/* should be 0xAA */

	 /* End of CMD8, chip disable and dummy byte */
	 _card_disable();
	 _spi_read_write(DUMMY_BYTE);

	 /* Check voltage range be 2.7-3.6V	*/
	 if(buff[2]==0x01 && buff[3]==0xAA)
	 {
		for(retry=0; retry<0xFFF; retry++)
		{
			r1 = _send_command(CMD55, 0, 0);			/* should be return 0x01 */
			if(r1!=0x01)
			{
#ifdef PRINT_INFO
				printf("Send CMD55 should return 0x01, response=0x%02x\r\n", r1);
#endif
				return r1;
			}

			r1 = _send_command(ACMD41, 0x40000000, 0);	/* should be return 0x00 */
			if(r1 == 0x00)
			{
				retry = 0;
				break;
			}
		}

		/* Timeout return */
		if(retry == 0xFFF)
		{
#ifdef PRINT_INFO
			printf("Send ACMD41 should return 0x00, response=0x%02x\r\n", r1);
#endif
			return 3;
		}

		/* Read OCR by CMD58 */
	    r1 = _send_command_hold(CMD58, 0, 0);
	    if(r1!=0x00)
	    {
#ifdef PRINT_INFO
			printf("Send CMD58 should return 0x00, response=0x%02x\r\n", r1);
#endif
            return r1;		/* response error, return r1 */
	    }

	    buff[0] = _spi_read_write(DUMMY_BYTE);
		buff[1] = _spi_read_write(DUMMY_BYTE);
		buff[2] = _spi_read_write(DUMMY_BYTE);
		buff[3] = _spi_read_write(DUMMY_BYTE);

		/* End of CMD58, chip disable and dummy byte */
		_card_disable();
		_spi_read_write(DUMMY_BYTE);

	    /* OCR -> CCS(bit30)  1: SDV2HC	 0: SDV2 */
	    if(buff[0] & 0x40)
	    {
           CardInfo.CardType = CARDTYPE_SDV2HC;
#ifdef PRINT_INFO
		   printf("Card Type                     : SD V2HC\r\n");
#endif
	    }
	    else
	    {
           CardInfo.CardType = CARDTYPE_SDV2;
#ifdef PRINT_INFO
		   printf("Card Type                     : SD V2\r\n");
#endif
	    }

		/* Set spi speed high */
	    microsd_spiHighSpeed(1);
		}
   }
   return 0;
}



/*******************************************************************************
* Function Name  : microsd_getCardInfo
* Description    : Get SD Card Information
* Input          : None
* Output         : None
* Return         : 0��NO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int microsd_getCardInfo(PMSD_CARDINFO cardinfo){
	uint8_t r1;
	uint8_t CSD_Tab[16];
	uint8_t CID_Tab[16];

	/* Send CMD9, Read CSD */
	r1 = _send_command(CMD9, 0, 0xFF);
	if(r1 != 0x00){
		return r1;
	}

	if(_read_buffer(CSD_Tab, 16, RELEASE)){
		return 1;
	}

	/* Send CMD10, Read CID */
	r1 = _send_command(CMD10, 0, 0xFF);
	if(r1 != 0x00){
		return r1;
	}

	if(_read_buffer(CID_Tab, 16, RELEASE)){
		return 2;
	}

	/* Byte 0 */
	cardinfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
	cardinfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
	cardinfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
	/* Byte 1 */
	cardinfo->CSD.TAAC = CSD_Tab[1] ;
	/* Byte 2 */
	cardinfo->CSD.NSAC = CSD_Tab[2];
	/* Byte 3 */
	cardinfo->CSD.MaxBusClkFrec = CSD_Tab[3];
	/* Byte 4 */
	cardinfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
	/* Byte 5 */
	cardinfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
	cardinfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
	/* Byte 6 */
	cardinfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
	cardinfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
	cardinfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
	cardinfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
	cardinfo->CSD.Reserved2 = 0; /* Reserved */
	cardinfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
	/* Byte 7 */
	cardinfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
	/* Byte 8 */
	cardinfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
	cardinfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
	cardinfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
	/* Byte 9 */
	cardinfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
	cardinfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
	cardinfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
	/* Byte 10 */
	cardinfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
	cardinfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
	cardinfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
	/* Byte 11 */
	cardinfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
	cardinfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
	/* Byte 12 */
	cardinfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
	cardinfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
	cardinfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
	cardinfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
	/* Byte 13 */
	cardinfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
	cardinfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
	cardinfo->CSD.Reserved3 = 0;
	cardinfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
	/* Byte 14 */
	cardinfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
	cardinfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
	cardinfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
	cardinfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
	cardinfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
	cardinfo->CSD.ECC = (CSD_Tab[14] & 0x03);
	/* Byte 15 */
	cardinfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
	cardinfo->CSD.Reserved4 = 1;

	if(cardinfo->CardType == CARDTYPE_SDV2HC){
		/* Byte 7 */
		cardinfo->CSD.DeviceSize = (u16)(CSD_Tab[8]) *256;
		/* Byte 8 */
		cardinfo->CSD.DeviceSize += CSD_Tab[9] ;
	}

	cardinfo->Capacity = cardinfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
	cardinfo->BlockSize = MSD_BLOCKSIZE;

	/* Byte 0 */
	cardinfo->CID.ManufacturerID = CID_Tab[0];
	/* Byte 1 */
	cardinfo->CID.OEM_AppliID = CID_Tab[1] << 8;
	/* Byte 2 */
	cardinfo->CID.OEM_AppliID |= CID_Tab[2];
	/* Byte 3 */
	cardinfo->CID.ProdName1 = CID_Tab[3] << 24;
	/* Byte 4 */
	cardinfo->CID.ProdName1 |= CID_Tab[4] << 16;
	/* Byte 5 */
	cardinfo->CID.ProdName1 |= CID_Tab[5] << 8;
	/* Byte 6 */
	cardinfo->CID.ProdName1 |= CID_Tab[6];
	/* Byte 7 */
	cardinfo->CID.ProdName2 = CID_Tab[7];
	/* Byte 8 */
	cardinfo->CID.ProdRev = CID_Tab[8];
	/* Byte 9 */
	cardinfo->CID.ProdSN = CID_Tab[9] << 24;
	/* Byte 10 */
	cardinfo->CID.ProdSN |= CID_Tab[10] << 16;
	/* Byte 11 */
	cardinfo->CID.ProdSN |= CID_Tab[11] << 8;
	/* Byte 12 */
	cardinfo->CID.ProdSN |= CID_Tab[12];
	/* Byte 13 */
	cardinfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
	/* Byte 14 */
	cardinfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
	/* Byte 15 */
	cardinfo->CID.ManufactDate |= CID_Tab[14];
	/* Byte 16 */
	cardinfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
	cardinfo->CID.Reserved2 = 1;

	return 0;
}



/*******************************************************************************
* Function Name  : _read_buffer
* Description    : None
* Input          : - *buff:
*				   - len:
*				   - release:
* Output         : None
* Return         : 0��NO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int _read_buffer(uint8_t *buff, uint16_t len, uint8_t release){
	uint8_t r1;
	uint16_t retry;

	/* Card enable, Prepare to read	*/
	_card_enable();

	/* Wait start-token 0xFE */
	for(retry=0; retry<2000; retry++){
		r1 = _spi_read_write(DUMMY_BYTE);
		if(r1 == 0xFE){
			retry = 0;
			break;
		}
	}

	/* Timeout return	*/
	if(retry == 2000){
		_card_disable();
		return 1;
	}

	/* Start reading */
	for(retry=0; retry<len; retry++){
		*(buff+retry) = _spi_read_write(DUMMY_BYTE);
	}

	/* 2bytes dummy CRC */
	_spi_read_write(DUMMY_BYTE);
	_spi_read_write(DUMMY_BYTE);

	/* chip disable and dummy byte */
	if(release){
		_card_disable();
		_spi_read_write(DUMMY_BYTE);
	}

	return 0;
}



/*******************************************************************************
* Function Name  : microsd_readSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int microsd_readSingleBlock(uint32_t sector, uint8_t *buffer){
	uint8_t r1;

	/* if ver = SD2.0 HC, sector need <<9 */
	if(CardInfo.CardType != CARDTYPE_SDV2HC){
		sector = sector<<9;
	}

	/* Send CMD17 : Read single block command */
	r1 = _send_command(CMD17, sector, 0);

	if(r1 != 0x00){
		return 1;
	}

	/* Start read and return the result */
	r1 = _read_buffer(buffer, MSD_BLOCKSIZE, RELEASE);

	/* Send stop data transmit command - CMD12 */
	_send_command(CMD12, 0, 0);

	return r1;
}



/*******************************************************************************
* Function Name  : microsd_readMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int microsd_readMultiBlock(uint32_t sector, uint8_t *buffer, uint32_t NbrOfSector){
	uint8_t r1;
	uint32_t i;

	/* if ver = SD2.0 HC, sector need <<9 */
	if(CardInfo.CardType != CARDTYPE_SDV2HC){
		sector = sector<<9;
	}

	/* Send CMD18 : Read multi block command */
	r1 = _send_command(CMD18, sector, 0);
	if(r1 != 0x00){
		return 1;
	}

	/* Start read	*/
	for(i=0; i<NbrOfSector; i++){
		if(_read_buffer(buffer+i*MSD_BLOCKSIZE, MSD_BLOCKSIZE, HOLD)){
			/* Send stop data transmit command - CMD12	*/
			_send_command(CMD12, 0, 0);
			/* chip disable and dummy byte */
			_card_disable();
			return 2;
		}
	}

	/* Send stop data transmit command - CMD12 */
	_send_command(CMD12, 0, 0);

	/* chip disable and dummy byte */
	_card_disable();
	_spi_read_write(DUMMY_BYTE);

	return 0;
}



/*******************************************************************************
* Function Name  : microsd_writeSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int microsd_writeSingleBlock(uint32_t sector, uc8 *buffer){
	uint8_t r1;
	uint16_t i;
	uint32_t retry;

	/* if ver = SD2.0 HC, sector need <<9 */
	if(CardInfo.CardType != CARDTYPE_SDV2HC){
		sector = sector<<9;
	}

	/* Send CMD24 : Write single block command */
	r1 = _send_command(CMD24, sector, 0);

	if(r1 != 0x00){
		return 1;
	}

	/* Card enable, Prepare to write */
	_card_enable();
	_spi_read_write(DUMMY_BYTE);
	_spi_read_write(DUMMY_BYTE);
	_spi_read_write(DUMMY_BYTE);
	/* Start data write token: 0xFE */
	_spi_read_write(0xFE);

	/* Start single block write the data buffer */
	for(i=0; i<MSD_BLOCKSIZE; i++){
		_spi_read_write(*buffer++);
	}

	/* 2Bytes dummy CRC */
	_spi_read_write(DUMMY_BYTE);
	_spi_read_write(DUMMY_BYTE);

	/* MSD card accept the data */
	r1 = _spi_read_write(DUMMY_BYTE);
	if((r1&0x1F) != 0x05){
		_card_disable();
		return 2;
	}

	/* Wait all the data programm finished */
	retry = 0;
	while(_spi_read_write(DUMMY_BYTE) == 0x00){
		/* Timeout return */
		if(retry++ == 0x40000){
			_card_disable();
			return 3;
		}
	}

	/* chip disable and dummy byte */
	_card_disable();
	_spi_read_write(DUMMY_BYTE);

	return 0;
}



/*******************************************************************************
* Function Name  : microsd_writeMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int microsd_writeMultiBlock(uint32_t sector, uc8 *buffer, uint32_t NbrOfSector){
	uint8_t r1;
	uint16_t i;
	uint32_t n;
	uint32_t retry;

	/* if ver = SD2.0 HC, sector need <<9 */
	if(CardInfo.CardType != CARDTYPE_SDV2HC){
		sector = sector<<9;
	}

	/* Send command ACMD23 berfore multi write if is not a MMC card */
	if(CardInfo.CardType != CARDTYPE_MMC){
		_send_command(ACMD23, NbrOfSector, 0x00);
	}

	/* Send CMD25 : Write nulti block command	*/
	r1 = _send_command(CMD25, sector, 0);

	if(r1 != 0x00){
		return 1;
	}

	/* Card enable, Prepare to write */
	_card_enable();
	_spi_read_write(DUMMY_BYTE);
	_spi_read_write(DUMMY_BYTE);
	_spi_read_write(DUMMY_BYTE);

	for(n=0; n<NbrOfSector; n++){
		/* Start multi block write token: 0xFC */
		_spi_read_write(0xFC);

		for(i=0; i<MSD_BLOCKSIZE; i++){
			_spi_read_write(*buffer++);
		}

		/* 2Bytes dummy CRC */
		_spi_read_write(DUMMY_BYTE);
		_spi_read_write(DUMMY_BYTE);

		/* MSD card accept the data */
		r1 = _spi_read_write(DUMMY_BYTE);
		if((r1&0x1F) != 0x05){
			_card_disable();
			return 2;
		}

		/* Wait all the data programm finished	*/
		retry = 0;
		while(_spi_read_write(DUMMY_BYTE) != 0xFF){
			/* Timeout return */
			if(retry++ == 0x40000){
				_card_disable();
				return 3;
			}
		}
	}

	/* Send end of transmit token: 0xFD */
	r1 = _spi_read_write(0xFD);
	if(r1 == 0x00){
		return 4;
	}

	/* Wait all the data programm finished */
	retry = 0;
	while(_spi_read_write(DUMMY_BYTE) != 0xFF){
		/* Timeout return */
		if(retry++ == 0x40000){
			_card_disable();
			return 5;
		}
	}

	/* chip disable and dummy byte */
	_card_disable();
	_spi_read_write(DUMMY_BYTE);

	return 0;
}


#include "ff.h"
#include "diskio.h"
void microsd_test(void){
	/* Test SD */
	int16_t i;
	int16_t filecounter;

	FIL fsrc;					/* File objects */
	FATFS Fatfs;				/* File system specific */
	FRESULT res;				/* FatFs function common result code */
	UINT br, bw;				/* File read/write count */
	DSTATUS resCard;			/* SDcard status */
	int8_t ramBufferWrite[1024];	/* Temporary buffer for write file */
	int8_t ramBufferRead[1024];	/* Temporary buffer for read file */
	int8_t StringBuffer[] = "Welcome to Embedded Design World !!!";

	/* Step 1 */
	/* Initialization file buffer write */
	printf("Step 1: Initialization file buffer write\r\n");
	filecounter = sizeof(StringBuffer);

	for(i = 0; i < filecounter ; i++)
		ramBufferWrite[i] = StringBuffer[i];

	/* Initialize SD Card */

	/* Step 2 */
	/* Detect micro-SD */
	printf("Step 2: Detecting micro-SD\r\n");
	while(1){
		resCard = disk_initialize(0);       /*Check micro-SD card status */

		switch(resCard){
		case STA_NOINIT:                    /* Drive not initialized */
			break;
		case STA_NODISK:                    /* No medium in the drive */
			printf("NO Disk\r\n");
			break;
		case STA_PROTECT:                   /* Write protected */
			break;
		default:
			break;
		}

		if (!resCard){
			printf("Disk Initialized\r\n");
			break;                /* Drive initialized. */
		}
		delay_ms(1);
	}

	/* Step 3 */
	/* Initializing filesystem */
	printf("Step 3: Initializing filesystem\r\n");
	if (f_mount(0, &Fatfs) != FR_OK){
		/* Error.No micro-SD with FAT32 is present */
		printf("Error.No micro-SD with FAT32 is present\r\n");
		while(1);
	}

	/*Step 4*/
	/* Open  the file for write */
	printf("Step 4: Opening  the file for write\r\n");
	res = f_open(&fsrc, "logfile.txt",  FA_WRITE);
	if (res != FR_OK){
		/*  If file does not exist create it*/
		res = f_open(&fsrc, "logfile.txt", FA_CREATE_ALWAYS | FA_WRITE );
		if (res != FR_OK){
			/* Error. Cannot create the file */
			printf("Error. Cannot create the file\r\n");
			while(1);
		}
	}

	/* Step 5 */
	/* Set the file write pointer to first location */
	printf("Step 5: Set the file write pointer to first location\r\n");
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK){
		/* Error. Cannot set the file write pointer */
		printf("Error. Cannot set the file write pointer\r\n");
		while(1);
	}

	/* Step 6 */
	/* Write a buffer to file */
	printf("Step 6: Write a buffer to file\r\n");
	res = f_write(&fsrc, ramBufferWrite, filecounter, &bw);
	if ((res != FR_OK) || (filecounter != bw)){
		/* Error. Cannot write the file */
		printf("Error. Cannot write the file\r\n");
		while(1);
	}

	/* Step 7 */
	/* Close the file */
	printf("Step 7: Close the file\r\n");
	f_close(&fsrc);
	if (res != FR_OK){
		/* Error. Cannot close the file */
		printf("Error. Cannot close the file\r\n");
		while(1);
	}

	/* Step8 */
	/* Open the file for read */
	printf("Step 8: Opening file to read\r\n");
	res = f_open(&fsrc, "logfile.txt",  FA_READ);
	if (res != FR_OK){
		/* Error. Cannot open the file */
		printf("Error. Cannot open the file\r\n");
		while(1);
	}

	/* Step11 */
	/* Close the file */
	printf("Step 11: Closing the file\r\n");
	res = f_close(&fsrc);
	if (res != FR_OK){
		/* Error. Cannot close the file */
		printf("Error. Cannot close the file\r\n");
		while(1);
	}
}



/*******************************************************************************
* Function Name  : _send_command
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int _send_command(uint8_t cmd, uint32_t arg, uint8_t crc){
	uint8_t r1;
	uint8_t retry;

	/* Dummy byte and chip enable */
	_spi_read_write(DUMMY_BYTE);
	_card_enable();

	/* Command, argument and crc */
	_spi_read_write(cmd | 0x40);
	_spi_read_write(arg >> 24);
	_spi_read_write(arg >> 16);
	_spi_read_write(arg >> 8);
	_spi_read_write(arg);
	_spi_read_write(crc);

	/* Wait response, quit till timeout */
	for(retry=0; retry<200; retry++){
		r1 = _spi_read_write(DUMMY_BYTE);
		if(r1 != 0xFF){
			break;
		}
	}

	/* Chip disable and dummy byte */
	_card_disable();
	_spi_read_write(DUMMY_BYTE);

	return r1;
}



/*******************************************************************************
* Function Name  : _send_command_hold
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int _send_command_hold(uint8_t cmd, uint32_t arg, uint8_t crc){
	uint8_t r1;
	uint8_t retry;

	/* Dummy byte and chip enable */
	_spi_read_write(DUMMY_BYTE);
	_card_enable();

	/* Command, argument and crc */
	_spi_read_write(cmd | 0x40);
	_spi_read_write(arg >> 24);
	_spi_read_write(arg >> 16);
	_spi_read_write(arg >> 8);
	_spi_read_write(arg);
	_spi_read_write(crc);

	/* Wait response, quit till timeout */
	for(retry=0; retry<200; retry++){
		r1 = _spi_read_write(DUMMY_BYTE);
		if(r1 != 0xFF){
			break;
		}
	}

	return r1;
}

/*******************************************************************************
* Function Name  : _spi_read_write
* Description    : None
* Input          : - data:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int _spi_read_write(uint8_t data){
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(MICROSD_SPI_PER, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(MICROSD_SPI_PER, data);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(MICROSD_SPI_PER, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI1 bus */
	return SPI_I2S_ReceiveData(MICROSD_SPI_PER);
}
