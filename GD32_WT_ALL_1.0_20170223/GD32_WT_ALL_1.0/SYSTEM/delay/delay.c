


#include "include.h"	



/*****************************************************************
@�� �� ����delay_ms(void)                                 
@��    �ܣ���ʱ����
@��    �룺������
@��    ������
@��    ע��������  TickInit()����ܵ��ã����ж��ﲻ�ܵ��ã�����Ϊtick
           �ж����ȼ�������Ϊ��ͣ�                                               
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





































