/*
 * motor.c
 *
 * Created: 2.4.2020 10:59:50
 *  Author: Kristjan
 */ 
#include "motor.h"
#include <avr/interrupt.h>


void motor_Init()
{
	motor_INA_PORT &= ~(1<<motor_INA_PIN);
	motor_INB_PORT &= ~(1<<motor_INB_PIN);
	motor_INA_DDR |= (1<<motor_INA_PIN);
 	motor_INB_DDR |= (1<<motor_INB_PIN);
	motor_SEL0_DDR |= (1<<motor_SEL0_PIN);
	motor_SEL0_PORT &= ~(1<<motor_SEL0_PIN);
	
	motor_PWM_PORT &= ~(1<<motor_PWM_PIN);
	motor_PWM_DDR |= (1<<motor_PWM_PIN);
	
	motor_SWA_DDR &= ~(1<< motor_SWA_PIN);
	motor_SWA_PORT |= (1<< motor_SWA_PIN);	//pullup
	
	motor_SWB_DDR &= ~(1<< motor_SWB_PIN);
	motor_SWB_PORT |= (1<< motor_SWB_PIN);	//pullup
		
	// ext interrupt za koncna stikala
	EICRA = (1<<ISC01) | (1<<ISC11);//Falling EDGE
	EIMSK = (1<<INT1) | (1<<INT0); //Enable interrupts
	EIFR = (1<<INTF1) | (1<<INTF0); //Clear flags
		
	TCCR1A = (1<<COM1A1) | (1<<WGM11) | (1<<WGM10);
	TCCR1B = (1<<CS10) | (1<<WGM12);
	OCR1A = 0;
}

ISR (INT0_vect)//SWA
{
	OCR1A = 0;
}

ISR(INT1_vect)//SWB
{
	OCR1A = 0;
}

void motor_SetDutyCycle(uint16_t dutyCycle)
{
	if (dutyCycle>MAX_DC) dutyCycle = MAX_DC;
	OCR1A = dutyCycle;
}

uint16_t motor_GetPosition()
{
	uint16_t *p;
	p = ADC_results_p();
	return (MOTOR_POS_MAX-*(p+MOTOR_POS_CH));
}

void motor_SetDirVdih() 
{
	motor_INA_PORT &= ~(1<<motor_INA_PIN);
	_delay_us(10);	//dead time
	motor_INB_PORT |= (1<<motor_INB_PIN);
	motor_SEL0_PORT &= ~(1<<motor_SEL0_PIN);
}

void motor_SetDirIzdih() 
{
	motor_INB_PORT &= ~(1<<motor_INB_PIN);
	_delay_us(10);	//dead time
	motor_INA_PORT |= (1<<motor_INA_PIN);
	motor_SEL0_PORT |= (1<<motor_SEL0_PIN);
}