/*
===============================================================================
 Name        : main.c
 Author      : WIZnet
 Version     :
 Copyright   : WIZnet Co., Ltd. 2014
 Description : main definition
===============================================================================
*/

/*
 * @brief WIZ550web Firmware
 *
 * @note
 * Copyright(C) WIZnet Co., Ltd. 2014
 * All rights reserved.
 *
 */

/*****************************************************************************
 * Includes
 ****************************************************************************/

#include "stm32f10x.h"
#include "common.h"
#include "dataflash.h"
#include "httpServer.h"
#include "userHandler.h"
#include "uartHandler.h"
#include "rccHandler.h"
#include "gpioHandler.h"
#include "adcHandler.h"
#include "ConfigData.h"
#include "ConfigMessage.h"
#include "timerHandler.h"
#include "flashHandler.h"
#include "atcmd.h"
#include "dhcp.h"
#include "dhcp_cb.h"
#include "ftpd.h"
#include "i2cHandler.h"
#include "eepromHandler.h"

#ifdef _USE_SDCARD_
#include "ff.h"
#include "diskio.h"
#include "ffconf.h"
#else
#include "dataflash.h"
#endif

#include "stm32f10x_w5500.h"
#include "stm32f10x_sd.h"

///////////////////////////////////////
// Debugging Message Printout enable //
///////////////////////////////////////
#define _MAIN_DEBUG_
//#define _WEB_DEBUG_
#if defined (_MAIN_DEBUG_) || defined (_WEB_DEBUG_)
#include <stdio.h>
#endif

//////////////////////////////
// Shared Buffer Definition //
//////////////////////////////
#define DATA_BUF_SIZE   2048

uint8_t RX_BUF[DATA_BUF_SIZE];
uint8_t TX_BUF[DATA_BUF_SIZE];
#if defined(F_APP_FTP)
uint8_t FTP_DBUF[_MAX_SS];
#endif

////////////////////////////////
// W5500 HW Socket Definition //
////////////////////////////////
#if defined(F_APP_FTP)
#define MAX_HTTPSOCK	4
#else
#define MAX_HTTPSOCK	6
#endif

#define SOCK_CONFIG		0
#define SOCK_DHCP		1
#if defined(F_APP_FTP)
uint8_t socknumlist[] = {4, 5, 6, 7};
#else
uint8_t socknumlist[] = {2, 3, 4, 5, 6, 7};
#endif
//////////////////////////////////////////

int g_mkfs_done = 0;
int g_sdcard_done = 0;

/* Network Configuration */
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
                            .ip = {192, 168, 0, 100},
                            .sn = {255, 255, 255, 0},
                            .gw = {192, 168, 0, 1},
                            .dns = {8, 8, 8, 8},
                            .dhcp = NETINFO_STATIC };


void network_config(void);
void network_display(void);

FATFS *fatfs;				/* File system specific */

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief    Main routine for WIZ550web firmware
 * @return   Function should not exit.
 */
int main(void)
{
	uint8_t i;
#if defined (_MAIN_DEBUG_) && defined (_USE_SDCARD_)
	uint8_t ret;
#endif
#if defined(F_APP_FTP)
	wiz_NetInfo gWIZNETINFO;
#endif

	S2E_Packet *value = get_S2E_Packet_pointer();

	// MCU Initialization
	RCC_Configuration();
	NVIC_Configuration();
//	Timer_Configuration();

	// LED Initialization
	LED_Init(LED1);
	LED_Init(LED2);
	LED_Off(LED1);
	LED_Off(LED2);

	g_sdcard_done = 0;
//	g_spiflash_flag = 0;


#if defined(MULTIFLASH_ENABLE)
	probe_flash();
#endif

	/* Initialize the I2C EEPROM driver ----------------------------------------*/
#if defined(EEPROM_ENABLE)
#if defined(EEPROM_ENABLE_BYI2CPERI)
	I2C1Initialize();
#elif defined(EEPROM_ENABLE_BYGPIO)
	EE24AAXX_Init();
#endif
#endif

	// Load the Configuration data
//	load_S2E_Packet_from_storage();

	// UART Initialization
//	USART1_Configuration();
//	USART2_Configuration();
	usart2_init();

	microsd_test();

	// W5500 Initialization
	w5500_init();

	network_config();
//	network_display();

	// ADC Initialization
//	adc_dmamulti_init();

#ifdef _MAIN_DEBUG_
    printf("\r\n=======================================\r\n");
	printf(" WIZnet WIZ550web Revision %d.%d\r\n", HW_MAJOR_VER, HW_MINOR_VER);
	printf(" Embedded Web Server Module\r\n");
	printf(" Firmware Version %d.%d.%d\r\n", MAJOR_VER, MINOR_VER, MAINTENANCE_VER);
	printf("=======================================\r\n");

//	printf(" # Device Name : %s\r\n\r\n", value->module_name);
#endif

//	// Initialize Network Information
//	if(value->options.dhcp_use)		// DHCP
//	{
//		uint32_t ret;
//		uint8_t dhcp_retry = 0;
//
//#ifdef _MAIN_DEBUG_
//		printf(" - DHCP Client running\r\n");
//#endif
//
//		DHCP_init(SOCK_DHCP, TX_BUF);
//		reg_dhcp_cbfunc(w5500_dhcp_assign, w5500_dhcp_assign, w5500_dhcp_conflict);
//
//		while(1)
//		{
//			ret = DHCP_run();
//
//			if(ret == DHCP_IP_LEASED)
//			{
//#ifdef _MAIN_DEBUG_
//				printf(" - DHCP Success: DHCP Leased time : %ld Sec.\r\n\r\n", getDHCPLeasetime());
//#endif
//				break;
//			}
//			else if(ret == DHCP_FAILED)
//			{
//				dhcp_retry++;
//#ifdef _MAIN_DEBUG_
//				if(dhcp_retry <= 3) printf(" - DHCP Timeout occurred and retry [%d]\r\n", dhcp_retry);
//#endif
//			}
//
//			if(dhcp_retry > 3)
//			{
//#ifdef _MAIN_DEBUG_
//				printf(" - DHCP Failed\r\n\r\n");
//#endif
//				value->options.dhcp_use = 0;
//				Net_Conf();
//				break;
//			}
//
//			do_udp_config(SOCK_CONFIG);
//		}
//	}
//	else 								// Static
//	{
//		Net_Conf();
//	}

#ifdef _MAIN_DEBUG_
	display_Net_Info();
#endif

#if defined(F_SPI_FLASH)
	ret = flash_mount();

	if(ret > 0)
	{
		LED_On(LED2);
#ifdef _MAIN_DEBUG_
		sd_displayCardInfo(ret);
#endif
	}
#endif

#ifdef _USE_SDCARD_
	DSTATUS resCard;			/* SDcard status */
	// SD card Initialization
//	ret = mmc_mount();
	/* Step2 */
	/* Detect micro-SD */
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
#if defined(DEBUG_ENABLE_USART)
			printf("Disk Initialized\r\n");
#endif
			break;                /* Drive initialized. */
		}
		delay_ms(1);
	}

	/* Step3 */
	/* Initialize filesystem */
	if (f_mount(0, fatfs) != FR_OK){
		/* Error.No micro-SD with FAT32 is present */
		printf("Error.No micro-SD with FAT32 is present\r\n");
		while(1);
	}

	if(ret <= 0)
	{
#ifdef _MAIN_DEBUG_
		printf("\r\n - Can't mount SD card: Please Reboot WIZ550web Board or Insert SD card\r\n");
#endif
		//while(!(ret = mmc_mount()));
	}

	if(ret > 0)
	{
		LED_On(LED1);
#ifdef _MAIN_DEBUG_
//		sd_displayCardInfo(ret);
#endif
	}

