/**
 * @file:   sdcard.c
 * @brief:  SD card control functions.
 * @date:   22 kwi 2014
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

#include <sdcard.h>
#include <spi1.h>
#include <timers.h>
#include <stdio.h>

/**
 * @addtogroup SD_CARD
 * @{
 */

/*
 * SD commands (SPI command subset) as per SanDisk Secure Digital Card product manual.
 * TODO Add all commands
 */
#define SD_GO_IDLE_STATE            0   ///< Resets SD Card.
#define SD_SEND_OP_COND             1   ///< Activates the card initialization process, sends host capacity.
#define SD_SEND_IF_COND             8   ///< Asks card whether it can operate in given voltage range.
#define SD_SEND_CSD                 9   ///< Ask for card specific data (CSD).
#define SD_SEND_CID                 10  ///< Ask for card identification (CID).
#define SD_STOP_TRANSMISSION        12  ///< Forces a card to stop transmission during a multiple block read operation.
#define SD_SEND_STATUS              13  ///< Ask for status register contents.
#define SD_SET_BLOCKLEN             16  ///< Selects block length in bytes for all following block commands
#define SD_READ_SINGLE_BLOCK        17  ///< Reads a block of size set by SET_BLOCKLEN
#define SD_READ_MULTIPLE_BLOCK      18  ///< Continuously transfers data blocks from card to host until interrupted by STOP_TRANSMISSION
#define SD_WRITE_BLOCK              24  ///< Writes a block of size set by SET_BLOCKLEN
#define SD_WRITE_MULTIPLE_BLOCK     25  ///< Continuously writes blocks of data until a stop transmission token is sent
#define SD_PROGRAM_CSD              27  ///< Programs the programmable bits of CSD
#define SD_ERASE_WR_BLK_START_ADDR  32  ///< Sets the address of the first write block to be erased
#define SD_ERASE_WR_BLK_END_ADDR    33  ///< Sets the address of the last write block of the continuous range to be erased
#define SD_ERASE                    38  ///< Erases all previously selected write blocks
#define SD_APP_CMD                  55  ///< Next command is application specific command
#define SD_READ_OCR                 58  ///< Reads OCR register
#define SD_CRC_ON_OFF               59  ///< Turns CRC on or off
/*
 * Application specific commands, ACMD
 */
#define SD_ACMD_SEND_OP_COND        41  ///< Activates the card initialization process, sends host capacity.
#define SD_ACMD_SEND_SCR            51  ///< Reads SD Configuration register

/*
 * Other SD defines
 */
#define SD_IF_COND_CHECK  0xaa    ///< Check pattern for SEND_IF_COND command
#define SD_IF_COND_VOLT   (1<<8)  ///< Signifies voltage range 2.7-3.6V
#define SD_ACMD41_HCS     (1<<30) ///< Host can handle SDSC and SDHC cards

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t args);
static uint8_t SD_GetResponseR3orR7(uint8_t* buf);

/**
 * @brief SD Card R1 response structure
 */
typedef union {

  struct {
    uint8_t inIdleState         :1; ///<
    uint8_t eraseReset          :1; ///<
    uint8_t illegalCommand      :1; ///<
    uint8_t commErrorCRC        :1; ///<
    uint8_t eraseSequenceError  :1; ///<
    uint8_t addressErrror       :1; ///<
    uint8_t parameterError      :1; ///<
    uint8_t reserved            :1; ///< Reserved (always 0)
  } flags;

  uint8_t responseR1; ///< R1 response fields as byte
} SD_ResponseR1;

/**
 * @brief SD Card R2 response structure
 */
typedef union {

  struct {
    uint16_t cardLocked          :1; ///<
    uint16_t wpEraseSkip         :1; ///<
    uint16_t error               :1; ///<
    uint16_t errorCC             :1; ///<
    uint16_t cardFailedECC       :1; ///<
    uint16_t wpViolation         :1; ///<
    uint16_t eraseParam          :1; ///<
    uint16_t outOfRange          :1; ///<
    uint16_t inIdleState         :1; ///<
    uint16_t eraseReset          :1; ///<
    uint16_t illegalCommand      :1; ///<
    uint16_t commErrorCRC        :1; ///<
    uint16_t eraseSequenceError  :1; ///<
    uint16_t addressErrror       :1; ///<
    uint16_t parameterError      :1; ///<
    uint16_t reserved            :1; ///< Reserved (always 0)
  } flags;

  uint16_t responseR2; ///< R1 response fields as byte

} SD_ResponseR2;

/**
 * @brief OCR register
 */
