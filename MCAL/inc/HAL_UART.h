/*
 * HAL_uart.h
 *
 *  Created on: 12-Jan-2019
 *      Author: Altamash Abdul Rahim
 */

#ifndef MCAL_HAL_uart_H_
#define MCAL_HAL_uart_H_

#include "BlueNRG_x_device.h"
#include "BlueNRG1_conf.h"
#include "HAL_TYPE.h"
#include "HAL_SYS.h"

#define UART_4_5  0
#define UART_8_11 1

/*uart BAUD RATE RANGE*/
#define MAX_BAUD	921600
#define MIN_BAUD	4800

/*uart DMA MACRO*/
#define uart_DMA_RX 	DMA_CH0
#define uart_DMA_TX 	DMA_CH1
#define DMA_ERROR_RX	DMA_FLAG_TE0
#define DMA_ERROR_TX	DMA_FLAG_TE1
#define DMA_RX_CPLT		DMA_FLAG_TC0
#define DMA_TX_CPLT		DMA_FLAG_TC1
#define DMA_RX_GLOBAL	DMA_FLAG_GL0
#define DMA_TX_GLOBAL	DMA_FLAG_GL1

/*uart ERROR CODE*/
#define HAL_uart_ERROR_NONE         0x00000000U   /*!< No error            */
#define HAL_uart_ERROR_PE           0x00000001U   /*!< Parity error        */
#define HAL_uart_ERROR_NE           0x00000002U   /*!< Noise error         */
#define HAL_uart_ERROR_FE           0x00000004U   /*!< Frame error         */
#define HAL_uart_ERROR_ORE          0x00000008U   /*!< Overrun error       */
#define HAL_uart_ERROR_DMA          0x00000010U   /*!< DMA transfer error  */


/*uart Structure*/
typedef struct
{
	uint8_t 						idx;
	uint32_t						baudrate;
	uint8_t							mode;
	volatile StateTypeDef			state;
	uint8_t							ErrorCode;
	uint8_t*						RxPtr;
	volatile uint16_t    			RxSize;
	uint8_t*						TxPtr;
	volatile uint16_t				TxSize;
}UartHandleTypeDef;

#define HAL_POLLING_MODE  	0
#define HAL_INTERRUPT_MODE	1
#define HAL_DMA_MODE		2
/*Function Prototype*/
void HAL_UART_INIT(UartHandleTypeDef* uart);
HAL_TYPE HAL_UART_SEND(UartHandleTypeDef* uart, uint8_t* data, int len,int timeout);
HAL_TYPE HAL_UART_RECEIVE(UartHandleTypeDef* uart, uint8_t* data, int len, int timeout);
HAL_TYPE HAL_UART_SEND_IT(UartHandleTypeDef* uart, uint8_t* data, int len);
HAL_TYPE HAL_UART_RECEIVE_IT(UartHandleTypeDef* uart, uint8_t* data, int len);
HAL_TYPE HAL_UART_SEND_DMA(UartHandleTypeDef* uart, uint8_t* data, int len);
HAL_TYPE HAL_UART_RECEIVE_DMA(UartHandleTypeDef* uart, uint8_t* data, int len);
void HAL_UART_IRQ(UartHandleTypeDef* uart);


/*Private Define*/
#define _IS_VALID_BAUD(x)	(MIN_BAUD<=x) ? ((x<=MAX_BAUD)?(1):(0)):(0)
#define _IS_VALID_MODE(x)	((x==HAL_POLLING_MODE)				||	\
							 (x==HAL_INTERRUPT_MODE)			||	\
							 (x==HAL_DMA_MODE))
#endif /* MCAL_HAL_uart_H_ */
