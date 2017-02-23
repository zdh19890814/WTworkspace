#include "include.h"




void USB_VoiceSend(void)
{
	//if(VoiceCallFlag)
	{
		if(I2S_DMA_BuffRec_OK==TRUE)
		{
			I2S_DMA_BuffRec_OK=FALSE;
			
			if(I2S_DMA_BuffRec_NO==1)
			{
				USB_SendVoiceData(I2S_DMA_BuffRec2);
			}
			else
			{
				USB_SendVoiceData(I2S_DMA_BuffRec1);
			}
		}
	}
}


void SendBack(uint16_t* buf)
{ 
	if(I2S_DMA_BuffSend_NO == 1)
	{
		memcpy(I2S_DMA_BuffSend2, buf, 2 * I2S_SEND_SIZE);
	}
	else
	{
		memcpy(I2S_DMA_BuffSend1, buf, 2 * I2S_SEND_SIZE);
	}
}
void I2S_VoiceSendBack(void)
{
	//if(VoiceCallFlag)
	{
		if(I2S_DMA_BuffRec_OK==TRUE)
		{
			I2S_DMA_BuffRec_OK=FALSE;
			if(I2S_DMA_BuffRec_NO==1)
			{
				SendBack(I2S_DMA_BuffRec2);
			}
			else
			{
				SendBack(I2S_DMA_BuffRec1);
			}
		}
	}
}




/*****************************************************************
@�� �� ����I2S_SendVoice()                       
@��    �ܣ���USB�յ����������ݸ��Ƶ�I2S�������Է���
@��    �룺
@��    ������
@��    ע��                                    
*****************************************************************/
void I2S_SendVoice(void)
{
#ifdef I2S_HALF_BUFFER
		if(I2S_DMA_BuffSend_NO == 1)
		{
			memcpy((char *)I2S_DMA_BufferSend2, USB_DataRec.USB.data, 2 * I2S_SEND_SIZE);
			I2S_DMA_Buff2Send_OK = 0;
		}
		else
		{
			memcpy((char *)I2S_DMA_BufferSend1, USB_DataRec.USB.data, 2 * I2S_SEND_SIZE);
			I2S_DMA_Buff1Send_OK = 0;
		}
#else
		if(I2S_DMA_BuffSend_NO == 1)
		{
			memcpy((char *)I2S_DMA_BuffSend2, USB_DataRec.USB.data, 2 * I2S_SEND_SIZE);
			if(I2S_DMA_Buff1Send_OK)
			{
				DMA2_InitStructure.DMA_BufferSize = I2S_SEND_SIZE;
				DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend2;
				DMA_Init(DMA2_Channel2,&DMA2_InitStructure);
				I2S_DMA_BuffSend_NO = 0;
			}
			I2S_DMA_Buff2Send_OK = 0;
		}
		
		else
		{
			memcpy((char *)I2S_DMA_BuffSend1, USB_DataRec.USB.data, 2 * I2S_SEND_SIZE);
			if(I2S_DMA_Buff2Send_OK)
			{
				DMA2_InitStructure.DMA_BufferSize = I2S_SEND_SIZE;
				DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend1;
				DMA_Init(DMA2_Channel2,&DMA2_InitStructure);
				I2S_DMA_BuffSend_NO = 1;
			}
			I2S_DMA_Buff1Send_OK = 0;
		}
#endif
}

/*****************************************************************
@�� �� ����USB_RecData()                       
@��    �ܣ����ֻ�����USB����
@��    �룺
@��    ������
@��    ע��                                    
*****************************************************************/

void USB_RecData()
{
	if(USB_EP3_Rec(USB_DataRec.data,USB_EP3_REC_NUM + USB_HEAD_NUM))//�жϽ��գ�ָ�
	{
		if(USB_DataRec.USB.protocol != 0)
		{
			DebugOut("[error] ep3 interupt receive com: pro = %d, tos = %d\r\n",USB_DataRec.USB.protocol,USB_DataRec.USB.tos);
			USB_DataRec.USB.tos = 0;
		}
		else
		{
			DebugOut("ep3 interupt receive com: tos = %d\r\n",USB_DataRec.USB.tos);
			return;//�жϽ��յ�ָ����ز��ٽ�����������
		}
	}
	
	if(USB_EP2_Rec(USB_DataRec.data,USB_EP2_REC_NUM + USB_HEAD_NUM))//�������գ����������ţ�
	{
		if(USB_DataRec.USB.protocol != 0)
		{
			DebugOut("[error] ep2 bulk receive data: pro = %d, tos = %d , %d\r\n",USB_DataRec.USB.protocol,USB_DataRec.USB.tos,USB_DataRec.USB.data[0]);
			USB_DataRec.USB.tos = 0;
		}
		else
		{
			if(USB_DataRec.USB.tos == 5)
			{
				if(VoiceCallFlag == 0)
				{
					USB_DataRec.USB.tos = 0;
				}
			}
			else if(USB_DataRec.USB.tos == 2)
			{
//				DebugOut("tos = 0-------------\r\n");
//				DebugOut("ep2 bulk receive message task: %s\r\n",USB_DataRec.USB.data + 2);
//				delay_ms(1000);
//				DebugOut("message send \r\n");
//				USB_DataRec.USB.data[0]  = 2;
//				USB_EP1_Send(USB_DataRec.data, USB_EP1_SEND_NUM + USB_HEAD_NUM);
//				USB_DataRec.USB.tos = 0;
			}
			else
			{
				DebugOut("[error] ep2 bulk receive data: tos = %d\r\n",USB_DataRec.USB.tos);
				USB_DataRec.USB.tos = 0;
			}
		}
	}
}

