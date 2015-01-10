/**
 * @file    keys.c
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

/**
 * @addtogroup KEYS
 * @{
 */

#define DEBOUNCE_TIME 200 ///< Key debounce time in ms
#define REPEAT_TIME   20  ///< Key repeat time (after this time repeat goes inactive)

/**
 * @brief Key structure typedef.
 */
typedef struct {
  uint8_t id;  ///< KEY_ID
  void (*PressCallback)(void);
  void (*RepeatCallback)(void);
  uint16_t len;  ///<
  uint16_t count;  ///<
} KEY_TypeDef;

uint8_t currentColumn; ///< Selected keyboard column
/**
 * @brief Initialize matrix keyboard
 */
void KEYS_Init(void) {

  KEYS_HAL_Init();
  // first column
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

  static uint8_t keyId    = KEY_NONE; // stores the pressed button ID
  static uint8_t lastKey  = KEY_NONE; // stores last key press for hold
  uint8_t keyValid        = KEY_NONE; // hold a valid debounced key ID
  uint8_t currentKey      = KEY_NONE; // stores temporary key received from HAL (may be glitch)

  static uint8_t repeatFlag = 0; // repeat flag
  static uint32_t debounceTimer = 0; // timer for counting debounce time
  static uint32_t repeatTimer = 0;

  int8_t row = KEYS_HAL_ReadRow();

  // if a key press has been recognized
  if (row != -1) {
    currentKey = (currentColumn << 4) | row;
  } else if (TIMER_DelayTimer(REPEAT_TIME, repeatTimer)) { // repeat timeout
    repeatFlag = 0;
    lastKey = KEY_NONE;
  }

  // if key value changed start debounce timer for new key
  if (keyId != currentKey && currentKey != KEY_NONE) {

    if (lastKey == currentKey &&
        !TIMER_DelayTimer(REPEAT_TIME, repeatTimer)) { // if last key still pressed
      repeatFlag = 1;
      repeatTimer = TIMER_GetTime();
    } else { // new key
      keyId = currentKey; // store the new key
      debounceTimer = TIMER_GetTime(); // start debounce timer
      lastKey = KEY_NONE;
      repeatFlag = 0;
    }

  }
  // if debounce finished, the key is valid
  if (!repeatFlag && keyId != KEY_NONE &&
      TIMER_DelayTimer(DEBOUNCE_TIME, debounceTimer)) {
    keyValid = keyId;
    println("You pressed a key 0x%02x.", keyValid);
    lastKey = keyId; // store new last pressed key
    keyId = KEY_NONE;
    repeatTimer = TIMER_GetTime(); // start repeatTimer
  } else if (repeatFlag) {
    keyValid = lastKey;
  }

  // update column
  currentColumn++;

  // if last column reached
  if (currentColumn == 4) {
    currentColumn = 0;
  }

  KEYS_HAL_SelectColumn(currentColumn);

  // if key is valid return ID, if not returns KEY_NONE
  return keyValid;
}
/**
 * @}
 */
