/*
 * CFile1.c
 *
 * Created: 2. 04. 2020 09:27:40
 *  Author: mjankovec
 */ 
//Opisi mode, da vemo kaj pocnemo

#include "modeC_PCV.h"

//TODO: Add ability to softly transition to other modes.

void modeC_PCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
{
	static uint8_t dihanje_state = 0;
	static uint16_t InspiratoryTiming;
	static uint16_t ExpiratoryTiming;

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
					
		case 2: //start ramp-up
		Control->mode=CTRL_PAR_MODE_REGULATE_PRESSURE;
		Control->target_pressure = 0;
		InspiratoryTiming = 0;
		dihanje_state++;
		break;
					
		case 3: //ramp-up
		InspiratoryTiming+= TIME_SLICE_MS;
		Control->target_pressure = Settings->PeakInspPressure/Settings->P_ramp*InspiratoryTiming;
		if (InspiratoryTiming >= Settings->P_ramp)	// gremo v constant pressure
		{
			Control->target_pressure = Settings->PeakInspPressure;
			dihanje_state++;
		}
		break;
					
		case 4: //constant pressure
		InspiratoryTiming+=TIME_SLICE_MS;
		//Detect end condition of inspiratory cycle
		if (InspiratoryTiming > Settings->inspiratory_t)
		{
			ExpiratoryTiming = 0;
			dihanje_state++;
			Control->mode=CTRL_PAR_MODE_TARGET_SPEED;
			Control->target_speed = -300;
		}
		//Settings->volume_t should contain MAX permisible volume
		else if (Measured->volume_t >= Settings->volume_t)
		{
			ReportError(Limits_VolumeTooHigh,NULL/*"Error! Max Volume reached before end of cycle"*/);
			ExpiratoryTiming = 0;
			dihanje_state++;
			Control->mode=CTRL_PAR_MODE_TARGET_SPEED;
			Control->target_speed = -300;
		}
		//Detect max motor position - Insufficient volume/time Error
		if (Control->mode == CTRL_PAR_MODE_STOP)	
		{
			ReportError(Limits_InsufficientVolume,NULL/*"Error! Target Volume could not be reached"*/);
			ExpiratoryTiming = 0;
			dihanje_state++;
			Control->mode=CTRL_PAR_MODE_TARGET_SPEED;
			Control->target_speed = -300;
		}

		break;

//TODO: Should there be a pause before expiration???
					
		case 5: //izdih
		ExpiratoryTiming+=TIME_SLICE_MS;
		if (InspiratoryTiming > Settings->expiratory_t)	// izdih
		{
			dihanje_state=0;
//			Control->mode = CTRL_PAR_MODE_STOP;	//should not be needed
		}
		break;

		default:
		ReportError(ModeC_PCV_UnknownState,NULL/*"Error: Unknown state in C_PCV state machine"*/);
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}

}