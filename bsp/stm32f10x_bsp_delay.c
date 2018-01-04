/*******************************************************************************
 * @File:			stm32f10x_bsp_delay.c
 * @Author:			Aytac Dilek
 * @email:			aytacdilek@gmail.com
 * @Web:
 * @Date:			03.16.2016
 * @Description:	STM32F10x library for delay functions
 * @Version:		1.0
 * @Platform:		OPEN103Z-B
 * @CPU:			STM32F103ZET6
 * @IDE:			CooCox CoIDE 1.7.8
 * @GCC:			4.8 2014q2
 * @Module:			None
 * @Function:		None
 * @Notes:			None
*******************************************************************************/

/* Includes *******************************************************************/
#include "stm32f10x_bsp_delay.h"



volatile uint32_t millisec = 0;
uint32_t micros = 0;
uint8_t freqMhz = 72;



/* Global Functions ***********************************************************/

/*******************************************************************************
* Function:		delay_us
* Description:	Delay us
* Input:		None
* Output:		None
* Return:		None
* Attention:	None
*******************************************************************************/
void delay_init(void){
	if (SysTick_Config(SystemCoreClock / 1000)){
		/* Capture error */
		while (1);
	}

	//Just making sure that the Systick Interrupt is top priority
	//Or the timer wont be accurate
	NVIC_SetPriority(SysTick_IRQn, 0);
}



/*******************************************************************************
* Function:		delay_us
* Description:	Delay us
* Input:		None
* Output:		None
* Return:		None
* Attention:	None
*******************************************************************************/
void delay_us(uint16_t us){
#if defined(DELAY_ENABLE_LOOP)
	uint16_t i, j;

	for (i = 0; i < us; ++i) {
		for (j = 0; j < 3; ++j) {
			asm("nop");
		}
	}
#elif defined(DELAY_ENABLE_SYSTICK)
//	SysTick_Config(SystemCoreClock / DELAY_TICK_FREQUENCY_MS);
//	TimingDelay = us+1;
//	while (TimingDelay != 0);
#endif
}



/*******************************************************************************
 * Function Name:	delay_ms
 * Description:		Delay ms
 * Input:			None
 * Output:			None
 * Return:			None
 * Note:			None
 ******************************************************************************/
void delay_ms(uint16_t ms){
#if defined(DELAY_ENABLE_LOOP)
	uint16_t i, j, k;

	for (i = 0; i < ms; ++i) {
		for (j = 0; j < 1125; ++j) {
			for (k = 0; k < 3; ++k) {
				asm("nop");
			}
		}
	}
#elif defined(DELAY_ENABLE_SYSTICK)
//	SysTick_Config(SystemCoreClock / DELAY_TICK_FREQUENCY_MS);
	millisec = ms;
	while ((milisec - ms) != 0);
#endif
}


uint32_t millis(void)
{
	return millisec;
}


/*******************************************************************************
 * Function Name:	SysTick_Handler
 * Description:		This function handles SysTick Handler.
 * Input:			None
 * Output:			None
 * Return:			None
 * Note:			None
 ******************************************************************************/
void SysTick_Handler(void){
//	TimingDelay_Decrement();
	millisec++;
}
