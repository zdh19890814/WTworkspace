#include "include.h"

char BDdata[BDdata_SIZE];
int BDtimeflag = 0;
int BD_datanum = 0;
//u16 DMA1_men_len;//保存DMA每次数据传送的长度 	
//char rev_buf[REV_NUM][100];   //接收缓存
char SendBuff[SEND_BUF_SIZE]; 	//发送数据缓冲区
u8 BDS_TouchuanFlag = 0;

/*****************************************************************/
/*@函 数 名：GetComma(uchar num,char *str)                       */
/*@功    能：查找第num个数据                                     */
/*@修改日期：2016年7月6日                                       */
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


//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_CHx:DMA通道CHx
//cpar:外设地址
//cmar:存储器地址
//cndtr:数据传输量 
// void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
//{
// 	DMA_InitTypeDef DMA_InitStructure;
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
//	
//    DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值

//	DMA1_men_len=cndtr;
//	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
//	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从内存读取发送到外设
//	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在正常模式
//	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
//	DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
//	  	
//} 
//开启一次DMA传输
//void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
//{ 
//	DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART1 TX DMA1 所指示的通道
// 	DMA_SetCurrDataCounter(DMA_CHx,DMA1_men_len);//DMA通道的DMA缓存的大小
// 	DMA_Cmd(DMA_CHx, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
//}	  

/*****************************************************************/
/*@函 数 名：UART5_BD_Init()                                      */
/*@功    能：背板串口5初始化                                     */
/*@修改日期：2016年8月12日                                       */
/***************************************************************/
void UART5_BD_Init()
{    
  //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    //NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能GPIOC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	//使能GPIOD时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//使能UART5时钟
  
	//UART5_TX   GPIOC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.12
   
    //UART5_RX	  GPIOD.2初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOD.2

  //UART5 NVIC 配置
/*    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
*/  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(UART5, &USART_InitStructure); //初始化串口5
//    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启串口接受中断
    USART_Cmd(UART5, ENABLE);                    //使能串口5
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
/*@函 数 名：USART3_DMA_Init()                                   */
/*@功    能：串口3初始化、DMA初始化                               */
/*@修改日期：2016年7月22日                                       */
/***************************************************************/
void USART3_DMA_Init()
{    
  //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART3时钟
  
	//USART3_TX   GPIOB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.10
   
  //USART3_RX	  GPIOB.11初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.11  

  //Usart3 NVIC 配置
/*    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
*/  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART3, &USART_InitStructure); //初始化串口3
//    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
    USART_Cmd(USART3, ENABLE);                    //使能串口3 
  
    //初始化DMA
//    MYDMA_Config(DMA1_Channel3,(u32)&USART3->DR,(u32)SendBuff,SEND_BUF_SIZE);//DMA1通道3,外设为串口3,存储器为SendBuff,长度SEND_BUF_SIZE.
//    USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); //使能串口3的DMA发送      
//    MYDMA_Enable(DMA1_Channel3);//开始一次DMA传输！
    
}

/********************************************************************************/
/*@函 数 名：BDS_SET_Pattern(u8 mode)                                           */
/*@功    能：设置模式                                                           */
/*@修改日期：2016年7月30日                                                      */
/*@备注：传入参数mode为uint8型，0->B1I单系统定位                                */
/*                             1->GPS单系统定位                                 */
/*                             2->GLONASS单系统定位                             */
/*                             3->B3C单系统定位                                 */
/*                             4->多系统组合定位(B3C+GPS+GLONASS)               */
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
/*@函 数 名：BDS_GET_NumOfStl(void)                              */
/*@功    能：获取卫星数                                          */
/*@修改日期：2016年7月30日                                       */
/*@备    注：将查找帧头部分封装到函数内部                        */
/*			 删除传入参数改用全局变量                            */
/*			 修复部分bug                                         */
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
    //查找帧头及特定标识符
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(1, buf)];

	if (ch1 == 'G' && ch2 == 'G' && ch3 == 'A') //$BDGGA 
	{
		if (status != ',')
		{            
            s = &buf[GetComma(7, buf)];//第7个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
            i = i - 1;			
            strncpy(BDdata+BD_datanum, s, i);
			      BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}
/*****************************************************************/
/*@函 数 名：BDS_GET_Speed(void)                                 */
/*@功    能：获取速度信息                                        */
/*@修改日期：2016年7月30日                                       */
/*@备    注：将查找帧头部分封装到函数内部                        */
/*			 删除传入参数改用全局变量                            */
/*			 修复部分bug                                         */
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
            s = &buf[GetComma(7, buf)];//第7个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
            BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}
/*****************************************************************/
/*@函 数 名：BDS_GET_Date(void)                                  */
/*@功    能：获取日期信息                                        */
/*@修改日期：2016年7月30日                                       */
/*@备    注：将查找帧头部分封装到函数内部                        */
/*			 删除传入参数改用全局变量                            */
/*			 修复部分bug                                         */
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
            s = &buf[GetComma(9, buf)];//第9个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
            BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}