typedef union {

  struct {
    uint32_t reserved   :15;
    uint32_t volt27to28 :1;
    uint32_t volt28to29 :1;
    uint32_t volt29to30 :1;
    uint32_t volt30to31 :1;
    uint32_t volt31to32 :1;
    uint32_t volt32to33 :1;
    uint32_t volt33to34 :1;
    uint32_t volt34to35 :1;
    uint32_t volt35to36 :1;
    uint32_t switchingTo18      :1;
    uint32_t reserved2          :5;
    uint32_t cardCapacityStatus :1; ///< 0 - SDSC, 1 - SDHC, valid only after power up bit is 1
    uint32_t cardPowerUpStatus  :1; ///< Set to 0 if card has not finished power up routine
  } bits;

  uint32_t ocr;

} SD_OCR;

/**
 * @brief Initialize the SD card.
 */
void SD_Init(void) {

  int i;
  uint8_t buf[10];

  SPI1_Init(); // Initialize SPI interface.

  SPI1_Select();

  // Synchronize card with SPI
  for (i = 0; i < 100; i++) {
    SPI1_Transmit(0xff);
  }

  SD_ResponseR1 resp;

  // CMD0
  resp.responseR1 = SD_SendCommand(SD_GO_IDLE_STATE, 0);

  // Check response errors
  if (resp.responseR1 != 0x01) {
    printf("SD_GO_IDLE_STATE error\r\n");
  }

  // CMD8
  resp.responseR1 = SD_SendCommand(SD_SEND_IF_COND,
      SD_IF_COND_VOLT | SD_IF_COND_CHECK); // voltage range and check pattern

  SD_GetResponseR3orR7(buf);

  // Check response errors
  if (resp.responseR1 != 0x01) {
    printf("SD_SEND_IF_COND error\r\n");
  }

  // Check if card supports given voltage range
  if ((buf[3] != SD_IF_COND_CHECK) || (buf[2] != (SD_IF_COND_VOLT>>8))) {
    printf("SD_SEND_IF_COND error\r\n");
    for (i=0; i<4; i++) {
      printf("%02x ", buf[i]);
    }
    printf("\r\n");

  }

  // CMD58
  resp.responseR1 = SD_SendCommand(SD_READ_OCR, 0);

  SD_GetResponseR3orR7(buf);

  // Check response errors
  if (resp.responseR1 != 0x01) {
    printf("SD_READ_OCR error\r\n");
  }

  // Send OCR to terminal
  for (i=0; i<4; i++) {
    printf("%02x ", buf[i]);
  }
  printf("\r\n");

  // Send ACMD41 until card goes out of IDLE state
  for (i=0; i<10; i++) {

    resp.responseR1 = SD_SendCommand(SD_APP_CMD, 0);
    resp.responseR1 = SD_SendCommand(SD_ACMD_SEND_OP_COND, SD_ACMD41_HCS);
    TIMER_Delay(20);
    if (resp.responseR1 == 0x00) { // Card left IDLE state and no errors
      break;
    }

    if (i == 9) {
      printf("Failed to initialize SD card\r\n");
      while(1);
    }
  }

  // Read Card Capacity Status - SDSC or SDHC?
  resp.responseR1 = SD_SendCommand(SD_READ_OCR, 0);
  SD_GetResponseR3orR7(buf);

  // Check response errors
  if (resp.responseR1 != 0x01) {
    printf("SD_READ_OCR error\r\n");
  }

  // Send OCR to terminal
  for (i=0; i<4; i++) {
    printf("%02x ", buf[i]);
  }
  printf("\r\n");

  SPI1_Deselect();

//  for (i=0; i<10; i++) {
//    printf("%02x ", SPI1_Transmit(0xff));
//  }
//  printf("\r\n");

//  while (SPI1_Transmit(0xff) != 1);

//  while (1) {
//    resp.responseR1 = SPI1_Transmit(0xff);
//    if (resp.flags.inIdleState) {
//      break;
//    }
//  }

//
//  do {
//
//    SD_SendCommand(SD_SEND_OP_COND, 0);
//
//    for (i=0; i < 8; i++) {
//      resp.responseR1 = SPI1_Transmit(0xff);
//      if (resp.responseR1 == 0) {
//        break;
//      }
//    }
//
//  } while (resp.responseR1 != 0);

//  resp.flags.eraseReset == 1

//  args[2] = 0x01;
//  args[3] = 0xaa;
//
//  i = 0;
//  do {
//    status = SD_SendCommand(0x48, args, 0x87);
//    TIMER_Delay(5);
//    i++;
//  } while (status != 1);
//
//  printf("CMD8, status = %02x (%d iterations)\r\n", status, i);

//  args[2] = 0;
//  args[3] = 0;
//
//  i = 0;
//  do {
//    args[0] = 0;
//    status = SD_SendCommand(0x77, args, 0xff);
//    args[0] = 0x40;
//    TIMER_Delay(5);
//    status = SD_SendCommand(0x69, args, 0xff);
//    i++;
//    TIMER_Delay(50);
//  } while (status != 0);
//
//  printf("ACMD41, status = %02x (%d iterations)\r\n", status, i);

//  args[0] = 0;
//  i = 0;
//  do {
//    status = SD_SendCommand(0x40+58, args, 0xff);
//    i++;
//    TIMER_Delay(50);
//  } while (status != 0);
//
//  printf("OCR, status = %02x (%d iterations)\r\n", status, i);

  // 4 bytes of OCR
//  SPI1_Transmit(0xff);
//  SPI1_Transmit(0xff);
//  status = SPI1_Transmit(0xff);
//  SPI1_Transmit(0xff);
//  SPI1_Transmit(0xff);
//  SPI1_Transmit(0xff); // One additional for safety
//
//  printf("OCR, 3rd byte  = %02x\r\n", status);

//  SPI1_Deselect();
}