/*****************************************************************
@�� �� ����USB_SendVoiceData(uint16_t* buf)                              
@��    �ܣ������������ݸ�USB�ֻ�
@��    �룺buf ��������ָ��
@��    ������
@��    ע��                                    
*****************************************************************/
void USB_SendVoiceData(uint16_t* buf)
{ 
//	int i;
	u8 *media_data = (u8 *)buf;

	USB_DataSend.USB.protocol = 0;
	USB_DataSend.USB.tos = 5;  //��������
	USB_DataSend.USB.length = USB_EP1_SEND_NUM;
	USB_DataSend.USB.dir = 1;
	USB_DataSend.USB.req_flag = 0;

	USB_DataSend.USB.crc[0] = 1;
  USB_DataSend.USB.crc[1] = 1;

	
	memcpy(USB_DataSend.USB.data, media_data, USB_EP1_SEND_NUM);
	USB_EP1_Send(USB_DataSend.data, USB_EP1_SEND_NUM + USB_HEAD_NUM);//�������ͣ�������
}

/*****************************************************************
@�� �� ����USB_SendCom(void)                                 
@��    �ܣ��� USB_DataSend ���͸�USB�ֻ�
@��    �룺��
@��    ������
@��    ע������֮ǰ��� USB_DataSend ��ֵ                                      
*****************************************************************/
void USB_SendCom()
{
	USB_DataSend.USB.protocol = 0;
	DebugOut("USB back:tos=%d, %2x %2x %2x %2x\r\n",USB_DataSend.USB.tos,USB_DataSend.USB.data[0],USB_DataSend.USB.data[1],USB_DataSend.USB.data[2],USB_DataSend.USB.data[3]);
	USB_EP3_Send(USB_DataSend.data, USB_EP3_REC_NUM + USB_HEAD_NUM);//�жϷ��ͣ�ָ�
	
	//DebugOut("USB back closed--------------\r\n");
}

/*****************************************************************
@�� �� ����USB_SendMes(u8 *mes)                                 
@��    �ܣ������ŷ��͸�USB�ֻ�
@��    �룺mes ��������ָ�� ������'\0'��β
@��    ������
@��    ע����                                       
*****************************************************************/
void USB_SendMes(u8 *mes)
{
	USB_DataSend.USB.protocol = 0;
	USB_DataSend.USB.tos = 2;
	USB_DataSend.USB.data[0] = 2;
	memcpy(USB_DataSend.USB.data + 2, (const char *)mes, strlen((const char *)mes) + 1);
	DebugOut("USB back:tos=%d, %2x mes:%s\r\n",USB_DataSend.USB.tos,USB_DataSend.USB.data[0],USB_DataSend.USB.data + 2);
	USB_EP1_Send(USB_DataSend.data, USB_EP1_SEND_NUM + USB_HEAD_NUM);//�������ͣ����ţ�
}

/*****************************************************************
@�� �� ����USB_VoiceSendBack(void)
@��    �ܣ�USB�������ݻػ�
@��    �룺��
@��    ������
@��    ע����                                       
*****************************************************************/
void USB_VoiceSendBack(void)
{
	DebugOut("%x",USB_DataRec.USB.data[3]);
	USB_EP1_Send(USB_DataRec.data, USB_EP2_REC_NUM + USB_HEAD_NUM);
	USB_DataRec.USB.tos = 0;
}



