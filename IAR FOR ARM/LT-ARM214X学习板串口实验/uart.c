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
#include <NXP/iolpc2148.h>

#define DIVISOR_LATCH_SET_bit  0x7
#define EIGHT_BIT_CHARS          3
#define PARITY_ENABLE_bit      0x3   // Parity enable bit in UxLCR is bit 3
#define STOP_BIT_SELECT_bit    0x2   // Stop bit control in UxLCR is bit 2

#define SetBit(x,y);       ( (x) |= (1<<y) )
#define ClrBit(x,y);       ( (x) &= (~(1<<y)) )

void sendByte(char port,char byte);

void InitUart(char port,unsigned int XFre,unsigned int Brate)
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
  }
}

//发送字符串
void sendStr(char port,char *Str,int len)
{
  while(len>0)
  {
    sendByte(port,*Str++);
    --len;
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

//读取一个字符
unsigned char readChar(char port)
{
  if(port == 1)
  {
    while (U1LSR_bit.DR == 0);
    return U1RBR;
  }
  else if(port == 0)
  {
    //when U0LSR_bit.DR is 1 - U0RBR contains valid data
    while (U0LSR_bit.DR == 0);
    return U0RBR;
  }
  
  return 0;
}