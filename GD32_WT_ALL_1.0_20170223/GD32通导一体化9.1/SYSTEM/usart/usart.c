#include "include.h"

void at_buffer_handler(u8 *pAtcStrBuff, u16 usAtcStrLen);



u16 UartDebug_Cnt=0;
u8 UartDebug_R_Buf[MAX_UART_DEBUG_BUFF_SIZE]={0};


u16 BuffIndex = 0;
u8 RecBuff_1[MAX_REC_BUFF_SIZE + 1];
u8 RecBuff_2[MAX_REC_BUFF_SIZE + 1];	


u8 readBufFirst_TTL[MAX_AT_COM_BUF_SIZE] = { 0 };
//char readBufSecond_TTL[MAX_AT_MSM_BUF_SIZE] = { 0 };
/*
ComBufΪѭ��buffer��MsmBuf[MsmBufIndexStart]Ϊ��������ݣ������ȱ����������MsmBufIndexStart�Լ�һ��MsmBuf[MsmBufIndexEnd-1]Ϊ�������ݣ����յ������ݺ�MsmBufIndexEnd�Լ�һ
*/
u8 ComBuf[MAX_AT_COM_BUF_NUM][MAX_AT_COM_BUF_SIZE];

//u8 MsmBuf[MAX_AT_MSM_BUF_NUM][MAX_AT_MSM_BUF_SIZE];
//int MsmBufIndexStart = 0;
//int MsmBufIndexEnd = 0;
int ComBufIndexStart = 0;
int ComBufIndexEnd = 0;
char readBuf_Flag = 0;
char readSms_Flag = 0;
int lenAtRspFirst = 0;
//int lenAtRspSecond = 0;


u8 AT_MuxBuff[MAX_AT_MUX_BUF_NUM][MAX_AT_MUX_BUF_SIZE + 2];//ͷ�����ֽڱ�ʾ����
int MuxBufIndexStart = 0;
int MuxBufIndexEnd = 0;



u8 VoiceCallFlag = 0;

u8 ResponseBuff[MAX_AT_RESPONSE_BUF_NUM][MAX_AT_COM_BUF_SIZE];
int ResponseBufIndex = 0;

#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 



#ifdef GD32
int fputc(int ch, FILE *f)
{      
	while((UART4->SR&0X40)==0);//ѭ������,ֱ���������   
    UART4->DR = (u8) ch;      
	return ch;
}
#else
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif

#endif 

/*****************************************************************
@�� �� ����DebugUartInit(void)                                 
@��    �ܣ����Դ��ڳ�ʼ��
@��    �룺��
@��    ������
@��    ע�����ݺ궨�����ѡ�񴮿ڳ�ʼ��                                                      
*****************************************************************/

void DebugUartInit()
{
#ifdef	DEBUG_PRINT 
	
	#ifdef GD32
		UART4_Init();
	#else
		USART1_Init();
	#endif
	
#else
	
	#ifdef	DEBUG_INPUT
		
		#ifdef GD32
			UART4_Init();
		#else
			USART1_Init();
		#endif
	
	#endif
	
#endif
}

void USART1_Init(void)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	USART_InitTypeDef  USART_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;//���ڵ�IO��ʼ��
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA,&GPIO_InitStruct); //TX	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA,&GPIO_InitStruct); //RX	

	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure);
				
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1,ENABLE);
	
}

void UART4_Init(void)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	USART_InitTypeDef  USART_InitStructure;
	NVIC_InitTypeDef 	 NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);// RCC_APB2PeriphClockCmd(USART_RCC,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 , ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;//���ڵ�IO��ʼ��
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOC,&GPIO_InitStruct); //TX	GPIO_Pin_9
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOC,&GPIO_InitStruct); //RX	GPIO_Pin_10

	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4,&USART_InitStructure);
    
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//USART4 �����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
				
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART4,ENABLE);
	
}




