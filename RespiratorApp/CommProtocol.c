/*
 * CommProtocol.c
 *
 * Created: 3. 04. 2020 17:50:54
 *  Author: maticpi
 */ 
#include "CommProtocol.h"

#define ValidateAndApplyReceivedValue(typ, value, setting, min, max, err) \
case typ: if ((value >= min ) && (value <= max ) ) {setting = value;} else {ReportError(err, NULL/*""*/);} break
/*
typedef enum
{
	eT_INT8,
	eT_UINT8,
	eT_INT16,
	eT_UINT16,
	eT_INT32,
	eT_UINT32
} eParamType_t;

void ValidateAndApplyReceivedValue(int32_t value, void* setting, eParamType_t type, int32_t min, int32_t max, ErrCodes_t, err)
{
	if ( (value >= min) && (value <=  max) )
	{
		switch (type)
		{
			case eT_INT8:   *(int8_t*)  setting = value;
			case eT_UINT8:  *(uint8_t*) setting = value;
			case eT_INT16:  *(int16_t*) setting = value;
			case eT_UINT16: *(uint16_t*)setting = value;
			case eT_INT32:  *(int32_t*) setting = value;
			case eT_UINT32: *(uint32_t*)setting = value;
		}
	}
	else 
	{
		ReportError(Err, NULL);
	}
}*/

int PrepareStatusMessage(uint32_t timestamp, int16_t Flow, int16_t Pressure, int16_t Volume, int16_t MotorPosition, \
int16_t MotorCurrent, int16_t MotorDutyCycle, uint16_t BreathCounter, uint8_t Status, uint8_t Error, char *p_msg)
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

	*(int16_t *)p_msg = MotorCurrent;
	p_msg +=2;

	*(int16_t *)p_msg = MotorDutyCycle;
	p_msg +=2;
	
	*(uint16_t *)p_msg = BreathCounter;
	p_msg +=2;
	
	*(uint8_t *)p_msg = Status;
	p_msg +=1;					

	*(uint8_t *)p_msg = Error;
	p_msg +=1;
	
	*(p_msg) = 0xAA;

	return (1+1+MSG_CORE_LENGTH+1);
}

int ReportAllCurrentSettings(char *p_msg, int MAX_LENGTH, RespSettings_t *Settings)
{
	int length = sizeof(RespSettings_t)+3;
	if (MAX_LENGTH >= (sizeof(RespSettings_t)+3) )
	{
		*p_msg = 0x56;
		p_msg++;
	
		*p_msg = length-3;
		p_msg++;

		memcpy(p_msg,Settings,sizeof(RespSettings_t));
		
		p_msg+=sizeof(RespSettings_t);
		*p_msg = 0xAA;
	}
	else
	{
		length = 0;
	}

	return length;
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

//TODO: target pressure, 
//	inhale trigger 
//		-flow
//		-negative pressure
void ProcessMessages(char data, RespSettings_t* Settings, uint8_t* newdata)
{
	static uint8_t state = 0;
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
//				case 'A':	//calculated from I and E
				case 'P':
				case 'T':
				case 'S':
				case '1':
				case '2':
				case '3':
				case '4':
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
					/*case '0': value = MODE_STOP; state++; break;
					case 'V': value = MODE_C_VCV; state++; break;
					case 'P': value = MODE_C_PCV; state++; break;
					case 'C': value = MODE_CPAP; state++; break;
*/					case '0': 
					case '1': 
					case '2': 
					case '3':
					case '4':
					case '5':
					case '6':
						value = data-'0';
						state++;
						break;
					default:
					ReportError(ComRxUnknownMode,NULL/*"Unknown mode received"*/);
					state = 0;
					break;
				}
				break;
			}
			else	//Parameters with ASCII numerical value:
			{		// R, I, E, V, A, P, T, 1, 2, 3, 4
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
					case 'M': Settings->new_mode = value; break;	//Already validated
					//case statements are included in the below macros
ValidateAndApplyReceivedValue('R', value, Settings->target_Pramp_time,		SETTINGS_RAMPUP_MIN, SETTINGS_RAMPUP_MAX, ComRxRampOutsideLimits);
ValidateAndApplyReceivedValue('I', value, Settings->target_inspiratory_time,SETTINGS_INHALE_TIME_MIN, SETTINGS_INHALE_TIME_MAX, ComRxInhaleTmOutsideLimits);
ValidateAndApplyReceivedValue('E', value, Settings->target_expiratory_time,	SETTINGS_EXHALE_TIME_MIN, SETTINGS_EXHALE_TIME_MAX, ComRxExhaleTmOutsideLimits);
ValidateAndApplyReceivedValue('V', value, Settings->target_volume,			SETTINGS_VOLUME_MIN, SETTINGS_VOLUME_MAX, ComRxVolumeOutsideLimits);
//ValidateAndApplyReceivedValue('A', value, Settings->breathing_rate,			SETTINGS_BREATHING_R_MIN, SETTINGS_BREATHING_R_MAX, ComRxBreathingRateOtsideLimits);
ValidateAndApplyReceivedValue('P', value, Settings->PEEP,					SETTINGS_PEEP_MIN, SETTINGS_PEEP_MAX, ComRxPEEPOutsideLimits);
ValidateAndApplyReceivedValue('T', value, Settings->PeakInspPressure,		SETTINGS_PRESSURE_MIN, SETTINGS_PRESSURE_MAX, ComRxMaxPressureOutsideLimits);
ValidateAndApplyReceivedValue('S', value, Settings->target_pressure,		SETTINGS_PRESSURE_MIN, SETTINGS_PRESSURE_MAX, ComRxTargetPressureOutsideLimits);
					
ValidateAndApplyReceivedValue('1', value, Settings->PID_P,	SETTINGS_PID_P_MIN, SETTINGS_PID_P_MAX, ComRxPIDPOutsideLimits);
ValidateAndApplyReceivedValue('2', value, Settings->PID_I,	SETTINGS_PID_I_MIN, SETTINGS_PID_I_MAX, ComRxPIDIOutsideLimits);
ValidateAndApplyReceivedValue('3', value, Settings->PID_D,	SETTINGS_PID_D_MIN, SETTINGS_PID_D_MAX, ComRxPIDDOutsideLimits);
ValidateAndApplyReceivedValue('4', value, Settings->MOT_POS,SETTINGS_MOTOR_POSITION_MIN, SETTINGS_MOTOR_POSITION_MAX, ComRxMOTPOSOutsideLimits);
				}
				state = 0;
				*newdata = 1;
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
