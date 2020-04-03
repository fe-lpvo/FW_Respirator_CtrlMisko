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

// Input - koncna stikala - PD2 in PD3
#define motor_SWA_DDR DDRD
#define motor_SWA_PORT PORTD
#define motor_SWA_PIN 2

#define motor_SWB_DDR DDRD
#define motor_SWB_PORT PORTD
#define motor_SWB_PIN 3

// meritev pozicije motorja - kanal ADC
#define MOTOR_POS_CH	3
#define MOTOR_POS_MIN	20
#define MOTOR_POS_MAX	500

// maksimalna vredfnost registra za dutycylce
#define MAX_DC	1023
// ******************************
// Konec Uporabniskih nastavitev
// ******************************

#define MOTOR_OK 1
#define MOTOR_ERR 0

void motor_Init();
void motor_SetDutyCycle(uint16_t dutyCycle);
void motor_SetDirVdih();
void motor_SetDirIzdih();
char motor_SetDirection(char direction);
uint16_t motor_GetPosition();

#endif /* MOTOR_H_ */