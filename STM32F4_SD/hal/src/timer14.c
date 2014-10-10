/**
 * @file: 	timer14.c
 * @brief:	   
 * @date: 	10 paź 2014
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

#include <stm32f4xx.h>
#include <timer14.h>

static volatile uint32_t usCount; ///< Microsecond counter

/**
 * @brief Initialize timer14 as microsecond counter
 */
void TIMER14_Init(void) {

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 84; // overflow every microsecond
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

  // initialize interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM8_TRG_COM_TIM14_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);

  TIM_Cmd(TIM14, ENABLE); // enable timer
}
/**
 * @brief Get time value
 * @return Time in microseconds
 */
uint32_t TIMER14_GetTime(void) {

  return usCount;

}
/**
 * @brief IRQ handler for TIM14
 */
void TIM8_TRG_COM_TIM14_IRQHandler(void) {

  if((TIM_GetFlagStatus(TIM14, TIM_FLAG_Update) != RESET)) {
    // clear flag
    TIM_ClearFlag(TIM14, TIM_FLAG_Update);
    // update microsecond count
    usCount++;
  }
}


