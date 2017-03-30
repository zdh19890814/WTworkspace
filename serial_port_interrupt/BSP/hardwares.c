#include "hardwares.h"
#include <includes.h>

//全局变量
unsigned int HC595_DATA = 0xFFFFFFFF;
void HC595_165_Init(void)
{
	IO1DIR_bit.P1_25 = 1;
	IO1SET_bit.P1_25 = 1;
	PINSEL0_bit.P0_5 = 0;
	IO0DIR |= MOSI|RCK|SCK0;
	IO1DIR |= SCLK;
	IO1CLR |= SCLK;
	IO0DIR |= RCK;
	
	HC595_WriteWord(HC595_DATA);
}

//写字
void HC595_WriteWord(unsigned int data)
{
  unsigned char i;
  IO0CLR = RCK;
  for(i=0;i<32;i++)
  {
    if(data&0x00000001)	IO0SET=MOSI;
    else	IO0CLR=MOSI;
    IO1SET=SCLK;
    data>>=1;
    IO1CLR=SCLK;
  }
  IO0SET = RCK;
}

//bit为位数,data为数据只能取0或1
void HC595_Pin(u8 bit,u32 data)
{
	HC595_DATA &= ~(1 << bit);
	HC595_DATA |= (data << bit);
	HC595_WriteWord(HC595_DATA);
}


//数码管

//数码管的输出对应值 从0~F加一个空不显示和一个负号
const unsigned char  CODE_TABLE[18] = {
  0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,
  0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71,0x00,0x40};
	
void DisplayStr(u8* str)
{
	u8 i;
	HC595_DATA &= 0xF00FFFFF;
	HC595_DATA |= (0x0f << 20);
	for(i=0;i<4;i++)
	{
		HC595_Pin(20 + i,0);
		if(str[i] == ' ')
			*((u8*)(&HC595_DATA)+3) = CODE_TABLE[16];
		else if(str[i] == '-')
			*((u8*)(&HC595_DATA)+3) = CODE_TABLE[17];
		else
			*((u8*)(&HC595_DATA)+3) = CODE_TABLE[str[i] - '0'];
		HC595_WriteWord(HC595_DATA);
		OSTimeDly(1);	// 延时提高亮度
		*((u8*)(&HC595_DATA)+3) = 0x00;
		HC595_Pin(20 + i,1);
	}
}

void DisplayNum(int num)
{
	u8 str[4],i;
	if(num > 9999 || num < -999)
		return;
	if(num < 0)
	{
		num = -num;
		str[0] = '-';
	}
	else
		str[0] = num / 1000 + '0';
	str[1] = num / 100 % 10 + '0';
	str[2] = num / 10 % 10 + '0';
	str[3] = num % 10 + '0';
/*	for(i=0;i<4;i++)
	{
		if(str[i] == '0')
		{
			str[i] = ' ';
		}
	}*/
	DisplayStr(str);
}

//#define HC165_CS(a)		HC595_Pin(1,a)

void HC165_CS(char flag)
{
  if(flag)
    HC595_DATA &= ~(1<<1);
  else
    HC595_DATA |= (1<<1);
	HC595_WriteWord(HC595_DATA);
}

unsigned char Read165(void)
{
  unsigned char RD=0,i;
  HC165_CS(1);
  IO1CLR_bit.P1_25 = 1; 
  Delayn(10);
  IO1SET_bit.P1_25 = 1;
  for(i=0;i<8;i++)
  {
    RD <<= 1; 
    if(IO0PIN&MISO)   
      RD |= 1;  
    IO0SET=SCK0;
    Delayn(10);
    IO0CLR=SCK0; 
  } 
  HC165_CS(0);
  return RD;
}

//延时周期数
void Delayn(unsigned long n)
{
  while(n--);
}

unsigned char KEY_TBL[4] = {0x08,0x10,0x20,0x40};
unsigned char flag = 0x00;

unsigned char ReadKey(void)
{
	unsigned char a,b=0,i;
	a = Read165();
	
	flag &= ~(a & 0x78);

	if((a & 0x78) != 0x78)
	{
		OSTimeDly(20);
		if((a & 0x78) != 0x78)
		{
			for(i=0;i<4;i++)
			{
				if((a & KEY_TBL[i]) == 0 && (flag & KEY_TBL[i]) == 0)
				{
					flag |= KEY_TBL[i];
					b = i+1;
					break;
				}
			}
		}
	}
	return b;
}
