/**
 * @file: 	fifo.c
 * @brief:	First in first out buffer implementation
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

#include "fifo.h"

/**
 * @brief Add a FIFO.
 * @param fifo Pointer to FIFO structure
 */
void FIFO_Add(FIFO_TypeDef* fifo) {

	if (fifo->len == 0 ) {
	}

	fifo->tail = 0;
	fifo->head = 0;
	fifo->count = 0;
}
/**
 * @brief Pushes data to FIFO.
 * @param fifo Pointer to FIFO structure
 * @param c Data byte
 * @return SUCCESS - data added, ERROR - FIFO is full
 */
ErrorStatus FIFO_Push(FIFO_TypeDef* fifo, uint8_t c) {

	if (fifo->count == fifo->len) {
		return ERROR;
	}

	fifo->buf[fifo->head++] = c; // Put char in buffer
	fifo->count++; // Increase counter

	if (fifo->head == fifo->len) {
		fifo->head = 0;
	}

	return SUCCESS;
}
/**
 * @brief Pops data from the FIFO.
 * @param fifo Pointer to FIFO structure
 * @param c data
 * @return SUCCESS - got valid data, ERROR - FIFO is empty
 */
ErrorStatus FIFO_Pop(FIFO_TypeDef* fifo, uint8_t* c) {

	if (fifo->count == 0) {
		return ERROR;
	}
	*c = fifo->buf[fifo->tail++];
	fifo->count--;

	if (fifo->tail == fifo->len) {
		fifo->tail = 0;
	}

	return SUCCESS;
}
/**
 * @brief Checks whether the FIFO is empty.
 * @param fifo Pointer to FIFO structure
 * @return 1 - FIFO is empty, 0 - FIFO is not empty
 */
uint8_t FIFO_IsEmpty(FIFO_TypeDef* fifo) {

	if (fifo->count == 0) {
		return 1;
	}

	return 0;
}
