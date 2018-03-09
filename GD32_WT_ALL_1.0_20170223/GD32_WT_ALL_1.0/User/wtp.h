#ifndef  __WTP_H__
#define  __WTP_H__
#include "include.h" 
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef  short int16;

 typedef struct 
 {
	 u8 tos :5; 				//ҵ������
	 u8 protocol :3;		//Э�����ƣ�000Ϊ��ͨЭ�飬001ΪLTEЭ�飬010Ϊ������Э�飬100Ϊ�������Э��
	 u8 req_flag:7; 		//�����־         
	 u8 dir :1;					//���䷽��0Ϊ�ֻ������壬1Ϊ���嵽�ֻ�
	 u16 length; 				//���ݰ��ܳ���
	 u8 crc[2]; 				//У���      
	 u8 data[USB_DATA_SIZE]; 	//���ݰ�
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
