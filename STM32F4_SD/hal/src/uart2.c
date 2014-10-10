/**
 * @file:   uart2.c
 * @brief:  Controlling UART
 * @date:   12 kwi 2014
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

#include <uart2.h>
#include <stm32f4xx.h>

/**
 * @addtogroup USART2
 * @{
 */

void    (*rxCallback)(uint8_t);   ///< Callback function for receiving data
uint8_t (*txCallback)(uint8_t*);  ///< Callback function for transmitting data

/**
 * @brief Initialize USART2
 * @param baud
 * @param rxCb
 * @param txCb
 */
void UART2_Init(uint32_t baud, void(*rxCb)(uint8_t), uint8_t(*txCb)(uint8_t*) ) {

  // assign the callbacks
  rxCallback = rxCb;
  txCallback = txCb;

  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  // Enable clocks for peripherals
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);

  // USART2 TX on PA2
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // USART2 RX on PA3
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Connect USART2 TX pin to AF2
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

  // USART initialization (standard 8n1)
  USART_InitStructure.USART_BaudRate            = baud;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  // Enable USART2
  USART_Cmd(USART2, ENABLE);

  // Enable RXNE interrupt
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  // Disable TXE interrupt - we enable it only when there is
  // data to send
  USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

  // Enable USART2 global interrupt
  NVIC_EnableIRQ(USART2_IRQn);

}
/**
 * @brief Enable transmitter.
 * @details This function has to be called by the higher layer
 * in order to start the transmitter.
 */
void UART2_TxEnable(void) {
  USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

/**
 * @brief IRQ handler for USART2
 */
void USART2_IRQHandler(void) {

  // If transmit buffer empty interrupt
  if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {

    uint8_t c;

    if (txCallback) { // if not NULL
      // get data from higher layer using callback
      if (txCallback(&c)) {
        USART_SendData(USART2, c); // Send data
      } else { // if no more data to send disable the transmitter
        USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
      }
    }
  }

  // If RX buffer not empty interrupt
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {

    uint8_t c = USART_ReceiveData(USART2); // Get data from UART

    if (rxCallback) { // if not NULL
      rxCallback(c); // send received data to higher layer
    }
  }
}

/**
 * @}
 */
