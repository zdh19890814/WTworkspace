
#include "include.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/******************************************************************************/
/*            			Cortex-M3 ����������ط�����                        */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	DebugOut("NMI\r\n");
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	DebugOut("HardFault\r\n");
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  DebugOut("MemManage_Handler\r\n");
	/* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  DebugOut("BusFault_Handler\r\n");
	/* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  DebugOut("UsageFault_Handler\r\n");
	/* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	DebugOut("SVC_Handler\r\n");
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	DebugOut("DebugMon_Handler\r\n");
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	DebugOut("PendSV_Handler\r\n");
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	SysTime++;//4.97�����һ��
}


#ifdef	DEBUG_INPUT
/*����1�ʹ���4�ǵ��Դ���*/
void USART1_IRQHandler(void)
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //?����??D??(?����?��?��?��y?Y��?D?��?0x0d 0x0a?��?2)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//?����??����?��?��?��y?Y
		
		if(Res=='\n')
		{
			UartDebug_R_Buf[UartDebug_Cnt-1] = '\0';
			//TestComConvert(UartDebug_R_Buf);
			//memcpy(USB_DataRec.USB.data,UartDebug_R_Buf + 2,UartDebug_Cnt - 2);//daxiao
			
			DebugComConvert(UartDebug_R_Buf);
			memcpy(USB_DataRec.USB.data + 1,UartDebug_R_Buf + 2,UartDebug_Cnt - 2);
			UartDebug_Cnt = 0;
		}
		else
		{
			UartDebug_R_Buf[UartDebug_Cnt++]=Res;
			if(UartDebug_Cnt >= MAX_UART_DEBUG_BUFF_SIZE)
			{
				UartDebug_Cnt = MAX_UART_DEBUG_BUFF_SIZE - 1;
			}
		}
	}		
} 


void UART4_IRQHandler(void)
{
	u8 Res;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) 
	{
		Res =USART_ReceiveData(UART4);//(USART1->DR);	
		
		if(Res=='\n')
		{
			UartDebug_R_Buf[UartDebug_Cnt-1] = '\0';
			//TestComConvert(UartDebug_R_Buf);
			//memcpy(USB_DataRec.USB.data,UartDebug_R_Buf + 2,UartDebug_Cnt - 2);//daxiao
			
			DebugComConvert(UartDebug_R_Buf);
			memcpy(USB_DataRec.USB.data + 1,UartDebug_R_Buf + 2,UartDebug_Cnt - 2);
			
			UartDebug_Cnt = 0;
		}
		else
		{
			UartDebug_R_Buf[UartDebug_Cnt++]=Res;
			if(UartDebug_Cnt >= MAX_UART_DEBUG_BUFF_SIZE)
			{
				UartDebug_Cnt = MAX_UART_DEBUG_BUFF_SIZE - 1;
			}
		}
	}		
} 
#endif
 


