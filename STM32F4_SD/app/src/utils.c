/**
 * @file: 	utils.c
 * @brief:	   
 * @date: 	20 lip 2014
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


#include <utils.h>
#include <stdio.h>
#include <timers.h>

/**
 * @addtogroup UTILS
 * @{
 */

/**
 * @brief Send data in hex format to terminal.
 * @param buf Data buffer.
 * @param length Number of bytes to send.
 * @warning Uses blocking delays so as not to overflow buffer.
 */
void hexdump(uint8_t* buf, uint32_t length) {

  uint32_t i = 0;

  while (length--) {

    printf("%02x ", buf[i]);

    i++;
    // new line every 16 chars
    if ((i % 16) == 0) {
      printf("\r\n");
    }
    // delay every 50 chars
    if ((i % 50) == 0) {
      TIMER_Delay(100); // Delay so as not to overflow buffer
    }
  }
}

/**
 * @}
 */

