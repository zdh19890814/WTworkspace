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
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_GLOBALS
#include <includes.h>


//系统时钟
#define Fosc        12000000UL        //晶振时钟 【Hz】
#define Fcclk       (Fosc * 5)        //系统频率，必须为Fosc的整数倍(1~32)，且<=60MHZ
#define Fcco	    (Fcclk * 4)	      //CCO频率，必须为Fcclk的2、4、8、16倍，范围为156MHz~320MHz
#define Fpclk       (Fcclk / 4) * 1   //VPB时钟频率，只能为(Fcclk / 4)的1 ~ 4倍

 
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
typedef            void     (*CPU_FNCT_VOID)(void);             /* See Note #2a.                                        */
typedef            void     (*CPU_FNCT_PTR )(void *);           /* See Note #2b.  

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
        INT32U  VIC_SpuriousInt;
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  BSP_PLL_Init         (void);
static  void  BSP_MAM_Init         (void);

static  void  BSP_Tmr_TickInit     (void);

static  void  BSP_VIC_Init         (void);

/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    MEMMAP  = 1;         
    
    BSP_PLL_Init();                                             /* Initialize PLL0 and the VPB Divider Register             */
    BSP_MAM_Init();                                             /* Initialize the Memory Acceleration Module                */
    BSP_VIC_Init();                                             /* Initialize the Vectored Interrupt Controller             */

    BSP_Tmr_TickInit();                                         /* Initialize the uC/OS-II tick interrupt                   */
  
    InitUart(0,Fpclk,115200);
 //   InitUart(1,Fpclk,115200);
}


/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Get the CPU clock frequency.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*********************************************************************************************************
*/

INT32U  BSP_CPU_ClkFreq (void)
{
    INT32U  msel;
    INT32U  cpu_clk_freq;


    msel         = (INT32U)(PLLCFG & 0x1F);
    cpu_clk_freq = BSP_MAIN_OSC_FREQ * (msel + 1);
    return (cpu_clk_freq);
}


/*
*********************************************************************************************************
*                                            BSP_CPU_PclkFreq()
*
* Description : Get the peripheral clock frequency.
*
* Argument(s) : none.
*
* Return(s)   : The peripheral clock frequency, in Hz.
*********************************************************************************************************
*/

INT32U  BSP_CPU_PclkFreq (void)
{
    INT32U  msel;
    INT32U  vpbdiv;
    INT32U  clk_freq;
    INT32U  pclk_freq;


    msel         = (INT32U)(PLLCFG   & 0x1F);
    clk_freq     = BSP_MAIN_OSC_FREQ * (msel + 1);

    vpbdiv       = (INT32U)(VPBDIV & 0x03);
    switch (vpbdiv) {
        case 0:
             pclk_freq = clk_freq / 4;
             break;

        case 1:
             pclk_freq = clk_freq;
             break;

        case 2:
             pclk_freq = clk_freq / 2;
             break;

        default:
             pclk_freq = clk_freq / 4;
             break;
    }
    return (pclk_freq);
}


/*
*********************************************************************************************************
*                                          OS_CPU_ExceptHndlr()
*
* Description : Handle any exceptions.
*
* Argument(s) : except_id     ARM exception type:
*
*                                  OS_CPU_ARM_EXCEPT_RESET             0x00
*                                  OS_CPU_ARM_EXCEPT_UNDEF_INSTR       0x01
*                                  OS_CPU_ARM_EXCEPT_SWI               0x02
*                                  OS_CPU_ARM_EXCEPT_PREFETCH_ABORT    0x03
*                                  OS_CPU_ARM_EXCEPT_DATA_ABORT        0x04
*                                  OS_CPU_ARM_EXCEPT_ADDR_ABORT        0x05
*                                  OS_CPU_ARM_EXCEPT_IRQ               0x06
*                                  OS_CPU_ARM_EXCEPT_FIQ               0x07
*
* Return(s)   : none.
*
* Caller(s)   : OS_CPU_ARM_EXCEPT_HANDLER(), which is declared in os_cpu_a.s.
*********************************************************************************************************
*/

void  OS_CPU_ExceptHndlr (INT32U  except_id)
{
    CPU_FNCT_VOID  pfnct;
    
    if (except_id == OS_CPU_ARM_EXCEPT_IRQ) {

        pfnct = (CPU_FNCT_VOID)VICVectAddr;                     /* Read the interrupt vector from the VIC                   */
        while (pfnct != (CPU_FNCT_VOID)0) {                     /* Make sure we don't have a NULL pointer                   */
          (*pfnct)();                                           /* Execute the ISR for the interrupting device              */
            VICVectAddr = 1;                                    /* Acknowlege the VIC interrupt                             */
            pfnct = (CPU_FNCT_VOID)VICVectAddr;                 /* Read the interrupt vector from the VIC                   */
        }

    } else {
                                                                /* Infinite loop on other exceptions.                       */
                                                                /* Should be replaced by other behavior (reboot, etc.)      */
        while (DEF_TRUE) {
            ;
        }
    }
}


