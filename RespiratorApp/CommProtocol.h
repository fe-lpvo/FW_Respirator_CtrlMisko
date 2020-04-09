/*
 * CommProtocol.h
 *
 * Created: 3. 04. 2020 17:51:10
 *  Author: maticpi
 */ 


#ifndef COMMPROTOCOL_H_
#define COMMPROTOCOL_H_

#include <string.h>
#include "CommonDefinitions.h"
#include "ErrorHandling.h"

int PrepareStatusMessage(uint32_t timestamp, int16_t Flow, int16_t Pressure, int16_t Volume, int16_t MotorPosition,\
int16_t MotorCurrent, int16_t MotorDutyCycle, uint16_t BreathCounter, uint8_t Status, uint8_t Error, char *p_msg);
int ReportAllCurrentSettings(char *p_msg, int MAX_LENGTH, RespSettings_t *Settings);

void ProcessMessages(char data, RespSettings_t* Settings, uint8_t* newdata);
#endif /* COMMPROTOCOL_H_ */