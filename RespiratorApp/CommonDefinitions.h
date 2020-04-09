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
#define MSG_CORE_LENGTH	20
#define TIME_SLICE_MS	2	//Timeslice in ms
#define STATUS_REPORTING_PERIOD	10	// ms

#define MODE_DEFAULT	6	//Kateri je default?
#define MODE_STOP	0
#define MODE_C_VCV	1		
#define MODE_C_PCV	2
#define MODE_AC_VCV	3
#define MODE_AC_PCV	4
#define MODE_CPAP	5
#define MODE_HW_TEST	6

//default settings
#define SETTINGS_DEFAULT_MODE					MODE_DEFAULT
#define SETTINGS_DEFAULT_RAMPUP_TIME_MS			100		// 50ms	???	100ms/TIME_SLICE_MS?
#define SETTINGS_DEFAULT_INHALE_TIME_MS			1250	// 1.25s
#define SETTINGS_DEFAULT_EXHALE_TIME_MS			3000	// 3s
#define SETTINGS_DEFAULT_TARGET_PRESSURE_MBAR	300		// mmH2O
#define SETTINGS_DEFAULT_MAX_PRESSURE_MBAR		400		// mmH2O
#define SETTINGS_DEFAULT_PEEP					50		// mmH2O
#define SETTINGS_DEFAULT_TARGET_VOLUME_ML		400		// milliliters

#define SETTINGS_DEFAULT_PID_P		64	// = P/SCALING_FACTOR = 0.5
#define SETTINGS_DEFAULT_PID_I		1	// = I/SCALING_FACTOR
#define SETTINGS_DEFAULT_PID_D		0	// = D/SCALING_FACTOR
#define	SETTINGS_DEFAULT_MOT_POS	-1	//	-1 = do nothing, 0< go to position

//settings limits
#define SETTINGS_RAMPUP_MIN			0
//#define SETTINGS_RAMPUP_MIN			50
#define SETTINGS_RAMPUP_MAX			200
#define SETTINGS_INHALE_TIME_MIN	100
#define SETTINGS_INHALE_TIME_MAX	2000
#define SETTINGS_EXHALE_TIME_MIN	100
#define SETTINGS_EXHALE_TIME_MAX	10000
#define SETTINGS_VOLUME_MIN			100
#define SETTINGS_VOLUME_MAX			1000
//#define SETTINGS_BREATHING_R_MAX	20
//#define SETTINGS_BREATHING_R_MIN	1
#define SETTINGS_PEEP_MAX			200
#define SETTINGS_PEEP_MIN			0
#define SETTINGS_PRESSURE_MIN		10
#define SETTINGS_PRESSURE_MAX		1000
#define SETTINGS_PID_P_MIN			INT16_MIN
#define SETTINGS_PID_P_MAX			INT16_MAX
#define SETTINGS_PID_I_MIN			INT16_MIN
#define SETTINGS_PID_I_MAX			INT16_MAX
#define SETTINGS_PID_D_MIN			INT16_MIN
#define SETTINGS_PID_D_MAX			INT16_MAX
#define SETTINGS_MOTOR_POSITION_MIN	0
#define SETTINGS_MOTOR_POSITION_MAX	1023

//Settings
typedef struct RESPIRATOR_SETTINGS{
	uint8_t current_mode;
	uint8_t	new_mode;		//If new_mode is different than current mode, do transition
	uint16_t target_Pramp_time;
	uint16_t target_inspiratory_time;
	uint16_t target_expiratory_time;
	uint16_t target_volume;
//	uint8_t breathing_rate;
	uint16_t PEEP;
	uint16_t PeakInspPressure;
	uint16_t target_pressure;
	int16_t PID_P;
	int16_t PID_I;
	int16_t PID_D;
	int16_t MOT_POS;
} RespSettings_t;

//Measured Parameters

typedef enum
{
	VOLUME_STOP,
	VOLUME_INTEGRATE,
	VOLUME_RESET
} MeasureVolumeMode_t;

typedef struct MEASURED_PARAMS{
	int16_t flow;
	int16_t pressure;
	int16_t volume_t;
	MeasureVolumeMode_t volume_mode;
} MeasuredParams_t;


//Control Parameters
#define CTRL_PAR_MODE_STOP							0
#define CTRL_PAR_MODE_TARGET_SPEED					1
#define CTRL_PAR_MODE_TARGET_POSITION_INHALE		2
#define CTRL_PAR_MODE_TARGET_POSITION				3
#define CTRL_PAR_MODE_REGULATE_PRESSURE_PID_RESET	4
#define CTRL_PAR_MODE_REGULATE_PRESSURE				5
#define CTRL_PAR_MODE_REGULATE_VOLUME				6
#define CTRL_PAR_MODE_REGULATE_FLOW					7

#define CTRL_PAR_MAX_POSITION	103
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
	int16_t BreathCounter;	//steje vdihe
	uint8_t status;	//stanje state machina za dihanje
	uint8_t Error;	//napake (bitwise)
} CtrlParams_t;



#endif /* COMMON_DEFINITIONS_H_ */