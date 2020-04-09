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
	int16_t flow, flow_negative;
	uint16_t *ADC_Results;
	
	ADC_Results=ADC_results_p();

	//TODO: according to calibration adjust scaling
	//flow = *(ADC_Results+ADC_CH_FLOW) - FLOW_MIN;
	flow		  = Lookup(*(ADC_Results+ADC_CH_FLOW  ),&Flow_table);
	flow_negative = Lookup(*(ADC_Results+ADC_CH_FLOW_N),&Flow_table);
	if (flow < 0) flow = -flow_negative;
	if (flow<FLOW_ZERO_TRESHOLD && flow>-FLOW_ZERO_TRESHOLD) flow = 0;
	Measured->flow=flow;
}

void MeasurePressure(MeasuredParams_t* Measured)
{
	int16_t pressure;
	uint16_t *ADC_Results;
	
	ADC_Results=ADC_results_p();
	
	//TODO: adjust scaling
	pressure = *(ADC_Results+ADC_CH_PRESSURE) - PRESSURE_MIN;
	if (pressure<PRESSURE_ZERO_TRESHOLD && pressure>-PRESSURE_ZERO_TRESHOLD) pressure = 0;
	
	Measured->pressure=pressure;
}

void MeasureVolume(MeasuredParams_t* Measured)
{
	static int32_t volume;
	//int volume_check=motor_GetPosition();
	int relative_vol_dif __attribute__((unused));
	
	switch (Measured->volume_mode)
	{
		case VOLUME_INTEGRATE:
			volume += (int32_t)Measured->flow * TIME_SLICE_MS;
		break;
		
		case VOLUME_RESET:
			volume = 0;
		break;
		
		default://do nothing
		break;
	}
	
	if (volume<0) volume=0;
	
	//TODO: compare volume to motor position and Report error if something seams fishy
	// integral
	
	
	//relative_vol_dif = (volume*100L)/volume_check;
	//if (relative_vol_dif > 120)	ReportError();
	//if (relative_vol_dif < 80) ReportError();
	
	// flow is in 0.01 l/min, but volume should be in cm3. [0.1 cm3/ms] = 10*1000/(60*1000*100) [0.01 l/min]
	// result will be in 0.1 cm3 = 0.1 ml
	
	Measured->volume_t=volume/600;
}