void USART2_IRQHandler(void)//���ڿ����жϣ����յ����ݻ����
{
	u16 num = 0;
	if(USART_GetITStatus(USART2,USART_IT_IDLE) == SET)
	{
		num = USART2->SR;
		num = USART2->DR; 
		DMA_Cmd(DMA1_Channel6,DISABLE);   
		num = MAX_REC_BUFF_SIZE -  DMA1_Channel6->CNDTR; 
		
		if(num)
		{
			if(MuxOpenFlag == 0)
			{
				DMA1_Channel6->CNDTR = MAX_REC_BUFF_SIZE;
				
				if(BuffIndex == 0)
				{ 
					BuffIndex = 1;  
					DMA1_Channel6->CMAR = (u32)RecBuff_2;
				}
				else
				{
					BuffIndex = 0;
					DMA1_Channel6->CMAR = (u32)RecBuff_1;
				}
				DMA_Cmd(DMA1_Channel6,ENABLE);
				if(BuffIndex == 1)
				{
					RecBuff_1[num]='\0';
					at_buffer_handler(RecBuff_1,num);
				}
				else
				{
					RecBuff_2[num]='\0';
					at_buffer_handler(RecBuff_2,num);
				}	
			}
			else
			{  
				DMA1_Channel6->CNDTR = MAX_REC_BUFF_SIZE;
				
				if(BuffIndex == 0)
				{ 
					BuffIndex = 1;  
					DMA1_Channel6->CMAR = (u32)RecBuff_2;
				}
				else
				{
					BuffIndex = 0;
					DMA1_Channel6->CMAR = (u32)RecBuff_1;
				}
				DMA_Cmd(DMA1_Channel6,ENABLE);
				if(BuffIndex == 1)
				{
					RecBuff_1[num]='\0';
					#ifdef MUX_MAIN
					memcpy(AT_MuxBuff[MuxBufIndexEnd] + 2,RecBuff_1,num);
					AT_MuxBuff[MuxBufIndexEnd][0] = num / 256;
					AT_MuxBuff[MuxBufIndexEnd][1] = num % 256;
					MuxBufIndexPlusPlus(MuxBufIndexEnd);
					#else
					AT_MuxBuffHandler(RecBuff_1,num);
					#endif
				}
				else
				{
					RecBuff_2[num]='\0';
					#ifdef MUX_MAIN
					memcpy(AT_MuxBuff[MuxBufIndexEnd] + 2,RecBuff_2,num);
					AT_MuxBuff[MuxBufIndexEnd][0] = num / 256;
					AT_MuxBuff[MuxBufIndexEnd][1] = num % 256;
					MuxBufIndexPlusPlus(MuxBufIndexEnd);
					#else
					AT_MuxBuffHandler(RecBuff_2,num);
					#endif
				}	
			}
				
		}
		else//û�����ݣ�˵����DMA�ж��д������
		{
			DMA_Cmd(DMA1_Channel6,ENABLE);
		}
	}
}


