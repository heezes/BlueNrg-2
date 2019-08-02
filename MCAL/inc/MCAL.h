/*
 * MCAL.h
 *
 *  Created on: 12-Jan-2019
 *      Author: Altamash Abdul Rahim
 */

#ifndef MCAL_INC_MCAL_H_
#define MCAL_INC_MCAL_H_

#include "string.h"
#include "stdio.h"

#ifdef BLUENRG2_DEVICE
	#include "HAL_UART.h"	
	#include "HAL_GPIO.h"
	#include "HAL_SYS.h"
#elif STM32F405
	#include "stm32f4xx_hal.h"
#else
	#error "Target Device not Defined"
#endif



#endif /* MCAL_INC_MCAL_H_ */
