#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 




#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))  	

#define MAX_UART_DEBUG_BUFF_SIZE 300
#define MAX_REC_BUFF_SIZE 500
extern u16  BuffIndex;
extern u8 RecBuff_1[MAX_REC_BUFF_SIZE + 1];
extern u8 RecBuff_2[MAX_REC_BUFF_SIZE + 1];	


extern u8 flag;
extern u16 UartDebug_Cnt;

extern u8 UartDebug_R_Buf[MAX_UART_DEBUG_BUFF_SIZE];

void TickInit(void);
void USART1_DMA_Init(void);
void USART2_DMA_Init(void);
void NVIC_AllInit(void);


#define TTL_COM_RX_FLAG0    0   //added by TTL on 2015.10.28 for fixing the bug of com rx
#define TTL_COM_RX_FLAG1    1   //added by TTL on 2015.10.28 for fixing the bug of com rx
#define TTL_COM_RX_FLAG2    2   //added by TTL on 2015.10.28 for fixing the bug of com rx
#define TTL_COM_RX_FLAG3    3   //added by TTL on 2015.10.28 for fixing the bug of com rx


//#ifdef DEBUG_PRINT
//#define DebugOut printf
//#else
//#define DebugOut 
//#endif




#ifdef DEBUG_PRINT
#define DebugOut printf
#else
#define DebugOut printf_no
#endif 


void printf_no(char* fmt, ...);

extern void at_buffer_handler(u8 *pAtcStrBuff, u16 usAtcStrLen);
char Mux_buffer_handler(u8 *pBuffer, u16 usBufferLen);




#define MAX_AT_COM_BUF_NUM 20
//#define MAX_AT_MSM_BUF_NUM 10
#define MAX_AT_COM_BUF_SIZE 600
//#define MAX_AT_MSM_BUF_SIZE 300
extern u8 ComBuf[MAX_AT_COM_BUF_NUM][MAX_AT_COM_BUF_SIZE];
//extern u8 MsmBuf[MAX_AT_MSM_BUF_NUM][MAX_AT_MSM_BUF_SIZE];

extern u8 readBufFirst_TTL[MAX_AT_COM_BUF_SIZE];
//extern char readBufSecond_TTL[MAX_AT_MSM_BUF_SIZE];



#define MAX_AT_MUX_BUF_NUM 10
#define MAX_AT_MUX_BUF_SIZE 1600 //MUX×î³¤
extern u8 AT_MuxBuff[MAX_AT_MUX_BUF_NUM][MAX_AT_MUX_BUF_SIZE + 2];
extern int MuxBufIndexStart;
extern int MuxBufIndexEnd;



//extern int MsmBufIndexStart;
//extern int MsmBufIndexEnd;
extern int ComBufIndexStart;
extern int ComBufIndexEnd;
extern char readBuf_Flag;
//extern char readSms_Flag;
extern int lenAtRspFirst;
//extern int lenAtRspSecond;

extern u8 VoiceCallFlag;

#define MAX_AT_RESPONSE_BUF_NUM 10
extern u8 ResponseBuff[MAX_AT_RESPONSE_BUF_NUM][MAX_AT_COM_BUF_SIZE];
extern int ResponseBufIndex;



u8 AT_SendDirect(u8 * str);
u8 AT_SendData(u8 *pData, u16 len);

void USART3SendStrWithEnd(u8 * str);
void USART5SendStrWithEnd(u8 * str);
u8 USART2SendStrWithEnd(u8 * str);
u8 USART2SendData(u8 *pData, u16 len);

u8 USART2SendDataWithEnd(u8 *pData, u16 len);
u8 AT_SendDataWithEnd(u8 *pData, u16 len);

u8 strStartsWith(u8 *line, const u8 *prefix);
u8 isSMSUnsolicited(u8 *line);
void DebugPrintData(u8 *pData, u16 len);
#define ComBufIndexPlusPlus(x)\
{\
	x++;\
	if (x >= MAX_AT_COM_BUF_NUM)\
	{\
		x = 0;\
	}\
}


#define ComBufIndexEndPlusN(x)\
{\
	ComBufIndexEnd += x;\
	if (ComBufIndexEnd >= MAX_AT_COM_BUF_NUM)\
	{\
		ComBufIndexEnd -= MAX_AT_COM_BUF_NUM;\
	}\
}






//#define MsmBufIndexPlusPlus(x)\
{\
	x++;\
	if (x >= MAX_AT_MSM_BUF_NUM)\
	{\
		x = 0;\
	}\
}


#define MuxBufIndexPlusPlus(x)\
{\
	x++;\
	if (x >= MAX_AT_MUX_BUF_NUM)\
	{\
		x = 0;\
	}\
}




#define Uart2_MAX_REC_BUFF_SIZE 300

extern u8 Uart2_RecBuff[Uart2_MAX_REC_BUFF_SIZE + 1];//¡ä??¨²2DMA¦Ì??¨®¨º??o¡ä?
extern char Uart2_MsmBuf[Uart2_MAX_REC_BUFF_SIZE + 1];
void AT_MuxBuffHandler(u8 *ComBuff,u16 len);
void USART2_DMA_Init(void);

void USART1_Init(void);
void UART4_Init(void);
void DebugUartInit(void);







#endif
