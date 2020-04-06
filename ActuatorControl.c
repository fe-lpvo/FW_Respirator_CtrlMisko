/*
 * ActuatorControl.c
 *
 * Created: 4. 04. 2020 10:14:58
 *  Author: maticpi
 */ 
#include "ActuatorControl.h"

void ActuatorControl(CtrlParams_t* Control, MeasuredParams_t* Measured, pidData_t *PIDdata)
{
	int16_t motorSpeed;

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
					MeasureVolume(Measured,1);
				}
			}
			break;
		}
		case CTRL_PAR_MODE_TARGET_POSITION:{
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
				MeasureVolume(Measured,1);
			}
			break;
		}
		case CTRL_PAR_MODE_REGULATE_PRESSURE:{
			motorSpeed = PID_Calculate(Control->target_pressure, Measured->pressure, PIDdata);
			
			if (motorSpeed == 0)
			{
				motor_SetDutyCycle(0);
			}
			else if (motorSpeed > 0)
			{
				if (Control->cur_position >= CTRL_PAR_MAX_POSITION)
				{
					motor_SetDutyCycle(0);
					Control->mode=CTRL_PAR_MODE_STOP;
				}
				else
				{
					motor_SetDirVdih();
					//Is there a need to transform linear motorSpeed to something unlinear for the motor?
					motor_SetDutyCycle(motorSpeed);
				}
			}
			else if (motorSpeed < 0)
			{
				if (Control->cur_position <= 0)
				{
					motor_SetDutyCycle(0);
					Control->mode=CTRL_PAR_MODE_STOP;
					MeasureVolume(Measured,1);
				}
				else
				{
					motor_SetDirIzdih();
					motor_SetDutyCycle(motorSpeed);
				}
			}
			break;
		}
		case CTRL_PAR_MODE_REGULATE_VOLUME:{
			motorSpeed = PID_Calculate(Control->target_volume, Measured->volume_t, PIDdata);
			
			if (motorSpeed == 0)
			{
				motor_SetDutyCycle(0);
			}
			else if (motorSpeed > 0)
			{
				if (Control->cur_position >= CTRL_PAR_MAX_POSITION)
				{
					Control->mode=CTRL_PAR_MODE_STOP;
					motor_SetDutyCycle(0);
				}
				else
				{
					motor_SetDirVdih();
					//Is there a need to transform linear motorSpeed to something unlinear for the motor?
					motor_SetDutyCycle(motorSpeed);
				}
			}
			else if (motorSpeed < 0)
			{
				if (Control->cur_position <= 0)
				{
					Control->mode=CTRL_PAR_MODE_STOP;
					motor_SetDutyCycle(0);
					MeasureVolume(Measured,1);
				}
				else
				{
					motor_SetDirIzdih();
					motor_SetDutyCycle(motorSpeed);
				}
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
