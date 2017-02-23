#include "include.h"

char BDdata[BDdata_SIZE];
int BDtimeflag = 0;
int BD_datanum = 0;
//u16 DMA1_men_len;//����DMAÿ�����ݴ��͵ĳ��� 	
//char rev_buf[REV_NUM][100];   //���ջ���
char SendBuff[SEND_BUF_SIZE]; 	//�������ݻ�����
u8 BDS_TouchuanFlag = 0;

/*****************************************************************/
/*@�� �� ����GetComma(uchar num,char *str)                       */
/*@��    �ܣ����ҵ�num������                                     */
/*@�޸����ڣ�2016��7��6��                                       */
/**************************************************************/
uchar GetComma(uchar num,char *str)
{
	uchar i,j = 0;
	int len=strlen(str);

	for(i = 0;i < len;i ++)
	{
		if(str[i] == ',')
			j++;
		if(j == num)
			return i + 1;	
	}
	if(j == 0)
	{
		for(i = 0;i < len;i ++)
		{
			if(str[i] == '*')
				j++;
			if(j == num)
				return i + 1;	
		}
	}
	return 0;	
}


//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx
//cpar:�����ַ
//cmar:�洢����ַ
//cndtr:���ݴ����� 
// void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
//{
// 	DMA_InitTypeDef DMA_InitStructure;
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
//	
//    DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

//	DMA1_men_len=cndtr;
//	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
//	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
//	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //����������ģʽ
//	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
//	DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
//	  	
//} 
//����һ��DMA����
//void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
//{ 
//	DMA_Cmd(DMA_CHx, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��
// 	DMA_SetCurrDataCounter(DMA_CHx,DMA1_men_len);//DMAͨ����DMA����Ĵ�С
// 	DMA_Cmd(DMA_CHx, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
//}	  

/*****************************************************************/
/*@�� �� ����UART5_BD_Init()                                      */
/*@��    �ܣ����崮��5��ʼ��                                     */
/*@�޸����ڣ�2016��8��12��                                       */
/***************************************************************/
void UART5_BD_Init()
{    
  //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    //NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��GPIOCʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	//ʹ��GPIODʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//ʹ��UART5ʱ��
  
	//UART5_TX   GPIOC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.12
   
    //UART5_RX	  GPIOD.2��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIOD.2

  //UART5 NVIC ����
/*    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
*/  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = 115200;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(UART5, &USART_InitStructure); //��ʼ������5
//    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
    USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ���5
}



 void BD_UartInit()
{
#ifdef GD32
	UART5_BD_Init();
#else
	USART3_DMA_Init();
#endif
}



/*****************************************************************/
/*@�� �� ����USART3_DMA_Init()                                   */
/*@��    �ܣ�����3��ʼ����DMA��ʼ��                               */
/*@�޸����ڣ�2016��7��22��                                       */
/***************************************************************/
void USART3_DMA_Init()
{    
  //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��GPIOBʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//ʹ��USART3ʱ��
  
	//USART3_TX   GPIOB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.10
   
  //USART3_RX	  GPIOB.11��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.11  

  //Usart3 NVIC ����
/*    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
*/  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = 115200;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART3, &USART_InitStructure); //��ʼ������3
//    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
    USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3 
  
    //��ʼ��DMA
//    MYDMA_Config(DMA1_Channel3,(u32)&USART3->DR,(u32)SendBuff,SEND_BUF_SIZE);//DMA1ͨ��3,����Ϊ����3,�洢��ΪSendBuff,����SEND_BUF_SIZE.
//    USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); //ʹ�ܴ���3��DMA����      
//    MYDMA_Enable(DMA1_Channel3);//��ʼһ��DMA���䣡
    
}

/********************************************************************************/
/*@�� �� ����BDS_SET_Pattern(u8 mode)                                           */
/*@��    �ܣ�����ģʽ                                                           */
/*@�޸����ڣ�2016��7��30��                                                      */
/*@��ע���������modeΪuint8�ͣ�0->B1I��ϵͳ��λ                                */
/*                             1->GPS��ϵͳ��λ                                 */
/*                             2->GLONASS��ϵͳ��λ                             */
/*                             3->B3C��ϵͳ��λ                                 */
/*                             4->��ϵͳ��϶�λ(B3C+GPS+GLONASS)               */
/********************************************************************************/
u8 BDS_SET_Pattern(u8 mode)
{
    u8 orders[5][30]={"$CCMSS,C,1,B1,C,,,,*0F","$CCMSS,C,1,L1,C,,,,*0F","$CCMSS,C,1,R1,C,,,,*0F","$CCMSS,C,1,B3,C,,,,*0F","$CCMSS,C,1,B3,C,L1,C,R1,C*17"};
    if(mode > 4)
    {
        DebugOut("unknown BDS mode %d\r\n",mode);
        return mode;
    }
    else
    {
        BDS_ComSendWithEnd(orders[mode]);
        DebugOut("BDS mode %d\r\n",mode);
        return 1;
    }
		
}


