/*
 * Respirator.c
 *
 * Created: 22.3.2020 17:18:24
 * Author : Kristjan
 */ 

#include <avr/io.h>
#include <string.h>
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

/*Include these for testing */
#include "parser_test.h"

//int32_t yy[100];	//global da compiler ne odoptimizira

int main(void)
{
	char msg[50];
	int length;
	char com_data;
	uint32_t mark1=0, mark2=0;
	uint8_t newSettingsReceived;
	uint8_t	operationMode=MODE_DEFAULT;
	
	RespSettings_t	Settings;
	MeasuredParams_t Measured;
	CtrlParams_t Control;
	pidData_t PIDdata;	//Same PID params if regulating P or V ? Probably not.
						//Maybe make PID params local to ActuatorControl?
	
	memset(&Settings,0,sizeof(RespSettings_t));
	memset(&Measured,0,sizeof(MeasuredParams_t));
	memset(&Control,0,sizeof(CtrlParams_t));
	memset(&PIDdata,0,sizeof(pidData_t));
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
	Settings.target_Pramp_time=SETTINGS_DEFAULT_RAMPUP_TIME_MS;
	Settings.target_inspiratory_time=SETTINGS_DEFAULT_INHALE_TIME_MS;
	Settings.target_expiratory_time=SETTINGS_DEFAULT_EXHALE_TIME_MS;
	Settings.target_volume=SETTINGS_DEFAULT_TARGET_VOLUME_ML;
	Settings.PEEP = SETTINGS_DEFAULT_PEEP;
	Settings.PeakInspPressure = SETTINGS_DEFAULT_MAX_PRESSURE_MBAR;
	Settings.target_pressure = SETTINGS_DEFAULT_TARGET_PRESSURE_MBAR;

	Settings.PID_P = SETTINGS_DEFAULT_PID_P;
	Settings.PID_I = SETTINGS_DEFAULT_PID_I;
	Settings.PID_D = SETTINGS_DEFAULT_PID_D;
	Settings.MOT_POS = SETTINGS_DEFAULT_MOT_POS;
	
	//TODO: read current state of the machine
	//Is it possible the get the exact state? 
	//Direction (Inspiration/Expiration), Position
	//duration of the break (maybe ms resolution RTC)?
	//maybe also external RAM with backup battery? Or FRAM?
		
	/*Uncomment next line to enable LCD */
	/* LCD_Init(); */
	LED_Init();
	ADC_Init();
	UART0_Init();
	Systime_Init();
	motor_Init();
	MeasureInit();
	PID_Init(Settings.PID_P,Settings.PID_I,Settings.PID_D,&PIDdata);
	
	sei();
	
	/* Uncomment next line for parser testing*/
	/* test_parser(); */
	int timeout=0;
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
				case MODE_STOP:  Control.mode = CTRL_PAR_MODE_STOP; break;
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
			ActuatorControl(&Control,&Measured,&Settings,&PIDdata);
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
			if (timeout > 0 )
			{
				timeout --;
			}
			else
			{
				LED2_Off();
			}
		}
		//Listen for commands
		if(UART0_DataReady())	//process received data 1 byte per loop
		{
			UART0_GetByte(&com_data);
			ProcessMessages(com_data, &Settings, &newSettingsReceived);
		}
		if (newSettingsReceived)
		{
			newSettingsReceived = 0;
			length=ReportAllCurrentSettings(msg,50,&Settings);
			if (length > 0)
			{
				LED2_On();
				timeout=20;
				UART0_SendBytes(msg,length);
			}
		}
	}
}

