/*
 * CIRC_BUFF.c
 *
 * Created: 31.3.2020 20:55:45
 *  Author: Kristjan
 */ 
#include "CIRC_BUFF.h"

void CircBuf_Init(struct CIRC_BUF_DATA *circBuf)
{
	circBuf->BufferIn = 0;
	circBuf->BufferOut = 0;
	circBuf->BufferNum = 0;
	circBuf->CircBufferAvg = 0;
	circBuf->BufferMaxLen = BUFFER_LENGTH;
}


char CircBuf_StoreData (uint16_t data, struct CIRC_BUF_DATA *circBuf)
{
	circBuf->CircBufferData[circBuf->BufferIn] = data;
	circBuf->BufferNum++;
	circBuf->BufferIn++;
	if (circBuf->BufferIn >= circBuf->BufferMaxLen) circBuf->BufferIn = 0;
	return BUF_OK;
}


char CircBuf_GetData (uint16_t* data, struct CIRC_BUF_DATA *circBuf)
{
    if(circBuf->BufferNum <= 0) return BUF_ERR;

	*data = circBuf->CircBufferData[circBuf->BufferOut];
	circBuf->BufferNum--;
	circBuf->BufferOut++;
	
	if (circBuf->BufferOut >= circBuf->BufferMaxLen) circBuf->BufferOut = 0;
	return BUF_OK;
}



char CircBuf_Cycle(uint16_t data, struct CIRC_BUF_DATA *circBuf)
{
	uint16_t valueToRemove;
	circBuf->CircBufferAvg += data;
	
	if(circBuf->BufferNum >= BUFFER_LENGTH)
	{
		CircBuf_GetData (&valueToRemove, circBuf);
		circBuf->CircBufferAvg -= valueToRemove;
	}
	CircBuf_StoreData (data, circBuf);
	return BUF_OK;
	
}

