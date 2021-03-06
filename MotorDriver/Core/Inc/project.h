// ---------------------------------------------------------------------------------
//   File        :project.h
//   Version     :v1.0.1
//   Date        :2011/02/17
//   Author      :nbk
//   Description :
// ---------------------------------------------------------------------------------

#ifndef __PROJECT_H___
#define __PROJECT_H___

#include <stdio.h>
#include <math.h>
#include "ssd1306.h"
#include "l3gd20.h"
#include "libMotor.h"
#include "libLogData.h"


#define FALSE		0
#define TRUE		1

#define MHZ						1000000L
#define dT	0.001f
#define TICK_PERIOD	(int)(dT*MHZ)
#define TICK_FREQ	(int)(1/dT+0.5f)
// ---------------------------------------------------------------------------------
// Data type definition
// ---------------------------------------------------------------------------------
typedef void (*type_fp) (void);

// -------------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------------

// IO pins definition. These macros make it easier to write & modify your program
#define LED0_PIN		GPIOB,GPIO_PIN_2
#define LMOTORDIR_PIN	GPIOB, GPIO_PIN_4
#define RMOTORDIR_PIN	GPIOB, GPIO_PIN_5

#define GPIO_Write_1(GPIOx, PinMask)	WRITE_REG(GPIOx->BSRR, (PinMask) & 0x0000FFFFU)
#define GPIO_Write_0(GPIOx, PinMask)	WRITE_REG(GPIOx->BRR, (PinMask) & 0x0000FFFFU)
#define GPIO_Write(pin, value)			(value)==1?GPIO_Write_1(pin):GPIO_Write_0(pin)

#define DI			__disable_irq()
#define EI			__enable_irq()
#define ABS(n)		((n)>=0?(n):(-n))		// Get absolute value
#define CLIP(n, min, max)	((n)=(n)>(max)?(max):((n)<(min)?(min):(n)))

#define LimitPI(a)		((a)>(M_PI)?(a-2*M_PI):((a)<(-M_PI)?((a)+2*M_PI):(a)))
#define DEG_RAD(a)		(((a)*M_PI)/180)
#define DEG01_RAD(a)	(((a)*M_PI)/1800)
#define RAD_DEG(a)		(((a)*180)/M_PI)
#define RAD_DEG01(a)	(((a)*1800)/M_PI)
#define Limit1800Deg(a)	( ((a)>1800) ? (-3600+(a)) : ( (a)<-1800 ? (3600+(a)) : (a) ) )




// ---------------------------------------------------------------------------------
//  Global variables
// ---------------------------------------------------------------------------------

extern char s[80];
extern GYRO_DrvTypeDef  *GyroDrv;

// ---------------------------------------------------------------------------------
//  Function prototypes
// ---------------------------------------------------------------------------------
void setup();
void loop();

/* Motor related routines (motor.c) */
void InitMotor(void);		/* motor initialisation */
void UpdateWheelPos();
void InitMotorData();
void PrintMotorValues(void);
void PrintLog();
void ClearLog();
void TestMotorMenu();

int PutChar(int c);
int GetChar(void);
void gotoxy(int x, int y);
void clrscr(void);
int8_t kbhit();
int RxChar(USART_TypeDef* usart);
int TxChar(USART_TypeDef* usart, char c);
int RxCharISR(int16_t uartNum);
int TxCharISR(int16_t uartNum, char c);
int8_t RxCharAvailableISR(int16_t uartNum);

#endif
