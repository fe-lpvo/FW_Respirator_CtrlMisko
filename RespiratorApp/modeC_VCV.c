/*
 * CFile1.c
 *
 * Created: 2. 04. 2020 09:27:40
 *  Author: mjankovec
 */ 
//Opisi mode, da vemo kaj pocnemo

#include "modeC_VCV.h"

//TODO: Add ability to softly transition to other modes.

void modeC_VCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
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
					
		case 2: //zacni ramp-up
		Control->mode=CTRL_PAR_MODE_TARGET_SPEED;
		Control->target_speed = 1023;
		InspiratoryTiming = 0;
		dihanje_state++;
		break;
					
		case 3: //ramp-up
		InspiratoryTiming+= TIME_SLICE_MS;
		//TODO: Tu kaj reguliramo? V konèni fazi bo verjetno treba
		//...
		if (InspiratoryTiming >= Settings->target_Pramp_time)	// gremo v constant pressure
		{
			//InspiratoryTiming = 0; //Smatram da nastavitev za inspiratory timing vkljuèuje P_ramp time
			dihanje_state++;
		}
		break;
					
		case 4: //constant pressure
		InspiratoryTiming+=TIME_SLICE_MS;
		//TODO: regulate pressure, adjust target pressure from cycle to cycle to achieve desired volume
		
		//Detect end condition of inspiratory cycle
		if ( (Measured->volume_t >= Settings->target_volume) || 
			 (InspiratoryTiming > Settings->target_inspiratory_time))
		{
			ExpiratoryTiming = 0;
			dihanje_state++;
			Control->mode=CTRL_PAR_MODE_TARGET_SPEED;
			Control->target_speed = -300;
		}
		if (Control->mode == CTRL_PAR_MODE_STOP)	//Error ! Motor reached max position befor inspiration completed
		{
			ReportError(Limits_InsufficientVolume,NULL/*"Error! Target Volume could not be reached"*/);
			ExpiratoryTiming = 0;
			dihanje_state++;
			Control->mode=CTRL_PAR_MODE_TARGET_SPEED;
			Control->target_speed = -300;
		}

		break;
					
		case 5: //izdih
		ExpiratoryTiming+=TIME_SLICE_MS;
		if (InspiratoryTiming > Settings->target_expiratory_time)	// izdih
		{
			dihanje_state=0;
//			Control->mode = CTRL_PAR_MODE_STOP;	//should not be needed
		}
		break;

		default:
		ReportError(ModeC_VCV_UnknownState,NULL/*"Error: Unknown state in C_VCV state machine"*/);
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}

}