/*
*********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*
*                             (c) Copyright 1992-2007, Micrium, Weston, FL
*                                          All Rights Reserved
*
*                                           Generic ARM Port
*
* File      : OS_CPU_C.C
* Version   : V1.84
* By        : Jean J. Labrosse
*             Jean-Denis Hatier
*
* For       : ARM7 or ARM9
* Mode      : ARM or Thumb
* Toolchain : IAR's EWARM V4.11a and higher
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>

/*$PAGE*/
/*
*********************************************************************************************************
*                                             LOCAL CONSTANTS
*
* Note(s) : 1) ARM_MODE_ARM is the CPSR bit mask for ARM Mode
*           2) ARM_MODE_THUMB is the CPSR bit mask for THUMB Mode
*           3) ARM_SVC_MODE_THUMB is the CPSR bit mask for SVC MODE + THUMB Mode
*           4) ARM_SVC_MODE_ARM is the CPSR bit mask for SVC MODE + ARM Mode
            5) OS_NTASKS_FP  establishes the number of tasks capable of supporting floating-point.  One
*              task is removed for the idle task because it doesn't do floating-point at all.
*           6) OS_FP_STORAGE_SIZE  currently allocates 128 bytes of storage in order to accomodate
*              thirty-two single-precision 32-bit, or sixteen double-precision 64-bit VFP registers.
*********************************************************************************************************
*/

#define  ARM_MODE_ARM           0x00000000
#define  ARM_MODE_THUMB         0x00000020

#define  ARM_SVC_MODE_THUMB    (0x00000013L + ARM_MODE_THUMB)
#define  ARM_SVC_MODE_ARM      (0x00000013L + ARM_MODE_ARM)

#define  OS_NTASKS_FP          (OS_MAX_TASKS + OS_N_SYS_TASKS - 1)
#define  OS_FP_STORAGE_SIZE            128L

/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0
static  INT16U  OSTmrCtr;
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)
{
    INT32U   size;
    OS_STK  *pstk;

                                                           /* Clear exception stack for stack checking.*/
    pstk = &OS_CPU_ExceptStk[0];
    size = OS_CPU_EXCEPT_STK_SIZE;
    while (size > 0) {
        size--;
       *pstk = (OS_STK)0;
    }

#if OS_STK_GROWTH == 1
    OS_CPU_ExceptStkBase = &OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE - 1];
#else
    OS_CPU_ExceptStkBase = &OS_CPU_ExceptStk[0];
#endif

#if OS_TMR_EN > 0
    OSTmrCtr = 0;
#endif
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookEnd (void)
{
  
}
#endif

/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskCreateHook (OS_TCB *ptcb)
{
    (void)ptcb;                                  /* Prevent compiler warning                           */
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskDelHook (OS_TCB *ptcb)
{
    (void)ptcb;                                            /* Prevent compiler warning                 */
}
#endif

/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
void  OSTaskIdleHook (void)
{

}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskStatHook (void)
{
  
}
#endif

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*              2) All tasks run in SVC mode.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
{
    OS_STK *stk;
    INT32U  task_addr;


    opt       = opt;                             /* 'opt' is not used, prevent warning                 */
    stk       = ptos;                            /* Load stack pointer                                 */
    task_addr = (INT32U)task & ~1;               /* Mask off lower bit in case task is thumb mode      */
    *(stk)    = (INT32U)task_addr;               /* Entry Point                                        */
    *(--stk)  = (INT32U)0x14141414L;             /* R14 (LR)                                           */
    *(--stk)  = (INT32U)0x12121212L;             /* R12                                                */
    *(--stk)  = (INT32U)0x11111111L;             /* R11                                                */
    *(--stk)  = (INT32U)0x10101010L;             /* R10                                                */
    *(--stk)  = (INT32U)0x09090909L;             /* R9                                                 */
    *(--stk)  = (INT32U)0x08080808L;             /* R8                                                 */
    *(--stk)  = (INT32U)0x07070707L;             /* R7                                                 */
    *(--stk)  = (INT32U)0x06060606L;             /* R6                                                 */
    *(--stk)  = (INT32U)0x05050505L;             /* R5                                                 */
    *(--stk)  = (INT32U)0x04040404L;             /* R4                                                 */
    *(--stk)  = (INT32U)0x03030303L;             /* R3                                                 */
    *(--stk)  = (INT32U)0x02020202L;             /* R2                                                 */
    *(--stk)  = (INT32U)0x01010101L;             /* R1                                                 */
    *(--stk)  = (INT32U)p_arg;                   /* R0 : argument                                      */
    if ((INT32U)task & 0x01) {                   /* See if task runs in Thumb or ARM mode              */
        *(--stk) = (INT32U)ARM_SVC_MODE_THUMB;   /* CPSR  (Enable IRQ and FIQ interrupts, THUMB-mode)  */
    } else {
        *(--stk) = (INT32U)ARM_SVC_MODE_ARM;     /* CPSR  (Enable IRQ and FIQ interrupts, ARM-mode)    */
    }

    return (stk);
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void  OSTaskSwHook (void)
{

}
#endif

/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSTCBInitHook (OS_TCB *ptcb)
{

}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
  
}
#endif


