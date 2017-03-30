#ifndef  _UART_H_
#define _UART_H_

void InitUart(char port,unsigned int XFre,unsigned int Brate);
void sendStr(char port,char *Str, int len);
void sendByte(char port,char byte);
unsigned char readChar(char port);

#endif