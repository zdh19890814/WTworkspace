#include "include.h"

//#include "stm32f10x_spi.h"
//u16 DMA1_MEM_LEN;//����DMAÿ�����ݴ��͵ĳ���


uint16_t I2S_DMA_DoubleBufferSend[I2S_SEND_SIZE * 2];
uint16_t I2S_DMA_DoubleBufferRec[I2S_REC_SIZE * 2];

uint16_t *I2S_DMA_BufferRec1 = I2S_DMA_DoubleBufferRec;
uint16_t *I2S_DMA_BufferRec2 = I2S_DMA_DoubleBufferRec + I2S_REC_SIZE;
uint16_t *I2S_DMA_BufferSend1 = I2S_DMA_DoubleBufferSend;
uint16_t *I2S_DMA_BufferSend2 = I2S_DMA_DoubleBufferSend + I2S_SEND_SIZE;



uint16_t I2S_DMA_BuffSend1[I2S_SEND_SIZE];
uint16_t I2S_DMA_BuffSend2[I2S_SEND_SIZE];
uint16_t I2S_DMA_BuffRec1[I2S_REC_SIZE];
uint16_t I2S_DMA_BuffRec2[I2S_REC_SIZE];











uint8_t I2S_DMA_BuffRec_NO = 0;
volatile u8 I2S_DMA_BuffRec_OK = 0;
volatile u8 I2S_DMA_Buff1Rec_OK = 0;
volatile u8 I2S_DMA_Buff2Rec_OK = 0;


uint8_t I2S_DMA_BuffSend_NO = 0;
volatile u8 I2S_DMA_Buff1Send_OK = 0;
volatile u8 I2S_DMA_Buff2Send_OK = 0;

u16 BufSendIndex = 0;



void Audio_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void I2S_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOG |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);

  /* I2S2 SD, CK and WS pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                  
	GPIO_Init(GPIOB, &GPIO_InitStructure);   
}
void Speaker_Config(void)
{

  /* Configure the initialization parameters */
  I2S_GPIO_Config();
  I2S_Config(I2S_Standard_PCMShort, I2S_MCLKOutput_Disable, I2S_AudioFreq_8k);
 

}



/*******************************************************************************
@�� �� ����I2S_Config()                 
@��    �ܣ�I2S2��I2S3��Ϊ�ӻ��շ��ľ�������
@��    �룺Standard��I2S֧�ֵ���ƵЭ��
		   MCLKOutput���Ƿ�����ʱ��
		   AudioFreq��ʱ��Ƶ��
@��    ������
*******************************************************************************/
void I2S_Config(uint16_t Standard, uint16_t MCLKOutput, uint16_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;

  /* Enable I2S2 APB1 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  /* Deinitialize SPI2 peripheral */
  SPI_I2S_DeInit(SPI2);

  /* I2S2 peripheral configuration */
  I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveRx;//I2S2 ����
  I2S_InitStructure.I2S_Standard = Standard;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;//����˫�ֽ�
  I2S_InitStructure.I2S_MCLKOutput = MCLKOutput;
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;//CPOLѡ��High����ֹ������λ
  I2S_Init(SPI2, &I2S_InitStructure);

#ifdef I2S_ON_OFF
	I2S_Cmd(SPI2,DISABLE);
#else
	I2S_Cmd(SPI2, ENABLE);
#endif
 
	/* Enable I2S3 APB1 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

	/* Deinitialize SPI3 peripheral */
	SPI_I2S_DeInit(SPI3);

	/* I2S3 peripheral configuration */
	I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;//I2S3 �ӷ�
	I2S_InitStructure.I2S_Standard = Standard;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;//����˫�ֽ�
	I2S_InitStructure.I2S_MCLKOutput = MCLKOutput;
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;//CPOLѡ��High����ֹ������λ
	I2S_Init(SPI3, &I2S_InitStructure);

#ifdef I2S_ON_OFF
	I2S_Cmd(SPI3,DISABLE);
#else
	I2S_Cmd(SPI3,ENABLE);
#endif
	

}


void I2S_ConfigS(uint16_t Standard, uint16_t MCLKOutput, uint16_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;

  /* Enable I2S2 APB1 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  /* Deinitialize SPI2 peripheral */
  SPI_I2S_DeInit(SPI2);

  /* I2S2 peripheral configuration */
  I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;
  I2S_InitStructure.I2S_Standard = Standard;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_MCLKOutput = MCLKOutput;
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;
  I2S_Init(SPI2, &I2S_InitStructure);


#ifdef I2S_ON_OFF
	I2S_Cmd(SPI2,DISABLE);
#else
	I2S_Cmd(SPI2, ENABLE);
#endif
	/* Enable I2S3 APB1 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

	/* Deinitialize SPI3 peripheral */
	SPI_I2S_DeInit(SPI3);

	/* I2S3 peripheral configuration */
	I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveRx;
	I2S_InitStructure.I2S_Standard = Standard;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_MCLKOutput = MCLKOutput;
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;
	I2S_Init(SPI3, &I2S_InitStructure);

	
#ifdef I2S_ON_OFF
	I2S_Cmd(SPI3,DISABLE);
#else
	I2S_Cmd(SPI3, ENABLE);
#endif

}



  DMA_InitTypeDef DMA2_InitStructure;
  DMA_InitTypeDef DMA1_InitStructure;
