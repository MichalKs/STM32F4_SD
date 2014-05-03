

#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"
#include "diskio.h"

#include "timers.h"
#include "led.h"
#include "uart.h"
#include "sdcard.h"


#define SYSTICK_FREQ 1000 ///< Frequency of the SysTick.

void softTimerCallback(void);

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

	SD_Init();

//	FATFS FatFs;
//	FIL file;
//	FRESULT result;
//
//	char buf[256];
//
//	printf("Mounting volume\r\n");
//	result = f_mount(&FatFs, "", 1); // Mount SD card
//
//	if (result) {
//		printf("Error mounting volume!\r\n");
//		while(1);
//	}
//
//	printf("Opening file: \"hello.txt\"\r\n");
//	result = f_open(&file, "hello.txt", FA_READ);
//
//	if (result) {
//		printf("Error opening file!\r\n");
//		while(1);
//	}
//
//	f_gets(buf, 256, &file);
//
//	printf("The file contains the following text:\n\"%s\"\r\n", buf);
//
//	f_close(&file); // Close file
//	f_mount(NULL, "", 1); // Unmount SD Card
	
	while (1){  
		TIMER_SoftTimersUpdate();
	}
}

void softTimerCallback(void) {

	LED_Toggle(LED0); // Toggle LED

	printf("Test string sent from STM32F4!!!\r\n"); // Print test string

}


uint32_t get_fattime() {

	return 0;
}
