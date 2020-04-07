/*
 * ActuatorControl.c
 *
 * Created: 4. 04. 2020 10:14:58
 *  Author: maticpi
 */ 
#include "ActuatorControl.h"

int32_t FIR(int16_t new_x)
{
	const int32_t b[]={2, 6, 15, 30, 54, 87, 131, 186, 253, 332, 422, 521, 629, 743, 860, 978, 1093, 1202, 1303, 1393, 1468, 1526, 1566, 1586};
	#define FILTER_LENGTH (sizeof(b)/sizeof(b[0])*2)
	static int32_t x[FILTER_LENGTH];
	static int index=0;
	int i;
	int32_t y;
	
	x[index]=new_x;
	y=0;
	for (i=0; i<FILTER_LENGTH/2; i++)
	{
		y+=b[i]*x[(index+FILTER_LENGTH-i)%FILTER_LENGTH] + b[i]*x[(index+1+i)%FILTER_LENGTH];
	}
	index++;
	if (index >= FILTER_LENGTH) index = 0;
	
	return (y>>15);
}

void ActuatorControl(CtrlParams_t* Control, MeasuredParams_t* Measured, pidData_t *PIDdata)
{
	int16_t motorSpeed;
	static int16_t lastDC;
	int16_t newDC;
	#define MAX_DC_CHANGE	5
	//TODO: Test for errors due to variable length!!!
	Control->cur_position = ((int32_t)motor_GetPosition() * 1024)/MOTOR_POS_CLOSED;
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
			if (Control->target_position - Control->cur_position >= 0)
			{
				if (Control->target_position - Control->cur_position > 2) newDC=MAX_DC;
				else
				{
					Control->mode=CTRL_PAR_MODE_STOP;
					motor_SetDutyCycle(0);
					MeasureVolume(Measured,1);
				}
			}
			else //if (Control->target_position - Control->cur_position < 0)
			{
				if (Control->target_position - Control->cur_position < -2) newDC=-MAX_DC/4;
				else
				{
					Control->mode=CTRL_PAR_MODE_STOP;
					motor_SetDutyCycle(0);
					MeasureVolume(Measured,1);
				}
			}
			if ((newDC-lastDC) > MAX_DC_CHANGE) {newDC = lastDC + MAX_DC_CHANGE;}
			else if ((newDC-lastDC) < -MAX_DC_CHANGE) {newDC = lastDC-MAX_DC_CHANGE;}
			if (newDC > 0)
			{
				motor_SetDirVdih();
				motor_SetDutyCycle(newDC);
			}
			else
			{
				motor_SetDirIzdih();
				motor_SetDutyCycle(-newDC);
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
	lastDC = newDC;
	Control->last_position = Control->cur_position;
}
