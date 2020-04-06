/*
 * Respirator.c
 *
 * Created: 22.3.2020 17:18:24
 * Author : Kristjan
 */ 

#include <avr/io.h>
#include "RespiratorApp/CommonDefinitions.h"
#include "LCD_Ili9341.h"
#include "config.h"
#include "systime.h"
#include "GPIO.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include "PID.h"
#include "ADC.h"
#include "CIRC_BUFF.h"
#include "RespiratorApp/CommProtocol.h"
#include "motor.h"
#include "ActuatorControl.h"
#include "RespiratorApp/modeC_VCV.h"
#include "UART0_IRQ.h"

int main(void)
{
	char msg[50];
	int length;
	uint16_t *ADC_Results;
	uint32_t mark1=0;
	uint8_t	operationMode=MODE_DEFAULT;
	
	RespSettings_t	Settings;
	MeasuredParams_t Measured;
	CtrlParams_t Control;
	pidData_t PIDdata;
	
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
	PID_Init(100,10,10,&PIDdata);
	ADC_Results=ADC_results_p();
	sei();
	
	while (1)
	{
		if (Has_X_MillisecondsPassed(5,&mark1))
		{
			// branje ADC:
			Measured.flow = *(ADC_Results+ADC_CH_FLOW);
			Measured.pressure = *(ADC_Results+ADC_CH_PRESSURE);
			Measured.volume_t = motor_GetPosition();
						
			//TODO: mode state machines must return HW independent control values
			switch (operationMode)
			{
//				case MODE_STOP:   break;
				case MODE_C_VCV:  modeC_VCV(&Settings, &Measured, &Control); break;
//				case MODE_C_PCV:  break;
//				case MODE_AC_VCV:  break;
//				case MODE_AC_PCV:  break;
//				case MODE_CPAP:	  break;
				default: 
					ReportError(ModeUnknownMode,NULL/*"Unknown operation mode"*/);
					operationMode = MODE_DEFAULT;
					break;
			}
			ActuatorControl(&Control,&Measured,&PIDdata);
			//koda traja xy us (140 us before hardware abstraction was implemented)
			
			//ADC_Results = ADC_results_p();	//Zakaj se ta pointer vsakic na novo prebere?
			
			//Report Status to the GUI
			LED1_On();
			length=PrepareStatusMessage(GetSysTick(), Measured.flow,\
							Measured.pressure, Measured.volume_t, msg);
			UART0_SendBytes(msg,length);
			LED1_Off();
		}
	}
}

