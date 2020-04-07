/*
 * CommProtocol.h
 *
 * Created: 3. 04. 2020 17:51:10
 *  Author: maticpi
 */ 


#ifndef COMMPROTOCOL_H_
#define COMMPROTOCOL_H_

#include "CommonDefinitions.h"
#include "ErrorHandling.h"

int PrepareStatusMessage(uint32_t timestamp, int16_t Flow, int16_t Pressure, \
		int16_t Volume, int16_t MotorPosition, \
		int16_t MotorCurrent, int16_t MotorDutyCycle, char *p_msg);


#endif /* COMMPROTOCOL_H_ */