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
#include "RespiratorApp/modeC_PCV.h"
#include "RespiratorApp/modeHWtest.h"
#include "UART0_IRQ.h"
#include "Measure.h"

//int32_t yy[100];	//global da compiler ne odoptimizira

int main(void)
{
	char msg[50];
	int length;
	uint32_t mark1=0, mark2=0;
	uint8_t	operationMode=MODE_DEFAULT;
	
	RespSettings_t	Settings;
	MeasuredParams_t Measured;
	CtrlParams_t Control;
	pidData_t PIDdata;	//Same PID params if regulating P or V ? Probably not.
						//Maybe make PID params local to ActuatorControl?
	/*
	int i;
	
	for (i=0;i<100;i++)
	{
		yy[i]=FIR(1023);
	}*/
	
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
	MeasureInit();
	PID_Init(64,1,0,&PIDdata);
	
	sei();
	
	while (1)
	{
		// na 2 ms
		if (ADC_scan_complete())
		{
			LED1_On();
			// branje ADC:
			MeasureFlow(&Measured);
			MeasurePressure(&Measured);
			MeasureVolume(&Measured);
						
			//TODO: mode state machines must return HW independent control values
			switch (operationMode)
			{
//				case MODE_STOP:   break;
				case MODE_C_VCV:  modeC_VCV(&Settings, &Measured, &Control); break;
				case MODE_C_PCV:  modeC_PCV(&Settings, &Measured, &Control); break;
//				case MODE_AC_VCV:  break;
//				case MODE_AC_PCV:  break;
//				case MODE_CPAP:	  break;
				case MODE_HW_TEST: modeHWtest(&Settings, &Measured, &Control); break;
				default: 
					ReportError(ModeUnknownMode,NULL/*"Unknown operation mode"*/);
					operationMode = MODE_DEFAULT;
					break;
			}
			ActuatorControl(&Control,&Measured,&PIDdata);
			LED1_Off();
			//koda traja xy us (140 us before hardware abstraction was implemented)
		}
		
		// na 2 ms
		if (Has_X_MillisecondsPassed(TIME_SLICE_MS,&mark1))
		{
			ADC_Start_First_Conversion();
		}
		
		//Report Status to the GUI
		if (Has_X_MillisecondsPassed(STATUS_REPORTING_PERIOD,&mark2))
		{
			length=PrepareStatusMessage(GetSysTick(), Measured.flow, Measured.pressure, Measured.volume_t, motor_GetPosition(), motor_GetCurrent(), motor_GetPWM(), Control.BreathCounter, Control.status, Control.Error, msg);
			UART0_SendBytes(msg,length);
		}
	}
}

