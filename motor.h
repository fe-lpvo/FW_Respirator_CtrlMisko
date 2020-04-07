/*
 * motor.h
 *
 * Created: 2.4.2020 10:59:59
 *  Author: Kristjan
 */ 


#ifndef MOTOR_H_
#define MOTOR_H_

#include <avr/io.h>
#include "config.h"
#include <util/delay.h>
#include "ADC.h"

// ******************************
// Uporabniske nastavitve
// ******************************

// PWM Out PIN = PB1
#define motor_PWM_DDR DDRB
#define motor_PWM_PORT PORTB
#define motor_PWM_PIN 1

// Direction pins PD4 in PD5
#define motor_INA_DDR DDRD
#define motor_INA_PORT PORTD
#define motor_INA_PIN 5

#define motor_INB_DDR DDRD
#define motor_INB_PORT PORTD
#define motor_INB_PIN 4
// Current sense selection pins PD4 in PD5
#define motor_SEL0_DDR DDRB
#define motor_SEL0_PORT PORTB
#define motor_SEL0_PIN 0

// Input - koncna stikala - PD2 in PD3 - normaly closed - ?e je 0, je vse OK, ?e je 1, je motor povozil stikalo
#define motor_SWA_DDR DDRD
#define motor_SWA_PORT PORTD
#define motor_SWA_PORTIN PIND
#define motor_SWA_PIN 2

#define motor_SWB_DDR DDRD
#define motor_SWB_PORT PORTD
#define motor_SWB_PORTIN PIND
#define motor_SWB_PIN 3

// meritev pozicije motorja - kanal ADC
#define MOTOR_POS_CH	ADC_CH_POSITION
#define MOTOR_POS_RAW_OPEN	830	// --> 0
#define MOTOR_POS_RAW_CLOSED 740	// -->?
#define MOTOR_POS_BREAKINGPOINT_OPEN 885
#define MOTOR_POS_BREAKINGPOINT_CLOSED 726
#define MOTOR_POS_OPEN	0		// pos: \ /
#define MOTOR_POS_CLOSED	(MOTOR_POS_RAW_OPEN - MOTOR_POS_RAW_CLOSED) // pos: ||

// maksimalna vredfnost registra za dutycylce
#define MAX_DC	1023
// ******************************
// Konec Uporabniskih nastavitev
// ******************************

#define MOTOR_OK 1
#define MOTOR_ERR 0

#define MOTOR_GET_SW_A	(motor_SWA_PORTIN&(1<<motor_SWA_PIN))
#define MOTOR_GET_SW_B	(motor_SWB_PORTIN&(1<<motor_SWB_PIN))

typedef enum
{
	MOTOR_VDIH,
	MOTOR_IZDIH
}MotorDir_t;

void motor_Init();
void motor_SetDutyCycle(uint16_t dutyCycle);
void motor_SetDirVdih();
void motor_SetDirIzdih();
char motor_SetDirection(char direction);
int16_t motor_GetPosition();
int16_t motor_GetCurrent();

#endif /* MOTOR_H_ */