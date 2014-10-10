/**
 * @file: 	keys.c
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

#include <keys.h>
#include <timers.h>
#include <stdio.h>
#include <keys_hal.h>

#ifndef DEBUG
  #define DEBUG
#endif

#ifdef DEBUG
  #define print(str, args...) printf("KEYS--> "str"%s",##args,"\r")
  #define println(str, args...) printf("KEYS--> "str"%s",##args,"\r\n")
#else
  #define print(str, args...) (void)0
  #define println(str, args...) (void)0
#endif

#define DEBOUNCE_TIME 200 ///< Key debounce time in ms

/**
 * Key structure typedef.
 */
typedef struct {
	uint8_t id;		///< KEY_ID
	void (*PressCallback)(void);
	void (*RepeatCallback)(void);
	uint16_t len;	///<
	uint16_t count;	///<
} KEY_TypeDef;



uint8_t currentColumn; ///< Selected keyboard column

void KEYS_Init(void) {

  KEYS_HAL_Init();
  currentColumn = 0;

  // select first column as default
  KEYS_HAL_SelectColumn(0);

}

/**
 * @brief Checks if any keys are set.
 * @details Run this function in main loop to check for pressed keys.
 * TODO Add repeat, function calling
 */
uint8_t KEYS_Update(void) {

  static uint8_t keyId = KEY_NONE;
  uint8_t keyValid = KEY_NONE; // hold a valid debounced key ID
  uint8_t currentKey = KEY_NONE;

  static uint32_t debounceTimer = 0; // timer for counting debounce time

  int8_t row = KEYS_HAL_ReadRow();

  // if a keypress has been recongized
  if (row != -1) {
    currentKey = (currentColumn << 4) | row;
//    println("You pressed a key in row %d, column %d.", row, currentColumn);
  }

  // if key value changed start debounce timer for new key
  if (keyId != currentKey && currentKey != KEY_NONE) {
    keyId = currentKey;
    debounceTimer = TIMER_GetTime();
  }
  // if debounce finished, the key is valid
  if (keyId != KEY_NONE && TIMER_DelayTimer(DEBOUNCE_TIME, debounceTimer)) {
    keyValid = keyId;
    println("You pressed a key 0x%02x.", keyValid);
    keyId = KEY_NONE;
  }

  // update column
  currentColumn++;

  if (currentColumn == 4) {
    currentColumn = 0;
  }

  KEYS_HAL_SelectColumn(currentColumn);

  // if key is valid return ID, if not returns KEY_NONE
  return keyValid;
}


