/*
 * Stepper.h
 *
 * Created: 22.3.2020 17:21:38
 *  Author: Kristjan
 */ 


#ifndef STEPPER_H_
#define STEPPER_H_

#include <avr/io.h>
#include "config.h"
#include <util/delay.h>

void Init_Stepper(char stepPin, char dirPin);
void Stepper_Steps(char direction, uint16_t steps, uint16_t delay_us);


#endif /* STEPPER_H_ */