/*
 * CFile1.c
 *
 * Created: 2. 04. 2020 09:27:40
 *  Author: mjankovec
 */ 
//Opisi mode, da vemo kaj pocnemo

#include "modeC_VCV.h"
#include "../Measure.h"
//TODO: Add ability to softly transition to other modes.

//Pri lovljenju ustreznega volumna ima prednost ÈAS (Target inspiratory time)

void modeC_VCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
{
	static int8_t dihanje_state = -1;
	static uint16_t timing;
	
	static uint16_t insp_time;
	static uint16_t exp_time;
	static uint16_t pressure;
	static uint16_t volume;
	static uint16_t MAXpressure;
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
			MAXpressure = Settings->PeakInspPressure;
			volume = Settings->target_volume;
			dihanje_state++;
		break;
		case 0: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
			Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
			Control->target_position  = 0;
			timing=0;
			dihanje_state++;
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
			//Reload all settings, if needed change mode, etc.
			if (Settings->new_mode != MODE_C_VCV)
			{
				Settings->current_mode = Settings->new_mode;
				dihanje_state = -1;
				break;
			}
			insp_time = Settings->target_inspiratory_time;
			exp_time = Settings->target_expiratory_time;
			pramp_time = Settings->target_Pramp_time;
			MAXpressure = Settings->PeakInspPressure;
			volume = Settings->target_volume;
			//PID values reload every time control mode changes
		
			//start cycle
			Measured->volume_mode = VOLUME_RESET;
			Control->BreathCounter++;
			Control->mode=CTRL_PAR_MODE_REGULATE_VOLUME_PID_RESET;
			Control->target_volume = 0;
			timing=0;
			dihanje_state++;
			//comment-out the next 2 lines when P-ramp is finished
			//Control->target_volume = flow;
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
			if (Measured->volume_t > volume*10 - innertia_offset*10)
			{
				dihanje_state++;
			}
			//Errors:
			if (Control->cur_position >= CTRL_PAR_MAX_POSITION)	//Came too far - wait in this position until insp
			{
				dihanje_state++;
			}
		break;

		case 6: //(Only in case of Error in previous state) motor je prisel do konca, pocakaj, da mine cas vdiha
			timing += TIME_SLICE_MS;
			Control->mode = CTRL_PAR_MODE_STOP;
			if (timing > insp_time)
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

}
/*
void modeC_VCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
{
	static uint8_t	pass_number = 0;
	static int8_t dihanje_state = -1;
	static uint16_t timing;
	
	static uint16_t insp_time;
	static uint16_t exp_time;
	static uint16_t pressure;
	static uint16_t volume;
	static uint16_t MAXpressure;
	static uint16_t pramp_time;
	int16_t innertia_offset;
	
	static uint16_t pramp_volume;
	static uint16_t final_volume;
	uint16_t required_flow;
	uint16_t resistance;
	uint16_t new_pressure;
	uint16_t avg_flow;

	//required_flow=(volume-pramp_volume)/(insp_t-Pramp_t)
	//avg_flow = achieved_volume/time
	//resistance = pressure / avg_flow
	//new_pressure = resistance * required_flow

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
		volume = Settings->target_volume;
		pass_number = 0;
		dihanje_state++;
		break;
		case 0: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
		Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
		Control->target_position  = 0;
		timing=0;
		dihanje_state++;
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
		//Reload all settings, if needed change mode, etc.
		if (Settings->new_mode != MODE_C_VCV)
		{
			Settings->current_mode = Settings->new_mode;
			dihanje_state = -1;
			break;
		}
		insp_time = Settings->target_inspiratory_time;
		exp_time = Settings->target_expiratory_time;
		pramp_time = Settings->target_Pramp_time;
		pressure = Settings->target_pressure;
		MAXpressure = Settings->PeakInspPressure;
		volume = Settings->target_volume;
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
			if (pass_number == 1)
			{
				required_flow=(volume-pramp_volume)/(insp_time-pramp_time);
				avg_flow = final_volume-pramp_volume/(insp_time-pramp_time);
				resistance = pressure / avg_flow;
				new_pressure = resistance * required_flow;
				Control->target_pressure = new_pressure;
			}
			dihanje_state=0;
		}
		//Alternate condition - max volume reached. Should probably issue a warning
		innertia_offset = ((int32_t)Measured->flow*3)/4;	//ml
		if (Measured->volume_t > volume*10 - innertia_offset*10)
		{
			dihanje_state++;
		}
		//Errors:
		if (Control->cur_position >= CTRL_PAR_MAX_POSITION)	//Came too far - wait in this position until insp
		{
			dihanje_state++;
		}
		break;

		case 6: //(Only in case of Error in previous state) motor je prisel do konca, pocakaj, da mine cas vdiha
		timing += TIME_SLICE_MS;
		Control->mode = CTRL_PAR_MODE_STOP;
		if (timing > insp_time)
		{
			dihanje_state=0;
		}
		break;
		
		default:
		//ReportError(ModeC_VCV_UnknownState,NULL);
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}

}
*/