/*
 * ActuatorControl.c
 *
 * Created: 4. 04. 2020 10:14:58
 *  Author: maticpi
 */ 
#include "ActuatorControl.h"

int32_t FIR(int16_t new_x);

void ActuatorControl(CtrlParams_t* Control, MeasuredParams_t* Measured, RespSettings_t *Settings, pidData_t *PIDdata)
{
	int16_t motorSpeed;
	//static int16_t lastDC;
	int16_t newDC;
	#define MAX_DC_CHANGE	5
	//TODO: Test for errors due to variable length!!!
	Control->cur_position = motor_GetPosition();
	//TODO: determine appropriate multiplier for speed. Current time difference is 5 ms 
	Control->cur_speed = Control->cur_position - Control->last_position;
	
	switch(Control->mode)
	{
		case CTRL_PAR_MODE_STOP:
			motor_SetDutyCycle(0);
			PID_Reset_Integrator(PIDdata);
			break;
		case CTRL_PAR_MODE_HOLD_MAX_CLOSED_POSITION:
			motor_SetDutyCycle(MOTOR_MIN_DC);
			break;
		
		case CTRL_PAR_MODE_TARGET_SPEED:
			if (Control->target_speed > 0)
			{
				if (Control->cur_position < CTRL_PAR_MAX_POSITION)	//Obey if within limits
				{
					motor_SetDir(MOTOR_DIR_VDIH);
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
					motor_SetDir(MOTOR_DIR_IZDIH);
					motor_SetDutyCycle(-Control->target_speed);	//if and negation is probably faster than abs()
				}
				else
				{
					motor_SetDutyCycle(0);	//Stop if too far
				}
			}
			break;

		
		case CTRL_PAR_MODE_TARGET_POSITION_INHALE:

			newDC=0;
			if (Control->target_position - Control->cur_position >= 0)
			{
				if (Control->target_position - Control->cur_position > 2) newDC=MOTOR_MAX_DC;
				else
				{
					newDC = MOTOR_MAX_DC/2;
					//Control->mode=CTRL_PAR_MODE_STOP;
				}
			}
			//if ((newDC-lastDC) > MAX_DC_CHANGE) {newDC = lastDC + MAX_DC_CHANGE;}
			//else if ((newDC-lastDC) < -MAX_DC_CHANGE) {newDC = lastDC-MAX_DC_CHANGE;}
			newDC=FIR(newDC);
			if (newDC > 0)
			{
				motor_SetDir(MOTOR_DIR_VDIH);
				motor_SetDutyCycle(newDC);
			}
			
		break;
		
		case CTRL_PAR_MODE_TARGET_POSITION:
			newDC=0;
			if (Control->target_position - Control->cur_position >= 0)
			{
				if (Control->target_position - Control->cur_position > 2) newDC=MOTOR_MAX_DC/2;
				else
				{
					Control->mode=CTRL_PAR_MODE_STOP;
				}
			}
			if (Control->target_position - Control->cur_position < 0)
			{
				if (Control->target_position - Control->cur_position < -2) newDC=-MOTOR_MAX_DC/4;
				else
				{
					Control->mode=CTRL_PAR_MODE_STOP;
					motor_SetDutyCycle(0);
				}
			}
			//if ((newDC-lastDC) > MAX_DC_CHANGE) {newDC = lastDC + MAX_DC_CHANGE;}
			//else if ((newDC-lastDC) < -MAX_DC_CHANGE) {newDC = lastDC-MAX_DC_CHANGE;}
			newDC=FIR(newDC);
			if (newDC > 0)
			{
				motor_SetDir(MOTOR_DIR_VDIH);
				motor_SetDutyCycle(newDC);
			}
			else
			{
				motor_SetDir(MOTOR_DIR_IZDIH);
				motor_SetDutyCycle(-newDC);
			}
		break;

		case CTRL_PAR_MODE_DUMMY_REGULATE_PRESSURE_PID_RESET:
			PID_Init(Settings->PID_P,Settings->PID_I,Settings->PID_D,PIDdata);
			Control->mode=CTRL_PAR_MODE_DUMMY_REGULATE_PRESSURE;
			//DO NOT PUT BREAK HERE!
		case CTRL_PAR_MODE_DUMMY_REGULATE_PRESSURE:
			//can only regulate inspiration		//pressure span 50mmH2O --> cca 14500 (14500/16 = cca 900)
			motorSpeed = PID_Calculate(Control->target_pressure/16, Measured->pressure/16, PIDdata);
			if (Control->target_speed > 0)
			{
				if (Control->cur_position < CTRL_PAR_MAX_POSITION)	//Obey if within limits
				{
					motor_SetDir(MOTOR_DIR_VDIH);
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
					motor_SetDir(MOTOR_DIR_IZDIH);
					motor_SetDutyCycle(-Control->target_speed);	//if and negation is probably faster than abs()
				}
				else
				{
					motor_SetDutyCycle(0);	//Stop if too far
				}
			}
			break;
		
		case CTRL_PAR_MODE_REGULATE_PRESSURE_PID_RESET:
			PID_Init(Settings->PID_P,Settings->PID_I,Settings->PID_D,PIDdata);
			Control->mode=CTRL_PAR_MODE_REGULATE_PRESSURE;
			//DO NOT PUT BREAK HERE!
		case CTRL_PAR_MODE_REGULATE_PRESSURE:
			//can only regulate inspiration		//pressure span 50mmH2O --> cca 14500 (14500/16 = cca 900)
			motorSpeed = PID_Calculate(Control->target_pressure/16, Measured->pressure/16, PIDdata);
			//motorSpeed = FIR(motorSpeed);
			motor_SetDir(MOTOR_DIR_VDIH);
			
			if (Control->cur_position >= CTRL_PAR_MAX_POSITION)
			{
//				Control->mode=CTRL_PAR_MODE_STOP;
				motorSpeed = MOTOR_MIN_DC;
			}
			else
			{
				if (motorSpeed<MOTOR_MIN_DC) motorSpeed = MOTOR_MIN_DC;					
			}
			motor_SetDutyCycle(motorSpeed);	
		break;

		
		case CTRL_PAR_MODE_REGULATE_VOLUME_PID_RESET:
			PID_Init(Settings->PID_P,Settings->PID_I,Settings->PID_D,PIDdata);
			Control->mode=CTRL_PAR_MODE_REGULATE_VOLUME;
		case CTRL_PAR_MODE_REGULATE_VOLUME:
			//can only regulate inspiration		//volume span 1000, measured span 10000
			motorSpeed = PID_Calculate(Control->target_volume, Measured->volume_t/10, PIDdata);
			motor_SetDir(MOTOR_DIR_VDIH);
			if (Control->cur_position >= CTRL_PAR_MAX_POSITION)
			{
				//				Control->mode=CTRL_PAR_MODE_STOP;
				motorSpeed = MOTOR_MIN_DC;
			}
			else
			{
				if (motorSpeed<MOTOR_MIN_DC) motorSpeed = MOTOR_MIN_DC;
			}
			motor_SetDutyCycle(motorSpeed);
		break;

		case CTRL_PAR_MODE_REGULATE_FLOW_PID_RESET:
			PID_Init(Settings->PID_P,Settings->PID_I,Settings->PID_D,PIDdata);
			Control->mode=CTRL_PAR_MODE_REGULATE_FLOW;
		case CTRL_PAR_MODE_REGULATE_FLOW:
			//can only regulate inspiration		//flow span 150 l/min --> *6 to use approximately the same PID params
			motorSpeed = PID_Calculate(Control->target_flow*6, Measured->flow*6, PIDdata);	//flow range +-150 l/min
			motor_SetDir(MOTOR_DIR_VDIH);
			if (Control->cur_position >= CTRL_PAR_MAX_POSITION)
			{
				//				Control->mode=CTRL_PAR_MODE_STOP;
				motorSpeed = MOTOR_MIN_DC;
			}
			else
			{
				if (motorSpeed<MOTOR_MIN_DC) motorSpeed = MOTOR_MIN_DC;
			}
			motor_SetDutyCycle(motorSpeed);
		break;

		default: //Error: Stop immediately
		ReportError(ActuatorCtrlUnknownMode,NULL/*"Unknown actuator control mode"*/);
		Control->mode=CTRL_PAR_MODE_TARGET_POSITION;
		motor_SetDutyCycle(0);
		break;
	}
	Control->last_position = Control->cur_position;
}

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