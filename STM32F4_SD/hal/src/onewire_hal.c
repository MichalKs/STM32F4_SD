/**
 * @file: 	onewire_hal.c
 * @brief:	ONEWIRE low level functions
 * @date: 	9 paź 2014
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


#include <onewire_hal.h>
#include <stm32f4xx.h>

#define ONEWIRE_PIN   GPIO_Pin_1
#define ONEWIRE_PORT  GPIOC
#define ONEWIRE_CLK   RCC_AHB1Periph_GPIOC

/**
 * @brief Initialize ONEWIRE hardware
 */
void ONEWIRE_HAL_Init(void) {

  RCC_AHB1PeriphClockCmd(ONEWIRE_CLK, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure pin in output open drain mode
  GPIO_InitStructure.GPIO_Pin   = ONEWIRE_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  ONEWIRE_HAL_ReleaseBus();
  GPIO_Init(ONEWIRE_PORT, &GPIO_InitStructure);

}

/**
 * @brief Release the bus. Resistor will pull it up.
 */
void ONEWIRE_HAL_ReleaseBus(void) {

  GPIO_SetBits(ONEWIRE_PORT, ONEWIRE_PIN);
}

/**
 * @brief Pull bus low.
 */
void ONEWIRE_HAL_BusLow(void) {
  GPIO_ResetBits(ONEWIRE_PORT, ONEWIRE_PIN);
}

/**
 * @brief Read the bus
 * @return Read bus state (high or low)
 */
uint8_t ONEWIRE_HAL_ReadBus(void) {
  return GPIO_ReadInputDataBit(ONEWIRE_PORT, ONEWIRE_PIN);
}
