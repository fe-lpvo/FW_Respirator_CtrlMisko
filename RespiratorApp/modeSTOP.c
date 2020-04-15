/*
 * modeSTOP.c
 *
 * Created: 10. 04. 2020 07:15:32
 *  Author: maticpi
 */ 

#include "modeSTOP.h"

void modeSTOP(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control)
{
	Control->status = 0;	// shrani stanje dihanja

	Control->mode = CTRL_PAR_MODE_STOP;
	Settings->current_mode = Settings->new_mode;
}