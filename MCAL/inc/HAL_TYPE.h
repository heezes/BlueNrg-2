/*
 * HAL_TYPE.h
 *
 *  Created on: 12-Jan-2019
 *      Author: Altamash Abdul Rahim
 */

#ifndef INC_HAL_TYPE_H_
#define INC_HAL_TYPE_H_

/*Peripheral States*/
typedef enum {
	STATE_IDLE = 0, /*Peripheral is Available for Use*/
	STATE_BUSY_TX, /*Peripheral is Busy Transmitting*/
	STATE_BUSY_RX, /*Peripheral is Busy Receiving*/
	STATE_ERROR /*Peripheral Error*/
} StateTypeDef;


typedef enum
{HAL_OK = 0, HAL_ERROR = -1, HAL_BUSY = -2, HAL_TIMEOUT = -4}HAL_TYPE;

/*Private Defines*/
#define MAX_TIMEOUT 0xFFFF

#endif /* INC_HAL_TYPE_H_ */