/*****************************************************************/
/*@�� �� ����BDS_GET_NumOfStl(void)                              */
/*@��    �ܣ���ȡ������                                          */
/*@�޸����ڣ�2016��7��30��                                       */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�                        */
/*			 ɾ�������������ȫ�ֱ���                            */
/*			 �޸�����bug                                         */
/*****************************************************************/
void BDS_GET_NumOfStl(void)
{
    uchar ch1,ch2,ch3,status;
    char *buf;
    char *s;
    uchar i;
    int cnt;
    
    for(cnt = 0;cnt<SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&SendBuff[cnt+4]=='G')
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
    //����֡ͷ���ض���ʶ��
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(1, buf)];

	if (ch1 == 'G' && ch2 == 'G' && ch3 == 'A') //$BDGGA 
	{
		if (status != ',')
		{            
            s = &buf[GetComma(7, buf)];//��7������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;			
            strncpy(BDdata+BD_datanum, s, i);
			      BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}
/*****************************************************************/
/*@�� �� ����BDS_GET_Speed(void)                                 */
/*@��    �ܣ���ȡ�ٶ���Ϣ                                        */
/*@�޸����ڣ�2016��7��30��                                       */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�                        */
/*			 ɾ�������������ȫ�ֱ���                            */
/*			 �޸�����bug                                         */
/*****************************************************************/
void BDS_GET_Speed(void)
{
    uchar ch1,ch2,ch3,status;
    char *buf;
    char *s;
    uchar i;
    int cnt;
    
    for(cnt = 0;cnt<SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&SendBuff[cnt+4]=='M')
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
    
    ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(2, buf)];

	if (ch1 == 'R' && ch2 == 'M' && ch3 == 'C')  // $BDRMC
	{
		if (status == 'A')
		{            
            s = &buf[GetComma(7, buf)];//��7������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
            BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}
/*****************************************************************/
/*@�� �� ����BDS_GET_Date(void)                                  */
/*@��    �ܣ���ȡ������Ϣ                                        */
/*@�޸����ڣ�2016��7��30��                                       */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�                        */
/*			 ɾ�������������ȫ�ֱ���                            */
/*			 �޸�����bug                                         */
/*****************************************************************/
void BDS_GET_Date(void)
{
	uchar ch1,ch2,ch3,status;
	char *buf;
    char *s;
    uchar i;
    int cnt;
    
    for(cnt = 0;cnt<SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&SendBuff[cnt+4]=='M')
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
   
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(2, buf)];

	if (ch1 == 'R' && ch2 == 'M' && ch3 == 'C')  // $BDRMC
	{
		if (status == 'A')
		{            
            s = &buf[GetComma(9, buf)];//��9������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
            BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}

/********************************************************************************/
/*@�� �� ����BD_ReceiveData(void)                                               */
/*@��    �ܣ���ȡ����                                                           */
/*@�޸����ڣ�2016��8��9��                                                       */
/*@��ע��    �ж��н��ձ�������                                                 */
/******************************************************************************/
u8 BD_ReceiveData(void)
{
	int i;
	TimeTemp = SysTime;
#ifdef GD32
	for(i=0;i<SEND_BUF_SIZE;i++)
	{
		TimeTemp = SysTime;
	   while(!USART_GetFlagStatus(UART5, USART_FLAG_RXNE))//�ȴ����ձ�־����λ
		 WaitTimeOut(BD_REC_TIME);
	   SendBuff[i]=UART5->DR; //��ȡ���ݣ�ͬʱ������ձ�־λ��
	}
#else
	for(i=0;i<SEND_BUF_SIZE;i++)
	{
		TimeTemp = SysTime;
	   while(!USART_GetFlagStatus(USART3, USART_FLAG_RXNE))//�ȴ����ձ�־����λ
		 WaitTimeOut(BD_REC_TIME);
	   SendBuff[i]=USART3->DR; //��ȡ���ݣ�ͬʱ������ձ�־λ��
		 
	}
#endif
	return 1;	
}
 


void BDS_Handle()
{
	if(BDS_TouchuanFlag)
	{
		if(BDS_touchuan_open() != 1)//���͸����������ʧ�ܣ���ر�͸��
		{
			DebugOut("[timeout] BDS touchuan OFF\r\n");
			BDS_TouchuanFlag = 0;
		}
		else
		{
			DebugOut("BDS touchuan data send\r\n");
		}
	}
}

/********************************************************************************/
/*@�� �� ����BDS_touchuan_open(void)                                            */
/*@��    �ܣ�����͸����                                                       */
/*@�޸����ڣ�2016��8��9��                                                       */
/*@��ע��                                                                      */
/******************************************************************************/
u8 BDS_touchuan_open(void)
{
    u32 i;
		u8 idle_flag = 0;
		TimeTemp = SysTime;
#ifdef GD32
	  SendBuff[i]=UART5->SR; 
	  SendBuff[i]=UART5->DR; //�����־λ
    for(i=0;i< SEND_BUF_SIZE;i++)
		{
			 while(!USART_GetFlagStatus(UART5, USART_FLAG_RXNE))//�ȴ��������ݱ�־λ
			 {
				 if((SysTime - TimeTemp) > 100000)
				 {
					 return FLAG_TIME_OUT;//��ʱ��ʧ�ܷ���
				 }
				 if(USART_GetFlagStatus(UART5,USART_FLAG_IDLE))
				 {
					 idle_flag = 1;//��⵽����״̬��־����ʾһ֡���ݽ���
					 break;
				 }
			 }
			
			 if(idle_flag)
			 {
				 break;
			 }
			 TimeTemp = SysTime;	
			 SendBuff[i]=UART5->DR;		//��������	
		}
#else
	  SendBuff[i]=USART3->SR; 
	  SendBuff[i]=USART3->DR; 
    for(i=0;;i++)
		{
			
			 while(!USART_GetFlagStatus(USART3, USART_FLAG_RXNE))
			 {
				 if((SysTime - TimeTemp) > 100000)
				 {
					 return FLAG_TIME_OUT;//��ʱ��ʧ�ܷ���
				 }
				 if(USART_GetFlagStatus(USART3,USART_FLAG_IDLE))
				 {
					 idle_flag = 1;
					 break;
				 }
			 }
			
			 if(idle_flag)
			 {
				 break;
			 }
			 TimeTemp = SysTime;	
			 SendBuff[i]=USART3->DR;			
		}
#endif

		SendBuff[i] = 0;//��β ������
		USB_DataSend.USB.protocol = 0;
		USB_DataSend.USB.tos = 26;	
		USB_DataSend.USB.data[0] = 8;
		USB_DataSend.USB.length = i + 1;
		memcpy(USB_DataSend.USB.data + 1, SendBuff,i + 1);
		USB_EP1_Send(USB_DataSend.data, USB_EP1_SEND_NUM + USB_HEAD_NUM);//��������
		DebugOut("BDS touchuan data:\r\n%s",SendBuff);
		DebugOut("BDS touchuan data len = %d, data[2] = %d, data[3] = %d\r\n",i + 1,USB_DataSend.data[2],USB_DataSend.data[3]);
		return 1;	
		
}



/********************************************************************************/
/*@�� �� ����BDS_touchuan_close(void)                                           */
/*@��    �ܣ�����͸���ر�                                                       */
/*@�޸����ڣ�2016��8��9��                                                       */
/*@��ע��                                                                      */
/******************************************************************************/
void BDS_touchuan_close(void)
{
    
    TIM_Cmd(TIM1,DISABLE);
    BDtimeflag=0;
}	
/********************************************************************************/
/*@�� �� ����BDS_map_open(void)                                                 */
/*@��    �ܣ�������ͼģʽ��                                                    */
/*@�޸����ڣ�2016��8��9��                                                       */
/*@��ע��                                                                      */
/******************************************************************************/
void BDS_map_open(void)
{
	if(TIM_GetFlagStatus(TIM1 , TIM_FLAG_Update) != RESET ) 
	{
		BDtimeflag++;
		TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	}
	if(BDtimeflag == timenum_map)
	{		
		BD_ReceiveData();		
		
        BDdata[BD_datanum++]='1';		
		BDS_GET_NumOfStl();
		
		BDdata[BD_datanum++]='2';
		BDS_GET_Speed();
		
		BDdata[BD_datanum++]='3';
        BDS_GET_Time();//��ȡʱ����Ϣ 
    
		BDdata[BD_datanum++]='4';
        BDS_GET_Location();//��ȡ����γ����Ϣ
		
		DebugOut("BDS map:%s\r\n",BDdata);

		BD_datanum=0;
		BDtimeflag=0; 		
	}
}
/********************************************************************************/
/*@�� �� ����BDS_map_close(void)                                                */
/*@��    �ܣ�������ͼģʽ�ر�                                                    */
/*@�޸����ڣ�2016��8��9��                                                       */
/*@��ע��                                                                      */
/******************************************************************************/
void BDS_map_close(void)
{
    
    TIM_Cmd(TIM1,DISABLE);
    BDtimeflag=0;
}
/********************************************************************************/
/*@�� �� ����Tim1_Init(u16 period_num)                                          */
/*@��    �ܣ���ʱ��1��ʼ��                                                      */
/*@�޸����ڣ�2016��8��9��                                                       */
/*@��ע��                                                                      */
/******************************************************************************/
void Tim1_Init(u16 period_num)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	TIM_DeInit(TIM1);
	TIM_TimeBaseStructure.TIM_Period=period_num;//װ��ֵ
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//��Ƶϵ��
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM1,TIM_FLAG_Update);
	//TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM1,ENABLE);
}
/*****************************************************************/
/*@�� �� ����BDS_GET_Time(void)                                  */
/*@��    �ܣ���ȡʱ����Ϣ                                        */
/*@�޸����ڣ�2016��7��30��                                       */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�                        */
/*			 ɾ�������������ȫ�ֱ���                            */
/*			 �޸�����bug                                         */
/*****************************************************************/
void BDS_GET_Time(void)
{
	uchar ch1,ch2,ch3,status;
	char *buf;
    char *s;
    uchar i;
    int cnt,c,d;
    
    for(cnt = 0;cnt<SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&(SendBuff[cnt+4]=='G'||SendBuff[cnt+4]=='M'))
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
    
  
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(1, buf)];

	if ((ch1 == 'G' && ch2 == 'G' && ch3 == 'A')||(ch1 == 'R' && ch2 == 'M' && ch3 == 'C'))  //$BDGGA || $BDRMC
	{
		if (status != ',')
		{            
            s = &buf[GetComma(1, buf)];//��һ������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
            c = (int)BDdata[BD_datanum]-0x30;
			d = (int)BDdata[BD_datanum+1]-0x30;
			d = c * 10 + d;
			d = (d + 8) % 24;
			c = d / 10;
			d = d % 10;
			BDdata[BD_datanum] = (char)(c+0x30);
			BDdata[BD_datanum+1] = (char)(d+0x30);
			BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}
/********************************************************************************/
/*@�� �� ����BDS_GET_Location(void)                                             */
/*@��    �ܣ���ȡ����γ����Ϣ                                                   */
/*@�޸����ڣ�2016��7��30��                                                      */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�									   */
/*			 ɾ�������������ȫ�ֱ���                                         */
/*			 �޸�����bug                                                      */
/******************************************************************************/
void BDS_GET_Location(void)
{
	uchar ch1,ch2,ch3,status;
	char *buf;
    char *s;
    uchar i;
    int cnt;
    
	for(cnt = 0;cnt< SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&SendBuff[cnt+4]=='G')
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
 
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(1, buf)];

	if (ch1 == 'G' && ch2 == 'G' && ch3 == 'A')  //$BDGGA
	{
		if (status != ',')
		{
            //��ȡγ����Ϣ
			s = &buf[GetComma(2, buf)];//��2������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BDdata[BD_datanum+4] = BDdata[BD_datanum+3];
            BDdata[BD_datanum+3] = BDdata[BD_datanum+2];
            BDdata[BD_datanum+2] = '.';
			BD_datanum+=i;
			
            s = &buf[GetComma(3, buf)];//��2������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            BDdata[BD_datanum++] = *s;                    //latitude.NS
			     // BDdata[BD_datanum++] = '\0'; 
            
            //��ȡ������Ϣ
            s = &buf[GetComma(4, buf)];//��2������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
						i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BDdata[BD_datanum+5] = BDdata[BD_datanum+4];
            BDdata[BD_datanum+4] = BDdata[BD_datanum+3];
            BDdata[BD_datanum+3] = '.';
			BD_datanum+=i;
			
            s = &buf[GetComma(5, buf)];//��2������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            BDdata[BD_datanum++] = *s;                    //latitude.EW
            BDdata[BD_datanum++] = '\0';
		}
	}
}


/*****************************************************************/
/*@�� �� ����BDS_GET_Height(void)                                 */
/*@��    �ܣ���ȡ�߶���Ϣ                                        */
/*@�޸����ڣ�2016��8��11��                                       */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�                        */
/*			 ɾ�������������ȫ�ֱ���                            */
/*			 �޸�����bug                                         */
/*****************************************************************/
void BDS_GET_Height(void)
{
    uchar ch1,ch2,ch3,status;
	char *buf;
    char *s;
    uchar i;
    int cnt;
    
	for(cnt = 0;cnt<SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&SendBuff[cnt+4]=='G')
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
    //����֡ͷ���ض���ʶ��
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(1, buf)];

	if (ch1 == 'G' && ch2 == 'G' && ch3 == 'A') //$BDGGA 
	{
		if (status != ',')
		{            
            s = &buf[GetComma(9, buf)];//��9������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BD_datanum+=i;
            
            s = &buf[GetComma(10, buf)];//��10������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            BDdata[BD_datanum++] = *s;     //���ε�λ M
            BDdata[BD_datanum++] = '\0';
		}
	}
}

