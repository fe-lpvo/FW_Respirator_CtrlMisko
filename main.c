/*
 * Respirator.c
 *
 * Created: 22.3.2020 17:18:24
 * Author : Kristjan
 */ 

#include <avr/io.h>
#include "LCD_Ili9341.h"
#include "config.h"
#include "systime.h"
#include "GPIO.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include "PID.h"
#include "ADC.h"
#include "CIRC_BUFF.h"
#include "UART0.h"
#include "motor.h"

#define NUMBER_OF_BYTES	10

int main(void)
{
	uint8_t msg[50],*p_msg;
	uint16_t *ADC_Results;
	uint32_t	mark1=0;
	uint8_t dihanje_state = 0;
	uint16_t timer_10ms;
	
	// 5 ms je time slice
	uint8_t Settings_rampup=20;	// 50ms
	uint16_t Settings_vdih_t=250; // 1.25s
	uint16_t Settings_izdih_t=600;	//3s
	uint16_t Settings_volume_t=250;	//500 ml
	
	uint16_t Flow, Pressure, Volume;
		
	LED_Init();
	ADC_Init();
	UART0_Init();
	Systime_Init();
	motor_Init();
	ADC_Results=ADC_results_p();
	sei();
	
	while (1)
	{
		if (Has_X_MillisecondsPassed(5,&mark1))
		{
			// branje ADC:
			Flow = *(ADC_Results++);
			Pressure = *(ADC_Results);
			Volume = motor_GetPosition();
						
			// dihanje
			switch (dihanje_state)
			{
				case 0: // za?etek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
					motor_SetDirIzdih();
					motor_SetDutyCycle(200);
					dihanje_state++;
				break;
				
				case 1: // cakaj, da so klesce narazen
					if (motor_GetPosition()<MOTOR_POS_MIN)
					{
						motor_SetDutyCycle(0);
						dihanje_state++;
					}	
				break;
				
				case 2: //zacni ramp-up
					motor_SetDirVdih();
					motor_SetDutyCycle(1023);
					dihanje_state++;
					timer_10ms = 0;
				break;
				
				case 3: //ramp-up
					timer_10ms++;
					if (timer_10ms >= Settings_rampup)	// gremo v constant pressure
					{
						timer_10ms = 0;
						dihanje_state++;
						motor_SetDutyCycle(1023);
					}
				break;
				
				case 4: //constant pressure
					timer_10ms++;
					if (motor_GetPosition()>=Settings_volume_t || timer_10ms > Settings_vdih_t) // izdih
					{
						timer_10ms = 0;
						dihanje_state++;
						motor_SetDutyCycle(300);
						motor_SetDirIzdih();
					}
				break;
				
				case 5: //izdih
					timer_10ms++;
					if (motor_GetPosition()<MOTOR_POS_MIN) motor_SetDutyCycle(0);	//konec izdiha
					if (timer_10ms > Settings_izdih_t)	// izdih
					{
						dihanje_state=0;
						motor_SetDutyCycle(0);
					}
				break;
				
				default:
					motor_SetDutyCycle(0);
				break;
			}
			
			
			//koda traja 140 us
			LED1_Tgl();
			p_msg = msg;
			ADC_Results = ADC_results_p();	//Zakaj se ta pointer vsakic na novo prebere?
			
			*p_msg = 0x55;
			p_msg++;
			
			*p_msg = NUMBER_OF_BYTES;
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
			UART0_SendBytes((char*)msg,1+1+NUMBER_OF_BYTES+1);
			LED1_Off();
		}
	}
}