#if 0
    res = f_open(&file, "0:/wr_test.txt", FA_CREATE_NEW | FA_CREATE_ALWAYS | FA_WRITE);
    printf("f_open:%d\r\n", res);
    strncpy(szbuff, "WIZnet WiKi", 11);
    res = f_write(&file, szbuff, 11, &br);

    printf("f_write:%d\r\n", res);
    printf("WIZnet WiKi\r\n");
    printf("Written 11bytes.\r\n");

    f_close(&file);
#endif

#else
	// External DataFlash Initialization
	DataFlash_Init(); // DF CS Init
#endif

//	atc_init();

	httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist);

#ifdef _USE_WATCHDOG_
	reg_httpServer_cbfunc(NVIC_SystemReset, IWDG_ReloadCounter); // Callback: STM32 MCU Reset / WDT Reset (IWDG)

#else
	reg_httpServer_cbfunc(NVIC_SystemReset, NULL); // Callback: STM32 MCU Reset
#endif
//	IO_status_init();

#if defined(F_APP_FTP)
	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	ftpd_init(gWIZNETINFO.ip);	// Added by James for FTP
#endif

#ifdef _USE_WATCHDOG_
	// IWDG Initialization: STM32 Independent WatchDog
	IWDG_Configureation();
#endif

	// Main Routine
	while(1)
	{
#ifdef _USE_WATCHDOG_
		IWDG_ReloadCounter(); // Feed IWDG
#endif

//		do_udp_config(SOCK_CONFIG);				// Configuration tool handler
//		atc_run();								// AT Command handler

//		if(value->options.dhcp_use)
//			DHCP_run();	// DHCP client handler

#if defined(FACTORY_FW)
		factory_run();
		if (g_factoryfw_flag == 1)
		{
			check_factory_uart1();
		}
#endif
#if defined(F_SPI_FLASH) && defined(F_APP_FTP)
		sflash_run();
#endif

		for(i = 0; i < MAX_HTTPSOCK; i++)
			httpServer_run(i);	// HTTP server handler

#if defined(F_APP_FTP)
		ftpd_run(FTP_DBUF);
#endif
#ifdef _USE_WATCHDOG_
		IWDG_ReloadCounter(); // Feed IWDG
#endif
	} // End of main routine
}


/* wizchip netconf */
void network_config(void){
	ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
}

void network_display(void){
	uint8_t tmpstr[6] = {0,};

	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);

	// Display Network Information
	ctlwizchip(CW_GET_ID, (void*)tmpstr);

	if(gWIZNETINFO.dhcp == NETINFO_DHCP)
		printf("\r\n===== %s NET CONF : DHCP =====\r\n",(char*)tmpstr);
	else
		printf("\r\n===== %s NET CONF : Static =====\r\n",(char*)tmpstr);
	ctlwizchip(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	printf(" MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
	printf(" IP : %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
	printf(" GW : %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
	printf(" SN : %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
	printf("=======================================\r\n");
}


