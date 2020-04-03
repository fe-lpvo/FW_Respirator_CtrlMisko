/*
 * ADC.c
 *
 * Created: 30.3.2020 20:04:41
 *  Author: 
 */ 

#include "ADC.h"
#include "GPIO.h"
#include <avr/interrupt.h>

uint8_t ADC_channel = 0;
uint8_t ADC_complete = 0;
static volatile uint16_t Filter_Array_0[ADC_FILTER_N];
static volatile uint16_t Filter_Array_1[ADC_FILTER_N];
uint16_t ADC_results_int[4]={0,0,0,0};	// interni rezultati - double buffering
uint16_t ADC_results[4];	//rezultati, ki se vrnejo v main - inicializacija na 0 zagotovljena
static volatile uint8_t filter_count, filter_count_old;

void ADC_Init()
{
	ADMUX |= (1<<REFS0) | (1<<MUX0); //Vcc and starting with ADC1
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2); //Enable ADC, INT, prescaler 128
	DIDR0 |= (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D) | (1<<ADC3D);
	ADC_channel = ADC_STARTCHAN;
	filter_count = 0;
	filter_count_old = 1;
	for (uint8_t i=0;i<ADC_FILTER_N;i++)
	{
		Filter_Array_0[i]=0;
		Filter_Array_1[i]=0;
	}
}

uint16_t *ADC_results_p(void)
{
	return ADC_results;
}

void ADC_Select_Channel(char channel)
{
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
}

void ADC_Start_Conversion()
{
	ADCSRA |= (1<<ADSC);
}

void ADC_Start_First_Conversion()
{
	ADC_channel = ADC_STARTCHAN;
	ADMUX = (ADMUX & 0xF0) | (ADC_STARTCHAN & 0x0F);
	ADCSRA |= (1<<ADSC);
}

char ADC_scan_complete()
{
	if (ADC_complete) 
	{
		ADC_complete = 0;
		return 1;
	}
	else return 0;
}

ISR(ADC_vect)
{
	switch (ADC_channel)
	{
		case 0: // moving average filter za kanal 0
			Filter_Array_0[filter_count] = ADC;
			ADC_results_int [ADC_channel] += ADC;
			ADC_results_int [ADC_channel] -= Filter_Array_0[filter_count_old];
		break;
		
		case 1: // moving average filter za kanal 1
			Filter_Array_1[filter_count] = ADC;
			ADC_results_int [ADC_channel] += ADC;
			ADC_results_int [ADC_channel] -= Filter_Array_1[filter_count_old];
			
			// upravljanje s kazalci na filter bufferje
			filter_count++;
			filter_count_old++;
				
			if (filter_count==ADC_FILTER_N-1)
			{
				filter_count_old = 0;
			}
			else if (filter_count==ADC_FILTER_N)
			{
				filter_count = 0;
			}
		break;
		
		default: // ostali kanali se ne filtrirajo
			ADC_results_int[ADC_channel]= ADC;
		break;
	}
	
	// upravljanje s kanali in pretvorbo ADC
	ADC_channel++;
	
	if (ADC_channel > ADC_STOPCHAN) 
	{	
		ADC_channel = 255;
		for (uint8_t i=0;i<=ADC_STOPCHAN;i++)
		{
			ADC_results[i] = ADC_results_int[i];
		}
		ADC_complete = 1;
	}
	else 
	{
		ADC_Select_Channel(ADC_channel);
		ADC_Start_Conversion();
	}
}