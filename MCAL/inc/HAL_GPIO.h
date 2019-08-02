/*
 * HAL_GPIO.h
 *
 *  Created on: 21-Jan-2019
 *      Author: Altamash Abdul Rahim
 */

#ifndef INC_HAL_GPIO_H_
#define INC_HAL_GPIO_H_

#include "BlueNRG_x_device.h"
#include "BlueNRG1_conf.h"
#include "HAL_TYPE.h"
#include "HAL_SYS.h"


void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif /* INC_HAL_GPIO_H_ */
