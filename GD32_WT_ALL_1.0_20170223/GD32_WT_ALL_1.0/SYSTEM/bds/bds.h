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

void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);//配置DMA1_CHx
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);//使能DMA1_CHx
void uart_init(u32 bound);//串口、DMA初始化

u8 BDS_SET_Pattern(u8 mode);//设置模式
void BDS_GET_NumOfStl(void);//获取卫星数
void BDS_GET_Speed(void);//获取速度信息 
void BDS_GET_Date(void);//获取日期信息 
void BDS_GET_Time(void);//获取时间信息
void BDS_GET_Location(void);//获取经、纬度信息

u8 BDS_touchuan_open(void);
void BDS_touchuan_close(void);
void BDS_map_open(void);
void BDS_map_close(void);
u8 BD_ReceiveData(void);
void Tim1_Init(u16 period_num);

u8 BDS_GET(u8 type);//获取信息
void BDS_ComSendWithEnd(u8 * str);

void USART3_DMA_Init(void);//串口3&DMA初始化
void BDS_Handle(void);

void BDS_GET_Height(void);
void BDS_GET_Direction(void);
void BD_UartInit(void);
void UART5_BD_Init(void);
#endif

