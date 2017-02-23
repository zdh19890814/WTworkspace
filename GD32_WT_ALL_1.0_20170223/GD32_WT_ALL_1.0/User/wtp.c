#include "include.h"
USB_Data USB_DataRec;
USB_Data USB_DataSend;

/*****************************************************************
@�� �� ����TaskHandle(void)                                 
@��    �ܣ�����wtpЭ�鴦����յ���USB����
@��    �룺��
@��    ������
@��    ע����                                       
*****************************************************************/
void TaskHandle(void)
{ 
	char flag = 0;
	char send_flag = 1;//Ӧ�����
	
	if(USB_DataRec.USB.tos)
	{ 
		switch (USB_DataRec.USB.tos)		 
    {
			case 1://����ҵ��
			{
				switch (USB_DataRec.USB.data[0])
				{
					case 1:flag = AT_VoiceCall(USB_DataRec.USB.data + 1);break;
					case 2:flag = AT_VoiceReceive();break;
					case 3:flag = AT_VoiceHangup();break;
					default:send_flag = 0;DebugOut("unknown call task:data[0] = %d\r\n",USB_DataRec.USB.data[0]);break;
				}
				USB_DataSend.USB.tos = USB_DataRec.USB.tos;
				USB_DataSend.USB.data[0] = USB_DataRec.USB.data[0];
				USB_DataSend.USB.data[1] = flag;
				break;
			}

			case 2://����ҵ��
			{
				switch (USB_DataRec.USB.data[0])
				{
					case 1:flag = AT_MessageSend(USB_DataRec.USB.data[1],USB_DataRec.USB.data + 2);break;
					default:send_flag = 0;DebugOut("unknown message task:data[0] = %d\r\n",USB_DataRec.USB.data[0]);break;
				}
				USB_DataSend.USB.tos = USB_DataRec.USB.tos;
				USB_DataSend.USB.data[0] = USB_DataRec.USB.data[0];
				USB_DataSend.USB.data[1] = flag;
				break;
			}

			case 3://��������ҵ��Ա
			{
				switch (USB_DataRec.USB.data[0])
				{
					case 1:flag = AT_RescueOpen();break;
					case 2:flag = AT_RescueClose();break;
					case 3:flag = AT_RescueStateSend();break;
					case 5:flag = AT_RescueSet(USB_DataRec.USB.data + 1);break;
					default:send_flag = 0;DebugOut("unknown rescue task:data[0] = %d\r\n",USB_DataRec.USB.data[0]);break;
				}
				USB_DataSend.USB.tos = USB_DataRec.USB.tos;
				USB_DataSend.USB.data[0] = USB_DataRec.USB.data[0];
				USB_DataSend.USB.data[1] = flag;
				break;
			}
			
			case 4://����ҵ��
			{
				switch (USB_DataRec.USB.data[0])
				{
					case 1:flag = BDS_SET_Pattern(USB_DataRec.USB.data[1]);break;
					case 2:flag = BDS_GET(USB_DataRec.USB.data[1]);memcpy(USB_DataSend.USB.data + 3, BDdata, strlen(BDdata) + 1);DebugOut("BD USB: %s\r\n",USB_DataSend.USB.data + 3);break;
					default:send_flag = 0;DebugOut("unknown BD task:data[0] = %d\r\n",USB_DataRec.USB.data[0]);break;
				}
				USB_DataSend.USB.tos = USB_DataRec.USB.tos;
				USB_DataSend.USB.data[0] = USB_DataRec.USB.data[0];
				USB_DataSend.USB.data[1] = USB_DataRec.USB.data[1];
				USB_DataSend.USB.data[2] = flag;
				break;
			}
			
			case 5://����ҵ��
			{
				LED1_T;
				I2S_SendVoice();//��USB���յ����������ݸ��Ƶ�I2S�����з���
				USB_VoiceSend1();

				//USB_VoiceSendBack();
				//DebugOut("%x",USB_DataRec.USB.data[3]);
				send_flag = 0;//����
				break;
			}

			case 25://����
			{
				switch (USB_DataRec.USB.data[0])
				{
					case 1:send_flag = 0;AT_SendComReq(USB_DataRec.USB.data + 1);break;
					case 2:send_flag = 0;BDS_ComSendWithEnd(USB_DataRec.USB.data + 1);break;
					default:send_flag = 0;DebugOut("unknown debug task:data[0] = %d\r\n",USB_DataRec.USB.data[0]);break;
				}
				break;
			}
			
			
			case 26://��ʼ��
			{
				switch (USB_DataRec.USB.data[0])
				{
					case 1:AT_OpenSucceed = AT_Init();flag = AT_OpenSucceed;break;
					case 2:PW_AT_ON;flag = 1;DebugOut("AT ON\r\n");break;
					case 3:PW_AT_OFF;flag = 1;DebugOut("AT OFF\r\n");break;
					case 4:PW_BD_ON;flag = 1;DebugOut("BD ON\r\n");break;
					case 5:PW_BD_OFF;flag = 1;DebugOut("BD OFF\r\n");break;
					case 6:BDS_TouchuanFlag = 1;flag = 1;DebugOut("BD touchuan ON\r\n");break;
					case 7:BDS_TouchuanFlag = 0;flag = 1;DebugOut("BD touchuan OFF\r\n");break;
					default:send_flag = 0;DebugOut("unknown Init task:data[0] = %d\r\n",USB_DataRec.USB.data[0]);break;
				}
				USB_DataSend.USB.tos = USB_DataRec.USB.tos;
				USB_DataSend.USB.req_flag = USB_DataRec.USB.req_flag;
				USB_DataSend.USB.data[0] = USB_DataRec.USB.data[0];
				USB_DataSend.USB.data[1] = flag;
				break;
			}
			
			default:
			{
				send_flag = 0;DebugOut("unknown debug task:tos = %d\r\n",USB_DataRec.USB.tos);
				break;
			}
	  }
		
		if(send_flag == 1)
		{
			USB_SendCom();
		}
		USB_DataRec.USB.tos = 0;
	}
}
