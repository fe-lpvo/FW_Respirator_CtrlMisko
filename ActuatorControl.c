/*
 * ActuatorControl.c
 *
 * Created: 4. 04. 2020 10:14:58
 *  Author: maticpi
 */ 
#include "ActuatorControl.h"

void ActuatorControl(CtrlParams_t* Control)
{
	//TODO: Test for errors due to variable length!!!
	Control->cur_position = (((int32_t)motor_GetPosition() - MOTOR_POS_MIN) * 1024)/(MOTOR_POS_MAX - MOTOR_POS_MIN);
	//TODO: determine appropriate multiplier for speed. Current time difference is 5 ms 
	Control->cur_speed = Control->cur_position - Control->last_position;
	
	switch(Control->mode)
	{
		case CTRL_PAR_MODE_STOP:{
			motor_SetDutyCycle(0);
			break;
		}
		case CTRL_PAR_MODE_TARGET_SPEED:{
			if (Control->target_speed > 0)
			{
				if (Control->cur_position < CTRL_PAR_MAX_POSITION)	//Obey if within limits
				{
					motor_SetDirVdih();
					motor_SetDutyCycle(Control->target_speed);
				}
				else
				{
					motor_SetDutyCycle(0);	//Stop if too far
				}
			}
			else
			{
				if (Control->cur_position > CTRL_PAR_MIN_POSITION)	//Obey if within limits
				{
					motor_SetDirIzdih();
					motor_SetDutyCycle(-Control->target_speed);	//if and negation is probably faster than abs()
				}
				else
				{
					motor_SetDutyCycle(0);	//Stop if too far
				}
			}
			break;
		}
		case CTRL_PAR_MODE_TARGET_POSITION:{
			//Recalculate target control position to absolute motor position units
			if (Control->target_position - Control->cur_position > 20)
			{
				motor_SetDirVdih();
				motor_SetDutyCycle(1023);
			}
			else if (Control->target_position - Control->cur_position > 2)	//A bit of dead zone ?
			{
				motor_SetDirVdih();
				motor_SetDutyCycle(200);
			}
			else if (Control->target_position - Control->cur_position < -20)
			{
				motor_SetDirIzdih();
				motor_SetDutyCycle(1023);
			}
			else if (Control->target_position - Control->cur_position < -2)
			{
				motor_SetDirIzdih();
				motor_SetDutyCycle(200);
			}
			else
			{
				Control->mode=CTRL_PAR_MODE_STOP;
				motor_SetDutyCycle(0);
			}
			break;
		}
		case CTRL_PAR_MODE_REGULATE_PRESSURE:{
			//Recalculate target control position to absolute motor position units
			if (Control->target_position - Control->cur_position > 20)
			{
				motor_SetDirVdih();
				motor_SetDutyCycle(1023);
			}
			else if (Control->target_position - Control->cur_position > 2)	//A bit of dead zone ?
			{
				motor_SetDirVdih();
				motor_SetDutyCycle(200);
			}
			else if (Control->target_position - Control->cur_position < -20)
			{
				motor_SetDirIzdih();
				motor_SetDutyCycle(1023);
			}
			else if (Control->target_position - Control->cur_position < -2)
			{
				motor_SetDirIzdih();
				motor_SetDutyCycle(200);
			}
			else
			{
				Control->mode=CTRL_PAR_MODE_STOP;
				motor_SetDutyCycle(0);
			}
			break;
		}
		default: //Error: Stop immediately
		ReportError(ActuatorCtrlUnknownMode,NULL/*"Unknown actuator control mode"*/);
		Control->mode=CTRL_PAR_MODE_TARGET_POSITION;
		motor_SetDutyCycle(0);
		break;
	}
	Control->last_position = Control->cur_position;
}
