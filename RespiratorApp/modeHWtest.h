/*
 * modeHWtest.h
 *
 * Created: 7. 04. 2020 08:37:14
 *  Author: maticpi
 */ 


#ifndef MODEHWTEST_H_
#define MODEHWTEST_H_

#include "CommonDefinitions.h"
#include "ErrorHandling.h"

//#define PRESSURE_TARGET	(PRESSURE_SPAN*8/10)
#define PRESSURE_INCREMENT	1

void modeHWtest(RespSettings_t* Settings, MeasuredParams_t* Measured, CtrlParams_t* Control);

#endif /* MODEHWTEST_H_ */