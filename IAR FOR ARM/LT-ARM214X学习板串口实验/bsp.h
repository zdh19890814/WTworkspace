/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                              NXP LPC2148
*                                                on the
*                                       IAR LPC2148 Kickstart Card
*
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : JJL
*********************************************************************************************************
*/

#ifndef  __BSP_H__
#define  __BSP_H__


typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */
/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_GLOBALS
#define  BSP_EXT
#else
#define  BSP_EXT  extern
#endif

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define  BSP_MAIN_OSC_FREQ             12000000L


#define  BSP_USE_SPI                           1
#define  BSP_USE_GPIO                          2

#define  BSP_CFG_SPI                  BSP_USE_SPI

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         BSP_Init                (void);
void         BSP_IntDisAll           (void);
INT32U   BSP_CPU_ClkFreq         (void);
INT32U   BSP_CPU_PclkFreq        (void);

/*
*********************************************************************************************************
*                                             LED SERVICES
*********************************************************************************************************
*/

void         BSP_LED_On              (INT8U        led);
void         BSP_LED_Off             (INT8U        led);
void         BSP_LED_Toggle          (INT8U        led);

/*
*********************************************************************************************************
*                                         PUSH BUTTON SERVICES
*********************************************************************************************************
*/

BOOLEAN  BSP_PB_GetStatus         (INT8U         pb);
void         BSP_PB_EINT0_Init        (void        (*isr)(void));
void         BSP_PB_EINT2_Init        (void        (*isr)(void));

/*
*********************************************************************************************************
*                                         ADC SERVICES
*********************************************************************************************************
*/

INT16U   BSP_ADC_GetStatus        (INT8U         adc);
void         BSP_ADC_EINT3_Init       (void         (*isr)(void));

/*
*********************************************************************************************************
*                                             TICK SERVICES
*********************************************************************************************************
*/

void         BSP_Tmr_TickISR_Handler  (void);

/*
*********************************************************************************************************
*                                             SPI SERVICES
*********************************************************************************************************
*/

void         BSP_SPI_Init             (void);




#endif