void I2S3DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//ʹ��DMA����
	
  DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	//DMA1_MEM_LEN=cndtr;
	DMA2_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
	DMA2_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA2_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //I2S3Դ��ַ
	DMA2_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA2_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA2_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA2_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA2_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA2_InitStructure.DMA_Mode = DMA_Mode_Circular;  //����������ģʽ
	DMA2_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA2_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA2_InitStructure); 
} 
void I2S2DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
  DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	//DMA1_MEM_LEN=cndtr;
	DMA1_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
	DMA1_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //I2S2Ŀ�ĵ�ַ
	DMA1_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA1_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA1_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA1_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA1_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA1_InitStructure); 
	  	
} 

/*******************************************************************************
@�� �� ����I2S_DMA_Init()                     
@��    �ܣ�DMAʹ��˫BUFFER�ֶ��л�ʱ�����ã�����I2S GPIO�ܽ�����
@��    �룺��
@��    ������
*******************************************************************************/

void I2S_DMA_Init()
{

	
	
	GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOG |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);

  /* I2S2 I2S3 SD, CK and WS pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                  
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	I2S_Config(I2S_Standard_PCMShort, I2S_MCLKOutput_Disable, I2S_AudioFreq_8k);


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//ʹ��DMAʱ��
	
    DMA_DeInit(DMA2_Channel2);   //��DMA2��ͨ��2�Ĵ�������Ϊȱʡֵ

	//DMA1_MEM_LEN=cndtr;
	DMA2_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;  //DMA�������ַ
	DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend2;  //DMA�ڴ����ַ
	DMA2_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //I2S3 ������Ŀ�ĵ�ַ
	DMA2_InitStructure.DMA_BufferSize = I2S_SEND_SIZE;  //DMAͨ����DMA����Ĵ�С
	DMA2_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA2_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA2_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA2_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA2_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA2_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA2_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA2_Channel2, &DMA2_InitStructure); 
	

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	DMA_ITConfig(DMA2_Channel2,DMA_IT_TC,ENABLE);//DMA�Ǵ�������ж�
	DMA_Cmd(DMA2_Channel2, ENABLE); 
//---------------------------------------------------------------------------------------------------------
	//I2S2DMA_Config(DMA1_Channel5,(u32)&SPI2->DR,(u32)I2S_DMA_BuffSend2,I2S_SEND_SIZE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA1����
	
    DMA_DeInit(DMA1_Channel4);   //��DMA1��ͨ��4�Ĵ�������Ϊȱʡֵ

	DMA1_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;  //DMA�������ַ
	DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec2;  //DMA�ڴ����ַ
	DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //I2S2������Դ��ַ
	DMA1_InitStructure.DMA_BufferSize = I2S_REC_SIZE;  //DMAͨ����DMA����Ĵ�С
	DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA1_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA1_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA1_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA1_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel4, &DMA1_InitStructure);

	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx,ENABLE);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);//DMA�Ǵ�������ж�
    DMA_Cmd(DMA1_Channel4, ENABLE); 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}


/*******************************************************************************
@�� �� ����I2S_DMA_Init_D()                    
@��    �ܣ�DMAʹ�ô�������ж�ʱ�����ã�����I2S GPIO�ܽ�����
@��    �룺��
@��    ������
*******************************************************************************/

void I2S_DMA_Init_D()
{

	DMA_InitTypeDef DMA_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOG |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);

  /* I2S2 SD, CK and WS pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);  //����оƬ��PA15Ĭ����JTDI��PB3Ĭ����JTDO��������Ҫ��Ĭ��ʧ�ܣ��������ùܽ�


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                  
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	I2S_Config(I2S_Standard_PCMShort, I2S_MCLKOutput_Disable, I2S_AudioFreq_8k);


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//ʹ��DMA����
	
    DMA_DeInit(DMA2_Channel2);   //��DMA2��ͨ��2�Ĵ�������Ϊȱʡֵ

	//DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_DoubleBufferSend;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //I2S3������Ŀ�ĵ�ַ
	DMA_InitStructure.DMA_BufferSize = I2S_SEND_SIZE * 2;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA2_Channel2, &DMA_InitStructure); 
	

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	DMA_ITConfig(DMA2_Channel2,DMA_IT_TC,ENABLE);//DMA��������ж�
	DMA_ITConfig(DMA2_Channel2,DMA_IT_HT,ENABLE);//DMA��������ж�
	DMA_Cmd(DMA2_Channel2, ENABLE); 
//---------------------------------------------------------------------------------------------------------
	//I2S2DMA_Config(DMA1_Channel5,(u32)&SPI2->DR,(u32)I2S_DMA_BuffSend2,I2S_SEND_SIZE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
    DMA_DeInit(DMA1_Channel4);   //��DMA1��ͨ��4�Ĵ�������Ϊȱʡֵ

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_DoubleBufferRec;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//I2S2����Դ��ַ
	DMA_InitStructure.DMA_BufferSize = I2S_REC_SIZE * 2;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel4, &DMA_InitStructure); 

	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx,ENABLE);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);//DMA��������ж�
	DMA_ITConfig(DMA1_Channel4,DMA_IT_HT,ENABLE);//DMA��������ж�
    DMA_Cmd(DMA1_Channel4, ENABLE); 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
