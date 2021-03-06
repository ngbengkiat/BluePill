// ---------------------------------------------------------------------------------
// libUart.c
// Created on: 07-Jan-2010
// Author: nbk
// Desc  :
// UARTx_init() to initialise UARTx
// RXCharISR() & TxCharISR() to get receive/transmit char from/to UART
// GetChar() & PutChar() to get or put char on stdio.
// ---------------------------------------------------------------------------------
#include "main.h"
#include "project.h"
#include <stdio.h>


//Change UART_STDIO according to UART used
//uartNum is the USART number minus 1, so USART1 -> 0
int16_t UART_STDIO = 0;			//To store the STDIO UART number

// These variables are used for UART ISR transmit & receive buffers
#define BUFSIZE		64
#define NUM_UART	3
static unsigned char rxBuffer[NUM_UART][BUFSIZE];
static unsigned char txBuffer[NUM_UART][BUFSIZE];
volatile static int16_t rxReadIndex[NUM_UART], rxWriteIndex[NUM_UART];
volatile static int16_t txReadIndex[NUM_UART], txWriteIndex[NUM_UART];
volatile int8_t txEnableFlag[NUM_UART];

USART_TypeDef* usartLut[NUM_UART] = {USART1, USART2, USART3 };
int16_t usartIRQnLut[NUM_UART] = {USART1_IRQn, USART2_IRQn,USART3_IRQn };

// ---------------------------------------------------------------------------------
// @brief : To init one of the UART as STDIO port (for printf(), GetChar() etc)
// @retval: none
void STDIO_init( ) {

    // turn off buffers, so IO occurs immediately
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

}



// ---------------------------------------------------------------------------------
// @brief : To check if a rx character from UART is available
// @param : uartNum is the USART number minus 1, so USART1 -> 0
// @retval: TRUE if character is available, else FALSE
int8_t RxCharAvailableISR(int16_t uartNum) {

	return rxWriteIndex[uartNum]!=rxReadIndex[uartNum];
}
// ---------------------------------------------------------------------------------
// @brief : To get a character from UART through interrupt
// @param : uartNum is the USART number minus 1, so USART1 -> 0
// @retval: character received
int RxCharISR(int16_t uartNum) {
	unsigned char ch;

	while(rxWriteIndex[uartNum]==rxReadIndex[uartNum]);

	ch = rxBuffer[uartNum][rxReadIndex[uartNum]];

	rxReadIndex[uartNum]++;

	if (rxReadIndex[uartNum]>=BUFSIZE)
		rxReadIndex[uartNum]=0;
	return ch;
}

// ---------------------------------------------------------------------------------
// @brief : To put a character into usart through interrupt
// @param : c-character to send,
// @param : uartNum is the USART number minus 1, so USART1 -> 0
// @retval: none
int TxCharISR(int16_t uartNum, char c) {

	// If tx buffer is full, wait
	while((txWriteIndex[uartNum]==(txReadIndex[uartNum]-1)) ||
		(txWriteIndex[uartNum]==(txReadIndex[uartNum]+BUFSIZE-1)) ) {

//		if (bSWFlag) {
//			break;
//		}
	}


	txBuffer[uartNum][txWriteIndex[uartNum]] = c;

	DI;
	txWriteIndex[uartNum]++;

	if (txWriteIndex[uartNum]>=BUFSIZE)
		txWriteIndex[uartNum]=0;
	EI;

	if (!txEnableFlag[uartNum]) {
		// turn ON tx interrupt to begin the transfer
		txEnableFlag[uartNum] = TRUE;
		LL_USART_EnableIT_TXE(usartLut[uartNum]);
	}
	return c;
}

// @brief : To check if a character is available for reading
// @param : none
// @retval: true if character is available, else false.
int8_t kbhit(){
	return (rxWriteIndex[UART_STDIO]!=rxReadIndex[UART_STDIO]);
//	return (USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET);
}

#ifdef POLLING
// @brief : To get a character from UART through polling
// @param : none
// @retval: character received
int RxChar(USART_TypeDef* usart) {
	while(USART_GetFlagStatus(usart, USART_IT_RXNE) == RESET);
	return USART_ReceiveData(usart);
}

// ---------------------------------------------------------------------------------
// @brief : To put a character into UART through polling
// @param c: character to send
// @retval: none
int TxChar(USART_TypeDef* usart, char c) {
	while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
   	USART_SendData(usart,c);
   	return 0;
}
#endif





// @brief : General usart ISR
// @param : uartNum is the USART number minus 1, so USART1 -> 0
// @retval: none
void USART_ISR(int16_t uartNum) {
	char ch;
	USART_TypeDef*	uart;

	uart = usartLut[uartNum];

	if (LL_USART_IsActiveFlag_ORE(uart)==SET) {
		LL_USART_ClearFlag_ORE(uart) ;
		LL_USART_ReceiveData8(uart);
	}

	if (LL_USART_IsActiveFlag_RXNE(uart)==SET)
	{
		rxBuffer[uartNum][rxWriteIndex[uartNum]++] = LL_USART_ReceiveData8(uart);

		if (rxWriteIndex[uartNum]>=BUFSIZE)
			rxWriteIndex[uartNum]=0;
	}

	if (LL_USART_IsActiveFlag_TXE(uart)==SET) {

		if (txWriteIndex[uartNum]!=txReadIndex[uartNum]) {

			ch = txBuffer[uartNum][txReadIndex[uartNum]];

			LL_USART_TransmitData8(uart, ch);

			txReadIndex[uartNum]++;
			if (txReadIndex[uartNum]>=BUFSIZE)
				txReadIndex[uartNum]=0;

		}
		else {

			LL_USART_DisableIT_TXE(uart);
		    txEnableFlag[uartNum] = FALSE;
		}

	}
}

// ---------------------------------------------------------------------------------
// @brief : To get a character from stdio, which is UART_STDIO
// @param : none
// @retval: character received
int __io_getchar() {
	return RxCharISR(UART_STDIO);
}

// ---------------------------------------------------------------------------------
// @brief : To put a character into stdio, which is UART_STDIO
// @param c: character to send
// @retval: none

int __io_putchar(uint8_t c) {
	return TxCharISR(UART_STDIO,(char)c);
}

// ---------------------------------------------------------------------------------
// @brief : To get a character from stdio, which is UART_STDIO
// @param : none
// @retval: character received
int GetChar() {
	return RxCharISR(UART_STDIO);				//isr version
//	return RxChar(usartLut[UART_STDIO]);		//Polling version
}

// ---------------------------------------------------------------------------------
// @brief : To put a character into stdio, which is UART_STDIO
// @param c: character to send
// @retval: none
int PutChar(int c) {
	return TxCharISR(UART_STDIO,(char)c);				//isr version
	//return TxChar(usartLut[UART_STDIO], c);	//Polling version
}
