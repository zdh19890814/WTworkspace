/********************************************************************************
*
*  File Name     : usbio.h
*  Author   	 : 王保礼
*  Create Date   : 2016.06.01
*  Version   	 : 1.0
*  Description   : USB端点读写函数
*  History       : 1. Data:
*                     Author:
*                     Modification:
*
********************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USBIO_H_
#define _USBIO_H_

#include "include.h"
extern u16 EP2_RevNum;
extern u16 EP3_RevNum;
extern u8 EP2_RevFlag;
extern u8 EP1_RevFlag;
extern u8 EP3_RevFlag;
#define TIME_USB_SEND 1000


/* Exported Functions --------------------------------------------------------*/
extern uint32_t USB_SendData(uint8_t bEpNum,uint8_t *data,uint32_t dataNum);
extern uint8_t USB_GetData(uint8_t bEpNum,uint8_t *data,uint32_t dataNum);
u8 WaitEp2InFlag(u16 time_ms);
u8 WaitEp1InFlag(u16 time_ms);
u8 WaitEp3InFlag(u16 time_ms);
u8 USB_EP2_Send(u8 *data,u32 dataNum);
u8 USB_EP1_Send(u8 *data,u32 dataNum);
u8 USB_EP3_Send(u8 *data,u32 dataNum);
u8 USB_EP2_Rec(u8 *data,u32 dataNum);
u8 USB_EP1_Rec(u8 *data,u32 dataNum);
u8 USB_EP3_Rec(u8 *data,u32 dataNum);
#endif //_USBIO_H_

