#ifndef  _UART_H_
#define  _UART_H_
#include <includes.h>

#define DIVISOR_LATCH_SET_bit  0x7
#define EIGHT_BIT_CHARS          3
#define PARITY_ENABLE_bit      0x3   // Parity enable bit in UxLCR is bit 3
#define STOP_BIT_SELECT_bit    0x2   // Stop bit control in UxLCR is bit 2

#define SetBit(x,y);       ( (x) |= (1<<y) )
#define ClrBit(x,y);       ( (x) &= (~(1<<y)) )

void InitUart(char port,unsigned int XFre,unsigned int Brate);
void senduart(char port,unsigned int len,unsigned char *Str);
INT8U readuart(INT8U x, INT32U n, INT8U *buff, INT16U timeout);

void sendByte(char port,char byte);

void IRQ_UART0(void);
void IRQ_UART1(void);


#endif