void DMA1_Channel6_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC6))
  {
		DMA_Cmd(DMA1_Channel6, DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_GL6);//�������DMA1_IT_GL5��־��
		
		if(MuxOpenFlag == 0)
		{
			DMA1_Channel6->CNDTR = MAX_REC_BUFF_SIZE;
			if(BuffIndex == 0)
			{    
				BuffIndex = 1;  
				DMA1_Channel6->CMAR = (u32)RecBuff_2;
			}
			else
			{
				BuffIndex = 0;  
				DMA1_Channel6->CMAR = (u32)RecBuff_1;
			}
			DMA_Cmd(DMA1_Channel6,ENABLE);
			if(BuffIndex == 1)
			{
				RecBuff_1[MAX_REC_BUFF_SIZE] = '\0';
				at_buffer_handler(RecBuff_1,MAX_REC_BUFF_SIZE);
			}
			else
			{
				RecBuff_2[MAX_REC_BUFF_SIZE] = '\0';
				at_buffer_handler(RecBuff_2,MAX_REC_BUFF_SIZE);
			}
		}
		else
		{
			DMA1_Channel6->CNDTR = MAX_REC_BUFF_SIZE;
		
			if(BuffIndex == 0)
			{    
				BuffIndex = 1;  
				DMA1_Channel6->CMAR = (u32)RecBuff_2;
			}
			else
			{
				BuffIndex = 0;  
				DMA1_Channel6->CMAR = (u32)RecBuff_1;
			}
			DMA_Cmd(DMA1_Channel6,ENABLE);
			if(BuffIndex == 1)
			{
				RecBuff_1[MAX_REC_BUFF_SIZE] = '\0';
				
				#ifdef MUX_MAIN
				memcpy(AT_MuxBuff[MuxBufIndexEnd] + 2,RecBuff_1,MAX_REC_BUFF_SIZE);
				AT_MuxBuff[MuxBufIndexEnd][0] = MAX_REC_BUFF_SIZE / 256;
				AT_MuxBuff[MuxBufIndexEnd][1] = MAX_REC_BUFF_SIZE % 256;
				MuxBufIndexPlusPlus(MuxBufIndexEnd);
				#else
				AT_MuxBuffHandler(RecBuff_1,MAX_REC_BUFF_SIZE);
				#endif
				
			}
			else
			{
				RecBuff_2[MAX_REC_BUFF_SIZE] = '\0';
				AT_MuxBuffHandler(RecBuff_2,MAX_REC_BUFF_SIZE);
				#ifdef MUX_MAIN
				memcpy(AT_MuxBuff[MuxBufIndexEnd] + 2,RecBuff_2,MAX_REC_BUFF_SIZE);
				AT_MuxBuff[MuxBufIndexEnd][0] = MAX_REC_BUFF_SIZE / 256;
				AT_MuxBuff[MuxBufIndexEnd][1] = MAX_REC_BUFF_SIZE % 256;
				MuxBufIndexPlusPlus(MuxBufIndexEnd);
				#else
				AT_MuxBuffHandler(RecBuff_2,MAX_REC_BUFF_SIZE);
				#endif
			}
		}
	}
}
#ifdef I2S_HALF_BUFFER
//���մ�ģ��PCM��������
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4)!=RESET)//���ж�
	{
		//LED2_T;
		DMA_ClearITPendingBit(DMA1_IT_GL4);
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		I2S_DMA_BuffRec_NO = 1;
		I2S_DMA_Buff2Rec_OK = 1;
		I2S_DMA_BuffRec_OK = 1;
	}
	else if(DMA_GetITStatus(DMA1_IT_HT4)!=RESET)//�����ж�
	{
		//LED2_T;
		DMA_ClearITPendingBit(DMA1_IT_GL4);
		DMA_ClearITPendingBit(DMA1_IT_HT4);
		I2S_DMA_BuffRec_NO = 2;
		I2S_DMA_Buff1Rec_OK = 1;
		I2S_DMA_BuffRec_OK = 1;
	}
}
//�������ݵ�ģ��PCM
void DMA2_Channel2_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_IT_TC2)!=RESET)//���ж�
	{
		LED2_T;
		DMA_ClearITPendingBit(DMA2_IT_GL2);
		DMA_ClearITPendingBit(DMA2_IT_TC2);
		I2S_DMA_BuffSend_NO = 1;
		I2S_DMA_Buff2Send_OK = 1;
	}
	else if(DMA_GetITStatus(DMA2_IT_HT2)!=RESET)//�����ж�
	{
		LED2_T;
		DMA_ClearITPendingBit(DMA2_IT_GL2);
		DMA_ClearITPendingBit(DMA2_IT_HT2);
		I2S_DMA_BuffSend_NO = 2;
		I2S_DMA_Buff1Send_OK = 1;
	}
}

#else
//����
	void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4)!=RESET)
	{
		LED2_T;
		DMA_ClearITPendingBit(DMA1_IT_GL4);
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA1_InitStructure.DMA_BufferSize = I2S_REC_SIZE; 
		if(I2S_DMA_BuffRec_NO==0)
		{
			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec1;
			DMA_Init(DMA1_Channel4,&DMA1_InitStructure);
			I2S_DMA_BuffRec_NO=1;
		}
		else
		{
			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec2;
			DMA_Init(DMA1_Channel4,&DMA1_InitStructure);
			I2S_DMA_BuffRec_NO=0;
		}
		I2S_DMA_BuffRec_OK=TRUE;
	}
}
//����
void DMA2_Channel2_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_IT_TC2)!=RESET)
	{
		DMA_ClearITPendingBit(DMA2_IT_GL2);
		DMA_ClearITPendingBit(DMA2_IT_TC2);

		DMA2_InitStructure.DMA_BufferSize = I2S_SEND_SIZE;
		if(I2S_DMA_BuffSend_NO==0)
		{
			DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend1;
			DMA_Init(DMA2_Channel2,&DMA2_InitStructure);
			I2S_DMA_BuffSend_NO=1;
			I2S_DMA_Buff2Send_OK=TRUE;
		}
		else
		{
			DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend2;
			DMA_Init(DMA2_Channel2,&DMA2_InitStructure);
			I2S_DMA_BuffSend_NO=0;
			I2S_DMA_Buff1Send_OK=TRUE;
		}
		BufSendIndex = 0;

	}
}
#endif



