/*
 * CFile1.c
 *
 * Created: 2. 04. 2020 09:27:40
 *  Author: mjankovec
 */ 
//Opisi mode, da vemo kaj pocnemo

#include "modeVCV.h"

//Verjetno bo treba postopno zagnat vsak mode in ga tudi ugasnit
//mogoèe bolj smiselno, da so to deli state machine?
/*
void modeVCVstart()
{

}

void modeVCVstop()
{

}*/

void modeVCV(uint16_t Flow, uint16_t Pressure, uint16_t Volume, RespSettings_t* Settings)
{
	static uint8_t dihanje_state = 0;
	static uint16_t timer_10ms;

	switch (dihanje_state)
	{
		case 0: // za?etek vdiha, preveri, ce so klesce narazen, sicer jih daj narazen
		motor_SetDirIzdih();
		motor_SetDutyCycle(200);
		dihanje_state++;
		break;
					
		case 1: // cakaj, da so klesce narazen
		if (motor_GetPosition()<MOTOR_POS_MIN)
		{
			motor_SetDutyCycle(0);
			dihanje_state++;
		}
		break;
					
		case 2: //zacni ramp-up
		motor_SetDirVdih();
		motor_SetDutyCycle(1023);
		dihanje_state++;
		timer_10ms = 0;
		break;
					
		case 3: //ramp-up
		timer_10ms++;
		if (timer_10ms >= Settings->rampup)	// gremo v constant pressure
		{
			timer_10ms = 0;
			dihanje_state++;
			motor_SetDutyCycle(1023);
		}
		break;
					
		case 4: //constant pressure
		timer_10ms++;
		if (motor_GetPosition()>=Settings->volume_t || timer_10ms > Settings->vdih_t) // izdih
		{
			timer_10ms = 0;
			dihanje_state++;
			motor_SetDutyCycle(300);
			motor_SetDirIzdih();
		}
		break;
					
		case 5: //izdih
		timer_10ms++;
		if (motor_GetPosition()<MOTOR_POS_MIN) motor_SetDutyCycle(0);	//konec izdiha
		if (timer_10ms > Settings->izdih_t)	// izdih
		{
			dihanje_state=0;
			motor_SetDutyCycle(0);
		}
		break;
					
		default:
		motor_SetDutyCycle(0);
		break;
	}

}