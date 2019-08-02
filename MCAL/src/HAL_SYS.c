/*
 * HAL_SYS.c
 *
 *  Created on: 19-Jan-2019
 *      Author: Altamash Abdul Rahim
 */

#include "HAL_SYS.h"

#define uwTickFreq 1

volatile uint32_t lSystickCounter=0;

void SystickInit(void)
{
	SysTick_Config(SYST_CLOCK/1000U  - 1);
}

void HAL_IncTick(void)
{lSystickCounter++;}

uint32_t HAL_GetTick(void)
{return lSystickCounter;}

void HAL_Delay(uint32_t delay)
{
	uint32_t tickstart = HAL_GetTick();
	uint32_t wait = delay;
	wait += (uint32_t)(uwTickFreq);
	while((HAL_GetTick() - tickstart) < wait)
	{
	}
}
