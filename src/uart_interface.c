/*
 * uart_interface.c
 *
 *  Created on: 04-Apr-2019
 *  Author: Altamash Abdul Rahim
 *	Description:
 *
 *
 * Vecmocon Technologies Private Limited [CONFIDENTIAL]
 * Unpublished Copyright (c) 2019 [Vecmocon Technologies Private Limited], All Rights Reserved.
 *
 *
 * NOTICE:  All information contained herein is, and remains the property of COMPANY. The intellectual and technical concepts contained
 * herein are proprietary to COMPANY and may be covered by Indian and Foreign Patents, patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is strictly forbidden unless prior written permission is obtained
 * from COMPANY.  Access to the source code contained herein is hereby forbidden to anyone except current COMPANY employees, managers or contractors who have executed
 * Confidentiality and Non-disclosure agreements explicitly covering such access.
 *
 *
 * The copyright notice above does not evidence any actual or intended publication or disclosure  of  this source code, which includes 
 * information that is confidential and/or proprietary, and is a trade secret, of  COMPANY.   ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC  PERFORMANCE,
 * OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS  SOURCE CODE  WITHOUT  THE EXPRESS WRITTEN CONSENT OF COMPANY IS STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE
 * LAWS AND INTERNATIONAL TREATIES.  THE RECEIPT OR POSSESSION OF  THIS SOURCE CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS 
 * TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE, OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR IN PART.                
 *
 * Change log:
 */

#include "uart_interface.h"
#include "MCAL.h"
#include "stdio.h"

UartHandleTypeDef uart;

extern void BleQueueStoreFromISR(uint8_t data);

/*Microcontroller Dependent Callbacks*/
void HAL_uart_RxCallback(UartHandleTypeDef* uart)
{
	BleQueueStoreFromISR(*(uart->RxPtr));
	if(uart->mode == HAL_INTERRUPT_MODE)
		HAL_UART_RECEIVE_IT(uart, uart->RxPtr,1);
}

void HAL_uart_ErrorCallback(UartHandleTypeDef* uart)
{
	/*TODO: Add Error Handling
	 * NOTE: Incase of DMA ERROR RESTART the DMA Transmission
	 * */
#if DEBUG
	iprintf("UART ERRR: %d\n", uart->ErrorCode);
#endif
}

/*Initializes your UART
 * The COM Structure setting has no effect on the Peripheral.
 * Any changes in the Peripheral have to be done from here
 * */
void UART_Initialize(uint8_t instance)
{
	uart.idx = 1;
	uart.baudrate = 115200;
	uart.mode = (uint8_t)HAL_DMA_MODE;
	HAL_UART_INIT(&uart);
}
void UART_DeInitialize(uint8_t instance)
{
	/*Add DeInit Logic Here*/
}

int UART_Send(uint8_t instance, uint8_t* buf, int len, int timeout)
{
	return (HAL_UART_SEND(&uart,buf,len,timeout) == HAL_OK) ? 0 : -1;
}

int UART_SendNonBlocking(uint8_t instance, uint8_t* buf, int len)
{
	int ret = -1;
	uart.RxPtr = buf;
	if(uart.mode == HAL_INTERRUPT_MODE)
	{ ret = (HAL_UART_SEND_IT(&uart, uart.RxPtr, len) == HAL_OK) ? 0 : -1;}
	else if(uart.mode == HAL_DMA_MODE)
	{ ret = (HAL_UART_SEND_DMA(&uart, uart.RxPtr, len) == HAL_OK) ? 0 : -1;}
	else
		return ret;
	return ret;
}

int UART_Receive(uint8_t instance, uint8_t* buf, int len, int timeout)
{
	return (HAL_UART_RECEIVE(&uart,buf,len,timeout) == HAL_OK) ? 0:1;
}

int UART_StartReceiveNonBlocking(uint8_t instance, uint8_t* buf, int len)
{
	int ret = -1;
	if(uart.mode == HAL_INTERRUPT_MODE)
	{ ret = (HAL_UART_RECEIVE_IT(&uart, (uint8_t*)buf, len) == HAL_OK) ? 0 : -1;}
	else if(uart.mode == HAL_DMA_MODE)
	{ ret = (HAL_UART_RECEIVE_DMA(&uart, (uint8_t*)buf, len) == HAL_OK) ? 0 : -1;}
	else
		return ret;
	return ret;
	return 0;
}

