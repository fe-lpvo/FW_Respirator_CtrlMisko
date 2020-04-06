/*
 * Common.h
 *
 * Created: 3. 04. 2020 17:25:53
 *  Author: maticpi
 */ 


#ifndef COMMON_DEFINITIONS_H_
#define COMMON_DEFINITIONS_H_

#include <inttypes.h>
#include <stddef.h>

//app defines
#define MSG_CORE_LENGTH	10
#define TIME_SLICE_MS	5	//Timeslice in ms

#define MODE_DEFAULT	1	//Kateri je default?
#define MODE_STOP	0
#define MODE_C_VCV	1		
#define MODE_C_PCV	2
#define MODE_AC_VCV	3
#define MODE_AC_PCV	4
#define MODE_CPAP	5

//default settings
#define SETTINGS_DEFAULT_MODE				MODE_DEFAULT
#define SETTINGS_DEFAULT_RAMPUP_TIME_MS		100		// 50ms	???	100ms/TIME_SLICE_MS?
#define SETTINGS_DEFAULT_INHALE_TIME_MS		1250	// 1.25s
#define SETTINGS_DEFAULT_EXHALE_TIME_MS		3000	//3s
#define SETTINGS_DEFAULT_VOLUME_ML			250		//500 mililiters?

//settings limits
#define SETTINGS_RAMPUP_MIN			50
#define SETTINGS_RAMPUP_MAX			200
#define SETTINGS_INHALE_TIME_MIN	100
#define SETTINGS_INHALE_TIME_MAX	2000
#define SETTINGS_EXHALE_TIME_MIN	100
#define SETTINGS_EXHALE_TIME_MAX	10000
#define SETTINGS_VOLUME_MIN			100
#define SETTINGS_VOLUME_MAX			1000

//Settings
typedef struct RESPIRATOR_SETTINGS{
	uint8_t current_mode;
	uint8_t	new_mode;		//If new_mode is different than current mode, do transition
	uint8_t P_ramp;
	uint16_t inspiratory_t;
	uint16_t expiratory_t;
	uint16_t volume_t;
	uint16_t PeakInspPressure;
} RespSettings_t;

//measurement factors
#define VOLUME_ADJ_FACTOR	TIME_SLICE_MS/1024

//Measured Parameters
typedef struct MEASURED_PARAMS{
	int16_t flow;
	int16_t pressure;
	int16_t volume_t;
} MeasuredParams_t;


//Control Parameters
#define CTRL_PAR_MODE_STOP				0
#define CTRL_PAR_MODE_TARGET_SPEED		1
#define CTRL_PAR_MODE_TARGET_POSITION	2
#define CTRL_PAR_MODE_REGULATE_PRESSURE	3
#define CTRL_PAR_MODE_REGULATE_VOLUME	4

#define CTRL_PAR_MAX_POSITION	1023
#define CTRL_PAR_MIN_POSITION	0

typedef struct CONTROL_PARAMS{
	uint8_t mode;		//regulate speed/position
	int16_t	target_speed;		//max: +-1023, positive value = ispiration
	int16_t target_position;	//max: +-1023, 0 = completely exhaled, theoretically should not go below 0
	int16_t cur_speed;
	int16_t cur_position;
	int16_t last_position;
	int16_t max_speed;	//0-1023 speed limit for target position mode - not yet used / does it make sense?
	int16_t target_pressure;
	int16_t target_volume;
} CtrlParams_t;



#endif /* COMMON_DEFINITIONS_H_ */