/*
 * Measure.c
 *
 * Created: 6. 04. 2020 07:06:55
 *  Author: maticpi
 */ 
#include "Measure.h"
#include "Lookup.h"

const int16_t flowdata[] PROGMEM = {-3430,-2890,-2368,-1800,-1250,-800,-480,-150,280,825,1395,1990,2490,3000,3440,3887,4345,4790,5220,5550,5950,6300,6700,7050,7400,7770,8078,8377,8670,9000,9300,9600,9900,10162,10488,10900,11300,11700,12100,12500,13000,13400,13800,14200,14627,15055,15448,15789,16058}; 
	// size = 49, step = 250, xMin = 0

lookup_table_t Flow_table;

void MeasureInit()
{
	Lookup_init(&Flow_table,LOCATION_FLASH,250,0,49,(int16_t*)flowdata);
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

