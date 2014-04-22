/**
 * @file: 	uart.h
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

#ifndef UART_H_
#define UART_H_

#include <stm32f4xx.h>

void UART2_Init(void);
void USART2_Putc(uint8_t c);
uint8_t USART2_Getc(void);

#endif /* UART_H_ */
