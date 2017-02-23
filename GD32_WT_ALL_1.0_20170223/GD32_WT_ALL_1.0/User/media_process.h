#ifndef _MEDIA_PROCESS_H
#define _MEDIA_PROCESS_H
#include "stm32f10x.h"

void USB_SendVoiceData(uint16_t* buf);
void USB_SendMes(u8 * mes);

void USB_SendCom(void);
void SendBack(uint16_t* buf);
void USB_MessageTest(void);
void USB_VoiceTest(void);
void USB_RecData(void);

void USB_VoiceSend(void);

void I2S_SendVoice(void); 


void I2S_VoiceSendBack(void);
void USB_VoiceSendBack(void);


void USB_VoiceSend1(void);
void USB_VoiceSend_D(void);
#endif
