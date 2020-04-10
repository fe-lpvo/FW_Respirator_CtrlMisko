/*
 * CFile1.c
 *
 * Created: 2. 04. 2020 09:27:40
 *  Author: mjankovec
 */ 
//Opisi mode, da vemo kaj pocnemo

//Pri lovljenju ustreznega volumna ima prednost ÈAS (Target inspiratory time)
#include "modeC_VCV.h"
#include "../Measure.h"
#include "../GPIO.h"

#if 0

#define PRESSURE_CTRL_LOOP_THERSHOLD	50

#define MODE_STATE_FIRST_RUN				-1
#define MODE_STATE_EXP_START				0
#define MODE_STATE_EXP_ZERO_POS_WAIT		1
#define MODE_STATE_EXP_WAIT					2
#define MODE_STATE_INSP_INIT				3
#define MODE_STATE_INSP_PREP				4
#define MODE_STATE_INSP_PRAMP				5
#define MODE_STATE_INSP_CONST_P				6
#define MODE_STATE_INSP_MAX_VOL				7
#define MODE_STATE_INSP_MAX_POS				8
#define MODE_STATE_INSP_MAX_PRESSURE		9

void modeC_VCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
{
	static int8_t dihanje_state = -1;
	static int16_t timing;
	
	#define PRAMP_OFFSET	50
	
	static uint16_t SETinsp_time;
	static uint16_t SETexp_time;
	static uint16_t SETpressure;
	static uint16_t SET_PEEP;
	static uint16_t MAXpressure;
	static uint16_t MAXvolume;
	static uint16_t SETpramp_time;
	int16_t innertia_offset;
	static int16_t PreStartBoostTime;
	int16_t MeasuredPressure = ((int32_t)Measured->pressure * PRESSURE_MAX_MMH2O)/PRESSURE_SPAN;
	
	Control->status = dihanje_state;	// shrani stanje dihanja
	
	//State machine starts with exhalation.
	//At the end of exhalation all the settings are reloaded into local copies (state 3?).
	//Local settings are used during the rest of the cycle,
	//so that incompatible settings can not be loaded at the wrong time

	switch (dihanje_state)
	{
		case MODE_STATE_FIRST_RUN:	//First time: init local settings, etc
		SETinsp_time = Settings->target_inspiratory_time;
		SETexp_time = Settings->target_expiratory_time;
		SETpramp_time = Settings->target_Pramp_time;
		SET_PEEP = Settings->PEEP;
		SETpressure = Settings->target_pressure;
		MAXpressure = Settings->PeakInspPressure;
		MAXvolume = Settings->target_volume;
		dihanje_state=MODE_STATE_EXP_START;
		break;
		case MODE_STATE_EXP_START: // zacetek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
		Control->mode = CTRL_PAR_MODE_TARGET_POSITION;
		Control->target_position  = 0;
		timing=0;
		dihanje_state=MODE_STATE_EXP_ZERO_POS_WAIT;
		LED2_Off();
		LED3_Off();
		LED4_Off();
		break;
		
		case MODE_STATE_EXP_ZERO_POS_WAIT: // cakaj, da so klesce narazen
		timing += TIME_SLICE_MS;
		if (Control->mode == CTRL_PAR_MODE_STOP)
		{
			//reload settings, pid parameters, reset stuff,...
			dihanje_state=MODE_STATE_EXP_WAIT;
		}
		break;

		case MODE_STATE_EXP_WAIT: // cakaj na naslednji vdih
		timing += TIME_SLICE_MS;
		if (timing > SETexp_time)
		{
			dihanje_state=MODE_STATE_INSP_INIT;
		}
		break;
		
		case MODE_STATE_INSP_INIT: //start inspiratory cycle
		//Reload all settings, if needed change mode, etc.
		if (Settings->new_mode != MODE_C_VCV)
		{
			Settings->current_mode = Settings->new_mode;
			dihanje_state = MODE_STATE_FIRST_RUN;
			break;
		}
		SETinsp_time = Settings->target_inspiratory_time;
		SETexp_time = Settings->target_expiratory_time;
		SETpramp_time = Settings->target_Pramp_time;
		SETpressure = Settings->target_pressure;
		MAXpressure = Settings->PeakInspPressure;
		MAXvolume = Settings->target_volume;
		//PID values reload every time control mode changes
		
		//start cycle
		Measured->volume_mode = VOLUME_RESET;
		Control->BreathCounter++;
		Control->mode=CTRL_PAR_MODE_DUMMY_REGULATE_PRESSURE_PID_RESET;
		Control->target_speed = 1000;
		PreStartBoostTime =	23 - SETpramp_time/20;
		timing=-PreStartBoostTime;

		dihanje_state=MODE_STATE_INSP_PREP;
		//comment-out the next 2 lines when P-ramp is finished
		//Control->target_pressure = ((int32_t)pressure * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - insp_time*PRESSURE_INCREMENT;
		//dihanje_state++;
		break;
		
		case MODE_STATE_INSP_PREP: //pre-start (get the motor going)
		Measured->volume_mode = VOLUME_INTEGRATE;
		timing += TIME_SLICE_MS;
		if (timing >= 0)
		{
			Control->mode=CTRL_PAR_MODE_REGULATE_PRESSURE;
			Control->target_pressure = ((int32_t)(SET_PEEP+PRAMP_OFFSET) * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O;
			dihanje_state=MODE_STATE_INSP_PRAMP;
			SETexp_time = SETexp_time - PreStartBoostTime;
			timing=0;
			LED4_On();
		}
		if (MeasuredPressure > SETpressure)
		{
			Control->mode = CTRL_PAR_MODE_REGULATE_PRESSURE;
			Control->target_pressure = ((int32_t)SETpressure * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - SETinsp_time*PRESSURE_INCREMENT;
			LED2_On();
			SETexp_time = SETexp_time - (PreStartBoostTime + timing);
			timing = 0;
			dihanje_state = MODE_STATE_INSP_CONST_P;	//direktno na const. pressure step
		}
		break;
		
		case MODE_STATE_INSP_PRAMP: //P-ramp
		LED1_On();
		timing += TIME_SLICE_MS;
		Control->target_pressure = ((((SETpressure-SET_PEEP-PRAMP_OFFSET)*(int32_t)timing)/SETpramp_time + SET_PEEP+PRAMP_OFFSET) * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - SETinsp_time*PRESSURE_INCREMENT;
		if (timing >= SETpramp_time)	// gremo v constant pressure
		{
			Control->target_pressure = ((int32_t)SETpressure * PRESSURE_SPAN) / PRESSURE_MAX_MMH2O - SETinsp_time*PRESSURE_INCREMENT;
			dihanje_state=MODE_STATE_INSP_CONST_P;
		}
		break;
		
		case MODE_STATE_INSP_CONST_P: //cakaj da mine INHALE_TIME ali da motor pride do konca
		LED1_Off();
		timing += TIME_SLICE_MS;
		Control->target_pressure+=PRESSURE_INCREMENT;
		// ce je prisel do konca, zakljuci cikel vdiha
		if (timing > SETinsp_time)
		{
			dihanje_state=MODE_STATE_EXP_START;
		}
		//Alternate condition - max volume reached. Should probably issue a warning
		innertia_offset = 0;//((int32_t)Measured->flow*3)/4;	//ml
		if (Measured->volume_t > MAXvolume*10 - innertia_offset*10)
		{
			dihanje_state = MODE_STATE_INSP_MAX_VOL;
		}
		if (MeasuredPressure > MAXpressure)
		{
			dihanje_state = MODE_STATE_INSP_MAX_PRESSURE;
		}
		//Errors:
		if (Control->cur_position >= CTRL_PAR_MAX_POSITION)	//Came too far - wait in this position until insp
		{

			dihanje_state = MODE_STATE_INSP_MAX_POS;
		}
		break;

		case MODE_STATE_INSP_MAX_VOL: //(Only in case of Error in previous state) motor je prisel do konca, pocakaj, da mine cas vdiha
		timing += TIME_SLICE_MS;
		Control->mode = CTRL_PAR_MODE_STOP;
		if (timing > SETinsp_time)
		{
			dihanje_state=MODE_STATE_EXP_START;
		}
		break;

		case MODE_STATE_INSP_MAX_PRESSURE: //(Only in case of Error in previous state) motor je prisel do konca, pocakaj, da mine cas vdiha
		timing += TIME_SLICE_MS;
		Control->mode = CTRL_PAR_MODE_STOP;
		if (timing > SETinsp_time)
		{
			dihanje_state=MODE_STATE_EXP_START;
		}
		break;

		case MODE_STATE_INSP_MAX_POS: //(Only in case of Error in previous state) motor je prisel do konca, pocakaj, da mine cas vdiha
		timing += TIME_SLICE_MS;
		Control->mode = CTRL_PAR_MODE_STOP;
		if (timing > SETinsp_time)
		{
			dihanje_state=MODE_STATE_EXP_START;
		}
		break;
		
		default:
		//ReportError(ModeC_VCV_UnknownState,NULL/*"Error: Unknown state in C_VCV state machine"*/);
		Control->mode=CTRL_PAR_MODE_STOP;
		dihanje_state = 0;
		break;
	}
}
#else
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
			if (pass_number == 1)
			{
				required_flow=(volume-pramp_volume)/(insp_time-pramp_time);
				avg_flow = final_volume-pramp_volume/(insp_time-pramp_time);
				resistance = pressure / avg_flow;
				new_pressure = resistance * required_flow;
				Control->target_pressure = new_pressure;
			}
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

#endif
