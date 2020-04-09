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
	static int8_t dihanje_state = -1;
	static uint16_t timing;
	
	static uint16_t insp_time;
	static uint16_t exp_time;
	static uint16_t pressure;
	static uint16_t MAXpressure;
	static uint16_t MAXvolume;
	static uint16_t pramp_time;
	int16_t innertia_offset;
	
	Control->status = dihanje_state;	// shrani stanje dihanja
	
	//State machine starts with exhalation. 
	//At the end of exhalation all the settings are reloaded into local copies (state 3?).
	//Local settings are used during the rest of the cycle, 
	//so that incompatible settings can not be loaded at the wrong time
	
	switch (dihanje_state)
	{
		case -1:	//First time: init local settings, etc
			insp_time = Settings->target_inspiratory_time;
			exp_time = Settings->target_expiratory_time;
			pramp_time = Settings->target_Pramp_time;
			pressure = Settings->target_pressure;
			MAXpressure = Settings->PeakInspPressure;
			MAXvolume = Settings->target_volume;
			dihanje_state++;
			break;
		case 0: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
			Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
			Control->target_position  = 0;
			timing=0;
			dihanje_state++;
			LED4_Off();
			LED3_Off();
		break;
		
		case 1: // cakaj, da so klesce narazen
			timing += TIME_SLICE_MS;
			if (Control->mode == CTRL_PAR_MODE_STOP)
			{
				//reload settings, pid parameters, reset stuff,...
				dihanje_state++;
			}
		break;

		case 2: // cakaj na naslednji vdih
			timing += TIME_SLICE_MS;
			if (timing > exp_time)
			{
				dihanje_state++;
			}
		break;
		
		case 3: //start inspiratory cycle
			//Reload all settings etc.
			insp_time = Settings->target_inspiratory_time;
			exp_time = Settings->target_expiratory_time;
			pramp_time = Settings->target_Pramp_time;
			pressure = Settings->target_pressure;
			MAXpressure = Settings->PeakInspPressure;
			MAXvolume = Settings->target_volume;
			//PID values reload every time control mode changes
			
			//start cycle
			Measured->volume_mode = VOLUME_RESET;
			Control->BreathCounter++;
			Control->mode=CTRL_PAR_MODE_REGULATE_PRESSURE;
			Control->target_pressure = 0;
			timing=0;
			dihanje_state++;
			//comment-out the next 2 lines when P-ramp is finished
			Control->target_pressure = ((int32_t)pressure * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - insp_time*PRESSURE_INCREMENT;
			dihanje_state++;
		break;
		
		case 4: //P-ramp
			timing += TIME_SLICE_MS;
			Control->target_pressure = ((int32_t)MAXpressure*timing)/pramp_time;
			if (timing >= pramp_time)	// gremo v constant pressure
			{
				Control->target_pressure = ((int32_t)pressure * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - insp_time*PRESSURE_INCREMENT;
				dihanje_state++;
			}
		break;
		
		case 5: //cakaj da mine INHALE_TIME ali da motor pride do konca
			Measured->volume_mode = VOLUME_INTEGRATE;
			timing += TIME_SLICE_MS;
			Control->target_pressure+=PRESSURE_INCREMENT;
			// ce je prisel do konca, zakljuci cikel vdiha
			if (timing > insp_time)
			{
				dihanje_state=0;
			}
			//Alternate condition - max volume reached. Should probably issue a warning
			innertia_offset = ((int32_t)Measured->flow*3)/4;	//ml
			if (Measured->volume_t > MAXvolume*10 - innertia_offset*10)
			{
				LED3_On();
				dihanje_state++;
			}
			//Errors:
			if (Control->cur_position >= CTRL_PAR_MAX_POSITION)	//Came too far - wait in this position until insp
			{
				LED4_On();
				dihanje_state++;
			}
		break;

		case 6: //(Only in case of Error in previous state) motor je prisel do konca, pocakaj, da mine cas vdiha
			timing += TIME_SLICE_MS;
			Control->mode = CTRL_PAR_MODE_STOP;
			if (timing > insp_time)
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