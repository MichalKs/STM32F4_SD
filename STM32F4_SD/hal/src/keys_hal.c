/**
 * @file: 	keys_hal.c
 * @brief:	   
 * @date: 	6 paź 2014
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

#include <keys_hal.h>
#include <stm32f4xx.h>


/*
 * Pin and port mappings for matrix keyboard.
 */
#define KEYS_ROW0_PIN GPIO_Pin_11
#define KEYS_ROW1_PIN GPIO_Pin_12
#define KEYS_ROW2_PIN GPIO_Pin_13
#define KEYS_ROW3_PIN GPIO_Pin_14

#define KEYS_COL0_PIN GPIO_Pin_7
#define KEYS_COL1_PIN GPIO_Pin_8
#define KEYS_COL2_PIN GPIO_Pin_9
#define KEYS_COL3_PIN GPIO_Pin_10

#define KEYS_ROW_PORT   GPIOE
#define KEYS_ROW_CLOCK  RCC_AHB1Periph_GPIOE

#define KEYS_COL_PORT   GPIOE
#define KEYS_COL_CLOCK  RCC_AHB1Periph_GPIOE

/**
 * @brief Initialize 4x4 matrix keyboard
 */
void KEYS_HAL_Init(void) {

  // Enable clocks
  RCC_AHB1PeriphClockCmd(KEYS_ROW_CLOCK, ENABLE);
  RCC_AHB1PeriphClockCmd(KEYS_COL_CLOCK, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure row pins in input pulled-up mode
  GPIO_InitStructure.GPIO_Pin   = KEYS_ROW0_PIN | KEYS_ROW1_PIN |
      KEYS_ROW2_PIN | KEYS_ROW3_PIN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // irrelevant
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // irrelevant
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

  GPIO_Init(KEYS_ROW_PORT, &GPIO_InitStructure);

  // Configure column pins in output push/pull mode
  GPIO_InitStructure.GPIO_Pin   = KEYS_COL0_PIN | KEYS_COL1_PIN |
      KEYS_COL2_PIN | KEYS_COL3_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // less interference
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(KEYS_COL_PORT, &GPIO_InitStructure);

}
/**
 * @brief Select a column
 * @param col Column number
 */
void KEYS_HAL_SelectColumn(uint8_t col) {

  // set all columns high
  GPIO_SetBits(KEYS_COL_PORT, KEYS_COL0_PIN | KEYS_COL1_PIN |
      KEYS_COL2_PIN | KEYS_COL3_PIN);

  // set selected column as low
  switch (col) {
  case 0:
    GPIO_ResetBits(KEYS_COL_PORT, KEYS_COL0_PIN);
    break;
  case 1:
    GPIO_ResetBits(KEYS_COL_PORT, KEYS_COL1_PIN);
    break;
  case 2:
    GPIO_ResetBits(KEYS_COL_PORT, KEYS_COL2_PIN);
    break;
  case 3:
    GPIO_ResetBits(KEYS_COL_PORT, KEYS_COL3_PIN);
    break;
  default:
    break;
  }

}
/**
 * @brief Read keyboard row.
 * @return Row value
 */
int8_t KEYS_HAL_ReadRow(void) {

  uint16_t row_p = (GPIO_ReadInputData(KEYS_ROW_PORT)); // read row
  uint16_t row = ~row_p; // negate, because we use low level for keypress

  if (row & KEYS_ROW0_PIN)
    return 0;
  if (row & KEYS_ROW1_PIN)
    return 1;
  if (row & KEYS_ROW2_PIN)
    return 2;
  if (row & KEYS_ROW3_PIN)
    return 3;

  return -1;
}
