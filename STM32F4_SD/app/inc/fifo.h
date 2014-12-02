/**
 * @file:   fifo.h
 * @brief:  First in first out buffer implementation
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

#ifndef FIFO_H_
#define FIFO_H_

#include <inttypes.h>

/**
 * @defgroup  FIFO FIFO
 * @brief     First in first out buffer functions
 */

/**
 * @addtogroup FIFO
 * @{
 */

/**
 * @brief FIFO structure typedef.
 */
typedef struct {
  uint16_t head;   ///< Head
  uint16_t tail;   ///< Tail
  uint8_t* buf;    ///< Pointer to buffer
  uint16_t len;    ///< Maximum length of FIFO
  uint16_t count;  ///< Current number of data elements
} FIFO_TypeDef;

uint8_t FIFO_Add      (FIFO_TypeDef* fifo);
uint8_t FIFO_Push     (FIFO_TypeDef* fifo, uint8_t c);
uint8_t FIFO_Pop      (FIFO_TypeDef* fifo, uint8_t* c);
uint8_t FIFO_IsEmpty  (FIFO_TypeDef* fifo);

/**
 * @}
 */

#endif /* FIFO_H_ */
