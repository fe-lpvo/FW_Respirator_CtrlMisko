/*
 * Common.h
 *
 * Created: 3. 04. 2020 17:25:53
 *  Author: maticpi
 */ 


#ifndef COMMON_DEFINITIONS_H_
#define COMMON_DEFINITIONS_H_

#include <avr/io.h>
//#include "config.h"

//app defines
#define MSG_CORE_LENGTH	10
#define TIME_SLICE_MS	5	//Timeslice in ms

#define MODE_DEFAULT	1	//Kateri je default?
#define MODE_STOP	0
#define MODE_VCV	1		
#define MODE_PCV	2
#define MODE_CPAP	3

//default settings
#define SETTINGS_DEFAULT_MODE				MODE_DEFAULT
#define SETTINGS_DEFAULT_RAMPUP_TIME_MS		100/TIME_SLICE_MS	// 50ms	???	100ms/TIME_SLICE_MS?
#define SETTINGS_DEFAULT_INHALE_TIME_MS		1250/TIME_SLICE_MS	// 1.25s
#define SETTINGS_DEFAULT_EXHALE_TIME_MS		3000/TIME_SLICE_MS	//3s
#define SETTINGS_DEFAULT_VOLUME_ML			250					//500 mililiters?

//Settings
typedef struct RESPIRATOR_SETTINGS{
	uint8_t current_mode;
	uint8_t	new_mode;		//If new_mode is different than current mode, do transition
	uint8_t rampup;
	uint16_t vdih_t;
	uint16_t izdih_t;
	uint16_t volume_t;
} RespSettings_t;

//All Error codes




#endif /* COMMON_DEFINITIONS_H_ */