/*
*********************************************************************************************************
*                                           BSP_IntDisAll()
*
* Description : Disable ALL interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  BSP_IntDisAll (void)
{
    VICIntEnClear = 0xFFFFFFFFL;                                /* Disable ALL interrupts                                   */
}

/*
*********************************************************************************************************
*********************************************************************************************************
**                                     uC/OS-II TIMER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            BSP_Tmr_TickInit()
*
* Description : Initialize uC/OS-II's tick source.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

static  void  BSP_Tmr_TickInit (void)
{
    INT32U  pclk_freq;
    INT32U  tmr_reload;

                                                                /* VIC TIMER #0 Initialization                              */
    VICIntSelect &= ~(1 << VIC_TIMER0);                         /* Enable interrupts                                        */
    VICVectAddr2  = (INT32U)BSP_Tmr_TickISR_Handler;        /* Set the vector address                                   */
    VICVectCntl2  = 0x20 | VIC_TIMER0;                          /* Enable vectored interrupts                               */
    VICIntEnable  = (1 << VIC_TIMER0);                          /* Enable Interrupts                                        */

    pclk_freq     = BSP_CPU_PclkFreq();
    tmr_reload    = pclk_freq / OS_TICKS_PER_SEC;
    T0TCR         = 0;                                          /* Disable timer 0.                                         */
    T0PC          = 0;                                          /* Prescaler is set to no division.                         */

    T0MR0         = tmr_reload;
    T0MCR         = 3;                                          /* Interrupt on MR0 (reset TC)                              */

    T0CCR         = 0;                                          /* Capture is disabled.                                     */
    T0EMR         = 0;                                          /* No external match output.                                */
    T0TCR         = 1;                                          /* Enable timer 0                                           */
}


/*
*********************************************************************************************************
*                                       BSP_Tmr_TickISR_Handler()
*
* Description : Handle the timer interrupt that is used to generate TICKs for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  BSP_Tmr_TickISR_Handler (void)
{
    T0TCR &= ~DEF_BIT_00;
    T0TC   = 0;
    T0IR   = DEF_BIT_00;                                         /* Clear timer #0 interrupt                                 */
    T0TCR |= DEF_BIT_00;

    OSTimeTick();                                                /* Call uC/OS-II's OSTimeTick()                             */
}


/*
*********************************************************************************************************
*********************************************************************************************************
**                                          LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_PLL_Init()
*
* Description : Set up and activate the PLL
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

static  void  BSP_PLL_Init (void)
{
#if OS_CRITICAL_METHOD == 3                                     /* Allocate storage for CPU status register                 */
    OS_CPU_SR   cpu_sr = 0;
#endif
    
    /* 设置系统各部分时钟 */
       PLLCON = 1;
    #if ((Fcclk / 4) / Fpclk) == 1
            VPBDIV = 0;
    #endif
    #if ((Fcclk / 4) / Fpclk) == 2
            VPBDIV = 2;
    #endif
    #if ((Fcclk / 4) / Fpclk) == 4
            VPBDIV = 1;
    #endif
    #if (Fcco / Fcclk) == 2
            PLLCFG = ((Fcclk / Fosc) - 1) | (0 << 5);
    #endif
    #if (Fcco / Fcclk) == 4
            PLLCFG = ((Fcclk / Fosc) - 1) | (1 << 5);
    #endif
    #if (Fcco / Fcclk) == 8
            PLLCFG = ((Fcclk / Fosc) - 1) | (2 << 5);
    #endif
    #if (Fcco / Fcclk) == 16
            PLLCFG = ((Fcclk / Fosc) - 1) | (3 << 5);
    #endif
            
    OS_ENTER_CRITICAL();
    PLLFEED  = 0xAA;                                            /* Write to the PLL Feed register                           */
    PLLFEED  = 0x55;
    OS_EXIT_CRITICAL();

    while((PLLSTAT & (1 << 10)) == 0);

    PLLCON  |= 0x00000002;                                      /* Connect the PLL                                          */

    OS_ENTER_CRITICAL();
    PLLFEED  = 0xAA;                                            /* Write to the PLL Feed register                           */
    PLLFEED  = 0x55;
    OS_EXIT_CRITICAL();
}

/*
*********************************************************************************************************
*                                             BSP_MAM_Init()
*
* Description : Initialize the memory acceleration module.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

static  void  BSP_MAM_Init (void)
{
    MAMCR  = 0x00;                                              /* Disable the Memory Accelerator Module                    */
    MAMTIM = 0x03;                                              /* MAM fetch cycles are 3 CCLKs in duration                 */
    MAMCR  = 0x02;                                              /* Enable the Memory Accelerator Module                     */
}

/*
*********************************************************************************************************
*********************************************************************************************************
**                                            VIC FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        Vectored Interrupt Controller
*********************************************************************************************************
*/

static  void  BSP_VIC_Init (void)
{
    VICIntEnClear = 0xFFFFFFFF;                                 /* Disable ALL interrupts                                   */
    VICProtection = 0;                                          /* Setup interrupt controller                               */
}