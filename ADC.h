/*
 * ADC.h
 *
 * Created: 30.3.2020 20:04:50
 *  Author: 
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

// ******************************
// Uporabniske nastavitve
// ******************************

#define ADC_STARTCHAN	0
#define ADC_STOPCHAN	4
#define ADC_FILTER_N	16
//#define ADC_DOUBLE_BUFFERING

#define ADC_CH_FLOW			ADC_STARTCHAN
#define ADC_CH_PRESSURE		(ADC_STARTCHAN+1)
#define ADC_CH_MOTOR_CURRENT	(ADC_STARTCHAN+2)
#define ADC_CH_POSITION		(ADC_STARTCHAN+3)
#define ADC_CH_FLOW_N		(ADC_STARTCHAN+4)
//#define ADC_CH_FLOWINT		(ADC_STARTCHAN+5)	//virtual channel for volume - not yet used
// ******************************
// Konec Uporabniskih nastavitev
// ******************************

void ADC_Init();
void ADC_Select_Channel(char channel);
void ADC_Start_Conversion();
void ADC_Start_First_Conversion();
char ADC_scan_complete();
uint16_t *ADC_results_p(void);

#endif /* ADC_H_ */