/*****************************************************************
@�� �� ����TickInit(void)                                 
@��    �ܣ�Tickʱ�ӳ�ʼ��  
@��    �룺��
@��    ������
@��    ע��ÿ100us��tick�ж�SysTick_Handler()�� SysTime��һ��
					 Ϊ����ṩʱ���                                                             
*****************************************************************/
void TickInit(void)
{
	SysTick->LOAD  = (900 & SysTick_LOAD_RELOAD_Msk) - 1;        /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Cortex-M0 System Interrupts ������ȼ�*/
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = 
                   SysTick_CTRL_TICKINT_Msk   | 
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
}


/*****************************************************************
@�� �� ����NVIC_AllInit(void)                                 
@��    �ܣ������ж����ȼ�
@��    �룺��
@��    ������
@��    ע����                                                 
*****************************************************************/
void NVIC_AllInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//0-3,0-3

	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//UART2 �����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;//USART2 DMA �����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	

#ifdef GD32
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//USART1 �����ж�
#else
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//USART1 �����ж�
#endif
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
	
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
#ifdef	DEBUG_INPUT
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
#endif
	
	NVIC_Init(&NVIC_InitStructure);

	

	
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;   //USB�ж�Դ����
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}
	
/*****************************************************************
@�� �� ����USART2_DMA_Init(void)                                 
@��    �ܣ�ATģ�鴮�ڳ�ʼ��
@��    �룺��
@��    ������
@��    ע����                                                      
*****************************************************************/
void USART2_DMA_Init(void)
{
 GPIO_InitTypeDef   GPIO_InitStruct;
	USART_InitTypeDef  USART_InitStructure;
	DMA_InitTypeDef    DMA_Initstructure;



	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;//��??��2��?IO3?��??��
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA,&GPIO_InitStruct); //TX	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA,&GPIO_InitStruct); //RX		
 
	/*RTS,CTS*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;//
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA,&GPIO_InitStruct); //RTS
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA,&GPIO_InitStruct); //CTS	
	
	
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	//USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2,&USART_InitStructure);
		

	
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

	USART_Cmd(USART2,ENABLE);
	
	
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	DMA_Initstructure.DMA_PeripheralBaseAddr =  (u32)(&USART2->DR);;
	DMA_Initstructure.DMA_MemoryBaseAddr     = (u32)RecBuff_1;
	DMA_Initstructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Initstructure.DMA_BufferSize = MAX_REC_BUFF_SIZE;
	DMA_Initstructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Initstructure.DMA_MemoryInc =DMA_MemoryInc_Enable;
	DMA_Initstructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_Initstructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_Initstructure.DMA_Mode = DMA_Mode_Normal;
	DMA_Initstructure.DMA_Priority = DMA_Priority_High;
	DMA_Initstructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6,&DMA_Initstructure);
	DMA_Cmd(DMA1_Channel6,ENABLE);
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); 

}


u8 strStartsWith(u8 *line, const u8 *prefix)
{
	for (; *line != '\0' && *prefix != '\0'; line++, prefix++) {
		if (*line != *prefix) {
			return 0;
		}
	}
	return *prefix == '\0';
}




//char strStartsWith(const char *line, const char *prefix)
//{
//	for (; *line != '\0' && *prefix != '\0'; line++, prefix++) {
//		if (*line != *prefix) {
//			return 0;
//		}
//	}
//	return *prefix == '\0';
//}

const u8 * s_smsUnsoliciteds[] = {
	"+CMT:",
	"+CDS:",
	"+CBM:"
};



//int isSMSUnsolicited(const char *line)
//{
//	size_t i;

////	if (strStartsWith(line, "+CMT:\"HEX\""))
////		return 0;

//	for (i = 0; i < NUM_ELEMS(s_smsUnsoliciteds); i++) {
//		if (strStartsWith(line, s_smsUnsoliciteds[i])) {
//			return 1;
//		}
//	}

//	return 0;
//}

u8 isSMSUnsolicited(u8 *line)
{
	size_t i;

//	if (strStartsWith(line, "+CMT:\"HEX\""))
//		return 0;

	for (i = 0; i < NUM_ELEMS(s_smsUnsoliciteds); i++) {
		if (strStartsWith(line, s_smsUnsoliciteds[i])) {
			return 1;
		}
	}

	return 0;
}


void AT_MuxBuffHandler(u8 *Buff,u16 len)
{
	int i = 0;
	static int MuxBufCounter = 0;
	static u8 MuxState = 0;
	static u16 lenth = 0;
	static u8 MuxBufIndex = 0;
	static u8 num = 0;
	static u8 msm_flag = 0;
	
	for (i = 0; i < len; i++)
	{
		//DebugOut("Buff[%d]=%4d,State=%d,Count=%d,ComBuf[%d]=%d\r\n",i,Buff[i],MuxState,MuxBufCounter,MuxBufIndex,ComBuf[ComBufIndexEnd + num][MuxBufIndex-1]);
		switch(MuxState)
		{
			//---------
			case 0://�ȴ�0xf9ת״̬1
			{
				if(Buff[i] == 0xf9)
				{
					MuxState = 1;
				}
				break;
			}
			//---------
			case 1://�ȴ���f9 ��ȡͨ���ţ�ת״̬2
			{
				if(Buff[i] == 0xf9)
				{
					;
				}
				else
				{
					if(1)//ͨ����
					{
						//pBuff = ComBuf[ComBufIndexEnd + num];
						num = 0;
						MuxState = 2;
					}
				}
				break;
			}
			//---------
			case 2://����һ�ֽ�
			{
				MuxState = 3;
				break;
			}
			//---------
			case 3://�жϳ����м����ֽڱ�ʾ�����ֽڶ�ȡ�ֽ�����ֱ��ת5��˫�ֽ�ת4
			{
				lenth = Buff[i] >> 1;
				if(Buff[i] & 0x01)
				{
					MuxState = 5;
				}
				else
				{
					MuxState = 4;
				}	
				break;
			}
			
			//---------
			case 4://˫�ֽڣ���ȡ�ֽ�����ת5
			{
				lenth = (((u16)Buff[i]) << 7) | lenth;
				MuxState = 5;
				break;
			}
			
			
			//---------
			case 5://'\r'
			{
			//	DebugOut("lenth=%d\r\n",lenth);
				if(Buff[i] == '\r')
				{
					MuxState = 6;
				}
				else
				{
					MuxState = 0;
				}
				break;
			}
			
			//---------
			case 6://'\n'��ʼ��������
			{
				if(Buff[i] == '\n')
				{
					MuxState = 7;
					MuxBufCounter = 0;
					MuxBufIndex = 0;
					msm_flag = 0;
				}
				else
				{
					MuxState = 0;
				}
				break;
			}
			//---------
			case 7://�������ݣ����ȹ���ת״̬0������'\r'ת״̬8
			{
				ComBuf[ComBufIndexEnd + num][MuxBufIndex++] = Buff[i];
				MuxBufCounter++;
				
				if(Buff[i] == '\r')
				{
					if(isSMSUnsolicited(ComBuf[ComBufIndexEnd + num]))
					{
						if(msm_flag == 0)
						{
							msm_flag = 1;
						}
						else
						{
							MuxState = 8;
						}
					}
					else
					{
						MuxState = 8;
					}
					
				}
				if(MuxBufIndex == 2)
				{
					if(ComBuf[ComBufIndexEnd + num][0] == '>')
						if(ComBuf[ComBufIndexEnd + num][1] == ' ')
						{
							ComBuf[ComBufIndexEnd + num][MuxBufIndex] = 0;
							num++;
							msm_flag = 0;
							MuxBufIndex = 0;					
							MuxState = 9;
						}
				}
				if(lenth < MuxBufCounter + 2)
				{
					DebugOut("[error] mux lenth\r\n");
					MuxState = 0;
				}
				
				//DebugOut("state:%d, ComBuf[%d][%d]=%d\r\n",MuxState,ComBufIndexEnd + num,MuxBufIndex-1,ComBuf[ComBufIndexEnd + num][MuxBufIndex-1]);
				break;
			}
			//---------
			case 8://����'\n'���һ֡
			{
				if(Buff[i] == '\n')
				{
					ComBuf[ComBufIndexEnd + num][MuxBufIndex - 1] = 0;
					MuxBufCounter++;
					num++;
					MuxBufIndex = 0;	
					msm_flag = 0;
					MuxState = 9;
				}
				else
				{
					MuxState = 0;
				}
				break;
			}
			//---------
			case 9:
			{
				ComBuf[ComBufIndexEnd + num][MuxBufIndex++] = Buff[i];
				MuxBufCounter++;
				
				
				MuxState = 10;
				break;
			}
			//---------
			case 10:
			{
			//DebugOut("state:%d, %d == %d\r\n",MuxState,lenth,MuxBufCounter + 1);
				if((Buff[i] == '\n')&&(ComBuf[ComBufIndexEnd + num][MuxBufIndex - 1] == '\r'))
				{
					MuxBufIndex = 0;
					ComBuf[ComBufIndexEnd + num][MuxBufIndex - 1] = 0;
					MuxBufCounter ++;
					MuxState = 7;
				}
				else if((Buff[i] == 0xf9)&&(lenth == MuxBufCounter + 1))
				{
					ComBufIndexEndPlusN(num);
					MuxState = 0;
				}
				else
				{
					MuxState = 0;
				}
				break;
			}
			
			
		}
	}
}


































#if 1
void at_buffer_handler(u8 *pAtcStrBuff, u16 usAtcStrLen)
{
	int i = 0;
	if (usAtcStrLen > 0)
	{
		for (i = 0; i<usAtcStrLen; i++)
		{

			if (TTL_COM_RX_FLAG0 == readBuf_Flag)
			{
				if ('\r' == pAtcStrBuff[i])
				{
					readBuf_Flag = TTL_COM_RX_FLAG1;
					readSms_Flag = 0;
				}
			}
			
			
			
			
			
			
			else if (TTL_COM_RX_FLAG1 == readBuf_Flag)
			{
				if ('\n' == pAtcStrBuff[i])
				{
					readBuf_Flag = TTL_COM_RX_FLAG2;
				}
			}
			
			
			
			
			
			
			
			
			else if (TTL_COM_RX_FLAG2 == readBuf_Flag)
			{
				if ('\r' != pAtcStrBuff[i])
				{
					readBufFirst_TTL[lenAtRspFirst++] = pAtcStrBuff[i];
					if((pAtcStrBuff[i] == '>')&&(lenAtRspFirst == 1))
					{
						readBuf_Flag = TTL_COM_RX_FLAG3;
					}
				}
				else
				{
					readBufFirst_TTL[lenAtRspFirst++] = pAtcStrBuff[i];
					if (0 == lenAtRspFirst)
					{
						readBuf_Flag = TTL_COM_RX_FLAG1;
					}
					else
					{
						readBuf_Flag = TTL_COM_RX_FLAG3;
					}
				}
			}
			
			
			
			
			
			
			
			
			else if (TTL_COM_RX_FLAG3 == readBuf_Flag)
			{
				if ('\n' == pAtcStrBuff[i])
				{
					readBufFirst_TTL[lenAtRspFirst++] = pAtcStrBuff[i];
					
					if (0 == readSms_Flag)
					{
						if (isSMSUnsolicited(readBufFirst_TTL))
						{
							readSms_Flag = 1;
							readBuf_Flag = TTL_COM_RX_FLAG2;
						}
						else
						{
							readBufFirst_TTL[lenAtRspFirst-2] = '\0';
							memcpy(ComBuf[ComBufIndexEnd], (const char *)readBufFirst_TTL, strlen((const char *)readBufFirst_TTL) + 1);
							ComBufIndexPlusPlus(ComBufIndexEnd);

							lenAtRspFirst = 0;
							readBuf_Flag = TTL_COM_RX_FLAG0;
						}
					}
					else
					{
						readBufFirst_TTL[lenAtRspFirst-2] = '\0';
						memcpy(ComBuf[ComBufIndexEnd], (const char *)readBufFirst_TTL, strlen((const char *)readBufFirst_TTL) + 1);
						ComBufIndexPlusPlus(ComBufIndexEnd);

						lenAtRspFirst = 0;
						readBuf_Flag = TTL_COM_RX_FLAG0;
					}
				}
				else if((pAtcStrBuff[i] == ' ')&&(readBufFirst_TTL[0] == '>')&&(lenAtRspFirst == 1))
				{
					readBufFirst_TTL[1] = ' ';
					readBufFirst_TTL[2] = '\0';
					memcpy(ComBuf[ComBufIndexEnd], (const char *)readBufFirst_TTL, strlen((const char *)readBufFirst_TTL) + 1);
					ComBufIndexPlusPlus(ComBufIndexEnd);

					lenAtRspFirst = 0;
					readBuf_Flag = TTL_COM_RX_FLAG0;
				}
			}

		}
	}
	return;
}

#else

void at_buffer_handler(u8 *pAtcStrBuff, u16 usAtcStrLen)
{
	int i = 0;
	if (usAtcStrLen > 0)
	{
		for (i = 0; i<usAtcStrLen; i++)
		{
			if (TTL_COM_RX_FLAG0 == readBuf_Flag)
			{
				if ('\r' == pAtcStrBuff[i])
				{
					readBuf_Flag = TTL_COM_RX_FLAG1;
				}
			}
			else if (TTL_COM_RX_FLAG1 == readBuf_Flag)
			{
				if ('\n' == pAtcStrBuff[i])
				{
					readBuf_Flag = TTL_COM_RX_FLAG2;
				}
			}
			else if (TTL_COM_RX_FLAG2 == readBuf_Flag)
			{
				if ('\r' != pAtcStrBuff[i])
				{
					if (0 == readSms_Flag)
					{
						readBufFirst_TTL[lenAtRspFirst++] = pAtcStrBuff[i];
						if((pAtcStrBuff[i] == '>')&&(lenAtRspFirst == 1))
						{
							readBuf_Flag = TTL_COM_RX_FLAG3;
						}
					}
					else
					{
						readBufSecond_TTL[lenAtRspSecond++] = pAtcStrBuff[i];
					}
				}
				else
				{
					if (0 == readSms_Flag)
					{
						if (0 == lenAtRspFirst)
						{
							readBuf_Flag = TTL_COM_RX_FLAG1;
						}
						else
						{
							readBuf_Flag = TTL_COM_RX_FLAG3;
						}
					}
					else
					{
						readBufSecond_TTL[lenAtRspSecond] = '\0';
						readBuf_Flag = TTL_COM_RX_FLAG3;
					}

				}
			}
			else if (TTL_COM_RX_FLAG3 == readBuf_Flag)
			{
				if ('\n' == pAtcStrBuff[i])
				{
					if (0 == readSms_Flag)
					{
						readBufFirst_TTL[lenAtRspFirst] = '\0';

						if (isSMSUnsolicited(readBufFirst_TTL))
						{
							
							readSms_Flag = 1;
							readBuf_Flag = TTL_COM_RX_FLAG2;
							lenAtRspSecond = 0;
						}
						else
						{
	
							memcpy(ComBuf[ComBufIndexEnd], readBufFirst_TTL, strlen(readBufFirst_TTL) + 1);
							ComBufIndexPlusPlus(ComBufIndexEnd);

							lenAtRspFirst = 0;
							readBuf_Flag = TTL_COM_RX_FLAG0;
						}
					}
					else
					{
						memcpy(MsmBuf[MsmBufIndexEnd], readBufSecond_TTL, strlen(readBufSecond_TTL) + 1);
						MsmBufIndexPlusPlus(MsmBufIndexEnd);

						memcpy(ComBuf[ComBufIndexEnd], readBufFirst_TTL, strlen(readBufFirst_TTL) + 1);
						ComBufIndexPlusPlus(ComBufIndexEnd);
						
						readSms_Flag = 0;
						readBuf_Flag = TTL_COM_RX_FLAG0;

						lenAtRspFirst = 0;
						lenAtRspSecond = 0;
						
						//readBufFirst_TTL[0] = '/0';
						//readBufSecond_TTL[0] = '/0';
					}
				}
				else if((pAtcStrBuff[i] == ' ')&&(readBufFirst_TTL[0] == '>')&&(lenAtRspFirst == 1))
				{
					readBufFirst_TTL[1] = ' ';
					readBufFirst_TTL[2] = '\0';
					memcpy(ComBuf[ComBufIndexEnd], readBufFirst_TTL, strlen(readBufFirst_TTL) + 1);
					ComBufIndexPlusPlus(ComBufIndexEnd);

					lenAtRspFirst = 0;
					readBuf_Flag = TTL_COM_RX_FLAG0;
				}
			}

		}
	}
	return;
}
#endif

//void USART1SendStr(char * str)
//{
//	while((*str++)!='\0')
//	{
//		USART_SendData(USART1, *(str-1));
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
//	}
//}


//void USART1SendStrWithEnd(char * str)
//{
//	while((*str++)!='\0')
//	{
//		USART_SendData(USART1, *(str-1));
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
//	}
//	USART_SendData(USART1,'\r');
//	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
//	USART_SendData(USART1,'\n');
//	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
//}
u8 AT_SendDirect(u8 * str)
{
	return USART2SendStrWithEnd(str);
}

u8 USART2SendStrWithEnd(u8 * str)
{
	TimeTemp = SysTime;
	while((*str++)!='\0')
	{
		USART_SendData(USART2, *(str-1));
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
		WaitTimeOut(FLOW_CONTROL_TIME);
	}
	USART_SendData(USART2,'\r');
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
	WaitTimeOut(FLOW_CONTROL_TIME);

	USART_SendData(USART2,'\n');
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
	WaitTimeOut(FLOW_CONTROL_TIME);
	return 1;
}

u8 AT_SendData(u8 *pData, u16 len)
{
	return USART2SendData(pData,len);
}

u8 USART2SendData(u8 *pData, u16 len)
{
  u16 i;
	TimeTemp = SysTime;
	if(len > 0)
  {
		for(i = 0; i < len; i ++)
		{
			USART_SendData(USART2,pData[i]);
			while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
			WaitTimeOut(FLOW_CONTROL_TIME);
		}
  }
   return 1;
}

u8 AT_SendDataWithEnd(u8 *pData, u16 len)
{
	return USART2SendDataWithEnd(pData,len);
}

u8 USART2SendDataWithEnd(u8 *pData, u16 len)//u8 AT_SendDataWithEnd(u8 *pData, u16 len)
{
  u16 i;
	TimeTemp = SysTime;
	if(len > 0)
  {
		for(i = 0; i < len; i ++)
		{
			USART_SendData(USART2,pData[i]);
			while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
			WaitTimeOut(FLOW_CONTROL_TIME);
		}
  }
  USART_SendData(USART2,'\r');
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
	WaitTimeOut(FLOW_CONTROL_TIME);

	USART_SendData(USART2,'\n');
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)
	WaitTimeOut(FLOW_CONTROL_TIME);
	return 1;
}


void DebugPrintData(u8 *pData, u16 len)
{
#ifdef	DEBUG_PRINT
  u16 i;

	TimeTemp = SysTime;
	if(len > 0)
  {
		for(i = 0; i < len; i ++)
		{
			//USART_SendData(USART2,pData[i]);
			//while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET)//��ͬ����
			DebugOut("%c",pData[i]);
		}
  }
  return;
#endif
}







void USART3SendStrWithEnd(u8 * str)//stm32 ����
{
	while((*str++)!='\0')
	{
		USART_SendData(USART3, *(str-1));
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
	}
	USART_SendData(USART3,'\r');
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
	USART_SendData(USART3,'\n');
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
}


void USART5SendStrWithEnd(u8 * str)//gd32 ����
{
	while((*str++)!='\0')
	{
		USART_SendData(UART5, *(str-1));
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
	}
	USART_SendData(UART5,'\r');
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
	USART_SendData(UART5,'\n');
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
}




//void USART2SendStr(char * str)
//{
//	while((*str++)!='\0')
//	{
//		USART_SendData(USART2, *(str-1));
//		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
//	}
//	USART_SendData(USART2,'\r');
//	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
//	USART_SendData(USART2,'\n');
//	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
//}

//u8 Uart2_RecBuff[Uart2_MAX_REC_BUFF_SIZE + 1] = { '\0' }; //��??��2��??o��?
//char Uart2_MsmBuf[Uart2_MAX_REC_BUFF_SIZE + 1];//usart2ת��buffer


void printf_no(char* fmt, ...) {}

