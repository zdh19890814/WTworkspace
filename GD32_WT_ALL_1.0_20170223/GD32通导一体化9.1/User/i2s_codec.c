#include "include.h"

//#include "stm32f10x_spi.h"
//u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度


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
@函 数 名：I2S_Config()                 
@功    能：I2S2和I2S3作为从机收发的具体配置
@输    入：Standard：I2S支持的音频协议
		   MCLKOutput：是否有主时钟
		   AudioFreq：时钟频率
@输    出：无
*******************************************************************************/
void I2S_Config(uint16_t Standard, uint16_t MCLKOutput, uint16_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;

  /* Enable I2S2 APB1 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  /* Deinitialize SPI2 peripheral */
  SPI_I2S_DeInit(SPI2);

  /* I2S2 peripheral configuration */
  I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveRx;//I2S2 从收
  I2S_InitStructure.I2S_Standard = Standard;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;//数据双字节
  I2S_InitStructure.I2S_MCLKOutput = MCLKOutput;
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;//CPOL选择High，防止数据移位
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
	I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;//I2S3 从发
	I2S_InitStructure.I2S_Standard = Standard;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;//数据双字节
	I2S_InitStructure.I2S_MCLKOutput = MCLKOutput;
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;//CPOL选择High，防止数据移位
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

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//使能DMA传输
	
  DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值

	//DMA1_MEM_LEN=cndtr;
	DMA2_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
	DMA2_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA2_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //I2S3源地址
	DMA2_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA2_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA2_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA2_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA2_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA2_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在正常模式
	DMA2_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA2_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA2_InitStructure); 
} 
void I2S2DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
  DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值

	//DMA1_MEM_LEN=cndtr;
	DMA1_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
	DMA1_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //I2S2目的地址
	DMA1_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA1_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA1_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环模式
	DMA1_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA1_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA1_InitStructure); 
	  	
} 

/*******************************************************************************
@函 数 名：I2S_DMA_Init()                     
@功    能：DMA使用双BUFFER手动切换时的配置，包括I2S GPIO管脚配置
@输    入：无
@输    出：无
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

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//使能DMA时钟
	
    DMA_DeInit(DMA2_Channel2);   //将DMA2的通道2寄存器重设为缺省值

	//DMA1_MEM_LEN=cndtr;
	DMA2_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;  //DMA外设基地址
	DMA2_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffSend2;  //DMA内存基地址
	DMA2_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //I2S3 外设是目的地址
	DMA2_InitStructure.DMA_BufferSize = I2S_SEND_SIZE;  //DMA通道的DMA缓存的大小
	DMA2_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA2_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA2_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA2_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA2_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环模式
	DMA2_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA2_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA2_Channel2, &DMA2_InitStructure); 
	

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	DMA_ITConfig(DMA2_Channel2,DMA_IT_TC,ENABLE);//DMA是传输完成中断
	DMA_Cmd(DMA2_Channel2, ENABLE); 
//---------------------------------------------------------------------------------------------------------
	//I2S2DMA_Config(DMA1_Channel5,(u32)&SPI2->DR,(u32)I2S_DMA_BuffSend2,I2S_SEND_SIZE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA1传输
	
    DMA_DeInit(DMA1_Channel4);   //将DMA1的通道4寄存器重设为缺省值

	DMA1_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;  //DMA外设基地址
	DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_BuffRec2;  //DMA内存基地址
	DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //I2S2外设是源地址
	DMA1_InitStructure.DMA_BufferSize = I2S_REC_SIZE;  //DMA通道的DMA缓存的大小
	DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA1_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA1_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环模式
	DMA1_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA1_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel4, &DMA1_InitStructure);

	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx,ENABLE);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);//DMA是传输完成中断
    DMA_Cmd(DMA1_Channel4, ENABLE); 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}


/*******************************************************************************
@函 数 名：I2S_DMA_Init_D()                    
@功    能：DMA使用传输过半中断时的配置，包括I2S GPIO管脚配置
@输    入：无
@输    出：无
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
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);  //由于芯片的PA15默认是JTDI，PB3默认是JTDO，所以需要将默认失能，重新配置管脚


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

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//使能DMA传输
	
    DMA_DeInit(DMA2_Channel2);   //将DMA2的通道2寄存器重设为缺省值

	//DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_DoubleBufferSend;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //I2S3外设是目的地址
	DMA_InitStructure.DMA_BufferSize = I2S_SEND_SIZE * 2;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA2_Channel2, &DMA_InitStructure); 
	

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	DMA_ITConfig(DMA2_Channel2,DMA_IT_TC,ENABLE);//DMA传输完成中断
	DMA_ITConfig(DMA2_Channel2,DMA_IT_HT,ENABLE);//DMA传输过半中断
	DMA_Cmd(DMA2_Channel2, ENABLE); 
//---------------------------------------------------------------------------------------------------------
	//I2S2DMA_Config(DMA1_Channel5,(u32)&SPI2->DR,(u32)I2S_DMA_BuffSend2,I2S_SEND_SIZE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
    DMA_DeInit(DMA1_Channel4);   //将DMA1的通道4寄存器重设为缺省值

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)I2S_DMA_DoubleBufferRec;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//I2S2外设源地址
	DMA_InitStructure.DMA_BufferSize = I2S_REC_SIZE * 2;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel4, &DMA_InitStructure); 

	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx,ENABLE);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);//DMA传输完成中断
	DMA_ITConfig(DMA1_Channel4,DMA_IT_HT,ENABLE);//DMA传输过半中断
    DMA_Cmd(DMA1_Channel4, ENABLE); 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