/*
*********************************************************************************************************
*                                     INITIALIZE EXCEPTION VECTORS
*
* Description : This function initialize exception vectors to the default handlers.
*
* Arguments   : None.
*********************************************************************************************************
*/

void  OS_CPU_InitExceptVect (void)
{
    (*(INT32U *)OS_CPU_ARM_EXCEPT_UNDEF_INSTR_VECT_ADDR)       =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_UNDEF_INSTR_HANDLER_ADDR)    = (INT32U)OS_CPU_ARM_ExceptUndefInstrHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_SWI_VECT_ADDR)               =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_SWI_HANDLER_ADDR)            = (INT32U)OS_CPU_ARM_ExceptSwiHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_PREFETCH_ABORT_VECT_ADDR)    =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_PREFETCH_ABORT_HANDLER_ADDR) = (INT32U)OS_CPU_ARM_ExceptPrefetchAbortHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_DATA_ABORT_VECT_ADDR)        =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_DATA_ABORT_HANDLER_ADDR)     = (INT32U)OS_CPU_ARM_ExceptDataAbortHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_ADDR_ABORT_VECT_ADDR)        =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_ADDR_ABORT_HANDLER_ADDR)     = (INT32U)OS_CPU_ARM_ExceptAddrAbortHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_IRQ_VECT_ADDR)               =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_IRQ_HANDLER_ADDR)            = (INT32U)OS_CPU_ARM_ExceptIrqHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_FIQ_VECT_ADDR)               =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_FIQ_HANDLER_ADDR)            = (INT32U)OS_CPU_ARM_ExceptFiqHndlr;
}


/*
*********************************************************************************************************
*                              GET NUMBER OF FREE ENTRIES IN EXCEPTION STACK
*
* Description : This function computes the number of free entries in the exception stack.
*
* Arguments   : None.
*
* Returns     : The number of free entries in the exception stack.
*********************************************************************************************************
*/

INT32U  OS_CPU_ExceptStkChk (void)
{
    OS_STK  *pchk;
    INT32U   nfree;
    INT32U   size;


    nfree = 0;
    size  = OS_CPU_EXCEPT_STK_SIZE;
#if OS_STK_GROWTH == 1
    pchk = &OS_CPU_ExceptStk[0];
    while ((*pchk++ == (OS_STK)0) && (size > 0)) {    /* Compute the number of zero entries on the stk */
        nfree++;
        size--;
    }
#else
    pchk = &OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE - 1];
    while ((*pchk-- == (OS_STK)0) && (size > 0)) {    /* Compute the number of zero entries on the stk */
        nfree++;
        size--;
    }
#endif
    return (nfree);
}
