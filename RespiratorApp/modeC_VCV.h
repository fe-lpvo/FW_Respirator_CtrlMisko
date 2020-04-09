/*
 * modeVCV.h
 *
 * Created: 3. 04. 2020 17:09:50
 *  Author: maticpi
 */ 


#ifndef MODEVCV_H_
#define MODEVCV_H_

#include "CommonDefinitions.h"
#include "ErrorHandling.h"

#define PRESSURE_INCREMENT	1

void modeC_VCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control);

#endif /* MODEVCV_H_ */