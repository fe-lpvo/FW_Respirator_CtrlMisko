/*
 * parser_test.c
 *
 * Created: 9. 04. 2020 01:09:17
 *  Author: matja
 */ 

#include "parser_test.h"

void test_parser (void)
{
	RespSettings_t set = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	char rx_char;
	uint8_t done = 0, n;
		
	while(1)
	{
		if(UART0_DataReady() != 0)
		{
			UART0_GetByte(&rx_char);
			printf("%c", rx_char);
			ProcessMessages(rx_char, &set, &done);
			if(done != 0)
			{
				printf("MODE: %u\n\rP_RAMP: %u\n\rINSP_T: %u\n\rEXP_T: %u\n\rVOL: %u\n\rRATE: %u\n\rPEEP: %u\n\rPRESS: %u\n\r", set.new_mode, set.P_ramp, set.inspiratory_t, set.expiratory_t, set.volume_t, set.breathing_rate, set.PEEP, set.PeakInspPressure);
				done = 0;
			}
		}
		
	}
}