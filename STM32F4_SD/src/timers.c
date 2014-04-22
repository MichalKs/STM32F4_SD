/**
 * @file: 	timers.c
 * @brief:	Timing control functions.
 * @date: 	9 kwi 2014
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

#include "timers.h"
#include <stdio.h>


#define MAX_SOFT_TIMERS 10

static volatile uint32_t delayTimer; ///< Delay timer.
static volatile uint32_t sysTicks;

static uint8_t softTimerCount;

typedef struct {
	uint8_t id;
	uint32_t value;
	uint32_t max;
	uint8_t active;
	void (*overflowCallback)(void);
} TIMER_Soft_TypeDef;

TIMER_Soft_TypeDef softTimers[MAX_SOFT_TIMERS];

/**
 * @brief Initiate SysTick with a given frequency.
 * @param freq Required frequency of the timer
 *
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

void TIMER_StartSoftTimer(uint8_t id) {

	softTimers[id].value = 0;
	softTimers[id].active = 1; // start timer
}
void TIMER_PauseSoftTimer(uint8_t id) {

	softTimers[id].active = 0; // pause timer
}
void TIMER_ResumeSoftTimer(uint8_t id) {

	softTimers[id].active = 1; // start timer
}

void TIMER_SoftTimersUpdate() {

	static uint32_t prevVal;
	uint32_t delta;


	if (sysTicks >= prevVal) {

		delta = sysTicks - prevVal;

	} else { // if overflow occurs

		// the difference is the value that prevVal
		// has to UINT32_MAX + the new number of sysTicks
		delta = UINT32_MAX - prevVal + sysTicks;

	}

	prevVal += delta;

	uint8_t i;
	for (i = 0; i < softTimerCount; i++) {

		if (softTimers[i].active == 1) {

			softTimers[i].value += delta;

			if (softTimers[i].value >= softTimers[i].max) {
				softTimers[i].value = 0;
				if (softTimers[i].overflowCallback != NULL) {
					softTimers[i].overflowCallback();
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

	sysTicks++;

}
