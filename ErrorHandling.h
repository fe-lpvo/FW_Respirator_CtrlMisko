/*
 * ErrorHandling.h
 *
 * Created: 3. 04. 2020 18:32:05
 *  Author: maticpi
 */ 


#ifndef ERRORHANDLING_H_
#define ERRORHANDLING_H_

#include <inttypes.h>

#define USE_DEFAULT_ERROR_REPORTER	1
//Default Error reporter just stores errors in an Error Queue
//Copy "void ReportError(ErrCodes_t ErrCode, char ErrMsg[])"
//function and make your own hardware/application specific version


//Error codes
typedef enum ErrCodes
{
	NoError = 0x00,
	ErrQueueEmpty = 0x01,
	ComRxBuffOverrun = 0x10,
	ComRxUnknownState = 0x11,
	ComRxUnknownParameter = 0x12,
	ComRxNoSpaceAfterParam = 0x13,
	ComRxExpectingNumber = 0x14,
	ComRxNoEtx = 0x15,
	ComRxUnexpectedStx = 0x16,
	ComRxUnknownMode = 0x20,
	ComRxRampOutsideLimits = 0x21,
	ComRxInhaleOutsideLimits = 0x22,
	ComRxExhaleOutsideLimits = 0x23,
	ComRxVolumeOutsideLimits = 0x24
} ErrCodes_t;

#define ERROR_QUEUE_LENGTH	50
#define ERROR_QUEUE_STOP_WHEN_FULL	1

typedef struct ERR_QUEUE{
	ErrCodes_t ErrorQueue[ERROR_QUEUE_LENGTH]; //Array with ERROR_QUEUE_LENGTH stored errors
	uint8_t QueueIn;						//Index for writing into buffer
	uint8_t QueueOut;						//Index for reading from buffer
	uint8_t QueueNum;						//Number of stored data in buffer
} ErrQueue_t;

ErrQueue_t DefaultErrorQueue;

void ErrQueue_Init(struct ERR_QUEUE *ErrQueue);
void ErrQueue_StoreErr (ErrCodes_t ErrCode, struct ERR_QUEUE *ErrQueue);	//No point in returning error - who to? This is the error handling code.
ErrCodes_t ErrQueue_GetErr (ErrCodes_t* ErrCode, struct ERR_QUEUE *ErrQueue);
uint8_t ErrQueue_NumberOfErrors();

#if USE_DEFAULT_ERROR_REPORTER
void ReportError(ErrCodes_t ErrCode, char ErrMsg[]);
#endif

#endif /* ERRORHANDLING_H_ */