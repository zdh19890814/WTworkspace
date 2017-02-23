#ifndef  __WTP_H__
#define  __WTP_H__
#include "include.h" 
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef  short int16;

 typedef struct 
 {
	 u8 tos :5; 				//业务类型
	 u8 protocol :3;		//协议名称，000为卫通协议，001为LTE协议，010为自组网协议，100为管理控制协议
	 u8 req_flag:7; 		//请求标志         
	 u8 dir :1;					//传输方向，0为手机到背板，1为背板到手机
	 u16 length; 				//数据包总长度
	 u8 crc[2]; 				//校验和      
	 u8 data[USB_DATA_SIZE]; 	//数据包
 }usb_ProType;
 
void TaskHandle(void);
void usb_int(char* data);
typedef union 
{
	usb_ProType USB;
	u8 data[USB_DATA_SIZE + USB_HEAD_NUM];
}USB_Data;


extern USB_Data USB_DataRec;
extern USB_Data USB_DataSend;
//extern usb_ProType USB_Send;
 #endif
