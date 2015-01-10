/**
 * @file    keys.h
 * @brief   Matrix keyboard library
 * @date    5 maj 2014
 * @author  Michal Ksiezopolski
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

/**
 * @defgroup  KEYS KEYS
 * @brief     Matrix keyboard library
 */

/**
 * @addtogroup KEYS
 * @{
 */
typedef enum {
  KEY0 = 0x31,
  KEY1 = 0x00,
  KEY2 = 0x01,
  KEY3 = 0x02,
  KEY4 = 0x10,
  KEY5 = 0x11,
  KEY6 = 0x12,
  KEY7 = 0x20,
  KEY8 = 0x21,
  KEY9 = 0x22,
  KEYA = 0x03,
  KEYB = 0x13,
  KEYC = 0x23,
  KEYD = 0x33,
  KEY_HASH = 0x32,
  KEY_ASTERISK = 0x30,
  KEY_NONE = 0xff
} KEY_Id_Typedef;

void KEYS_Init(void);
uint8_t KEYS_Update(void);

/**
 * @}
 */

#endif /* KEYS_H_ */
