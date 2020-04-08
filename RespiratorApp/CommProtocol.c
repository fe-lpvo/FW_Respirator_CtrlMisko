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
void ProcessMessages(char data, RespSettings_t* Settings, comand_params_t** comands)
{
	static uint8_t state = 0;
	static char param, saved_param;
	static int32_t value, saved_value;
	static char string[15];
	static char str_ptr = 0;
	uint32_t converter;
	uint8_t number;
	uint8_t n;

		
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
				case 'V':	
				case 'A':
				case 'P': //known parameter
				{
					param = data;
					saved_param = data;
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
			if (data == ' ') 
			{
				state++;
			}
			else
			{
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
				if (data >= '0' && data <= '9'){ value=value*10+data-'0'; saved_value = value; break;}
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
					case 'R': if ((value >= comands[COM_RAMPUP]->param_min) && (value <= comands[COM_RAMPUP]->param_min)) Settings->P_ramp=value;
							  else ReportError(ComRxRampOutsideLimits, NULL/*"Received rampup value outside limits"*/);
							  break;
					
					case 'I': if ((value >= comands[COM_INSPIRATION_TIME].param_min) && (value <= comands[COM_INSPIRATION_TIME].param_max)) Settings->inspiratory_t=value;
								else ReportError(ComRxRampOutsideLimits, NULL/*"Received rampup value outside limits"*/);
								break;
					
					case 'E': if ((value >= comands[COM_EXPIRATION_TIME].param_min) && (value <= comands[COM_EXPIRATION_TIME].param_max)) Settings->expiratory_t=value;
								else ReportError(ComRxRampOutsideLimits, NULL/*"Received rampup value outside limits"*/);
								break;
					
					case 'V': if ((value >= comands[COM_VOLUME].param_min) && (value <= comands[COM_VOLUME].param_max)) Settings->volume_t=value;
								else ReportError(ComRxRampOutsideLimits, NULL/*"Received rampup value outside limits"*/);
								break;
								
					case 'A': if ((value >= comands[COM_BREATH_RATE].param_min) && (value <= comands[COM_BREATH_RATE].param_max)) Settings->volume_t=value;
								else ReportError(ComRxBreathingRateOtsideLimits, NULL/*"Received rampup value outside limits"*/);
								break;
					
					case 'P': if ((value >= comands[COM_PEEP].param_min) && (value <= comands[COM_PEEP].param_max)) Settings->volume_t=value;
								else ReportError(ComRxPEEPPutsideLimits, NULL/*"Received rampup value outside limits"*/);
								break;
				}
				//state++;
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
	/*
	case 5: // create writeback string
		string[str_ptr++] = '>'; /* write starting character 
		string[str_ptr++] = saved_param /* write comand type 
		str_ptr[str_ptr++] = ' '; /* Add space 
		if(saved_param == 'M') /* If message was setting new mode
		{
			switch (value)
			{
				case MODE_STOP:
					str_ptr[str_ptr++] = '0';
				case MODE_C_VCV:
					str_ptr[str_ptr++] = 'V';
				case MODE_C_PCV:
					str_ptr[str_ptr++] = 'P';
				case MODE_CPAP:
					str_ptr[str_ptr++] = 'C';
			}
		}
		else /* Convert number to ASCII - the HARD way 
		{
			converter = saved_value;
			n = 1;
			while(converter > 0)
			{
				number = converter % (10000 / n);
				if(number != 0)
				{
					str_ptr[str_ptr++] = number + '0';
				}
				converter /= 10;
				n++;
			}

		}
		*/
		
	
		
	
	
	if (data == '>' && state != 1)
	{
		//if STX character received in the middle of a packet, immediately jump to the appropriate state
		state=1;
		ReportError(ComRxUnexpectedStx,NULL/*"An STX character was received in the middle of a message - restarting state machine"*/);
	}
}

