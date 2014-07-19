/**
 * @file: 	spi.h
 * @brief:	SPI control functions
 * @date: 	22 kwi 2014
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

#ifndef SPI_H_
#define SPI_H_

#include <stm32f4xx.h>


uint8_t SPI1_Transmit(uint8_t data);
void SPI1_Init(void);
void SPI1_Select(void);
void SPI1_Deselect(void);
void SPI1_ReadBuffer(uint8_t* buf, uint32_t len);
void SPI1_SendBuffer(uint8_t* buf, uint32_t len);
void SPI1_TransmitBuffer(uint8_t* rx_buf, uint8_t* tx_buf, uint32_t len);

#endif /* SPI_H_ */
