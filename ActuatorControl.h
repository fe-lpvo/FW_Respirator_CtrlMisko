/*
 * ActuatorControl.h
 *
 * Created: 4. 04. 2020 10:15:43
 *  Author: maticpi
 */ 


#ifndef ACTUATORCONTROL_H_
#define ACTUATORCONTROL_H_

#include <inttypes.h>
#include "RespiratorApp/CommonDefinitions.h"
#include "RespiratorApp/ErrorHandling.h"
#include "motor.h"
#include "PID.h"
#include "Measure.h"

int32_t FIR(int16_t new_x);
void ActuatorControl(CtrlParams_t* Control, MeasuredParams_t* Measured, RespSettings_t *Settings, pidData_t *PIDdata);

#endif /* ACTUATORCONTROL_H_ */