/******************************************************************
  Copyright (C), 2008-2009, 力天电子，LiTian Tech.Co.Ltd.
  Module Name   : LT_ARM214X_TEST_ALL              
  File Name     : uart.c	            
  Author   	: HE CHENG         
  Create Date  	: 2009/10/19         
  Version   	: 1.0                
  Function      : 串口驱动                       
  Description  	: 无     
  Support     	: www.LT430.com  QQ:330508762                  
******************************************************************/
#include "uart.h"
#include <includes.h>

INT32U n=0;//接收长度计数

struct uart_struct
{
  OS_EVENT *Usem;
  INT8U *rebuf;
  INT8U num;
  INT8U ok;
}uart_str[2];


void InitUart(char port,unsigned int XFre,unsigned int Brate)//串口初始化
{
  if(port == 1) //串口1
  {
    /*Configure the pins that are connected to RX and TX on UART1 */
    PINSEL0_bit.P0_8 = 1;PINSEL0_bit.P0_9 = 1;
    
    /*Configure UART1 to 115200 buad, 8 bit, 1 stop, no parity */
    U1FCR = 1;                      // Enable FIFOs whether used or not
    SetBit(U1LCR,DIVISOR_LATCH_SET_bit); // U1LCR = 0X80-enable access to divisor
                                         // latch bit, necessary for setting baud rate
    U1LCR |= EIGHT_BIT_CHARS;            // Eight bits U1LCR = 0X83
    ClrBit(U1LCR,PARITY_ENABLE_bit);     // No parity
    ClrBit(U1LCR,STOP_BIT_SELECT_bit);   // One stop bit
    U1DLL = XFre / (Brate * 16);
    U1DLM = (XFre / (Brate * 16)) >> 8;
    ClrBit(U1LCR,DIVISOR_LATCH_SET_bit); // Disable access to divisor latch
    
        U1FCR &= ~(1<<0); 
	U1IER |= 0x01;		
			
	VICIntSelect &= ~(1 << VIC_UART1);                         /* Enable interrupts                                        */
	VICVectAddr4  = (INT32U)IRQ_UART1;                        /* Set the vector address                                   */
	VICVectCntl4  = 0x20 | VIC_UART1;                          /* Enable vectored interrupts                               */
	VICIntEnable  = (1 << VIC_UART1);                          /* Enable Interrupts                                        */

      uart_str[1].Usem=OSSemCreate(0);
      uart_str[1].ok=0;
      uart_str[1].num=1;
  }
  else if(port == 0)  //串口0
  {
      /*Configure the pins that are connected to RX and TX on UART1 */
      PINSEL0_bit.P0_0 = 1;PINSEL0_bit.P0_1 = 1;
      
      /*Configure UART1 to 9600 buad, 8 bit, 1 stop, no parity */
      U0FCR = 1;                      			// Enable FIFOs whether used or not
      SetBit(U0LCR,DIVISOR_LATCH_SET_bit); // U1LCR = 0X80-enable access to divisor
                                           // latch bit, necessary for setting baud rate
      U0LCR |= EIGHT_BIT_CHARS;            // Eight bits U1LCR = 0X83
      ClrBit(U0LCR,PARITY_ENABLE_bit);     // No parity
      ClrBit(U0LCR,STOP_BIT_SELECT_bit);   // One stop bit
      U0DLL = XFre / (Brate * 16);
      U0DLM = (XFre / (Brate * 16)) >> 8;
      ClrBit(U0LCR,DIVISOR_LATCH_SET_bit); // Disable access to divisor latch

        U0FCR &= ~(1<<0); 
	U0IER |= 0x01;		
			
	VICIntSelect &= ~(1 << VIC_UART0);                         /* Enable interrupts                                        */
	VICVectAddr1  = (INT32U)IRQ_UART0;                        /* Set the vector address                                   */
	VICVectCntl1  = 0x20 | VIC_UART0;                          /* Enable vectored interrupts                               */
	VICIntEnable  = (1 << VIC_UART0);                          /* Enable Interrupts                                        */

      uart_str[0].Usem=OSSemCreate(0);
      uart_str[0].ok=0;
      uart_str[0].num=1;
  }
}

INT8U readuart(INT8U x, INT32U n, INT8U *buff, INT16U timeout)//读串口数据
{
   INT8U err;
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register     */
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    OS_ENTER_CRITICAL();    
    uart_str[x].num=n;
    uart_str[x].rebuf=buff;
    uart_str[x].ok=1;
    OS_EXIT_CRITICAL();

    OSSemPend(uart_str[x].Usem, timeout, &err);
    return err; 
}

void IRQ_UART0(void)//串口0接收数据中断服务程序
{
  
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register     */
    OS_CPU_SR  cpu_sr = 0;
#endif
	OS_ENTER_CRITICAL();
	OSIntEnter();        
	
        if(uart_str[0].ok==1)
        {
          if((U0IIR & 0x0f) == 0x04)
          {
            *uart_str[0].rebuf++=U0RBR;
            n++;
          }
          if(n>=uart_str[0].num)
          {
            uart_str[0].ok=0;
            n=0;
            OSSemPost(uart_str[0].Usem);
          }
        }

	OSIntExit();
	OS_EXIT_CRITICAL();
}


void IRQ_UART1(void)//串口1接收数据中断服务程序
{
  
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register     */
    OS_CPU_SR  cpu_sr = 0;
#endif
	OS_ENTER_CRITICAL();
	OSIntEnter();        
	
        if(uart_str[1].ok==1)
        {
          if((U1IIR & 0x0f) == 0x04)
          {
            *uart_str[1].rebuf++=U1RBR;
            n++;
          }
          if(n>=uart_str[1].num)
          {
            uart_str[1].ok=0;
            n=0;
            OSSemPost(uart_str[1].Usem);
          }
        }

	OSIntExit();
	OS_EXIT_CRITICAL();
}



//发送字符串
//len=0,发送完整的字符串
//len!=0,发送指定长度数据
void senduart(char port,unsigned int len,unsigned char *Str)
{
  if(len==0)
  {
      while((*Str)!='\0')
      {
        sendByte(port,*Str);
        Str++;
      }
  }
  else
  {
        while(len!=0)
      {
        sendByte(port,*Str);
        Str++;
        len--;
      }
  }
  
}

//发送单个字符
void sendByte(char port,char byte)
{
  if(port == 1)
  {
    while (!(U1LSR &0x20)); //U1THR THRE bit set when U1THR contains valid data
    U1THR = byte;
  }
  else if(port == 0)
  {
    while (!(U0LSR &0x20)); //U1THR THRE bit set when U1THR contains valid data
    U0THR = byte;
  }
}

