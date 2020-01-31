/*
 * HAL_GPIO.c
 *
 *  Created on: 21-Jan-2019
 *      Author: Altamash Abdul Rahim
 */
#include "HAL_GPIO.h"

void LED_Init(void)
{
	GPIO_InitType gpio;
	gpio.GPIO_HighPwr = DISABLE;
	gpio.GPIO_Mode = GPIO_Output;
#ifdef BMS
	gpio.GPIO_Pin = GPIO_Pin_4;
#else
	gpio.GPIO_Pin = GPIO_Pin_1;
#endif
	gpio.GPIO_Pull = DISABLE;
	gpio.GPIO_Pull = DISABLE;
	GPIO_Init(&gpio);
}

void LED_On(void)
{
#ifdef BMS
	GPIO_SetBits(GPIO_Pin_4);
#else
	GPIO_SetBits(GPIO_Pin_1);
#endif
}

void LED_Off(void)
{
#ifdef BMS
	GPIO_ResetBits(GPIO_Pin_4);
#else
	GPIO_ResetBits(GPIO_Pin_1);
#endif
}

void LED_Toggle(void)
{
#ifdef BMS
	GPIO_ToggleBits(GPIO_Pin_4);
#else
	GPIO_ToggleBits(GPIO_Pin_1);
#endif
}


#ifdef VIM

void WAKEUP_Init(void)
{
	GPIO_InitType gpio;
	gpio.GPIO_HighPwr = DISABLE;
	gpio.GPIO_Mode = GPIO_Output;
	gpio.GPIO_Pin = GPIO_Pin_5;
	gpio.GPIO_Pull = DISABLE;
	GPIO_Init(&gpio);
}

void WAKEUP_Pulse(void)
{
	GPIO_SetBits(GPIO_Pin_5);
	HAL_Delay(50);
	GPIO_ResetBits(GPIO_Pin_5);
}
#endif
