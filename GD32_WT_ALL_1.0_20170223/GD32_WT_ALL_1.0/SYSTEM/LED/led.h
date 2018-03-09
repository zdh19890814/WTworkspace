#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


#ifdef GD32
#define LED1_H GPIO_SetBits(GPIOB,GPIO_Pin_10)				 
#define LED1_L GPIO_ResetBits(GPIOB,GPIO_Pin_10)
#define LED1_T GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_10)))

#define LED2_H GPIO_SetBits(GPIOB,GPIO_Pin_11)				 
#define LED2_L GPIO_ResetBits(GPIOB,GPIO_Pin_11)
#define LED2_T GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_11)))

#else
#define LED1_H GPIO_SetBits(GPIOE,GPIO_Pin_5)				 
#define LED1_L GPIO_ResetBits(GPIOE,GPIO_Pin_5)
#define LED1_T GPIO_WriteBit(GPIOE, GPIO_Pin_5, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_5)))

#define LED2_H ;			 
#define LED2_L ;
#define LED2_T ;

#endif


void LED_Init(void);//≥ı ºªØ



#endif