/*****************************************************************/
/*@�� �� ����BDS_GET_Direction(void)                                  */
/*@��    �ܣ���ȡ������Ϣ                                        */
/*@�޸����ڣ�2016��8��11��                                       */
/*@��    ע��������֡ͷ���ַ�װ�������ڲ�                        */
/*			 ɾ�������������ȫ�ֱ���                            */
/*			 �޸�����bug                                         */
/*****************************************************************/
void BDS_GET_Direction(void)
{
	uchar ch1,ch2,ch3,status;
	char *buf;
    char *s;
    uchar i;
    int cnt;
	
	for(cnt = 0;cnt<SEND_BUF_SIZE;cnt++)
    {
        if(SendBuff[cnt]=='$'&&SendBuff[cnt+4]=='M')
        {
            buf=&SendBuff[cnt];
            break;
        }
    }
    
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(2, buf)];

	if (ch1 == 'R' && ch2 == 'M' && ch3 == 'C')  // $BDRMC
	{
		if (status == 'A')
		{            
            s = &buf[GetComma(8, buf)];//��8������֮��ʼ�ĵ�һ���ַ����ڵĵ�ַ
            i=GetComma(1, s);//s�е�һ�����ŵ�λ��(�����ڵ�i���ַ�)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}


/********************************************************************************/
/*@�� �� ����BDS_GET(u8 type)                                                   */
/*@��    �ܣ���ȡ����                                                           */
/*@�޸����ڣ�2016��7��30��                                                      */
/*@��   ע���������typeΪu8�ͣ�0->��ȡ������       ����ֵ��type(0~6)            */
/*                         1->��ȡ�ٶ���Ϣ                 �򷵻�7����ʾ����    */
/*                         2->��ȡ������Ϣ                                     */
/*                         3->��ȡʱ����Ϣ                                     */
/*                         4->��ȡ����γ����Ϣ                                 */
/*                         5->��ȡ�߶���Ϣ                                     */
/*                         6->��ȡ������Ϣ                                     */
/******************************************************************************/
u8 BDS_GET(u8 type)
{
	u8 flag;
	DebugOut("BDS receive...\r\n");
	flag = BD_ReceiveData();
	if(flag != 1)
	{
		DebugOut("[timeout] BDS receive\r\n");
		return flag;
	}
	
	switch(type)
	{
		case 0:
				BDS_GET_NumOfStl();//��ȡ������  *BDdata
				break;
        case 1:
				BDS_GET_Speed();//��ȡ�ٶ���Ϣ 
				break;
		case 2:
				BDS_GET_Date();//��ȡ������Ϣ 
				break;
		case 3:
				BDS_GET_Time();//��ȡʱ����Ϣ
				break;
		case 4:
				BDS_GET_Location();//��ȡ����γ����Ϣ
				break;
		case 5:
				BDS_GET_Height();//��ȡ�߶���Ϣ
				break;
        case 6:
				BDS_GET_Direction();//��ȡ������Ϣ
				break;
			default:DebugOut("unknown BDS type : %d\r\n",type);return 7;
		}
	BD_datanum=0;
	DebugOut("BDS info %d----- %s\r\n",type,BDdata);
	return 1;
}




void BDS_ComSendWithEnd(u8 * str)
{
#ifdef GD32
	USART5SendStrWithEnd(str);
#else
	USART3SendStrWithEnd(str);
#endif
	DebugOut("[BD send]:%s\r\n",str);
}

