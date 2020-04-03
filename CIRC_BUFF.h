/*
 * CIRC_BUFF.h
 *
 * Created: 31.3.2020 20:55:55
 *  Author: Kristjan
 */ 


#ifndef CIRC_BUFF_H_
#define CIRC_BUFF_H_

#include <avr/io.h>

#define BUFFER_LENGTH 16
#define BUF_OK 1
#define BUF_ERR 0

typedef struct CIRC_BUF_DATA{
	uint16_t CircBufferData[BUFFER_LENGTH]; //Array with BUFFER_LENGTH stored data
	uint16_t CircBufferAvg;					//Average value
	uint16_t BufferIn;						//Index for writing into buffer
	uint16_t BufferOut;						//Index for reading from buffer
	uint16_t BufferNum;						//Number of stored data in buffer
	uint16_t BufferMaxLen;					// Variable holding buffer length
} circBufData_t;

void CircBuf_Init(struct CIRC_BUF_DATA *circBuf);
char CircBuf_StoreData (uint16_t data, struct CIRC_BUF_DATA *circBuf);
char CircBuf_GetData (uint16_t* data, struct CIRC_BUF_DATA *circBuf);
char CircBuf_Cycle(uint16_t data, struct CIRC_BUF_DATA *circBuf);


#endif /* CIRC_BUFF_H_ */