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
#include "CommProtocol.h"
#include "motor.h"
#include "modeVCV.h"

int main(void)
{
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
				case MODE_STOP: break;
				case MODE_VCV:  modeVCV(Flow,Pressure,Volume, &Settings); break;
//				case MODE_PCV:  modeVCV(Flow,Pressure,Volume, &Settings); break;
//				case MODE_CPAP: modeVCV(Flow,Pressure,Volume, &Settings); break;
				default: 
					//printf("Error");
					operationMode = MODE_DEFAULT;
					break;
			}
			//koda traja 140 us
			ADC_Results = ADC_results_p();	//Zakaj se ta pointer vsakic na novo prebere?
			SendStatus(GetSysTick(), Flow, Pressure, Volume);	//To trenutno dela brez prekinitev!
		}
	}
}