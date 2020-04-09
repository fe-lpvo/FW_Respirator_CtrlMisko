/*
 * modeC_PCV.h
 *
 * Created: 6. 04. 2020 06:59:15
 *  Author: maticpi
 */ 


#ifndef MODEC_PCV_H_
#define MODEC_PCV_H_

#include "CommonDefinitions.h"
#include "ErrorHandling.h"

#define PRESSURE_INCREMENT	1

void modeC_PCV(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control);

#endif /* MODEC_PCV_H_ */