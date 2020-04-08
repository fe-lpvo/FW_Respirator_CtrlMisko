/*
 * Comands.h
 *
 * Created: 7. 04. 2020 13:44:45
 *  Author: matja
 */ 

#include <stdint.h>

#define COMANDS_NBR 7

typedef enum {
				COM_MODE,
				COM_RAMPUP,
				COM_INSPIRATION_TIME,
				COM_EXPIRATION_TIME,
				COM_VOLUME,
				COM_BREATH_RATE,
				COM_PEEP,
				COM_MAX_PRESS
			}comand_list_t;
			
typedef enum {
			 DEV_SET_P,
			 DEV_SET_I,
			 DEV_SET_D,
			 DEV_SET_MOTOR
			}dev_comand_list_t;
			 
typedef enum{
			PARAM_CHAR,
			PARAM_U8,
			PARAM_S8,
			PARAM_U16,
			PARAM_S16,
			PARAM_U32,
			PARAM_S32
			}param_type_t;			 

			
			
			
typedef struct comand_params {
								comand_list_t comand;
								char comand_char;
								param_type_t param_type;
								uint32_t param_min;
								uint32_t param_max;
							}comand_params_t;