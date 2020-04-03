/*
 * Stepper.c
 *
 * Created: 22.3.2020 17:21:51
 *  Author: Kristjan
 */ 
#include "Stepper.h"

char g_stepPin = 0;
char g_dirPin = 0;

void Init_Stepper(char stepPin, char dirPin)
{
	    DDRE |= (1<<stepPin)|(1<<dirPin);     // Configure PORTE0, PORTE1,  as output
		g_stepPin = stepPin;
		g_dirPin = dirPin;
}

void Stepper_Steps(char direction, uint16_t steps, uint16_t delay_us)
{
	if(direction) PORTE |= (1<<g_dirPin);
	else PORTE &= ~(1<<g_dirPin);

	for(int i=0; i<steps; i++)
	{
		PORTE |=(1<<g_stepPin);
		for(int i = delay_us; i>=0; i--) _delay_us(1);
		PORTE &=~(1<<g_stepPin);
		for(int i = delay_us; i>=0; i--) _delay_us(1);
	}
}
