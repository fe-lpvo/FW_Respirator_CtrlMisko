/*
 * Comands.c
 *
 * Created: 7. 04. 2020 13:44:28
 *  Author: matja
 */ 

#include "Comands.h"

const comand_params_t device_comands[COMANDS_NBR] = {
											{COM_MODE, 'M', PARAM_CHAR, 0, 0},
											{COM_RAMPUP, 'R', PARAM_U8, 50, 20},
											{COM_INSPIRATION_TIME, 'I', PARAM_U16, 10, 5000},
											{COM_EXPIRATION_TIME, 'E', PARAM_U16, 10, 5000},
											{COM_VOLUME, 'V', PARAM_U16, 100, 1000},
											{COM_BREATH_RATE, 'A', PARAM_U8, 1, 20},
											{COM_PEEP, 'P', PARAM_U8, 0, 20},
											{COM_MAX_PRESS, 'T', PARAM_U8, 1, 100}	
									   };