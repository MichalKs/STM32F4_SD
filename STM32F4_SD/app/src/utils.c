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
void hexdump(const uint8_t const *buf, uint32_t length) {

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
//      TIMER_Delay(100); // Delay so as not to overflow buffer
    }
  }
  printf("\r\n");
}

/**
 * @brief Send data in hex and ASCII format to terminal.
 * @param buf Data buffer.
 * @param length Number of bytes to send.
 * @warning Uses blocking delays so as not to overflow buffer.
 */
void hexdumpC(const uint8_t const *buf, uint32_t length) {

  uint32_t i = 0;

  while (length--) {

    if (buf[i]>=' ' && buf[i] <= '~') {
      printf("%02x %c ", buf[i], buf[i]);
    } else { // nonalphanumeric as dot
      printf("%02x %c ", buf[i], '.');
    }

    i++;
    // new line every 16 chars
    if ((i % 8) == 0) {
      printf("\r\n");
    }
    // delay every 50 chars
    if ((i % 50) == 0) {
//      TIMER_Delay(100); // Delay so as not to overflow buffer
    }
  }
  printf("\r\n");
}

/**
 * @brief Send data in hex and ASCII format to terminal.
 * @param buf Data buffer.
 * @param length Number of bytes to send.
 * @warning Uses blocking delays so as not to overflow buffer.
 */
void hexdump16C(const uint16_t const *buf, uint32_t length) {

  uint32_t i = 0;

  while (length--) {

    if (buf[i]>=' ' && buf[i] <= '~') {
      printf("%04x %c ", buf[i], buf[i]);
    } else { // nonalphanumeric as dot
      printf("%04x %c ", buf[i], '.');
    }

    i++;
    // new line every 16 chars
    if ((i % 8) == 0) {
      printf("\r\n");
    }
    // delay every 50 chars
    if ((i % 50) == 0) {
//      TIMER_Delay(100); // Delay so as not to overflow buffer
    }
  }
  printf("\r\n");
}

/**
 * @}
 */

