/*
 * loop.c
 *
 *  Created on: Jul 29, 2021
 *      Author: nbk
 */
#include "main.h"

void loop(){
	while (1)
	{
		int i;

		float buf[3]={-1.0,-1.0,-1.0};;
		char s[] = "finally...\n";
		//char c = GetChar();

		//PutChar(c);
		GyroDrv->GetXYZ(buf);

		gotoxy(5,3);
		printf("gyro=%6.2f, %6.2f, %6.2f", buf[0], buf[1], buf[2]);
		gotoxy(5,5);
		printf("enc=%6d %6d", (int16_t)LL_TIM_GetCounter(TIM2), (int16_t)LL_TIM_GetCounter(TIM3));
		gotoxy(5,7);
		printf("i=%d",i++);

		// LED ON
		GPIO_Write(LED0_PIN, 1);

		HAL_Delay(100);

		// LED OFF
		GPIO_Write(LED0_PIN, 0);

		HAL_Delay(100);

	}
}