/********************************************************************************/
/*@函 数 名：BD_ReceiveData(void)                                               */
/*@功    能：获取数据                                                           */
/*@修改日期：2016年8月9日                                                       */
/*@备注：    中断中接收北斗数据                                                 */
/******************************************************************************/
u8 BD_ReceiveData(void)
{
	int i;
	TimeTemp = SysTime;
#ifdef GD32
	for(i=0;i<SEND_BUF_SIZE;i++)
	{
		TimeTemp = SysTime;
	   while(!USART_GetFlagStatus(UART5, USART_FLAG_RXNE))//等待接收标志被置位
		 WaitTimeOut(BD_REC_TIME);
	   SendBuff[i]=UART5->DR; //读取数据（同时清楚接收标志位）
	}
#else
	for(i=0;i<SEND_BUF_SIZE;i++)
	{
		TimeTemp = SysTime;
	   while(!USART_GetFlagStatus(USART3, USART_FLAG_RXNE))//等待接收标志被置位
		 WaitTimeOut(BD_REC_TIME);
	   SendBuff[i]=USART3->DR; //读取数据（同时清楚接收标志位）
		 
	}
#endif
	return 1;	
}
 


void BDS_Handle()
{
	if(BDS_TouchuanFlag)
	{
		if(BDS_touchuan_open() != 1)//如果透传接收数据失败，则关闭透传
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
/*@函 数 名：BDS_touchuan_open(void)                                            */
/*@功    能：北斗透传打开                                                       */
/*@修改日期：2016年8月9日                                                       */
/*@备注：                                                                      */
/******************************************************************************/
u8 BDS_touchuan_open(void)
{
    u32 i;
		u8 idle_flag = 0;
		TimeTemp = SysTime;
#ifdef GD32
	  SendBuff[i]=UART5->SR; 
	  SendBuff[i]=UART5->DR; //清除标志位
    for(i=0;i< SEND_BUF_SIZE;i++)
		{
			 while(!USART_GetFlagStatus(UART5, USART_FLAG_RXNE))//等待接收数据标志位
			 {
				 if((SysTime - TimeTemp) > 100000)
				 {
					 return FLAG_TIME_OUT;//超时则失败返回
				 }
				 if(USART_GetFlagStatus(UART5,USART_FLAG_IDLE))
				 {
					 idle_flag = 1;//检测到空闲状态标志，表示一帧数据结束
					 break;
				 }
			 }
			
			 if(idle_flag)
			 {
				 break;
			 }
			 TimeTemp = SysTime;	
			 SendBuff[i]=UART5->DR;		//保存数据	
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
					 return FLAG_TIME_OUT;//超时则失败返回
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

		SendBuff[i] = 0;//结尾 截至符
		USB_DataSend.USB.protocol = 0;
		USB_DataSend.USB.tos = 26;	
		USB_DataSend.USB.data[0] = 8;
		USB_DataSend.USB.length = i + 1;
		memcpy(USB_DataSend.USB.data + 1, SendBuff,i + 1);
		USB_EP1_Send(USB_DataSend.data, USB_EP1_SEND_NUM + USB_HEAD_NUM);//发送数据
		DebugOut("BDS touchuan data:\r\n%s",SendBuff);
		DebugOut("BDS touchuan data len = %d, data[2] = %d, data[3] = %d\r\n",i + 1,USB_DataSend.data[2],USB_DataSend.data[3]);
		return 1;	
		
}



/********************************************************************************/
/*@函 数 名：BDS_touchuan_close(void)                                           */
/*@功    能：北斗透传关闭                                                       */
/*@修改日期：2016年8月9日                                                       */
/*@备注：                                                                      */
/******************************************************************************/
void BDS_touchuan_close(void)
{
    
    TIM_Cmd(TIM1,DISABLE);
    BDtimeflag=0;
}	
/********************************************************************************/
/*@函 数 名：BDS_map_open(void)                                                 */
/*@功    能：北斗地图模式打开                                                    */
/*@修改日期：2016年8月9日                                                       */
/*@备注：                                                                      */
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
        BDS_GET_Time();//获取时间信息 
    
		BDdata[BD_datanum++]='4';
        BDS_GET_Location();//获取经、纬度信息
		
		DebugOut("BDS map:%s\r\n",BDdata);

		BD_datanum=0;
		BDtimeflag=0; 		
	}
}
/********************************************************************************/
/*@函 数 名：BDS_map_close(void)                                                */
/*@功    能：北斗地图模式关闭                                                    */
/*@修改日期：2016年8月9日                                                       */
/*@备注：                                                                      */
/******************************************************************************/
void BDS_map_close(void)
{
    
    TIM_Cmd(TIM1,DISABLE);
    BDtimeflag=0;
}
/********************************************************************************/
/*@函 数 名：Tim1_Init(u16 period_num)                                          */
/*@功    能：定时器1初始化                                                      */
/*@修改日期：2016年8月9日                                                       */
/*@备注：                                                                      */
/******************************************************************************/
void Tim1_Init(u16 period_num)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	TIM_DeInit(TIM1);
	TIM_TimeBaseStructure.TIM_Period=period_num;//装载值
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//分频系数
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM1,TIM_FLAG_Update);
	//TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM1,ENABLE);
}
/*****************************************************************/
/*@函 数 名：BDS_GET_Time(void)                                  */
/*@功    能：获取时间信息                                        */
/*@修改日期：2016年7月30日                                       */
/*@备    注：将查找帧头部分封装到函数内部                        */
/*			 删除传入参数改用全局变量                            */
/*			 修复部分bug                                         */
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
            s = &buf[GetComma(1, buf)];//第一个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
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
/*@函 数 名：BDS_GET_Location(void)                                             */
/*@功    能：获取经、纬度信息                                                   */
/*@修改日期：2016年7月30日                                                      */
/*@备    注：将查找帧头部分封装到函数内部									   */
/*			 删除传入参数改用全局变量                                         */
/*			 修复部分bug                                                      */
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
            //获取纬度信息
			s = &buf[GetComma(2, buf)];//第2个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BDdata[BD_datanum+4] = BDdata[BD_datanum+3];
            BDdata[BD_datanum+3] = BDdata[BD_datanum+2];
            BDdata[BD_datanum+2] = '.';
			BD_datanum+=i;
			
            s = &buf[GetComma(3, buf)];//第2个逗号之后开始的第一个字符所在的地址
            BDdata[BD_datanum++] = *s;                    //latitude.NS
			     // BDdata[BD_datanum++] = '\0'; 
            
            //获取经度信息
            s = &buf[GetComma(4, buf)];//第2个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
						i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BDdata[BD_datanum+5] = BDdata[BD_datanum+4];
            BDdata[BD_datanum+4] = BDdata[BD_datanum+3];
            BDdata[BD_datanum+3] = '.';
			BD_datanum+=i;
			
            s = &buf[GetComma(5, buf)];//第2个逗号之后开始的第一个字符所在的地址
            BDdata[BD_datanum++] = *s;                    //latitude.EW
            BDdata[BD_datanum++] = '\0';
		}
	}
}


