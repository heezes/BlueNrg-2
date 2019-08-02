/*
 * HAL_uart.c
 *
 *  Created on: 12-Jan-2019
 *      Author: Altamash Abdul Rahim
 */
/*
TODO: Add DeInit Functions
TODO: Add Parity and Data length option(5-8 bits)
TODO: Add Flow Control
TODO: Add state machine in DMA
*/

#include "HAL_uart.h"

static void uartGpio_Init(uint8_t idx);
static void uartIrq_Init(UartHandleTypeDef* uart);
static void uartDma_Init(UartHandleTypeDef* uart);
static HAL_TYPE uart_RECEIVE_IT(UartHandleTypeDef* uart);
static HAL_TYPE uart_SEND_IT(UartHandleTypeDef* uart);
static HAL_TYPE uart_RECEIVE_DMA(UartHandleTypeDef* uart);
static HAL_TYPE uart_SEND_DMA(UartHandleTypeDef* uart);

/*uart Initialization Funtion*/
void HAL_UART_INIT(UartHandleTypeDef* uart)
{
	assert_param(uart);
	assert_param(_IS_VALID_BAUD(uart->baudrate));
	assert_param(_IS_VALID_MODE(uart->mode));
	//asser_param(_IS_VALID_STATE);
	uartGpio_Init(uart->idx);
	/*Initialize uart PARAMS*/
	UART_InitType UART_InitStruct;
	UART_StructInit(&UART_InitStruct);
	UART_InitStruct.UART_BaudRate	= uart->baudrate;
	UART_InitStruct.UART_FifoEnable = ENABLE;
	UART_Init(&UART_InitStruct);
	/* Interrupt as soon as data is received. */
	UART_TxFifoIrqLevelConfig(FIFO_LEV_1_64);
	UART_RxFifoIrqLevelConfig(FIFO_LEV_1_64);
	if(uart->mode == (HAL_INTERRUPT_MODE))
	{
		uartIrq_Init(uart);
		uart->state = STATE_IDLE;
		UART_Cmd(ENABLE);
	}
	else if(uart->mode == (HAL_DMA_MODE))
	{
		UART_Cmd(ENABLE);
		uartDma_Init(uart);
	}
	else
		UART_Cmd(ENABLE);
}

/*uart Send Function
For Use:
1) Point the string to the TxPtr member of uart Struct.
2) Set TXSize member to string len.
3) Provide Timeout
*/
HAL_TYPE HAL_UART_SEND(UartHandleTypeDef* uart, uint8_t* data, int len,int timeout)
{
/*
	if(!UART_GetFlagStatus(UART_FLAG_BUSY))
	{
*/
		uart->TxPtr = data;
		uart->TxSize = len;
		uint32_t tick  = HAL_GetTick();
		while(uart->TxSize != 0)
		{
		  if(UART_GetFlagStatus(UART_FLAG_TXFE) == SET)
		  {
		  	  UART_SendData((uint16_t)(*uart->TxPtr++));
			  uart->TxSize--;
		  }
		  if((HAL_GetTick() - tick) > timeout)
			  return HAL_TIMEOUT;
		}
		return HAL_OK;
/*
	}
	else 
		return HAL_BUSY;
*/
}

/*uart Receive Function
For Use:
1) Point the Receive buffer to RxPtr 
2) Set Rx Size to lenght to receive.
3) Set Timeout for receiving. 
*/
HAL_TYPE HAL_UART_RECEIVE(UartHandleTypeDef* uart, uint8_t* data, int len, int timeout)
{
	if(!UART_GetFlagStatus(UART_FLAG_BUSY))
	{
		uart->TxPtr = data;
		uart->TxSize = len;
		uint32_t tick = HAL_GetTick();
		while(uart->RxSize > 0)
		{
 		  if(UART_GetFlagStatus(UART_FLAG_RXFE) == RESET)
		  {
		  	(*uart->RxPtr++) = (uint8_t)UART_ReceiveData();
			  uart->RxSize--;
		  }
		  if((HAL_GetTick() - tick) > timeout)
			  return HAL_TIMEOUT;
		}
		return HAL_OK;	
	}
	else
		return HAL_BUSY;
}

