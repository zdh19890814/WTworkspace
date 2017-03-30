#include <includes.h>
#include "hardwares.h"

int shownum = 0;

static  OS_STK AppTaskStartStk[APP_TASK_STK_SIZE];
static  OS_STK AppStkSeg[APP_TASK_STK_SIZE];
static  OS_STK AppStkLed[APP_TASK_STK_SIZE];
void TaskSegment(void *p_arg);
void TaskLed(void *p_arg);

INT8U buf[101];
static void AppTaskStart(void *p_arg);

void  main (void)
{
    BSP_IntDisAll();                                            /* Disable all interrupts until we are ready to accept them */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"              */

    OSTaskCreate(AppTaskStart,                               /* Create the start task                                    */
								(void *)0,
								(OS_STK *)&AppTaskStartStk[APP_TASK_STK_SIZE - 1],
								APP_TASK_START_PRIO);
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)       */
    
   // InitUart(1,)
}

static  void  AppTaskStart (void *p_arg)
{
  INT8U uerr;
	(void)p_arg;
	BSP_Init();                                                 /* Initialize BSP functions */
	HC595_165_Init();
	OSTaskCreate(TaskSegment,
							(void *)0,
							(OS_STK *)&AppStkSeg[APP_TASK_STK_SIZE - 1],
							APP_TASK_SEG_PRIO);
	
	OSTaskCreate(TaskLed,
							(void *)0,
							(OS_STK *)&AppStkLed[APP_TASK_STK_SIZE - 1],
							APP_TASK_LED_PRIO);
	
	while(1)
	{
		uerr=readuart(0,100,buf,0);
               // if(uerr==OS_NO_ERR)
                senduart(0,10,buf);
                buf[0]='\0';
	}
}

void TaskLed(void *p_arg)
{
	unsigned char a;
	while(1)
	{
		//a = ReadKey(); 
		//if(a == 1)
			shownum++;
		//else if(a == 2)
			//shownum--;
		
		OSTimeDly(1000);
	}
}

void TaskSegment(void *p_arg)
{
	while(1)
	{
		DisplayNum(shownum);
		OSTimeDly(20);
	}
}