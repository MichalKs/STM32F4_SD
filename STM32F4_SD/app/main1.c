


//int main(void)
//{
	

//	SD_Init();
//
//	uint8_t buf[1024];
//
//	SD_ReadSectors(buf, 0, 1);
//
//	TIMER_Delay(1000);
//	hexdump(buf, 512);
//
//	printf("After hexdump\r\n");

//	FAT_Init(SD_Init, SD_ReadSectors, SD_WriteSectors);

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
//	f_gets((char*)buf, 256, &file);
//
//	printf("The file contains the following text:\r\n\"%s\"\r\n", buf);
//
//	f_close(&file); // Close file
//	f_mount(NULL, "", 1); // Unmount SD Card
	

//}




