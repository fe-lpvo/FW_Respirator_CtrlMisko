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
#include "modeC_VCV.h"

int main(void)
{
	char msg[50];
	int length;
	uint16_t *ADC_Results;
	uint32_t mark1=0;
	uint8_t	operationMode=MODE_DEFAULT;
	
	RespSettings_t	Settings;
	uint16_t Flow, Pressure, Volume;
	
	//V konèni verziji se to prebere iz eeproma, 
	//da takoj nadaljujemo od koder smo konèali,
	//èe se sluèajno pobiramo iz nenamernega reseta
	Settings.current_mode=MODE_STOP;	
	Settings.new_mode=MODE_DEFAULT;
	Settings.P_ramp=SETTINGS_DEFAULT_RAMPUP_TIME_MS;
	Settings.inspiratory_t=SETTINGS_DEFAULT_INHALE_TIME_MS;
	Settings.expiratory_t=SETTINGS_DEFAULT_EXHALE_TIME_MS;
	Settings.volume_t=SETTINGS_DEFAULT_VOLUME_ML;
	
	//TODO: read current state of the machine
	//Is it possible the get the exact state? 
	//Direction (Inspiration/Expiration), Position
	//duration of the break (maybe ms resolution RTC)?
	//maybe also external RAM with backup battery? Or FRAM?
		
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
//				case MODE_STOP: break;
				case MODE_C_VCV: modeC_VCV(Flow,Pressure,Volume, &Settings); break;
//				case MODE_C_PCV: modeC_PCV(Flow,Pressure,Volume, &Settings); break;
//				case MODE_CPAP:	 modeCPAP( Flow,Pressure,Volume, &Settings); break;
				default: 
					//printf("Error");
					operationMode = MODE_DEFAULT;
					break;
			}
			//TODO: Get hardware abstracted actuation values from the mode state machine 
			//and execute them here
			
			//MotorControl(some settings);
			//ValveControl(some more settings);
			//etc...
			
			//koda traja xy us (140 us before hardware abstraction was implemented)
			
			//ADC_Results = ADC_results_p();	//Zakaj se ta pointer vsakic na novo prebere?
			
			//Report Status to the GUI
			LED1_On();
			length=PrepareStatusMessage(GetSysTick(), Flow, Pressure, Volume, msg);	//To trenutno dela brez prekinitev!
			UART0_SendBytes(msg,length);
			LED1_Off();
		}
	}
}