/*****************************************************************/
/*@函 数 名：BDS_GET_Height(void)                                 */
/*@功    能：获取高度信息                                        */
/*@修改日期：2016年8月11日                                       */
/*@备    注：将查找帧头部分封装到函数内部                        */
/*			 删除传入参数改用全局变量                            */
/*			 修复部分bug                                         */
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
    //查找帧头及特定标识符
	ch1 = buf[3];
	ch2 = buf[4];
	ch3 = buf[5];
	status = buf[GetComma(1, buf)];

	if (ch1 == 'G' && ch2 == 'G' && ch3 == 'A') //$BDGGA 
	{
		if (status != ',')
		{            
            s = &buf[GetComma(9, buf)];//第9个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BD_datanum+=i;
            
            s = &buf[GetComma(10, buf)];//第10个逗号之后开始的第一个字符所在的地址
            BDdata[BD_datanum++] = *s;     //海拔单位 M
            BDdata[BD_datanum++] = '\0';
		}
	}
}

/*****************************************************************/
/*@函 数 名：BDS_GET_Direction(void)                                  */
/*@功    能：获取航向信息                                        */
/*@修改日期：2016年8月11日                                       */
/*@备    注：将查找帧头部分封装到函数内部                        */
/*			 删除传入参数改用全局变量                            */
/*			 修复部分bug                                         */
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
            s = &buf[GetComma(8, buf)];//第8个逗号之后开始的第一个字符所在的地址
            i=GetComma(1, s);//s中第一个逗号的位置(逗号在第i个字符)
            i = i - 1;
            strncpy(BDdata+BD_datanum, s, i);
			BD_datanum+=i;
            BDdata[BD_datanum++] = '\0';
		}
	}
}


/********************************************************************************/
/*@函 数 名：BDS_GET(u8 type)                                                   */
/*@功    能：获取数据                                                           */
/*@修改日期：2016年7月30日                                                      */
/*@备   注：传入参数type为u8型，0->获取卫星数       返回值：type(0~6)            */
/*                         1->获取速度信息                 或返回7，表示出错    */
/*                         2->获取日期信息                                     */
/*                         3->获取时间信息                                     */
/*                         4->获取经、纬度信息                                 */
/*                         5->获取高度信息                                     */
/*                         6->获取航向信息                                     */
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
				BDS_GET_NumOfStl();//获取卫星数  *BDdata
				break;
        case 1:
				BDS_GET_Speed();//获取速度信息 
				break;
		case 2:
				BDS_GET_Date();//获取日期信息 
				break;
		case 3:
				BDS_GET_Time();//获取时间信息
				break;
		case 4:
				BDS_GET_Location();//获取经、纬度信息
				break;
		case 5:
				BDS_GET_Height();//获取高度信息
				break;
        case 6:
				BDS_GET_Direction();//获取航向信息
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

