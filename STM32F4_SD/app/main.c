/**
 * @file: 	main.c
 * @brief:	SD Card test
 * @date: 	9 kwi 2014
 * @author: Michal Ksiezopolski
 *
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
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"
#include "diskio.h"

#include "timers.h"
#include "led.h"
#include "uart2.h"
#include "sdcard.h"


#define SYSTICK_FREQ 1000 ///< Frequency of the SysTick.

void softTimerCallback(void);
void hexdump(uint8_t* buf, uint32_t length);

int main(void)
{
	
	UART2_Init(); // Initialize USART2 (for printf)
	TIMER_Init(SYSTICK_FREQ); // Initialize timer

	int8_t timerID = TIMER_AddSoftTimer(1000,softTimerCallback);
	TIMER_StartSoftTimer(timerID);

	LED_TypeDef led;
	led.nr=LED0;
	led.gpio=GPIOD;
	led.pin=12;
	led.clk=RCC_AHB1Periph_GPIOD;

	LED_Add(&led); // Add an LED

	printf("Starting program\r\n"); // Print a string to UART2

//	SD_Init();

//	uint8_t buf[1024];

//	SD_ReadSectors(buf,0,2);
//
//	TIMER_Delay(1000);
//	hexdump(buf, 1024);
//
//	printf("/nPo hexdumpie\n");


	FATFS FatFs;
	FIL file;
	FRESULT result;

	char buf[256];

	printf("Mounting volume\r\n");
	result = f_mount(&FatFs, "", 1); // Mount SD card

	if (result) {
		printf("Error mounting volume!\r\n");
		while(1);
	}

	printf("Opening file: \"hello.txt\"\r\n");
	result = f_open(&file, "hello.txt", FA_READ);

	if (result) {
		printf("Error opening file!\r\n");
		while(1);
	}

	f_gets((char*)buf, 256, &file);

	printf("The file contains the following text:\r\n\"%s\"\r\n", buf);

	f_close(&file); // Close file
	f_mount(NULL, "", 1); // Unmount SD Card
	
	while (1){  
		TIMER_SoftTimersUpdate();
	}
}

void hexdump(uint8_t* buf, uint32_t length) {

	uint32_t i = 0;

	while (length--) {

		printf("%02x ", buf[i]);

		i++;
		if ((i % 16) == 0) {
			printf("\r\n");
		}
		if ((i % 50) == 0) {
			TIMER_Delay(1000);
		}

	}

}

void softTimerCallback(void) {

	LED_Toggle(LED0); // Toggle LED

	printf("Test string sent from STM32F4!!!\r\n"); // Print test string

}


uint32_t get_fattime() {

	return 0;
}
