/*
 * HAL_SYS.h
 *
 *  Created on: 19-Jan-2019
 *      Author: Altamash Abdul Rahim
 */

#ifndef INC_HAL_SYS_H_
#define INC_HAL_SYS_H_

#include "BlueNRG_x_device.h"
#include "BlueNRG1_conf.h"
#include "HAL_TYPE.h"


void SystickInit(void);
void HAL_IncTick(void);
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t delay);

#define TICK_DEFINED	1

#endif /* INC_HAL_SYS_H_ */
