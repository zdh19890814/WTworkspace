#ifndef __BDS_H
#define __BDS_H
#include "include.h"
#define timenum_touchuan 50
#define timenum_map 50
#define uchar unsigned char
#define uint  unsigned int

#define SEND_BUF_SIZE 640	
//#define REV_NUM       10

#define BDdata_SIZE  30
extern u8 BDS_TouchuanFlag;
extern char BDdata[BDdata_SIZE];
extern int BDtimeflag;
extern int BD_datanum;

uchar GetComma(uchar num,char *str);

void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);//����DMA1_CHx
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);//ʹ��DMA1_CHx
void uart_init(u32 bound);//���ڡ�DMA��ʼ��

u8 BDS_SET_Pattern(u8 mode);//����ģʽ
void BDS_GET_NumOfStl(void);//��ȡ������
void BDS_GET_Speed(void);//��ȡ�ٶ���Ϣ 
void BDS_GET_Date(void);//��ȡ������Ϣ 
void BDS_GET_Time(void);//��ȡʱ����Ϣ
void BDS_GET_Location(void);//��ȡ����γ����Ϣ

u8 BDS_touchuan_open(void);
void BDS_touchuan_close(void);
void BDS_map_open(void);
void BDS_map_close(void);
u8 BD_ReceiveData(void);
void Tim1_Init(u16 period_num);

u8 BDS_GET(u8 type);//��ȡ��Ϣ
void BDS_ComSendWithEnd(u8 * str);

void USART3_DMA_Init(void);//����3&DMA��ʼ��
void BDS_Handle(void);

void BDS_GET_Height(void);
void BDS_GET_Direction(void);
void BD_UartInit(void);
void UART5_BD_Init(void);
#endif

