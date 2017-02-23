

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_desc.h"
#include "usb_lib.h"
#include "usb_endp.h"
#include "usbio.h"
#include "stdio.h"
#include "delay.h"
#include "include.h"


uint16_t EP3_RevNum=0;  //EP3接收数据长度
uint16_t EP2_RevNum=0;  //EP2接收数据长度
uint8_t EP2_RevFlag=1;  //EP2端点数据接收完成标志
uint8_t EP1_RevFlag=1;  //EP1端点数据接收完成标志
uint8_t EP3_RevFlag=1;  //EP1端点数据接收完成标志

  

/*****************************************************************
@函 数 名：USB_EP1_Send(u8 *data,u32 dataNum)                              
@功    能：批量传输的发送函数
@输    入：data  发送数据的指针，dataNum 发送长度
@输    出：发送成功返回1，失败返回0
@备    注：用于短信，语音，北斗透传信息等发送                                  
*****************************************************************/
u8 USB_EP1_Send(u8 *data,u32 dataNum)
{
	uint8_t send_time=0;
	uint8_t num=0;
	uint8_t i=0;

	if(dataNum > 64)   //大于64字节
	{
		send_time = dataNum / 64;
		for(i=0;i<send_time;i++)
    { 
			UserToPMABufferCopy(data, ENDP1_TXADDR, 64);//将数据拷贝到发送缓冲区
      SetEPTxCount(ENDP1, 64);
      SetEPTxValid(ENDP1);
      data += 64;
				
			WaitEp1InFlag(TIME_USB_SEND);//等待发送完成标识位
				
    }
    num = dataNum % 64;
		/*最后一次的数据发送*/
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
@函 数 名：USB_EP2_Rec(u8 *data,u32 dataNum)                              
@功    能：批量传输的接收函数
@输    入：data  接收数据的指针，dataNum 接收长度
@输    出：接收成功返回1，失败返回0
@备    注：用于短信，语音，北斗透传信息等接收                               
*****************************************************************/

u8 USB_EP2_Rec(u8 *data,u32 dataNum)
{
	uint16_t len=0;
	static uint32_t tmp=1;
	if(tmp) //确保只执行一次
  {
		EP2_RevNum = dataNum;
		tmp=0;
  }
	if(EP2_RevFlag == 0)    //指定长度数据接收完成
  {
		for(len=0;len<dataNum;len++)
		{
			*data=USB_EP2_Receive_Buffer[len];  //拷贝数据
			data++;
			USB_EP2_Receive_Buffer[len]=0;
		}
		EP2_RevFlag = 1;
		tmp=1;
		return 1;   //完成接收
	}
	return 0;   //未完成接收
}

/*****************************************************************
@函 数 名：USB_EP3_Send(u8 *data,u32 dataNum)                              
@功    能：中断传输的发送函数
@输    入：data  接收数据的指针，dataNum 接收长度
@输    出：发送成功返回1，失败返回0
@备    注：用于指令等小数据接收
*****************************************************************/

u8 USB_EP3_Send(u8 *data,u32 dataNum)
{
  
	UserToPMABufferCopy(data, ENDP3_TXADDR, dataNum);
    SetEPTxCount(ENDP3, dataNum);
    SetEPTxValid(ENDP3);
	return WaitEp3InFlag(TIME_USB_SEND);
}


/*****************************************************************
@函 数 名：USB_EP3_Rec(u8 *data,u32 dataNum)                              
@功    能：中断传输的接收函数
@输    入：data  接收数据的指针，dataNum 接收长度
@输    出：接收成功返回1，失败返回0
@备    注：用于指令等小数据接收
*****************************************************************/
u8 USB_EP3_Rec(u8 *data,u32 dataNum)
{
	uint16_t len=0;
	static uint32_t tmp=1;
	if(tmp) //确保只执行一次
  {
		EP3_RevNum = dataNum;
		tmp=0;
  }
	if(EP3_RevFlag == 0)    //指定长度数据接收完成
  {
		for(len=0;len<dataNum;len++)
		{
			*data=USB_EP3_Receive_Buffer[len];  //拷贝数据
			data++;
			USB_EP3_Receive_Buffer[len]=0;
		}
		EP3_RevFlag = 1;
		tmp=1;
		return 1;   //完成接收
	}
	return 0;   //未完成接收
}

/*****************************************************************
@函 数 名：WaitEp2InFlag(u16 time_ms)                              
@功    能：端点2超时等待发送函数
@输    入：time_ms 设置超时时间
@输    出：发送成功返回1，失败返回0
@备    注：                              
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
@函 数 名：WaitEp1InFlag(u16 time_ms)                              
@功    能：端点1超时等待发送函数
@输    入：time_ms 设置超时时间
@输    出：发送成功返回1，失败返回0
@备    注：                              
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
@函 数 名：WaitEp3InFlag(u16 time_ms)                              
@功    能：端点超时等待发送函数
@输    入：time_ms 设置超时时间
@输    出：发送成功返回1，失败返回0
@备    注：                              
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



