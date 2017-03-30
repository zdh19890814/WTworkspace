#ifndef __HARDWARES_H__
#define __HARDWARES_H__
	#include "uart.h"
		
	typedef enum {false=0,true=1}bool;

	#define u8 unsigned char
	#define u16 unsigned short
	#define u32 unsigned int
	
	#define         SCLK  (0x01<<24)
	#define 				SCK0	(1<<4)
	#define					SL		(1<<25)
	#define         MISO  (0x01<<5)
	#define         MOSI  (0x01<<6)
	#define         RCK   (0x01<<7)


	void HC595_165_Init(void);
	void HC595_WriteByte(unsigned char data);
	void HC595_WriteShort(unsigned short data);
	void HC595_WriteWord(unsigned int data);
	void HC595_Pin(u8 bit,u32 data);
	void DisplayStr(u8* str);
	void DisplayNum(int num);
	void Delayn(unsigned long n);
	unsigned char Read165(void);
	unsigned char ReadKey(void);

#endif
