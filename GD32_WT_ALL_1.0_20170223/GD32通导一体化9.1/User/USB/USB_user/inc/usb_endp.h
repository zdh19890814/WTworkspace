/********************************************************************************
*
*  File Name     : usb_endp.h
*  Author   	 : Íõ±£Àñ
*  Create Date   : 2016.06.01
*  Version   	 : 1.0
*  Description   : 
*  History       : 1. Data:
*                     Author:
*                     Modification:
*
********************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USB_ENDP_H_
#define _USB_ENDP_H_
#include "include.h"


extern uint8_t USB_EP2_Receive_Buffer[256];
extern uint8_t USB_EP1_Receive_Buffer[100];
extern uint8_t USB_EP3_Receive_Buffer[100];
extern uint8_t USB_Send_Buffer[256];
extern volatile uint8_t Endp2InFlag;
extern volatile uint8_t Endp1InFlag;
extern volatile uint8_t Endp3InFlag;

#endif
