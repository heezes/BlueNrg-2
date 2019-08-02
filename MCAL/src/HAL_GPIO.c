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
	gpio.GPIO_Pin = GPIO_Pin_4;
	gpio.GPIO_Pull = DISABLE;
	GPIO_Init(&gpio);
}


void LED_On(void)
{
	GPIO_SetBits(GPIO_Pin_4);
}

void LED_Off(void)
{
	GPIO_ResetBits(GPIO_Pin_4);
}

void LED_Toggle(void)
{
	GPIO_ToggleBits(GPIO_Pin_4);
}
