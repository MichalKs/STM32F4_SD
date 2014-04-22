/**
 * @file: 	uart.c
 * @brief:	   
 * @date: 	12 kwi 2014
 * @author: Michal Ksiezopolski
 * 
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the 
 * accompanying materials are made available 
 * under the terms of the GNU Public License 
 * v3.0 which accompanies this distribution, 
 * and is available at 
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#include "uart.h"
#include "fifo.h"

#define UART2_BUF_LEN 512 ///< UART buffer lengths

static uint8_t rxBuffer[UART2_BUF_LEN]; ///< Buffer for received data.
static uint8_t txBuffer[UART2_BUF_LEN]; ///< Buffer for transmitted data.

static FIFO_TypeDef rxFifo; ///< RX FIFO
static FIFO_TypeDef txFifo; ///< TX FIFO

/**
 * @brief Initialize USART2
 */
void UART2_Init(void) {

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// Enable clocks
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// USART2 TX on PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// USART2 RX on PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Connect USART2 TX pin to AF2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// USART initialization
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	// Enable USART2
	USART_Cmd(USART2, ENABLE);

    // Enable RXNE interrupt
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	// Disable TXE interrupt
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

	// Enable USART2 global interrupt
	NVIC_EnableIRQ(USART2_IRQn);

	// Initialize RX FIFO
	rxFifo.buf=rxBuffer;
	rxFifo.len=UART2_BUF_LEN;
	FIFO_Add(&rxFifo);

	// Initialize TX FIFO
	txFifo.buf=txBuffer;
	txFifo.len=UART2_BUF_LEN;
	FIFO_Add(&txFifo);

}
/**
 * @brief Send a char to USART2.
 * @param c Char to send.
 */
void USART2_Putc(uint8_t c) {

	FIFO_Push(&txFifo,c); // Put data in TX buffer
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE); // Enable transmit buffer empty interrupt

}
/**
 * @brief Get a char from USART2
 * @return Received char.
 * @warning Waiting function!
 */
uint8_t USART2_Getc(void) {

	uint8_t c;

	while (FIFO_IsEmpty(&rxFifo) == 1); // wait until buffer is empty

	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); // disable RX interrupt

	FIFO_Pop(&rxFifo,&c); // Get data from RX buffer

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // enable RX interrupt

	return c;
}
/**
 * @brief IRQ handler for USART2
 */
void USART2_IRQHandler(void) {

	// If transmit buffer empty interrupt
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {

    	uint8_t c;

    	if (FIFO_Pop(&txFifo,&c) == SUCCESS) { // If buffer not empty

    		USART_SendData(USART2, c); // Send data

    	} else {
    		// Turn off TXE interrupt
    		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    	}

    }

    // If received buffer not empty interrupt
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {

    	uint8_t c = USART_ReceiveData(USART2); // Get data from UART

    	FIFO_Push(&rxFifo, c); // Put data in RX buffer

    }

}

