/**
 * @file: 	comm.h
 * @brief:	   
 * @date: 	25 sie 2014
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

#ifndef COMM_H_
#define COMM_H_

#include <inttypes.h>

void    COMM_Init(uint32_t baud);
void    COMM_Putc(uint8_t c);
uint8_t COMM_Getc(void);
uint8_t COMM_GetFrame(uint8_t* buf, uint8_t* len);

#endif /* COMM_H_ */
