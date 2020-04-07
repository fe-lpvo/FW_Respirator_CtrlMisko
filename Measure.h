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
void MeasureVolume(MeasuredParams_t* Measured);

#define PRESSURE_MIN	1632	//=16*102
#define PRESSURE_MAX	14688	//=16*918
#define PRESSURE_SPAN	(PRESSURE_MAX - PRESSURE_MIN)
#define FLOW_MIN		1632
#define FLOW_MAX		14688	
#define FLOW_SPAN		(FLOW_MAX-FLOW_MIN)

#endif /* MEASURE_H_ */