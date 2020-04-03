/*
 * UART0.h
 *
 * Created: 27.3.2015 12:28:35
 *  Author: maticpi
 */ 


#ifndef UART0_H_
#define UART0_H_

#include <avr/io.h>
#include <stdio.h>
#include "config.h"

/*//Set your own function names for each UART
#define _Init() UART0_Init()
#define _SendStr(x) UART0_SendStr(x)
#define _SendBytes(x,y) UART0_SendBytes(x,y)
#define _putc(x,y) UART0_putc(x,y)
#define _DataReady() UART0_DataReady()
#define _GetByte() UART0_GetByte()
#define _put(x) UART0_put(x)
#define _str UART0_str
*/
typedef enum {
	UART_EOF	= -1,
	UART_OK		= 0
} UART_Status_t;

void UART0_Init();
int UART0_SendStr(char str[]);
int UART0_SendBytes(char data[], int num);
int UART0_putc(char c, FILE *stream);
int UART0_DataReady();
UART_Status_t UART0_GetByte(char* data);
UART_Status_t UART0_put(char data);

extern FILE UART0_str;

#endif /* UART0_H_ */