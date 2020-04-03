/*
 * UART0.c
 *
 * Created: 6.6.2014 15:48:07
 *  Author: maticpi
 */ 
#include "UART0.h"

FILE UART0_str = FDEV_SETUP_STREAM(UART0_putc,NULL,_FDEV_SETUP_WRITE);

int UART0_SendStr(char str[])
{
	int i;
	for (i=0; str[i]; i++)
	{
		if (UART0_put(str[i]) != UART_OK) return i;
	}
	return i;
}

int UART0_SendBytes(char data[], int num)
{
	int i;
	for (i=0; i<num; i++)
	{
		if (UART0_put(data[i]) != UART_OK) return i;
	}
	return i;
}

int UART0_putc(char c, FILE *stream)
{
	while ((UCSR0A & (1<<UDRE0)) == 0) {}
	UDR0 = c;
	return 0;
}

UART_Status_t UART0_put(char data)
{
	while ((UCSR0A & (1<<UDRE0)) == 0) {}
	UDR0 = data;
	return UART_OK;
}

UART_Status_t UART0_GetByte(char *data)
{
	if (UCSR0A & (1<<RXC0))
	{
		*data = UDR0;
		return UART_OK;
	}
	return UART_EOF;
}

int UART0_DataReady()
{
	return (UCSR0A & (1<<RXC0));
}

void UART0_Init()
{
	//1-output, 0-input
	DDRD |= (1<<1); //PD1 - TX0
	DDRD &= ~(1<<0); //PD0 - RX0
	
	//USART0 - UART0_ 115200 baud, 8bits, 1 stop bit, no parity, no flow control
	UCSR0A = 0;
	UCSR0C = (0<<UMSEL00) | (0<<UPM00) | (0<<USBS0) | (3<<UCSZ00);
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02);

	#ifndef BAUD0
		#warning "UART0.c: BAUD0 not defined! Assuming 115200."
		#define BAUD0 115200UL
	#endif
	#undef BAUD
	#define BAUD BAUD0
	#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	#if USE_2X
	UCSR0A |= (1 << U2X0);
	#else
	UCSR0A &= ~(1 << U2X0);
	#endif
}