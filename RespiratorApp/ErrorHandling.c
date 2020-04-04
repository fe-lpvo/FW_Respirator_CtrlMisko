/*
 * ErrorHandling.c
 *
 * Created: 3. 04. 2020 18:31:24
 *  Author: maticpi
 */ 

#include "ErrorHandling.h"

void ErrQueue_Init(struct ERR_QUEUE *ErrQueue)
{
	ErrQueue->QueueIn = 0;
	ErrQueue->QueueOut = 0;
	ErrQueue->QueueNum = 0;
}

void ErrQueue_StoreErr (ErrCodes_t ErrCode, struct ERR_QUEUE *ErrQueue)
{
#if ERROR_QUEUE_STOP_WHEN_FULL
	if (ErrQueue->QueueNum < ERROR_QUEUE_LENGTH)
#endif
	{
		ErrQueue->ErrorQueue[ErrQueue->QueueIn] = ErrCode;
		ErrQueue->QueueNum++;
		ErrQueue->QueueIn++;
		if (ErrQueue->QueueIn >= ERROR_QUEUE_LENGTH) ErrQueue->QueueIn = 0;
	}
}

ErrCodes_t ErrQueue_GetErr (ErrCodes_t* ErrCode, struct ERR_QUEUE *ErrQueue)
{
	if(ErrQueue->QueueNum <= 0) return ErrQueueEmpty;

	*ErrCode = ErrQueue->ErrorQueue[ErrQueue->QueueOut];
	ErrQueue->QueueNum--;
	ErrQueue->QueueOut++;
	
	if (ErrQueue->QueueOut >= ERROR_QUEUE_LENGTH) ErrQueue->QueueOut = 0;
	return NoError;
}

uint8_t ErrQueue_NumberOfErrors(struct ERR_QUEUE *ErrQueue)
{
	return ErrQueue->QueueNum;
}

#if USE_DEFAULT_ERROR_REPORTER
void ReportError(ErrCodes_t ErrCode, char ErrMsg[])
{
	ErrQueue_StoreErr(ErrCode,&DefaultErrorQueue);
	//printf("Error %d: %s,ErrCode,ErrMsg);
}
#endif