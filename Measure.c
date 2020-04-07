/*
 * Measure.c
 *
 * Created: 6. 04. 2020 07:06:55
 *  Author: maticpi
 */ 
#include "Measure.h"

void MeasureFlow(MeasuredParams_t* Measured)
{
	int16_t flow;
	uint16_t *ADC_Results;
	
	ADC_Results=ADC_results_p();

	//TODO: according to calibration adjust scaling
	flow = *(ADC_Results+ADC_CH_FLOW) - FLOW_MIN;
	Measured->flow=flow;
}

void MeasurePressure(MeasuredParams_t* Measured)
{
	int16_t pressure;
	uint16_t *ADC_Results;
	
	ADC_Results=ADC_results_p();
	
	//TODO: adjust scaling
	pressure = *(ADC_Results+ADC_CH_PRESSURE) - PRESSURE_MIN;
	
	Measured->pressure=pressure;
}

void MeasureVolume(MeasuredParams_t* Measured)
{
	static int32_t volume;
	static int reset=0;
	int volume_check=motor_GetPosition();
	int relative_vol_dif __attribute__((unused));
	
	//TODO: compare volume to motor position and Report error if something seams fishy
	volume += (int32_t)Measured->flow * TIME_SLICE_MS;
	
	relative_vol_dif = (volume*100L)/volume_check;
	//if (relative_vol_dif > 120)	ReportError();
	//if (relative_vol_dif < 80) ReportError();
	
	if (Measured->flow < - FLOW_MIN/2) reset=1;
	if (reset)
	{
		volume = 0;
		if (Measured->flow > 0) reset=0;
	}
	Measured->volume_t=volume/1024;
}

