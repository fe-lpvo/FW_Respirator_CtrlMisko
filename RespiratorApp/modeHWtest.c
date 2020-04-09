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
	/*

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
		//ReportError(ModeC_VCV_UnknownState,NULL"Error: Unknown state in C_VCV state machine"
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}
*/
	static uint8_t dihanje_state = 0;
	static uint16_t timing;
	
	Control->status = dihanje_state;	// shrani stanje dihanja
	
	switch (dihanje_state)
	{
		case 0: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
			Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
			Control->target_position  = 0;
			dihanje_state++;
			LED4_Off();
			
		break;
		
		case 1: // cakaj, da so klesce narazen
			if (Control->mode == CTRL_PAR_MODE_STOP)
			{
				dihanje_state++;
				timing=0;
			}
		break;

		case 2: // cakaj 3s na naslednji vdih
			timing += TIME_SLICE_MS;
			if (timing > EXHALE_TIME)
			{
				dihanje_state++;
			}
		break;
		
		case 3: //zacni vdih
			Measured->volume_mode = VOLUME_RESET;
			Control->BreathCounter++;
			Control->mode=CTRL_PAR_MODE_REGULATE_PRESSURE;
			Control->target_pressure = ((int32_t)Settings->target_pressure * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - Settings->target_inspiratory_time*PRESSURE_INCREMENT;
			timing=0;
			dihanje_state++;
		break;
		
		case 4: //cakaj da mine INHALE_TIME ali da se motor pride do konca
			Measured->volume_mode = VOLUME_INTEGRATE;
			timing += TIME_SLICE_MS;
			Control->target_pressure+=PRESSURE_INCREMENT;
			// ce je prisel do konca, zakljuci cikel vdiha
			if (Control->cur_position >= CTRL_PAR_MAX_POSITION) 
			{
				LED4_On();
				dihanje_state++;
			}
			if (timing > INHALE_TIME) 
			{
				LED4_Off();
				dihanje_state=0;
			}
		break;

		case 5: // motor je prisel do konca, pocakaj, da mine cas vdiha
			timing += TIME_SLICE_MS;
			Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
			Control->target_position  = 0;
			if (timing > INHALE_TIME)
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

}