/*
 * modeVCV.h
 *
 * Created: 3. 04. 2020 17:09:50
 *  Author: maticpi
 */ 


#ifndef MODEVCV_H_
#define MODEVCV_H_

#include "CommonDefinitions.h"
#include "motor.h"
#include "ADC.h"

void modeVCV(uint16_t Flow, uint16_t Pressure, uint16_t Volume, RespSettings_t* Settings);

#endif /* MODEVCV_H_ */