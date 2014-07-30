/**
 * @file:   timers.c
 * @brief:  Timing control functions.
 * @date:   9 kwi 2014
 * @author: Michal Ksiezopolski
 * 
 *
 * Control of the SysTick and software timers
 * incremented based on SysTick interrupts.
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

#include <stm32f4xx.h>

#include <timers.h>
#include <stdio.h>


/**
 * @addtogroup TIMER
 * @{
 */

#define MAX_SOFT_TIMERS 10 ///< Maximum number of soft timers.

static volatile uint32_t delayTimer;  ///< Delay timer.
static volatile uint32_t sysTicks;    ///< System clock.

static uint8_t softTimerCount;

/**
 * @brief Soft timer structure.
 */
typedef struct {
  uint8_t id;                     ///< Timer ID
  uint32_t value;                 ///< Current count value
  uint32_t max;                   ///< Overflow value
  uint8_t active;                 ///< Is timer active?
  void (*overflowCallback)(void); ///< Function called on overflow event
} TIMER_Soft_TypeDef;

static TIMER_Soft_TypeDef softTimers[MAX_SOFT_TIMERS]; ///< Array of soft timers

/**
 * @brief Initiate SysTick with a given frequency.
 * @param freq Required frequency of the timer in Hz
 */
void TIMER_Init(uint32_t freq) {

  RCC_ClocksTypeDef RCC_Clocks;

  RCC_GetClocksFreq(&RCC_Clocks); // Complete the clocks structure with current clock settings.

  SysTick_Config(RCC_Clocks.HCLK_Frequency / freq); // Set SysTick frequency
}
/**
 * @brief Delay function.
 * @param ms Milliseconds to delay.
 * @see delayTimer
 */
void TIMER_Delay(uint32_t ms) {

  delayTimer = ms;

  while (delayTimer); // Delay

}
/**
 * @brief Adds a soft timer
 * @param maxVal Overflow value of timer
 * @param fun Function called on overflow (should return void and accept no parameters)
 * @return Returns the ID of the new counter or error code
 * @retval -1 Error: too many timers
 */
int8_t TIMER_AddSoftTimer(uint32_t maxVal, void (*fun)(void)) {

  if (softTimerCount > MAX_SOFT_TIMERS) {
    printf("TIMERS: Reached maximum number of timers!");
    return -1;
  }

  softTimers[softTimerCount].id = softTimerCount;
  softTimers[softTimerCount].overflowCallback = fun;
  softTimers[softTimerCount].max = maxVal;
  softTimers[softTimerCount].value = 0;
  softTimers[softTimerCount].active = 0; // inactive on startup

  softTimerCount++;

  return (softTimerCount - 1);
}
/**
 * @brief Starts the timer (zeroes out current count value).
 * @param id Timer ID
 */
void TIMER_StartSoftTimer(uint8_t id) {

  softTimers[id].value = 0;
  softTimers[id].active = 1; // start timer
}
/**
 * @brief Pauses given timer (current count value unchanged)
 * @param id Timer ID
 */
void TIMER_PauseSoftTimer(uint8_t id) {

  softTimers[id].active = 0; // pause timer
}
/**
 * @brief Resumes a timer (starts counting from last value).
 * @param id Timer ID
 */
void TIMER_ResumeSoftTimer(uint8_t id) {

  softTimers[id].active = 1; // start timer
}
/**
 * @brief Updates all the timers and calls the overflow functions as
 * necessary
 *
 * @details This function should be called periodically in the main
 * loop of the program.
 */
void TIMER_SoftTimersUpdate(void) {

  static uint32_t prevVal;
  uint32_t delta;

  if (sysTicks >= prevVal) {

    delta = sysTicks - prevVal; // How much time passed from previous run

  } else { // if overflow occurs

    // the difference is the value that prevVal
    // has to UINT32_MAX + the new number of sysTicks
    delta = UINT32_MAX - prevVal + sysTicks;

  }

  prevVal += delta; // update time for the function

  uint8_t i;
  for (i = 0; i < softTimerCount; i++) {

    if (softTimers[i].active == 1) {

      softTimers[i].value += delta; // update active timer values

      if (softTimers[i].value >= softTimers[i].max) { // if overflow
        softTimers[i].value = 0; // zero out timer
        if (softTimers[i].overflowCallback != NULL) {
          softTimers[i].overflowCallback(); // call the overflow function
        }
      }
    }
  }
}
/**
 * @brief Interrupt handler for SysTick.
 */
void SysTick_Handler(void) {

  if (delayTimer) {
    delayTimer--; // Decrement delayTimer
  }

  sysTicks++; // Update system time

}

/**
 * @}
 */
