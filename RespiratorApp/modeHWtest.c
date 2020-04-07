/*
 * modeHWtest.c
 *
 * Created: 7. 04. 2020 08:36:55
 *  Author: maticpi
 */ 
#include "modeHWtest.h"
#include "../GPIO.h"
#include "../Measure.h"

void modeHWtest(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
{
#if 0
	static uint8_t dihanje_state = 0;
	switch (dihanje_state)
	{
		case 0: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
		Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
		Control->target_position  = 0;
		dihanje_state++;
		break;
		
		case 1: // cakaj, da so klesce narazen
		if (Control->mode == CTRL_PAR_MODE_STOP)
		{
			dihanje_state++;
		}
		break;
		
		case 2: //poslji jih na sredino
		Control->mode=CTRL_PAR_MODE_TARGET_POSITION;
		Control->target_position = 1023;
		dihanje_state++;
		break;
		
		case 3: //cakaj klesce 
		//Detect max motor position - Insufficient volume/time Error
		if (Control->mode == CTRL_PAR_MODE_STOP)
		{
			dihanje_state=0;
		}
		break;

		default:
		//ReportError(ModeC_VCV_UnknownState,NULL/*"Error: Unknown state in C_VCV state machine"*/);
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}
#else
	static uint8_t dihanje_state = 0;
	static uint32_t timing;
	switch (dihanje_state)
	{
		case 0: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
		Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
		Control->target_position  = 0;
		dihanje_state++;
		break;
		
		case 1: // cakaj, da so klesce narazen
		if (Control->mode == CTRL_PAR_MODE_STOP)
		{
			dihanje_state++;
		}
		break;
		
		case 2: //reguliraj pritisk 10s
		Control->mode=CTRL_PAR_MODE_REGULATE_PRESSURE;
		Control->target_pressure = PRESSURE_MAX/10*3;
		timing=0;
		dihanje_state++;
		break;
		
		case 3: //cakaj da mine 10s ali da se motor varnostno izklopi
		timing += TIME_SLICE_MS;
		if (Control->mode == CTRL_PAR_MODE_STOP)
		{
			LED4_On();
			dihanje_state=0;
		}
		if (timing > 10000) 
		{
			LED4_Off();
			dihanje_state=0;
		}
		break;

		default:
		//ReportError(ModeC_VCV_UnknownState,NULL/*"Error: Unknown state in C_VCV state machine"*/);
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}
#endif
}