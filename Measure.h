/*
 * Measure.h
 *
 * Created: 6. 04. 2020 07:06:36
 *  Author: maticpi
 */ 


#ifndef MEASURE_H_
#define MEASURE_H_

#include "ADC.h"
#include "RespiratorApp/CommonDefinitions.h"
#include "RespiratorApp/ErrorHandling.h"
#include "motor.h"

void MeasureFlow(MeasuredParams_t* Measured);
void MeasurePressure(MeasuredParams_t* Measured);
void MeasureVolume(MeasuredParams_t* Measured, int8_t reset);

#endif /* MEASURE_H_ */