/*****************************************************************
@�� �� ����USB_VoiceSend1(void)
@��    �ܣ��ȴ�I2S���������ݲ�����USB�ֻ�
@��    �룺��
@��    ������
@��    ע����                                       
*****************************************************************/
void USB_VoiceSend1(void)
{
#ifdef I2S_HALF_BUFFER
	TimeTemp = SysTime;
	while(!I2S_DMA_BuffRec_OK)
	{
		if ((SysTime - TimeTemp) > 10000)
		{
			break;//��ʱ����
		}
	}
	I2S_DMA_BuffRec_OK = 0;
	//DMA1_Channel4->CNDTR = I2S_REC_SIZE;
	if(I2S_DMA_BuffRec_NO == 1)
	{
		USB_SendVoiceData(I2S_DMA_BufferRec2);
		I2S_DMA_Buff2Rec_OK = 0;
	}
	else
	{
		USB_SendVoiceData(I2S_DMA_BufferRec1);
		I2S_DMA_Buff1Rec_OK = 0;
	}
#else
	TimeTemp = SysTime;
	while(!I2S_DMA_BuffRec_OK)
	{
		if ((SysTime - TimeTemp) > 10000)
		{
			break;
		}
	}

	I2S_DMA_BuffRec_OK = FALSE;
	//DMA1_Channel4->CNDTR = I2S_REC_SIZE;
	if(I2S_DMA_BuffRec_NO == 1)
	{
		USB_SendVoiceData(I2S_DMA_BuffRec2);
	}
	else
	{
		USB_SendVoiceData(I2S_DMA_BuffRec1);
	}
#endif
}




//	
//	
//	
//	if(I2S_DMA_BuffRec_OK==TRUE)
//	{	
//		I2S_DMA_BuffRec_OK = FALSE;
////DMA1_Channel4->CNDTR = I2S_REC_SIZE;
////#ifdef GD32
////		DebugOut("<%3d+%d>",I2S_REC_SIZE -  DMA1_Channel4->CNDTR,I2S_DMA_BuffRec_NO);
////#else
////		DebugOut("<%3d+%d>",I2S_REC_SIZE -  DMA1_Channel5->CNDTR,I2S_DMA_BuffRec_NO);
////#endif
//		
//		
//		
//		
//		
//		if(I2S_DMA_BuffRec_NO==1)
//		{
//			USB_SendVoiceData(I2S_DMA_BuffRec2);
//		}
//		else
//		{
//			USB_SendVoiceData(I2S_DMA_BuffRec1);
//		}
//	}
//	else
//	{
//#ifdef GD32
//		//DebugOut("<%3d-%d>",I2S_REC_SIZE -  DMA1_Channel4->CNDTR,I2S_DMA_BuffRec_NO);
//		DMA1_InitStructure.DMA_BufferSize = I2S_REC_SIZE;
//		
//		if(I2S_DMA_BuffRec_NO == 0)
//		{
//			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec1;
//			DMA_Init(DMA1_Channel4,&DMA1_InitStructure);
//			I2S_DMA_BuffRec_NO = 1;
//			USB_SendVoiceData(I2S_DMA_BuffRec2);
//		}
//		else
//		{
//			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec2;
//			DMA_Init(DMA1_Channel4,&DMA1_InitStructure);
//			I2S_DMA_BuffRec_NO = 0;
//			USB_SendVoiceData(I2S_DMA_BuffRec1);
//		}
//#else
//		//DebugOut("<%3d-%d>",I2S_REC_SIZE -  DMA1_Channel5->CNDTR,I2S_DMA_BuffRec_NO);
//		DMA1_InitStructure.DMA_BufferSize = I2S_REC_SIZE;
//		
//		if(I2S_DMA_BuffRec_NO == 0)
//		{
//			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec1;
//			DMA_Init(DMA1_Channel5,&DMA1_InitStructure);
//			I2S_DMA_BuffRec_NO=1;
//			USB_SendVoiceData(I2S_DMA_BuffRec2);
//		}
//		else
//		{
//			
//			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec2;
//			DMA_Init(DMA1_Channel5,&DMA1_InitStructure);
//			I2S_DMA_BuffRec_NO = 0;
//			USB_SendVoiceData(I2S_DMA_BuffRec1);
//		}
//#endif		
//	}
	





//void USB_VoiceTest()
//{
//	if(USB_EP2_Rec(USB_DataRec.data,300))
//	{
//		DebugOut("%x",USB_DataRec.data[3]);
//		USB_EP1_Send(USB_DataRec.data, 300);
//		USB_DataRec.USB.tos = 0;
//	}
//}
//void USB_MessageTest()
//{
//	if(USB_EP2_Rec(USB_DataRec.data,61))
//	{
//		USB_DataRec.data[61] = 0;
//		DebugOut("Message Test %d\r\n",USB_DataRec.data[60]);
//		AT_MessageSend(21,USB_DataRec.data);
//		USB_DataRec.USB.tos = 0;
//	}
//}
