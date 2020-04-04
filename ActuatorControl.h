/*
 * ActuatorControl.h
 *
 * Created: 4. 04. 2020 10:15:43
 *  Author: maticpi
 */ 


#ifndef ACTUATORCONTROL_H_
#define ACTUATORCONTROL_H_

#include <inttypes.h>
#include "CommonDefinitions.h"
#include "ErrorHandling.h"
#include "motor.h"


void ActuatorControl(CtrlParams_t* Control);

#endif /* ACTUATORCONTROL_H_ */