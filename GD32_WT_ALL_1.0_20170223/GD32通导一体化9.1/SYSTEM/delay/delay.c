


#include "include.h"	



/*****************************************************************
@函 数 名：delay_ms(void)                                 
@功    能：延时函数
@输    入：毫秒数
@输    出：无
@备    注：必须在  TickInit()后才能调用，在中断里不能调用！（因为tick
           中断优先级被设置为最低）                                               
*****************************************************************/
void delay_ms(u16 nms)
{
	TimeTemp = SysTime;
	while((SysTime - TimeTemp)< nms * 10);	
} 










//by baoli
void delay_s(u16 ns)
{
    while(ns--)
    {
        delay_ms(1000);
    }
}





































