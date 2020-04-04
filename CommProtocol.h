/*
 * CommProtocol.h
 *
 * Created: 3. 04. 2020 17:51:10
 *  Author: maticpi
 */ 


#ifndef COMMPROTOCOL_H_
#define COMMPROTOCOL_H_

#include "CommonDefinitions.h"
#include "ErrorHandling.h"
#include "UART0.h"	//No IRQs?
#include "GPIO.h"

int PrepareStatusMessage(uint32_t timestamp, uint16_t Flow, uint16_t Pressure, uint16_t Volume, char *p_msg);


#endif /* COMMPROTOCOL_H_ */