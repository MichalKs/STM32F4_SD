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

#include <timers.h>
#include <stdio.h>
#include <systick.h>
#include <timer14.h>

#ifndef DEBUG
  #define DEBUG
#endif

#ifdef DEBUG
  #define print(str, args...) printf("LED--> "str"%s",##args,"\r")
  #define println(str, args...) printf("LED--> "str"%s",##args,"\r\n")
#else
  #define print(str, args...) (void)0
  #define println(str, args...) (void)0
#endif

/**
 * @addtogroup TIMER
 * @{
 */

#define MAX_SOFT_TIMERS 10 ///< Maximum number of soft timers.

static uint8_t softTimerCount; ///< Count number of soft timers

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
 * @brief Initiate the system time interrupt with a given frequency.
 * @param freq Required frequency of the timer in Hz
 */
void TIMER_Init(uint32_t freq) {

  SYSTICK_Init(freq); // initialize sysTick for ms count

  // initialize TIMER14 as microsecond counter
  TIMER14_Init();

}
/**
 * @brief Returns the system time.
 * @return System time
 */
uint32_t TIMER_GetTime(void) {
  return SYSTICK_GetTime();
}

/**
 * @brief Delay function.
 * @param ms Milliseconds to delay.
 * @warning This is a blocking function. Use with care!
 */
void TIMER_Delay(uint32_t ms) {

  uint32_t startTime = TIMER_GetTime();
  uint32_t currentTime;

  while (1) { // Delay
    currentTime = TIMER_GetTime();
    if ((currentTime >= startTime) && (currentTime-startTime > ms)) {
      break;
    }
    // account for system timer overflow
    if ((currentTime < startTime) && (UINT32_MAX-startTime + currentTime > ms)) {
      break;
    }
  }
}

/**
 * @brief Delay function in us.
 * @param us Microseconds to delay.
 * @warning This is a blocking function. Use with care!
 */
void TIMER_DelayUS(uint32_t us) {

  uint32_t startTime = TIMER14_GetTime();
  uint32_t currentTime;

  while (1) { // Delay
    currentTime = TIMER14_GetTime();
    if ((currentTime >= startTime) && (currentTime-startTime > us)) {
      break;
    }
    // account for system timer overflow
    if ((currentTime < startTime) && (UINT32_MAX-startTime + currentTime > us)) {
      break;
    }
  }
}

/**
 * @brief Nonblocking delay function using
 * @param ms Delay time
 * @param startTime System time at start of delay (this has to be written before delay using TIMER_GetTime())
 * @retval 0 Delay value has not been reached (wait longer)
 * @retval 1 Delay value has been reached
 */
uint8_t TIMER_DelayTimer(uint32_t ms, uint32_t startTime) {

  uint32_t currentTime = TIMER_GetTime();

  if ((currentTime >= startTime) && (currentTime-startTime > ms)) {

    return 1;

  // account for system timer overflow
  } else if ((currentTime < startTime) && (UINT32_MAX-startTime + currentTime > ms)) {
    return 1;
  } else {
    return 0;
  }

}

/**
 * @brief Adds a soft timer
 * @param maxVal Overflow value of timer
 * @param fun Function called on overflow (should return void and accept no parameters)
 * @return Returns the ID of the new counter or error code (-1)
 * @retval -1 Error: too many timers
 */
int8_t TIMER_AddSoftTimer(uint32_t maxVal, void (*fun)(void)) {

  if (softTimerCount > MAX_SOFT_TIMERS) {
    println("TIMERS: Reached maximum number of timers!");
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
  uint32_t sysTicks = SYSTICK_GetTime();

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
 * @}
 */
