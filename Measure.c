/*
 * Measure.c
 *
 * Created: 6. 04. 2020 07:06:55
 *  Author: maticpi
 */ 
#include "Measure.h"
#include "Lookup.h"

const uint16_t flowdata[] PROGMEM = {0, 550, 1153, 1733, 2291, 2827, 3343, 3840, 4318, 4778, 5222, 5649,
	6062, 6461, 6846, 7220, 7582, 7934, 8277, 8611, 8937, 9257, 9571, 9881,
	10186, 10489, 10790, 11089, 11388, 11688, 11990, 12294, 12602, 12914, 13231, 13555,
13886, 14225, 14573, 14931, 15300}; // size = 41, step = 10, xMin = 100

lookup_table_t Flow_table;

void MeasureInit()
{
//	Lookup_init(&Flow_table,LOCATION_FLASH,100,100,41,flowdata);
	Lookup_init(&Flow_table,LOCATION_FLASH,424,163,41,(uint16_t*)flowdata);
}

void MeasureFlow(MeasuredParams_t* Measured)
{
	int16_t flow;
	uint16_t *ADC_Results;
	
	ADC_Results=ADC_results_p();

	//TODO: according to calibration adjust scaling
	//flow = *(ADC_Results+ADC_CH_FLOW) - FLOW_MIN;
	flow = Lookup(*(ADC_Results+ADC_CH_FLOW),&Flow_table);
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

