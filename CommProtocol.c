/*
 * CommProtocol.c
 *
 * Created: 3. 04. 2020 17:50:54
 *  Author: maticpi
 */ 
#include "CommProtocol.h"

void SendStatus(uint32_t timestamp, uint16_t Flow, uint16_t Pressure, uint16_t Volume)
{
	uint8_t msg[50],*p_msg;

	LED1_Tgl();
	p_msg = msg;
				
	*p_msg = 0x55;
	p_msg++;
				
	*p_msg = MSG_CORE_LENGTH;
	p_msg++;
				
	*(uint32_t *)p_msg = GetSysTick();
	p_msg +=4;
				
	*(uint16_t *)p_msg = Flow;
	p_msg +=2;
				
	*(uint16_t *)p_msg = Pressure;
	p_msg +=2;
				
	*(uint16_t *)p_msg = Volume;
	p_msg +=2;
				
	*(p_msg) = 0xAA;

	//STX+N+TIMESTAMP+4xADC+ETX
	UART0_SendBytes((char*)msg,1+1+MSG_CORE_LENGTH+1);
	LED1_Off();
}