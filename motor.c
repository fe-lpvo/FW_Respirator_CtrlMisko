/*
 * motor.c
 *
 * Created: 2.4.2020 10:59:50
 *  Author: Kristjan
 */ 
#include "motor.h"
#include <avr/interrupt.h>

MotorDir_t MotorDir;

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
	EICRA = (1<<ISC01) | (1<<ISC11);//rising EDGE - stikala normaly close
	EIMSK = (1<<INT1) | (1<<INT0); //Enable interrupts
	EIFR = (1<<INTF1) | (1<<INTF0); //Clear flags
		
	TCCR1A = (1<<COM1A1) | (1<<WGM11) | (1<<WGM10);
	TCCR1B = (1<<CS10) | (1<<WGM12);
	OCR1A = 0;
	MotorDir = MOTOR_DIR_UNDEFINED;
}

ISR (INT0_vect)//SWA
{
	OCR1A = MOTOR_MIN_DC;
}

ISR(INT1_vect)//SWB
{
	OCR1A = 0;
}

void motor_SetDutyCycle(uint16_t dutyCycle)
{
	if (dutyCycle>MOTOR_MAX_DC) dutyCycle = MOTOR_MAX_DC;
	if ( MotorDir==MOTOR_DIR_VDIH) // ko stiskamo, ne sme DC nikoli pasti na nic, tudi ce pride do konca
	{
		if (!MOTOR_GET_SW_A) OCR1A = dutyCycle;
		else OCR1A = MOTOR_MIN_DC;		
	}
	else	//izdih
	{
		if (!MOTOR_GET_SW_B) OCR1A = dutyCycle;
		else OCR1A=0;
	}
	
}

int16_t motor_GetPosition()
{
	uint16_t raw=*(ADC_results_p()+ADC_CH_POSITION);
	return (MOTOR_POS_RAW_OPEN-raw) ;
}

int16_t motor_GetCurrent()
{
	uint16_t raw=*(ADC_results_p()+ADC_CH_MOTOR_CURRENT);
	return raw;
}

int16_t motor_GetPWM()
{
	if (OCR1A==0) return 0;
	if (MotorDir == MOTOR_DIR_VDIH) return OCR1A;
	else return -OCR1A;
}

void motor_SetDir(MotorDir_t direction)
{
	if (MotorDir!=direction)
	{
		switch (direction)
		{
			case MOTOR_DIR_VDIH:
				motor_INA_PORT &= ~(1<<motor_INA_PIN);
				_delay_us(10);	//dead time
				motor_INB_PORT |= (1<<motor_INB_PIN);
				motor_SEL0_PORT &= ~(1<<motor_SEL0_PIN);
				MotorDir=MOTOR_DIR_VDIH;
			break;
			
			case MOTOR_DIR_IZDIH:
				motor_INB_PORT &= ~(1<<motor_INB_PIN);
				_delay_us(10);	//dead time
				motor_INA_PORT |= (1<<motor_INA_PIN);
				motor_SEL0_PORT |= (1<<motor_SEL0_PIN);
				MotorDir=MOTOR_DIR_IZDIH;
			break;
			
			default: //do nothing
			break;
		}
	}
}