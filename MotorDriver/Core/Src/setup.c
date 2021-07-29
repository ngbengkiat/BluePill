/*
 * setup.c
 *
 *  Created on: Jul 29, 2021
 *      Author: nbk
 */

#include "main.h"

GYRO_DrvTypeDef  *GyroDrv = &L3gd20Drv;
void setup(){
	STDIO_init();

	//L3GD20 gyro init
	GYRO_InitTypeDef gyroInitStruct;
	gyroInitStruct.Axes_Enable = L3GD20_AXES_ENABLE;
	gyroInitStruct.Band_Width = L3GD20_BANDWIDTH_1;
	gyroInitStruct.BlockData_Update = L3GD20_BlockDataUpdate_Continous;
	gyroInitStruct.Endianness = L3GD20_BLE_MSB;
	gyroInitStruct.Full_Scale = L3GD20_FULLSCALE_2000;
	gyroInitStruct.Output_DataRate = L3GD20_OUTPUT_DATARATE_1;
	gyroInitStruct.Power_Mode = L3GD20_MODE_ACTIVE;

	GyroDrv->Init(&gyroInitStruct);

	//OLED init
	SSD1306_Init();
	SSD1306_GotoXY(5, 0); // coordinates x, y
	SSD1306_Puts("Hello", &Font_11x18, 1); // print Hello
	SSD1306_GotoXY(0, 16); // coordinates x, y
	SSD1306_Puts("ssd1306 OLED", &Font_11x18, 1);
	SSD1306_GotoXY(0, 32); // coordinates x, y
	SSD1306_Puts("Line 32", &Font_11x18, 1);

	SSD1306_DrawLine(0, 55, 128, 55, 1);
	SSD1306_UpdateScreen(); // update screen

	clrscr();
	printf("Hello World! %s", __DATE__ __TIME__ );

}
