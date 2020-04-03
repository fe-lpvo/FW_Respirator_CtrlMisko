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
#define ADC_STOPCHAN	3
#define ADC_FILTER_N	16

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