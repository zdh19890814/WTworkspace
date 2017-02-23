

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_desc.h"
#include "usb_lib.h"
#include "usb_endp.h"
#include "usbio.h"
#include "stdio.h"
#include "delay.h"
#include "include.h"


uint16_t EP3_RevNum=0;  //EP3�������ݳ���
uint16_t EP2_RevNum=0;  //EP2�������ݳ���
uint8_t EP2_RevFlag=1;  //EP2�˵����ݽ�����ɱ�־
uint8_t EP1_RevFlag=1;  //EP1�˵����ݽ�����ɱ�־
uint8_t EP3_RevFlag=1;  //EP1�˵����ݽ�����ɱ�־

  

/*****************************************************************
@�� �� ����USB_EP1_Send(u8 *data,u32 dataNum)                              
@��    �ܣ���������ķ��ͺ���
@��    �룺data  �������ݵ�ָ�룬dataNum ���ͳ���
@��    �������ͳɹ�����1��ʧ�ܷ���0
@��    ע�����ڶ��ţ�����������͸����Ϣ�ȷ���                                  
*****************************************************************/
u8 USB_EP1_Send(u8 *data,u32 dataNum)
{
	uint8_t send_time=0;
	uint8_t num=0;
	uint8_t i=0;

	if(dataNum > 64)   //����64�ֽ�
	{
		send_time = dataNum / 64;
		for(i=0;i<send_time;i++)
    { 
			UserToPMABufferCopy(data, ENDP1_TXADDR, 64);//�����ݿ��������ͻ�����
      SetEPTxCount(ENDP1, 64);
      SetEPTxValid(ENDP1);
      data += 64;
				
			WaitEp1InFlag(TIME_USB_SEND);//�ȴ�������ɱ�ʶλ
				
    }
    num = dataNum % 64;
		/*���һ�ε����ݷ���*/
    UserToPMABufferCopy(data, ENDP1_TXADDR, num);
    SetEPTxCount(ENDP1, num);
    SetEPTxValid(ENDP1); 
		WaitEp1InFlag(TIME_USB_SEND);
	}
	else 
	{
		UserToPMABufferCopy(data, ENDP1_TXADDR, dataNum);
    SetEPTxCount(ENDP1, dataNum);
    SetEPTxValid(ENDP1);
		WaitEp1InFlag(TIME_USB_SEND);
	}
	return 1;
}
	
/*****************************************************************
@�� �� ����USB_EP2_Rec(u8 *data,u32 dataNum)                              
@��    �ܣ���������Ľ��պ���
@��    �룺data  �������ݵ�ָ�룬dataNum ���ճ���
@��    �������ճɹ�����1��ʧ�ܷ���0
@��    ע�����ڶ��ţ�����������͸����Ϣ�Ƚ���                               
*****************************************************************/

u8 USB_EP2_Rec(u8 *data,u32 dataNum)
{
	uint16_t len=0;
	static uint32_t tmp=1;
	if(tmp) //ȷ��ִֻ��һ��
  {
		EP2_RevNum = dataNum;
		tmp=0;
  }
	if(EP2_RevFlag == 0)    //ָ���������ݽ������
  {
		for(len=0;len<dataNum;len++)
		{
			*data=USB_EP2_Receive_Buffer[len];  //��������
			data++;
			USB_EP2_Receive_Buffer[len]=0;
		}
		EP2_RevFlag = 1;
		tmp=1;
		return 1;   //��ɽ���
	}
	return 0;   //δ��ɽ���
}

/*****************************************************************
@�� �� ����USB_EP3_Send(u8 *data,u32 dataNum)                              
@��    �ܣ��жϴ���ķ��ͺ���
@��    �룺data  �������ݵ�ָ�룬dataNum ���ճ���
@��    �������ͳɹ�����1��ʧ�ܷ���0
@��    ע������ָ���С���ݽ���
*****************************************************************/

u8 USB_EP3_Send(u8 *data,u32 dataNum)
{
  
	UserToPMABufferCopy(data, ENDP3_TXADDR, dataNum);
    SetEPTxCount(ENDP3, dataNum);
    SetEPTxValid(ENDP3);
	return WaitEp3InFlag(TIME_USB_SEND);
}


/*****************************************************************
@�� �� ����USB_EP3_Rec(u8 *data,u32 dataNum)                              
@��    �ܣ��жϴ���Ľ��պ���
@��    �룺data  �������ݵ�ָ�룬dataNum ���ճ���
@��    �������ճɹ�����1��ʧ�ܷ���0
@��    ע������ָ���С���ݽ���
*****************************************************************/
u8 USB_EP3_Rec(u8 *data,u32 dataNum)
{
	uint16_t len=0;
	static uint32_t tmp=1;
	if(tmp) //ȷ��ִֻ��һ��
  {
		EP3_RevNum = dataNum;
		tmp=0;
  }
	if(EP3_RevFlag == 0)    //ָ���������ݽ������
  {
		for(len=0;len<dataNum;len++)
		{
			*data=USB_EP3_Receive_Buffer[len];  //��������
			data++;
			USB_EP3_Receive_Buffer[len]=0;
		}
		EP3_RevFlag = 1;
		tmp=1;
		return 1;   //��ɽ���
	}
	return 0;   //δ��ɽ���
}

/*****************************************************************
@�� �� ����WaitEp2InFlag(u16 time_ms)                              
@��    �ܣ��˵�2��ʱ�ȴ����ͺ���
@��    �룺time_ms ���ó�ʱʱ��
@��    �������ͳɹ�����1��ʧ�ܷ���0
@��    ע��                              
*****************************************************************/

u8 WaitEp2InFlag(u16 time_ms)
{
	TimeTemp = SysTime;
	while(1)
	{
		if((SysTime - TimeTemp) > time_ms * 10)
		{
			DebugOut("[fail] USB ep2 send\r\n");
			return 0;
		}
		if(Endp2InFlag)
		{
			Endp2InFlag = 0;
			DebugOut("USB ep2 send OK %d\r\n",(SysTime - TimeTemp));
			return 1;
		}
	}
}

/*****************************************************************
@�� �� ����WaitEp1InFlag(u16 time_ms)                              
@��    �ܣ��˵�1��ʱ�ȴ����ͺ���
@��    �룺time_ms ���ó�ʱʱ��
@��    �������ͳɹ�����1��ʧ�ܷ���0
@��    ע��                              
*****************************************************************/
u8 WaitEp1InFlag(u16 time_ms)
{
	TimeTemp = SysTime;
	while(1)
	{
		if((SysTime - TimeTemp) > time_ms * 10)
		{
			DebugOut("[fail] USB ep1 send\r\n");
			return 0;
		}
		if(Endp1InFlag)
		{
			Endp1InFlag = 0;
			return 1;
		}
	}
}


/*****************************************************************
@�� �� ����WaitEp3InFlag(u16 time_ms)                              
@��    �ܣ��˵㳬ʱ�ȴ����ͺ���
@��    �룺time_ms ���ó�ʱʱ��
@��    �������ͳɹ�����1��ʧ�ܷ���0
@��    ע��                              
*****************************************************************/

 u8 WaitEp3InFlag(u16 time_ms)
{
	TimeTemp = SysTime;
	while(1)
	{
		if((SysTime - TimeTemp) > time_ms * 10)
		{
			DebugOut("[fail] USB ep3 send\r\n");
			return 0;
		}
		if(Endp3InFlag)
		{
			Endp3InFlag = 0;
			DebugOut("USB ep3 send OK %d\r\n",(SysTime - TimeTemp));
			return 1;
		}
	}
}



