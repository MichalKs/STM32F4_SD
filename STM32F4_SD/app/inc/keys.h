/**
 * @file: 	keys.h
 * @brief:	   
 * @date: 	5 maj 2014
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

#ifndef KEYS_H_
#define KEYS_H_

#include <inttypes.h>

typedef enum {
  KEY0,
  KEY1,
  KEY2,
  KEY3,
  KEY4,
  KEY5,
  KEY6,
  KEY7,
  KEY8,
  KEY9,
  KEY_HASH,
  KEY_ASTERISK,
  KEY_NONE = 0xff
} KEY_Id_Typedef;

void KEYS_Init(void);
uint8_t KEYS_Update(void);

#endif /* KEYS_H_ */