HAL_TYPE HAL_UART_SEND_IT(UartHandleTypeDef* uart, uint8_t* data, int len)
{
	if(uart->state == STATE_IDLE)
	{
		uart->state = STATE_BUSY_TX;
		uart->TxPtr = data;
		uart->TxSize = len;
		UART_ITConfig(UART_IT_TX|UART_IT_TXFE|UART_IT_OE|UART_IT_BE|UART_IT_PE|UART_IT_FE|UART_IT_RT, ENABLE);
		UART_SendData(*uart->TxPtr++);
		--uart->TxSize;
		return HAL_OK;
	}
	else
		return HAL_BUSY;
}

HAL_TYPE HAL_UART_RECEIVE_IT(UartHandleTypeDef* uart, uint8_t* data, int len)
{
	if(uart->state == STATE_IDLE)
	{
		uart->state = STATE_BUSY_RX;
		uart->RxPtr = data;
		uart->RxSize = len;
		UART_ITConfig(UART_IT_RX|UART_IT_OE|UART_IT_BE|UART_IT_PE|UART_IT_FE|UART_IT_RT, ENABLE);
		return HAL_OK;		
	}
	else 
		return HAL_BUSY;
}

HAL_TYPE HAL_UART_SEND_DMA(UartHandleTypeDef* uart, uint8_t* data, int len)
{
		uart->TxPtr = data;
		uart->TxSize = len;
		/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
		  /* Write to DMAy Channelx CMAR */
		uart_DMA_TX->CMAR = (uint32_t)uart->TxPtr;
		/*--------------------------- DMAy Channelx CNDTR Configuration ---------------*/
		DMA_SetCurrDataCounter(uart_DMA_TX, (uart->TxSize));
		DMA_Cmd(uart_DMA_TX, ENABLE);
		DMA_FlagConfig(uart_DMA_TX, DMA_FLAG_TC | DMA_FLAG_TE, ENABLE);
		return HAL_OK;
}

HAL_TYPE HAL_UART_RECEIVE_DMA(UartHandleTypeDef* uart, uint8_t* data, int len)
{
		uart->RxPtr = data;
		uart->RxSize = len;
		/*--------------------------- DMAy Channelx CNDTR Configuration ---------------*/
		  /* Write to DMAy Channelx CNDTR */
		uart_DMA_RX->CNDTR = (uint32_t)(uart->RxSize);
		/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
		  /* Write to DMAy Channelx CMAR */
		uart_DMA_RX->CMAR = (uint32_t)uart->RxPtr;
		DMA_Cmd(uart_DMA_RX, ENABLE);
		DMA_FlagConfig(uart_DMA_RX, DMA_FLAG_TC | DMA_FLAG_TE, ENABLE);
		return HAL_OK;
}

__weak void HAL_uart_RxCallback(UartHandleTypeDef* uart)
{

}

__weak void HAL_uart_TxCallback(UartHandleTypeDef* uart)
{

}

__weak void HAL_uart_ErrorCallback(UartHandleTypeDef* uart)
{

}

/*uart IRQ Handler
Can be used for both uart NVIC Irq and DMA Irq
*/
void HAL_UART_IRQ(UartHandleTypeDef* uart)
{
	uint32_t errorflags = UART_GetITStatus(UART_IT_RT | UART_IT_OE | UART_IT_BE | UART_IT_PE| UART_IT_FE);
	if((errorflags == RESET) && (DMA_GetFlagStatus(DMA_ERROR_RX | DMA_ERROR_TX) == RESET))
	{
		if((uart->mode == HAL_INTERRUPT_MODE))
		{
			/*Check Receiving IRQ*/
			if(UART_GetITStatus(UART_IT_RX))
				uart_RECEIVE_IT(uart);
			/*Check Transmitting IRQ*/
			else if(UART_GetITStatus(UART_IT_TX|UART_IT_TXFE))
				uart_SEND_IT(uart);
			else/*ADD other uart Interrupt Handling*/
				__NOP();
		}
		else
		{
			if(DMA_GetFlagStatus(DMA_RX_CPLT))
				uart_RECEIVE_DMA(uart);
			else
				uart_SEND_DMA(uart);
		}
	}
	else
	{	
		uart->ErrorCode = errorflags;
		if((errorflags & UART_IT_PE) == UART_IT_PE)
			UART_ClearITPendingBit(UART_IT_PE);
		if(DMA_GetFlagStatus(DMA_ERROR_RX | DMA_ERROR_TX) == SET)
			uart->ErrorCode = HAL_uart_ERROR_DMA;
		HAL_uart_ErrorCallback(uart);
	}
}

