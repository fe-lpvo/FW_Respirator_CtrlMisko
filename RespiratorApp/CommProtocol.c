/*
 * CommProtocol.c
 *
 * Created: 3. 04. 2020 17:50:54
 *  Author: maticpi
 */ 
#include "CommProtocol.h"

int PrepareStatusMessage(uint32_t timestamp, int16_t Flow, int16_t Pressure, int16_t Volume, int16_t MotorPosition, char *p_msg)
{
	//STX+N+TIMESTAMP+4xADC+ETX
	*p_msg = 0x55;
	p_msg++;
				
	*p_msg = MSG_CORE_LENGTH;
	p_msg++;
				
	*(uint32_t *)p_msg = timestamp;
	p_msg +=4;
				
	*(int16_t *)p_msg = Flow;
	p_msg +=2;
				
	*(int16_t *)p_msg = Pressure;
	p_msg +=2;
				
	*(int16_t *)p_msg = Volume;
	p_msg +=2;

	*(int16_t *)p_msg = MotorPosition;
	p_msg +=2;
				
	*(p_msg) = 0xAA;

	return (1+1+MSG_CORE_LENGTH+1);
}

//Protocol definition
//STX PARAM space VALUE ETX
//STX = '>'
//ETX = '\n'
//PARAM:
//	'M' = mode ('0'-stop, 'V'-VCV, 'P'-PCV, 'C'-CPAP)
//	'R' = rampup (50 - 200) ms
//  'I' = inspiratory time (???) ms
//	'E' = expiratory time (???) ms
//	'V' = volume (100-1000) ml
//
//Example:
//>M V\n
void ProcessMessages(char data, RespSettings_t* Settings)
{
	static uint8_t state;
	static char param;
	static int32_t value;
	
	switch (state)
	{
		case 0:	{//Waiting for STX
			if (data == '>') state++;
			break;
		}
		case 1: {//Waiting for PARAM
			value=0;
			switch (data){
				case 'M':
				case 'R':
				case 'I':
				case 'E':
				case 'V':	//known parameter
				{
					param = data;
					state++;
					break;
				}
				default:{
					ReportError(ComRxUnknownParameter,NULL/*"Received unknown parameter"*/);
					param=0;
					state=0;
					break;
				}
			}
			break;
		}
		case 2: {//Waiting for space
			if (data == ' ') state++;
			else{
				ReportError(ComRxNoSpaceAfterParam,NULL/*"Space missing after parameter"*/);
				state=0;
			}
			break;
		}
		case 3: //waiting for value and ETX
		////////////////////////////////////////////////
		// Parameter value parsing /////////////////////
		{
			if (param == 'M')
			{
				switch (data)
				{
					case '0': value = MODE_STOP; state++; break;
					case 'V': value = MODE_C_VCV; state++; break;
					case 'P': value = MODE_C_PCV; state++; break;
					case 'C': value = MODE_CPAP; state++; break;
					default:
					ReportError(ComRxUnknownMode,NULL/*"Unknown mode received"*/);
					state = 0;
					break;
				}
				break;
			}
			else	//Parameters with ASCII numerical value:
			{		// R, I, E, V
				if (data >= '0' && data <= '9'){ value=value*10+data-'0'; break;}
				else if (data == '\n') {state++;}	// !DO NOT BREAK HERE AS THE LAST CHAR IS ETX!
				else {
					ReportError(ComRxExpectingNumber,NULL/*"Expecting numerical value, received something else"*/);
					state = 0;
					break;
				}
			}
		}	// !DO NOT BREAK HERE AS THE LAST CHAR may be ETX! Every case already called brake if it was needed.
		// End of Parameter value parsing //////////////
		////////////////////////////////////////////////
		case 4:	{//Wait for ETX
			if (data == '\n')	//Yes! ETX received, communication finished, validate param value range
			{
				switch (param){
					case 'M': Settings->new_mode = value; break;
					case 'R': if ((value >= SETTINGS_RAMPUP_MIN) && (value <= SETTINGS_RAMPUP_MAX)) Settings->P_ramp=value;
							  else ReportError(ComRxRampOutsideLimits,NULL/*"Received rampup value outside limits"*/);
							  break;
					case 'I': if ((value >= SETTINGS_INHALE_TIME_MIN) && (value <= SETTINGS_INHALE_TIME_MAX)) Settings->inspiratory_t=value;
								else ReportError(ComRxRampOutsideLimits,NULL/*"Received rampup value outside limits"*/);
								break;
					case 'E': if ((value >= SETTINGS_EXHALE_TIME_MIN) && (value <= SETTINGS_EXHALE_TIME_MAX)) Settings->expiratory_t=value;
								else ReportError(ComRxRampOutsideLimits,NULL/*"Received rampup value outside limits"*/);
								break;
					case 'V': if ((value >= SETTINGS_VOLUME_MIN) && (value <= SETTINGS_VOLUME_MAX)) Settings->volume_t=value;
								else ReportError(ComRxRampOutsideLimits,NULL/*"Received rampup value outside limits"*/);
								break;
				}
			}
			else
			{
				ReportError(ComRxNoEtx,NULL/*"The last character was not ETX"*/);
			}
			state = 0;
			break;
		}
		default:
			ReportError(ComRxUnknownState,NULL/*"Receiver state machine: unknown state"*/);
			state=0;
			break;
	}
	
	if (data == '>' && state != 1)
	{
		//if STX character received in the middle of a packet, immediately jump to the appropriate state
		state=1;
		ReportError(ComRxUnexpectedStx,NULL/*"An STX character was received in the middle of a message - restarting state machine"*/);
	}
}