uint8_t SD_ReadSectors(uint8_t* buf, uint32_t sector, uint32_t count) {

  sector *= 512;

  SPI1_Select();

  uint8_t status;
//  do {
//    status = SD_SendCommand(0x40 | SD_READ_MULTIPLE_BLOCK, args, 0xff);
//    TIMER_Delay(5);
//  } while (status != 0);

  status = SD_SendCommand(SD_READ_MULTIPLE_BLOCK, sector);

  while (SPI1_Transmit(0xff) != 0);

  while (count) {
    while (SPI1_Transmit(0xff) != 0xfe); // wait for data token
    SPI1_ReadBuffer(buf, 512);
    SPI1_Transmit(0xff);
    SPI1_Transmit(0xff); // two bytes CRC
    count--;
    buf += 512; // move buffer pointer forward
  }


//  do {
//    status = SD_SendCommand(0x40 | SD_STOP_TRANSMISSION, args, 0xff);
//    TIMER_Delay(5);
//  } while (status != 0);

  status = SD_SendCommand(SD_STOP_TRANSMISSION, 0);

  while (SPI1_Transmit(0xff) != 0);

  while(!SPI1_Transmit(0xff));

  SPI1_Deselect();

  return 0;
}

uint8_t SD_WriteSectors(uint8_t* buf, uint32_t sector, uint32_t count) {


  sector *= 512;

  SPI1_Select();

  uint8_t status;

  do {
    status = SD_SendCommand(SD_WRITE_MULTIPLE_BLOCK, sector);
    TIMER_Delay(5);
  } while (status != 0);

  SPI1_Transmit(0xff);

  while (count) {
    SPI1_Transmit(0xfc); // send data token
    SPI1_SendBuffer(buf, 512);
    SPI1_Transmit(0xff);
    SPI1_Transmit(0xff); // two bytes CRC
    count--;
    buf += 512; // move buffer pointer forward
  }

  SPI1_Transmit(0xfd); // stop transmission
  SPI1_Transmit(0xff);
  while(!SPI1_Transmit(0xff));

  SPI1_Deselect();

  return 0;
}
/**
 * @brief Sends a command to the SD card.
 *
 * @details This function works for commands which return 1 byte
 * response - R1 response token. These commands are in the majority.
 *
 * @param cmd Command to send
 * @param args Command arguments: 4 bytes as a 32-bit number
 * @return Returns R1 response token
 */
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t args) {

  SPI1_Transmit(0x40 | cmd);
  SPI1_Transmit(args >> 24); // MSB first
  SPI1_Transmit(args >> 16);
  SPI1_Transmit(args >> 8);
  SPI1_Transmit(args);

  // CRC is irrelevant while using SPI interface - only checked for some commands.
  switch (cmd) {
  case SD_GO_IDLE_STATE:
    SPI1_Transmit(0x95);
    break;
  case SD_SEND_IF_COND:
    SPI1_Transmit(0x87);
    break;
  default:
    SPI1_Transmit(0xff);
  }
  // Practice has shown that a valid response token
  // is sent as the second byte by the card.
  // So, we send a dummy byte first.
  SPI1_Transmit(0xff);
  uint8_t ret = SPI1_Transmit(0xff);
//  printf("Response to cmd %d is %02x\r\n", cmd, ret);

  return ret;
}
static uint8_t SD_GetResponseR3orR7(uint8_t* buf) {

  uint8_t i = 0;
  buf[i++] = SPI1_Transmit(0xff);
  buf[i++] = SPI1_Transmit(0xff);
  buf[i++] = SPI1_Transmit(0xff);
  buf[i++] = SPI1_Transmit(0xff);

  return 0;
}
/**
 * @}
 */