static void uartGpio_Init(uint8_t idx)
{
	GPIO_InitType GPIO_InitStruct;
	if(idx)
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
	else
		GPIO_InitStruct.GPIO_Pin = 	GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = Serial1_Mode;
	GPIO_InitStruct.GPIO_HighPwr = DISABLE;
	GPIO_InitStruct.GPIO_Pull = DISABLE;
	GPIO_Init(&GPIO_InitStruct);
}

/*Configure uart IRQ*/
static void uartIrq_Init(UartHandleTypeDef* uart)
{
	  /* Enable the uart Interrupt */
	  NVIC_InitType NVIC_InitStructure;
	  NVIC_InitStructure.NVIC_IRQChannel = UART_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LOW_PRIORITY;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
}

/*Configure uart DMA*/
static void uartDma_Init(UartHandleTypeDef* uart)
{
	DMA_InitType DMA_InitStruct;
	NVIC_InitType NVIC_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStruct.DMA_PeripheralBaseAddr = UART_BASE;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_BufferSize = 1;//Setting Default min length
	/*Config DMA RX Stream*/
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	/*Initialize the configured*/
	DMA_Init(uart_DMA_RX, &DMA_InitStruct);
	UART_DMACmd(UART_DMAReq_Rx, ENABLE);
	/*Config the DMA Tx Stream*/
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	/*Initialize the configured*/
	DMA_Init(uart_DMA_TX, &DMA_InitStruct);
	UART_DMACmd(UART_DMAReq_Tx, ENABLE);

	/*Attach Interrupt to the DMA Stream*/
	NVIC_InitStruct.NVIC_IRQChannel = DMA_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = HIGH_PRIORITY;
	NVIC_Init(&NVIC_InitStruct);
}

/*HAL uart*/
static HAL_TYPE uart_RECEIVE_IT(UartHandleTypeDef* uart)
{
	if(uart->state == STATE_BUSY_RX)
	{
	   *(uart->RxPtr) = (uint8_t)UART_ReceiveData();
		if(--uart->RxSize==0)
		{
		   /* Clear the interrupt */
		   UART_ClearITPendingBit(UART_IT_RX);
		   uart->state = STATE_IDLE;
		   UART_ITConfig(UART_IT_RX|UART_IT_OE|UART_IT_BE|UART_IT_PE|UART_IT_FE|UART_IT_RT, DISABLE);
		   HAL_uart_RxCallback(uart);
		}
		return HAL_OK;
	}
	else
		return HAL_BUSY;
}
/**/
static HAL_TYPE uart_SEND_IT(UartHandleTypeDef* uart)
{
	if(uart->state == STATE_BUSY_TX)
	{
		UART_SendData((uint16_t)(*uart->TxPtr++));
		if(--uart->TxSize==0)
		{
			/* Clear the interrupt */
			UART_ClearITPendingBit(UART_IT_TX);
			uart->state = STATE_IDLE;
			UART_ITConfig(UART_IT_TX|UART_IT_TXFE|UART_IT_OE|UART_IT_BE|UART_IT_PE|UART_IT_FE|UART_IT_RT, DISABLE);
			HAL_uart_TxCallback(uart);
		}
		return HAL_OK;
	}
	return HAL_BUSY;
}
/**/
static HAL_TYPE uart_RECEIVE_DMA(UartHandleTypeDef* uart)
{
	 DMA_ClearFlag(DMA_RX_GLOBAL);
	 HAL_uart_RxCallback(uart);
	 return HAL_OK;
}
/**/
static HAL_TYPE uart_SEND_DMA(UartHandleTypeDef* uart)
{
	 DMA_ClearFlag(DMA_TX_GLOBAL);
	 DMA_Cmd(uart_DMA_TX, DISABLE);
	 DMA_FlagConfig(uart_DMA_TX, DMA_FLAG_TC | DMA_FLAG_TE, DISABLE);
	 HAL_uart_TxCallback(uart);
	 return HAL_OK;
}
