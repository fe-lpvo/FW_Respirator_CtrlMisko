/*
 * Respirator.c
 *
 * Created: 22.3.2020 17:18:24
 * Author : Kristjan
 */ 

#include <avr/io.h>
#include "CommonDefinitions.h"
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
#include "modeVCV.h"

int main(void)
{
	uint8_t msg[50],*p_msg;
	uint16_t *ADC_Results;
	uint32_t	mark1=0;
	uint8_t	operationMode=MODE_DEFAULT;
	
	RespSettings_t	Settings;
	uint16_t Flow, Pressure, Volume;
	
	//V konèni verziji se to prebere iz eeproma, 
	//da takoj nadaljujemo od koder smo konèali,
	//èe se sluèajno pobiramo iz nenamernega reseta
	Settings.current_mode=MODE_STOP;	
	Settings.new_mode=MODE_DEFAULT;
	Settings.rampup=SETTINGS_DEFAULT_RAMPUP_TIME_MS;
	Settings.vdih_t=SETTINGS_DEFAULT_INHALE_TIME_MS;
	Settings.izdih_t=SETTINGS_DEFAULT_EXHALE_TIME_MS;
	Settings.volume_t=SETTINGS_DEFAULT_VOLUME_ML;
		
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
			Flow = *(ADC_Results+ADC_CH_FLOW);
			Pressure = *(ADC_Results+ADC_CH_PRESSURE);
			Volume = motor_GetPosition();
						
			switch (operationMode)
			{
				case MODE_DEFAULT: modeVCV(Flow,Pressure,Volume, &Settings); break;
				default: 
					//printf("Error");
					operationMode = MODE_DEFAULT;
					break;
			}
			
			
			//koda traja 140 us
			LED1_Tgl();
			p_msg = msg;
			ADC_Results = ADC_results_p();	//Zakaj se ta pointer vsakic na novo prebere?
			
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
	}
}