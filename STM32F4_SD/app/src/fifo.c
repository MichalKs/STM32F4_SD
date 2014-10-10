/**
 * @file:   fifo.c
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

#include <fifo.h>
#include <stdio.h>

#ifndef DEBUG
  #define DEBUG
#endif

#ifdef DEBUG
  #define print(str, args...) printf("FIFO--> "str"%s",##args,"\r")
  #define println(str, args...) printf("FIFO--> "str"%s",##args,"\r\n")
#else
  #define print(str, args...) (void)0
  #define println(str, args...) (void)0
#endif

/**
 * @addtogroup FIFO
 * @{
 */

/**
 * @brief Add a FIFO.
 *
 * @details To add a FIFO, you need to define a FIFO_TypeDef
 * structure and initialize it with the proper length and
 * buffer pointer. The rest is handled automatically.
 *
 * @param fifo Pointer to FIFO structure
 * @retval 0 FIFO added successfully
 * @retval 1 Error: FIFO length is 0
 */
uint8_t FIFO_Add(FIFO_TypeDef* fifo) {

  if (fifo->len == 0 ) {
    println("Zero FIFO length");
    return 1;
  }

  fifo->tail  = 0;
  fifo->head  = 0;
  fifo->count = 0;

  return 0;
}
/**
 * @brief Pushes data to FIFO.
 * @param fifo Pointer to FIFO structure
 * @param c Data byte
 * @retval 0 Data added
 * @retval 1 Error: FIFO is full
 */
uint8_t FIFO_Push(FIFO_TypeDef* fifo, uint8_t c) {

  // Check for overflow
  if (fifo->count == fifo->len) {
    println("FIFO overflow");
    return 1;
  }

  fifo->buf[fifo->head++] = c; // Put char in buffer
  fifo->count++; // Increase counter


  if (fifo->head == fifo->len) {
    fifo->head = 0; // start from beginning
  }

  return 0;
}
/**
 * @brief Pops data from the FIFO.
 * @param fifo Pointer to FIFO structure
 * @param c data
 * @retval 0 Got valid data
 * @retval 1 Error: FIFO is empty
 */
uint8_t FIFO_Pop(FIFO_TypeDef* fifo, uint8_t* c) {

  // If FIFO is empty
  if (fifo->count == 0) {
//    println("FIFO is empty");
    return 1;
  }
  *c = fifo->buf[fifo->tail++];
  fifo->count--;

  if (fifo->tail == fifo->len) {
    fifo->tail = 0; // start from beginning
  }

  return 0;
}
/**
 * @brief Checks whether the FIFO is empty.
 * @param fifo Pointer to FIFO structure
 * @retval 1 FIFO is empty
 * @retval 0 FIFO is not empty
 */
uint8_t FIFO_IsEmpty(FIFO_TypeDef* fifo) {

  if (fifo->count == 0) {
    return 1;
  }

  return 0;
}

/**
 * @}
 */