//#ifdef GD32
//����


//#else
////����
//void DMA2_Channel1_IRQHandler(void)
//{

//	if(DMA_GetITStatus(DMA2_IT_TC1)!=RESET)
//	{

//		DMA_ClearITPendingBit(DMA2_IT_GL1);
//		DMA_ClearITPendingBit(DMA2_IT_TC1);
//		DMA2_InitStructure.DMA_BufferSize = I2S_REC_SIZE; 
//		if(I2S_DMA_BuffRec_NO==0)
//		{
//			DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec1;
//			DMA_Init(DMA2_Channel1,&DMA2_InitStructure);
//			I2S_DMA_BuffRec_NO=1;
//		}
//		else
//		{
//			DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec2;
//			DMA_Init(DMA2_Channel1,&DMA2_InitStructure);
//			I2S_DMA_BuffRec_NO=0;
//		}
//		I2S_DMA_BuffRec_OK=TRUE;
//	//	DMA_Cmd(DMA2_Channel1, ENABLE); 
//	}
//}
////����
//void DMA1_Channel5_IRQHandler(void)
//{
//	if(DMA_GetITStatus(DMA1_IT_TC5)!=RESET)
//	{

//		DMA_ClearITPendingBit(DMA1_IT_GL5);
//		DMA_ClearITPendingBit(DMA1_IT_TC5);

//		DMA1_InitStructure.DMA_BufferSize = I2S_SEND_SIZE;
//		if(I2S_DMA_BuffSend_NO==0)
//		{
//			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend1;
//			DMA_Init(DMA1_Channel5,&DMA1_InitStructure);
//			I2S_DMA_BuffSend_NO=1;
//			I2S_DMA_Buff2Send_OK=TRUE;
//		}
//		else
//		{
//			DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend2;
//			DMA_Init(DMA1_Channel5,&DMA1_InitStructure);
//			I2S_DMA_BuffSend_NO=0;
//			I2S_DMA_Buff1Send_OK=TRUE;
//		}
//		BufSendIndex = 0;
//		
//	//	DMA_Cmd(DMA2_Channel1, ENABLE); 
//	}
//}


//#endif

/******************************************************************************/
/*                 STM32F10x ���� �ж� ������                               */
/*  ������Ҫ�������жϺ��������档�жϵĺ������ֶ��Ѿ���startup_stm32f10x_xx.s*/
/*  ���ļ��ж�����ˣ����������д��                                          */
/******************************************************************************/
/**
  * @brief  USB�жϴ�����
  * @note	���жϺ���ֻ��STM32F105��STM32F107ϵ��оƬ��������ж�
  * @param  None
  * @retval None
  */
#ifndef STM32F10X_CL
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}
#endif /* STM32F10X_CL */
/**
  * @brief  This function handles OTG WakeUp interrupt request.
  * @note	None
  * @param  None
  * @retval None
  */
void OTG_FS_WKUP_IRQHandler(void)
{
  /* Initiate external resume sequence (1 step) */
  Resume(RESUME_EXTERNAL);  

}
/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @note	None
  * @param  None
  * @retval None
  */
#ifdef STM32F10X_CL
void OTG_FS_IRQHandler(void)
{
  STM32_PCD_OTG_ISR_Handler(); 
}
#endif /* STM32F10X_CL */





/*********************************END OF FILE**********************************/
