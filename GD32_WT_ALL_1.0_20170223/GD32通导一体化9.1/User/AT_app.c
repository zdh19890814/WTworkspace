#include "include.h"

//uint8_t data[USB_DATA_SIZE + USB_HEAD_NUM];
//uint8_t ep1_data[100];



//usb_ProType USB_Send;
//usb_ProType USB_DataRec.USB_Rec;




u8 MuxOpenFlag = 0;


volatile SYSTIME SysTime = 0;//时间戳，每100us在中断 SysTick_Handler() 里自加一
SYSTIME TimeTemp = 0;
volatile u8 AT_OpenInitState = 0;
u8 AT_OpenSucceed = 0;
struct voice VoiceHandle[1]={0};
const u8 *OpenCommandSend[] =
{
	"AT",
	"AT+CGCLASS=\"CC\"",
	"AT+DELAY=0",
	
	"AT+CFUN=1",
	
	"AT+CMEE=1",
	"AT+CREG=1",
	"AT+CGREG=1",
	"AT+CGEREP=1",
	"AT^DSCI=1",

	"AT+CNMI=2,2,2,1,0",

	//	"ATE1\r\n",
	"AT+COPS=3,0;+COPS?;+COPS=3,1;+COPS?;+COPS=3,2;+COPS?",

	"AT+CGREG?",
	"AT+CREG?",
	"AT+CGEQREQ=1,3,0,0",
	//话音途中关闭PS域
	"AT^DEPCD=0",

	//text PDU command   (	ACK PDU?)
	"AT+CSMS=0",
	"AT+CNMI= 2,1,0,1,0",
	"AT+CSCS=\"HEX\"",
	"AT+CMGF=0",
	"AT+CGSMS=1",
	"AT+CMMS=1",

	"AT^DEPCD=0",

	"AT+CSMS=0",
	"AT+CNMI=1,2,2,1,0",//<mode1>[,<mt2>[,<bm2>[,<ds1>[,<bfr>]
	"AT+CSCS=\"GSM\"",
	"AT+CMGF=0",
	"AT+CGSMS=1",
	"AT+CSCA=\"+8613888888877\",144",
	"AT+CMMS=1",

};





const u8 * s_smsStartOfResponse[] = {
	"> ",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};



const u8 * s_smsEndOfResponse[] = {
	"OK",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};


const u8 * s_RescueSend[] = {
	"+SSENDSTATUS=2",
	"+SSENDSTATUS=0",
	"+SSENDSTATUS=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};










const u8 * s_VoiceCall[] = {
	"OK",
	"BUSY",
	"NO ANSWER",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};

const u8 * s_VoiceReceiveATA[] = {
	"OK",
	"ERROR",
};

const u8 * s_VoiceHungup[] = {
	"OK",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};

//u8 isEndOfResponse(u8 *line)
//{
//	size_t i;
//	for (i = 0; i < NUM_ELEMS(s_smsEndOfResponse); i++) {
//		if (strStartsWith(line, s_smsEndOfResponse[i])) {
//			return (i+1);
//		}
//	}
//	return 0;
//}
/*
1.在字符串 line 中寻找 lelms 包含的字符串
	opt = 0，以其开头
	opt = 1，与其相等
2.返回
	0：没有
	x：lelms中的第 (x-1)个字符串

*/




/*****************************************************************
@函 数 名：SeekStr(const u8 **lelms,size_t num, u8 *line,u8 opt)                           
@功    能：在字符串 line 中寻找 lelms 包含的字符串
@输    入：lelms 指向预期可能回复的内容（num为0时，lelms为一级指针）
					 num 预期回复可能的个数
					 opt 比较类型 	
							opt = 0，以其开头
							opt = 1，与其相等
					 timeout_ms 超时时间
@输    出：返回为i表示等到了lelms[i-1]的内容 0表示没有
@备    注：无                                       
*****************************************************************/
size_t SeekStr(const u8 **lelms,size_t num, u8 *line,u8 opt)
{
	size_t i;
	if(opt == 0)//flag = 0，以其开头
	{
		if(num == 0)
		{
			if(strStartsWith(line, (const u8 *)lelms)) 
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			for (i = 0; i < num; i++) 
			{
				if(strStartsWith(line, lelms[i])) 
				{
					return (i+1);
				}
			}
			return 0;
		}
	}
	else
	{
		if(num == 0)
		{
			if(strcmp((const char *)line, (const char *)lelms) == 0) 
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			for (i = 0; i < num; i++) 
			{
				if(strcmp((const char *)line, (const char *)lelms[i]) == 0) 
				{
					return (i+1);
				}
			}
			return 0;
		}
	}
}
/*****************************************************************
@函 数 名：Gpio_Init(void)                                 
@功    能：Gpio 初始化
@输    入：无
@输    出：无
@备    注：包括USB使能，AT，BD使能，USB使能直接打开                                                          
*****************************************************************/
void Gpio_Init(void)
{
#ifdef GD32
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);	 //使能端口时钟

 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_PW_BD;
 GPIO_Init(GPIO_PW_BD, &GPIO_InitStructure);					
 PW_BD_OFF;
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_PW_AT;
 GPIO_Init(GPIO_PW_AT, &GPIO_InitStructure);					
 PW_AT_OFF;
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_USB_EN;
 GPIO_Init(GPIO_USB_EN, &GPIO_InitStructure);					
 USB_EN_ON;
#endif
}

const u8 * s_Message[] = {
	"+CMT:",
	"+CDS:",
	"+CBM:"
};
/*****************************************************************
@函 数 名：UnsolicitedHandle(u8 *com)                               
@功    能：处理卫通模块的指令
@输    入：命令（不包含\r\n F9）
@输    出：处理成功返回1，失败返回0
@备    注：无                                       
*****************************************************************/
u8 UnsolicitedHandle(u8 *com)
{
	u8 * temp;
	if(strStartsWith(com,"^ORIG"))//语音相关
	{
		if(VoiceHandle[0].state == VOICE_CALL_STATE_CALL_SEND)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_CALL_START;
		}
		/*else if(VoiceHandle[0].state == VOICE_CALL_STATE_RECEIVE_GET)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_RECEIVE_START;
		}*/
		else
		{
			DebugOut("[error] state unknown at line : %d\r\n",__LINE__);
		}
	}
	else if(strStartsWith(com,"^CONF"))//语音相关
	{
		if(VoiceHandle[0].state == VOICE_CALL_STATE_CALL_START)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_CALL_CONNECT;
		}
		else
		{
			DebugOut("[error] state unknown at line : %d\r\n",__LINE__);
		}
	}
	else if(strStartsWith(com,"^CONN"))//语音相关
	{
		if(VoiceHandle[0].state == VOICE_CALL_STATE_CALL_CONNECT)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_CALL_ACCEPT;//接通
		}
		else if(VoiceHandle[0].state == VOICE_CALL_STATE_RECEIVE_AGREE)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_RECEIVE_CONNECT;
		}
		else
		{
			DebugOut("[error] state unknown at line : %d\r\n",__LINE__);
		}
	}
	else if(strStartsWith(com,"NO CARRIER"))//语音相关
	{
		if(VoiceHandle[0].state != VOICE_CALL_STATE_IDLE)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_IDLE;
		}
		else
		{
			DebugOut("[error] state unknown at line : %d\r\n",__LINE__);
		}
	}
	else if(strStartsWith(com,"RING"))//语音相关
	{
		if(VoiceHandle[0].state == VOICE_CALL_STATE_IDLE)
		{
			//VoiceHandle[0].state = VOICE_CALL_STATE_RECEIVE_GET;
			VoiceHandle[0].state = VOICE_CALL_STATE_RECEIVE_START;
		}
		else
		{
			
			DebugOut("[error] state unknown at line : %d\r\n",__LINE__);
		}
	}
	else if(strStartsWith(com,"^CEND"))//语音相关
	{
		if(VoiceHandle[0].state != VOICE_CALL_STATE_IDLE)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_IDLE;
		}
		else
		{
			DebugOut("[error] state unknown at line : %d\r\n",__LINE__);
		}
	}
	else if(strStartsWith(com,"^SIMST: 1"))//开机回复，上电后收到次回复才能进行初始化
	{
		AT_OpenInitState = AT_OPEN_INIT_STATE_READY;
	}
	
	else if(strStartsWith(com,"^SIMST: 0"))//开机回复
	{
		AT_OpenInitState = AT_OPEN_INIT_STATE_0;
		AT_OpenSucceed = 0;
	}
	else if(strStartsWith(com,"^DSCI"))//通话状态上报信息
	{
		DebugOut("DSCI: %s\r\n",com);
		DSCI_Handler(com);//解析该指令，目前只解析号码（尚未测试）
	}
	else if(strStartsWith(com,"+SSENDSTATUS="))//紧急求生状态发送应答
	{
		if(com[13] == '0')
		{
			DebugOut("rescue back secceed\r\n");//应答成功
		}
		else if(com[13] == '1')
		{
			DebugOut("[fail] rescue back: %s\r\n",com);//应答失败
		}
		else if(com[13] == '2')
		{
			DebugOut("[fail] rescue back\r\n");//不会出现这种情况
		}
		else
		{
			DebugOut("[fail] rescue back: %s\r\n",com);
		}
		
		USB_DataSend.USB.tos = 3;
		USB_DataSend.USB.data[0] = 4;
		USB_DataSend.USB.data[1] = com[13] - '0' + 1;
		USB_SendCom();
	}
	else if(strStartsWith(com,"MODE:"))//入网成功
	{
		USB_DataSend.USB.tos = 1;
		USB_DataSend.USB.data[0] = 5;
		USB_DataSend.USB.data[1] = 1;
		USB_SendCom();
	}
	//else if(SeekStr(s_Message,NUM_ELEMS(s_Message), com,0))
	else if(isSMSUnsolicited(com))//收到短信，发给USB手机
	{
		temp = (u8 *)strstr((const char *)com,"\n");
		if(temp != NULL)
		{
			temp ++;
			USB_SendMes(temp);
		}
		else
		{
			DebugOut("[error] message recrlve : %s\r\n",com);
		}
		
	}
	else
	{
		UnexpectedHandle(com);
		return 0;
	}
	CallStateHandle();
	DebugOut("handled commend: %s\r\n",com);
	return 1;
}

/*****************************************************************
@函 数 名：DSCI_Handler(u8* com)                               
@功    能：解析DSCI 指令
@输    入：无
@输    出：无
@备    注：无                                       
*****************************************************************/
void DSCI_Handler(u8* com)
{
	u8* temp;
	u8 i;
	u8 len;
	u8 flag = 0;
	temp = (u8 *)strstr((const char *)com,"\"");
	
	if(temp != NULL)
	{
		temp ++;
		len = LEN_NUM < strlen((const char *)temp)?LEN_NUM:strlen((const char *)temp);
		for(i = 0;i < len;i++)
		{
			if(temp[i] == '"')
			{
				break;
			}
			else
			{
				if(VoiceHandle[0].num[i] != temp[i])
				{
					flag = 1;
					VoiceHandle[0].num[i] = temp[i];
				}
			}
		}
		if(VoiceHandle[0].num[i] != 0)
		{
			VoiceHandle[0].num[i] = 0;
			flag = 1;
		}
		if(flag)
		{
			SendCallState();
		}
	}
	else
	{
		DebugOut("[error] DSCI handle: %s\r\n",com);
	}
}
void UnexpectedHandle(u8 *com)
{
	//TODO:handle
	DebugOut("unexpected commend: %s\r\n",com);
}



void AddDetailInfo()
{
	memcpy(ResponseBuff[ResponseBufIndex++], (const char *)ComBuf[ComBufIndexStart], strlen((const char *)ComBuf[ComBufIndexStart]) + 1);
}


/*****************************************************************
@函 数 名：WaitForStr(const u8 **lelms,size_t num,u16 timeout_ms,u8 opt)                               
@功    能：发送玩指令等待卫通模块回复
@输    入：
					lelms 指向预期可能回复的内容（num为0时，lelms为一级指针）
					num 预期回复可能的个数
					opt 比较类型 	
						opt = 0，以其开头
						opt = 1，与其相等
					timeout_ms 超时时间
@输    出：
					返回为i表示等到了lelms[i-1]的内容（不会返回0）默认返回1表示成功
					超时返回 FLAG_TIME_OUT
@备    注：无                                       
*****************************************************************/
u8 WaitForStr(const u8 **lelms,size_t num,u16 timeout_ms,u8 opt)
{
	int flag = 0;
	TimeTemp = SysTime;
	ResponseBufIndex = 0;
	while (1)
	{
		if ((SysTime - TimeTemp) > timeout_ms * 10)
		{
			if(num)
				DebugOut("[timeout] wait for %s\r\n",lelms[0]);
			else
				DebugOut("[timeout] wait for %s\r\n",(const char *)lelms);
			return FLAG_TIME_OUT;
		}
		MuxHandle();
		for (;ComBufIndexEnd != ComBufIndexStart;)
		{
			if(flag == 0)
			{
				flag = SeekStr(lelms,num,ComBuf[ComBufIndexStart],opt);
				if(flag == 0)
				{
					if(UnsolicitedHandle(ComBuf[ComBufIndexStart]) == 0)
					{
						AddDetailInfo();
					}
				}
				else
				{
					AddDetailInfo();
				}
			}
			else
			{
				UnsolicitedHandle(ComBuf[ComBufIndexStart]);
			}
			
			ComBufIndexPlusPlus(ComBufIndexStart);
			if(flag)\
			{\
				return flag;\
			}\
		}//得到想要的结果就返回
		
		//}\
		if(flag)\
		{\
			return flag;\
		}//处理完当前所有消息后再返回
	}
}

void MuxHandle()
{	
#ifdef MUX_MAIN
	int sum;
	for (; MuxBufIndexEnd != MuxBufIndexStart;)//解析接收到的数据
	{
		sum = AT_MuxBuff[MuxBufIndexStart][0]*256 + AT_MuxBuff[MuxBufIndexStart][1];//获取数据长度
		get((char *)AT_MuxBuff[MuxBufIndexStart] + 2,sum);//解析
		//AT_MuxBuffHandler(AT_MuxBuff[MuxBufIndexStart] + 2,sum);
		MuxBufIndexPlusPlus(MuxBufIndexStart);
	}
	#endif
}


/*****************************************************************
@函 数 名：AT_ComHandle(void)                                 
@功    能：处理卫通模块发出的AT指令
@输    入：无
@输    出：无
@备    注：无                                       
*****************************************************************/
void AT_ComHandle()
{
	MuxHandle();//打开Mux时，如果选择在主函数解析MUX发出指令，则在此处理中断接收到的数据，解析出来等待处理
	
	for (;ComBufIndexEnd != ComBufIndexStart;)//处理解析完的指令
	{
		UnsolicitedHandle(ComBuf[ComBufIndexStart]);//处理该指令
		ComBufIndexPlusPlus(ComBufIndexStart);//指向下一条指令
	}
}

/*****************************************************************
@函 数 名：SendCallState(void)                                 
@功    能：向手机USB发送通话状态
@输    入：无
@输    出：无
@备    注：无                                       
*****************************************************************/
void SendCallState(void)
{
	USB_DataSend.USB.tos = 1;
	USB_DataSend.USB.length = strlen((const char *)VoiceHandle[0].num) + 2;
	USB_DataSend.USB.data[0] = 4;
	USB_DataSend.USB.data[1] = VoiceHandle[0].state;
	memcpy(USB_DataSend.USB.data + 2, VoiceHandle[0].num, strlen((const char *)VoiceHandle[0].num) + 1);//号码
	USB_SendCom();//发送给USB
}

/*****************************************************************
@函 数 名：CallStateHandle(void)                                 
@功    能：处理通话状态发生变化
@输    入：无
@输    出：无
@备    注：无                                       
*****************************************************************/
void CallStateHandle(void)
{
	static u8 call_state_last = 0;
	static u8 voice_call_last = 0;
		
	if(call_state_last != VoiceHandle[0].state)//通话状态发生变化，需要上报USB
	{
		call_state_last = VoiceHandle[0].state;
		DebugOut("call state %d\r\n",VoiceHandle[0].state);

		SendCallState();

		if((VoiceHandle[0].state == VOICE_CALL_STATE_CALL_ACCEPT)||(VoiceHandle[0].state == VOICE_CALL_STATE_RECEIVE_CONNECT))//通话与否
		{
			VoiceCallFlag = 1;
		}
		else
		{
			VoiceCallFlag = 0;
		}
		
		
		
		
		
		
		
#ifdef I2S_ON_OFF
		if(VoiceHandle[0].state == VOICE_CALL_STATE_IDLE)
		{
  		 DMA_Cmd(DMA2_Channel2, DISABLE); 	
			 memset(I2S_DMA_DoubleBufferSend,0,I2S_SEND_SIZE * 2);//清零消除杂音
	
			 I2S_Cmd(SPI2, DISABLE);
			 I2S_Cmd(SPI3, DISABLE);
			//DebugOut("IIS DISABLE\r\n");
		}
		
		if((VoiceHandle[0].state == VOICE_CALL_STATE_CALL_CONNECT) || (VoiceHandle[0].state == VOICE_CALL_STATE_RECEIVE_CONNECT))
		{
		
      DMA_Cmd(DMA2_Channel2, ENABLE); 
			I2S_Cmd(SPI2, ENABLE);
			I2S_Cmd(SPI3, ENABLE);
			//DebugOut("IIS ENABLE\r\n");
		}
#endif				
			
			
			
			
//			if(VoiceCallFlag)
//			{
//				I2S_Cmd(SPI2, ENABLE);
//				I2S_Cmd(SPI3, ENABLE);
//			}
//			else
//			{
//				I2S_Cmd(SPI2, DISABLE);
//				I2S_Cmd(SPI3, DISABLE);
//			}
	
		
		
		
		
		
		
		
		
		
		
		
		
		
		if(voice_call_last != VoiceCallFlag)//通话与否变化
		{
			voice_call_last = VoiceCallFlag;

		}
	}
}





//void PrintAllComAndMsm()
//{
//	for (; ComBufIndexEnd != ComBufIndexStart;)
//	{
//		DebugOut("New commend arrived:[%d]\r\n%s\r\n\r\n",ComBufIndexStart,ComBuf[ComBufIndexStart]);
//		ComBufIndexPlusPlus(ComBufIndexStart);
//	}
//	/*for (; MsmBufIndexEnd != MsmBufIndexStart;)
//	{
//		DebugOut("New message arrived:[%d]\r\n%s\r\n\r\n",MsmBufIndexStart,MsmBuf[MsmBufIndexStart]);
//		MsmBufIndexPlusPlus(MsmBufIndexStart);
//	}	*/
//}



//u8 AT_SendComReq(u8 * str)
//{
//	if(MuxOpenFlag == 0)
//	{
//		DebugOut("AT send:");
//		if(AT_SendDirect(str) == 1)
//		{
//			DebugOut("%s\r\n",str);
//			return 1;
//		}
//		else
//		{
//			DebugOut("[fail] %s\r\n",str);
//			return 0;
//		}
//	}
//	else
//	{
//		DebugOut("Mux send]:");
//		if(AT_SendMux(str) == 1)
//		{
//			
//			DebugOut("%s\r\n",str);
//			return 1;
//		}
//		else
//		{
//			DebugOut("[fail] %s\r\n",str);
//			return 0;
//		}
//	}
//}




u8 AT_SendComReq(u8 * str)
{
	return AT_SendDataReq(str,strlen((const char *)str));
}

/*****************************************************************
@函 数 名：AT_SendDataReq(u8 * data,u16 len)           
@功    能：卫通模块发生数据
@输    入：data 待发生数据，len 数据长度
@输    出：发送成功返回1，失败返回0（一般由于流控引起）
@备    注：
*****************************************************************/
u8 AT_SendDataReq(u8 * data,u16 len)
{
	if(MuxOpenFlag == 0)
	{
		DebugOut("AT send:");
		if(AT_SendDataWithEnd(data, len) == 1)
		{
			DebugPrintData(data, len);
			DebugOut(" OK\r\n");
			return 1;
		}
		else
		{
			DebugPrintData(data, len);
			DebugOut(" [fail]\r\n");
			return 0;
		}
	}
	else
	{
		DebugOut("Mux send:");
		if(AT_SendDataMux(data,len) == 1)
		{
			DebugPrintData(data, len);
			DebugOut(" OK\r\n");
			return 1;
		}
		else
		{
			DebugPrintData(data, len);
			DebugOut(" [fail] !\r\n");
			return 0;
		}
	}
}



/*****************************************************************
@函 数 名：PrintInfo(void)                                 
@功    能：等待返回结果过程中收到的其他消息
@输    入：无
@输    出：无
@备    注：目前仅仅用于打印                                        
*****************************************************************/

void PrintInfo()
{
	u8 j;
	for (j = 0; j < ResponseBufIndex; j++)
	{
		DebugOut("--%d)info: %s\r\n",j,ResponseBuff[j]);
	}
	//DebugOut("\r\n");
}


/*****************************************************************
@函 数 名：PowerOnInit(void)                                 
@功    能：上电后AT模块初始化
@输    入：无
@输    出：无
@备    注：需先收到上电成功返回的四条指令后调用AT_Init()进行初始化                                          
*****************************************************************/
void PowerOnInit()
{
	DebugOut("wait for AT model power on ...\r\n");
	TimeTemp = SysTime;
	while(AT_OpenInitState == AT_OPEN_INIT_STATE_0)//等待模块上电后的四条的指令，一般30s内会出现
	{
		if((SysTime - TimeTemp) > WAIT_INIT_WAIT)
		{
			break;
		}
		AT_ComHandle();
	}
	
	if((SysTime - TimeTemp) <= WAIT_INIT_WAIT)//收到四条指令
	{
		AT_OpenSucceed = AT_Init();
	}
	else//超时
	{
		DebugOut("[fail] AT init\r\n");
	}
}


/*****************************************************************
@函 数 名：AT_Init(void)                                 
@功    能：初始化模块
@输    入：无
@输    出：成功返回1
@备    注：依次发送OpenCommandSend包含的指令进行初始化                                       
*****************************************************************/

u8 AT_Init()
{
	int i = 0;
	u8 flag;
	u8 flag_time_out = 0;
	int num = NUM_ELEMS(OpenCommandSend);
	int num_ok = 0;
	DebugOut("AT init ......\r\n");
	for (i = 0; i < num; i++)	
	{
		DebugOut("[%d] ",i);
		if(AT_SendComReq(OpenCommandSend[i])!= 1)
		{
			return FLAG_SEND_FAIL;
		}
		
		if(i == 3)//CFUN 需要的时间较长
		{
			flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);
		}
		else
		{
			flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);
		}
		
		PrintInfo();
		if(flag == FLAG_TIME_OUT)								
		{
			flag_time_out = 1;
		}
		else
		{
			if(flag == 1)			
			{
				num_ok++;
			}
			DebugOut("[%d] result: %s\r\n",i,s_smsEndOfResponse[flag-1]);
		}
		DebugOut("\r\n");
	}
	DebugOut("AT init finished\r\n");
	DebugOut("%d commends is sent,and get %d OKs\r\n\r\n",num,num_ok);
	
	
	
#ifdef MUX_ON1
	if(AT_SendComReq("AT+CMUX=0,0,5,1600")!= 1)//打开MUX
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);

	if(flag == FLAG_TIME_OUT)								
	{
		flag_time_out = 1;
	}
	else
	{
		if(flag == 1)			
		{
			MuxOpenFlag = 1;
			DebugOut("MUX open OK.\r\n");
			ChannelOpen();//打开通道
		}
		else
		{
			DebugOut("[fail] Mux open: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
#endif
	
	if(flag_time_out == 1)
	{
		return FLAG_TIME_OUT;
	}
	else
	{
		if(num == num_ok)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
}
/*****************************************************************
@函 数 名：AT_VoiceCall(u8 *number)                                 
@功    能：拨打电话
@输    入：number 号码字符串指针 '\0'结尾
@输    出：成功返回1
@备    注：                                       
*****************************************************************/
u8 AT_VoiceCall(u8 *number)
{
	u8 flag;
	u8 CallSend[LEN_NUM + 5]="ATD\0";
	if(strlen((const char *) number) >= LEN_NUM)//号码长度过长
	{
		DebugOut("number is too long:%s\r\n",number);
		return 20;
	}
	memcpy(VoiceHandle[0].num, (const char *)number,1 + strlen((const char *) number));
	if(VoiceHandle[0].state == VOICE_CALL_STATE_IDLE)//空闲状态时才能拨号
	{
		strcat((char *)CallSend, (const char *)number);//拼接号码
		strcat((char *)CallSend, ";\0");
		DebugOut("voice call ......\r\n");
		
		if(AT_SendComReq(CallSend) != 1)//发送打电话指令（形如"ATD10086;"）
		{
			return FLAG_SEND_FAIL;
		}			
		flag = WaitForStr(s_VoiceCall,NUM_ELEMS(s_VoiceCall),WAIT_AT_TIME_MS_L1,0);//等待返回
		PrintInfo();//等待返回结果过程中收到的其他消息
		if(flag == 1)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_CALL_SEND;
			DebugOut("voice call OK\r\n");
		}
		else 
		{
			if(flag != FLAG_TIME_OUT)											//超时
			{
				DebugOut("[fail] voice call: %s\r\n",s_VoiceCall[flag-1]);
			}
		}
		return flag;
	}
	else
	{
		DebugOut("[fail] voice call unknow state: %d\r\n",VoiceHandle[0].state);
		return 0;
	}
		

}
/*****************************************************************
@函 数 名：AT_VoiceReceive()                               
@功    能：接通当前来电
@输    入：无
@输    出：成功返回1
@备    注：                           
*****************************************************************/
u8 AT_VoiceReceive()
{
	u8 flag;
	if(VoiceHandle[0].state == VOICE_CALL_STATE_RECEIVE_START)
	{
		DebugOut("voice receive] ......\r\n");
		if(AT_SendComReq("ATA")!= 1)
		{
			return FLAG_SEND_FAIL;
		}
		flag = WaitForStr(s_VoiceReceiveATA,NUM_ELEMS(s_VoiceReceiveATA),WAIT_AT_TIME_MS_L1,0);
		
		PrintInfo();
		
		if(flag == 1)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_RECEIVE_AGREE;
			DebugOut("voice receive OK\r\n");
		}
		else
		{
			if(flag != FLAG_TIME_OUT)											//超时 
			{
				DebugOut("[fail] voice receive: %s\r\n",s_VoiceReceiveATA[flag-1]);
			}
		}
		return flag;
	}
	else
	{
		DebugOut("[fail] voice receive unknow state: %d\r\n",VoiceHandle[0].state);
		return 0;
	}
}

/*****************************************************************
@函 数 名：AT_VoiceHangup()                               
@功    能：挂断当前所有通话
@输    入：无
@输    出：成功返回1
@备    注：                           
*****************************************************************/
u8 AT_VoiceHangup()
{
	u8 flag;
	if(VoiceHandle[0].state != VOICE_CALL_STATE_IDLE)
	{
		DebugOut("voice hungup] ......\r\n");
		if(AT_SendComReq("AT+CHUP")!= 1)
		{
			return FLAG_SEND_FAIL;
		}
		
		flag = WaitForStr(s_VoiceHungup,NUM_ELEMS(s_VoiceHungup),WAIT_AT_TIME_MS_L1,0);
		
		PrintInfo();
		
		if(flag == 1)
		{
			//VoiceHandle[0].state = VOICE_CALL_STATE_IDLE;
			DebugOut("voice hungup OK\r\n");

		}
		else
		{
			if(flag != FLAG_TIME_OUT)											//超时
			{
				DebugOut("[fail] voice hungup: %s\r\n",s_VoiceHungup[flag-1]);
			}

		}
		return flag;
	}
	else
	{
		DebugOut("[fail] voice hungup unknow state: %d\r\n",VoiceHandle[0].state);
		return 0;
	}
}


/*****************************************************************
@函 数 名：AT_RescueStateSend()                               
@功    能：紧急救生状态发送
@输    入：无
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueStateSend(void)
{
	u8 flag;

	if(AT_SendComReq("AT+SSENDSTATUS")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueSend,NUM_ELEMS(s_RescueSend),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue send]  OK: %s\r\n",s_RescueSend[flag-1]);
	}
	else
	{
		if(flag != FLAG_TIME_OUT)									
		{
			DebugOut("[fail] rescue send: %s\r\n",s_RescueSend[flag-1]);
		}
	}
	return flag;
}



const u8 * s_RescueTestSend[] = {
  "+STESTMODEON=0",
	"+STESTMODEON=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};


/*****************************************************************
@函 数 名：AT_RescueTestSendOpen()                               
@功    能：紧急救生自测打开
@输    入：无
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/

u8 AT_RescueTestSendOpen(void)
{
	u8 flag;
	u8 com[17] = "AT+STESTMODEON=";
	com[15] = 5;
	com[16] = '\0';
	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueTestSend,NUM_ELEMS(s_RescueTestSend),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue TestSendOpen]  OK: %s\r\n",s_RescueTestSend[flag-1]);
	}
	else
	{
		if(flag != FLAG_TIME_OUT)									
		{
			DebugOut("[fail] rescue TestSendOpen: %s\r\n",s_RescueTestSend[flag-1]);
		}
	}
	return flag;
	
}


/*****************************************************************
@函 数 名：AT_RescueTestSendClose()                               
@功    能：紧急救生自测关闭
@输    入：无
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueTestSendClose(void)
{
		u8 flag;
	u8 com[17] = "AT+STESTMODEON=";
	com[15] = 0xff;
	com[16] = '\0';
	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueTestSend,NUM_ELEMS(s_RescueTestSend),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue TestSendClose]  OK: %s\r\n",s_RescueTestSend[flag-1]);
	}
	else
	{
		if(flag != FLAG_TIME_OUT)									
		{
			DebugOut("[fail] rescue TestSendClose: %s\r\n",s_RescueTestSend[flag-1]);
		}
	}
	return flag;
	
}



const u8 * s_RescueMesSend[] = {
  "+SSENDSOS=0",
	"+SSENDSOS=2",
	"+SSENDSOS=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};

/*****************************************************************
@函 数 名：AT_RescueMesSend()                               
@功    能：紧急救生消息发送
@输    入：无
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueMesSend(void)
{
	u8 flag;

	if(AT_SendComReq("AT+SSENDSOS")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueMesSend,NUM_ELEMS(s_RescueMesSend),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue MesSend  OK: %s\r\n",s_RescueMesSend[flag-1]);
	}
	else
	{
		if(flag != FLAG_TIME_OUT)									
		{
			DebugOut("[fail] rescue MesSend: %s\r\n",s_RescueMesSend[flag-1]);
		}
	}
	return flag;
	
}





const u8 * s_RescueSetFre[] = {
	"+SSETFREQ=0",
	"+SSETFREQ=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};
/*****************************************************************
@函 数 名：AT_RescueSetFre(u16 sendfre,u16 recfre)                             
@功    能：紧急救生发送频点设置
@输    入：sendfre 发送频点，recfre接收频点
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSetFre(u16 sendfre,u16 recfre)
{
	u8 flag;
	u8 temp[8];
	u8 com[30] = "AT+SSETFREQ=";

	myitoa(sendfre,temp,0);
	strcat((char *)com,(const char *)temp);
	
	strcat((char *)com,",");
	
	myitoa(recfre,temp,0);
	strcat((char *)com,(const char *)temp);

	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueSetFre,NUM_ELEMS(s_RescueSetFre),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue set fre OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//3?ê±
		{
			DebugOut("[fail] rescue set fre: %s\r\n",s_RescueSetFre[flag-1]);
		}
	}
	return flag;
}



const u8 * s_RescueSetGain[] = {
	"+SSETGAIN=0",
	"+SSETGAIN=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};
/*****************************************************************
@函 数 名：AT_RescueSetGain(u8 gain)                          
@功    能：紧急救生发送增益设置
@输    入：gain 增益
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSetGain(u8 gain)
{
	u8 flag;
	u8 temp[8];
	u8 com[30] = "AT+SSETGAIN=";

	myitoa(gain,temp,0);
	strcat((char *)com,(const char *)temp);

	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	
	flag = WaitForStr(s_RescueSetGain,NUM_ELEMS(s_RescueSetGain),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue set gain OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//3?ê±
		{
			DebugOut("[fail] rescue set gain: %s\r\n",s_RescueSetGain[flag-1]);
		}
	}
	return flag;
}

const u8 * s_RescueSetCount[] = {
	"+SSETCOUNT=0",
	"+SSETCOUNT=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};
/*****************************************************************
@函 数 名：AT_RescueSetCount(u16 count)                        
@功    能：紧急救生发送次数设置
@输    入：count 次数
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSetCount(u16 count)
{
	u8 flag;
	u8 temp[8];
	u8 com[30] = "AT+SSETCOUNT=";

	myitoa(count,temp,0);
	strcat((char *)com,(const char *)temp);

	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueSetCount,NUM_ELEMS(s_RescueSetCount),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue set count OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//3?ê±
		{
			DebugOut("[fail] rescue set count: %s\r\n",s_RescueSetCount[flag-1]);
		}
	}
	return flag;
}



const u8 * s_RescueSetInterval[] = {
	"+SSETINTERVAL=0",
	"+SSETINTERVAL=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};
/*****************************************************************
@函 数 名：AT_RescueSetInterval(u16 interval)                      
@功    能：紧急救生发送间隔设置
@输    入：interval 间隔
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSetInterval(u16 interval)
{
	u8 flag;
	u8 temp[8];
	u8 com[30] = "AT+SSETINTERVAL=";

	myitoa(interval,temp,0);
	strcat((char *)com,(const char *)temp);

	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueSetInterval,NUM_ELEMS(s_RescueSetInterval),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue set interval OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)		 
		{
			DebugOut("[fail] rescue set interval: %s\r\n",s_RescueSetInterval[flag-1]);
		}
	}
	return flag;
}


const u8 * s_RescueSetArmy[] = {
	"+SSETARMY=0",
	"+SSETARMY=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};
/*****************************************************************
@函 数 名：AT_RescueSetArmy(u8 army)                    
@功    能：紧急救生军种设置
@输    入：army 军种
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSetArmy(u8 army)
{
	u8 flag;
	u8 temp[8];
	u8 com[30] = "AT+SSETARMY=";

	myitoa(army,temp,0);
	strcat((char *)com,(const char *)temp);

	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueSetArmy,NUM_ELEMS(s_RescueSetArmy),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue set army OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)		
		{
			DebugOut("[fail] rescue set army: %s\r\n",s_RescueSetArmy[flag-1]);
		}
	}
	return flag;
}

const u8 * s_RescueSetOffset[] = {
	"+SSETFO=0",
	"+SSETFO=1",
	"ERROR:",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"TOO MANY PARAMETER"
};
/*****************************************************************
@函 数 名：AT_RescueSetOffset(s16 offset)                   
@功    能：紧急救生频偏设置
@输    入：offset 频偏
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSetOffset(s16 offset)
{
	u8 flag;
	u8 temp[8];
	u8 com[30] = "AT+SSETFO=";

	myitoa(offset,temp,1);
	strcat((char *)com,(const char *)temp);

	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_RescueSetOffset,NUM_ELEMS(s_RescueSetOffset),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue set offset OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)		
		{
			DebugOut("[fail] rescue set offset: %s\r\n",s_RescueSetOffset[flag-1]);
		}
	}
	return flag;
}
/*****************************************************************
@函 数 名：AT_RescueSet(u8 *par)                  
@功    能：紧急救生 设置
@输    入：par 设置数据流
@输    出：发送成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueSet(u8 *par)
{
	u8 flag = 0;
	u8  temp0;
	u16 temp1;
	u16 temp2;
	s16 temp3;
	temp1 = par[0]*256 + par[1];
	temp2 = par[2]*256 + par[3];
	if(AT_RescueSetFre(temp1,temp2) != 1)	//设置频点
	{
		flag |= 0x02;
	}	
	temp0 = par[4];
	if(AT_RescueSetGain(temp0) != 1)	//设置增益
	{
		flag |= 0x04;
	}
	temp1 = par[5]*256 + par[6];
	if(AT_RescueSetCount(temp1) != 1)	//设置发送次数
	{
		flag |= 0x08;
	}
	temp1 = par[7]*256 + par[8];
	if(AT_RescueSetInterval(temp1) != 1)	//设置发送间隔
	{
		flag |= 0x10;
	}	
	temp1 = par[9];
	if(AT_RescueSetArmy(temp1) != 1)	//设置军种
	{
		flag |= 0x20;
	}
	temp3 = par[7]*256 + par[8];
	if(AT_RescueSetOffset(temp3) != 1)	//设置频偏
	{
		flag |= 0x40;
	}
	if(flag == 0)
		flag = 1;

	return flag;
}


/*****************************************************************
@函 数 名：myitoa(s16 num,u8 * str,u8 sign)                  
@功    能：数字转字符串
@输    入：num待转换数字，str转换后存储位置的指针，sign是否有符号
@输    出：无
@备    注：                           
*****************************************************************/

void myitoa(s16 num,u8 * str,u8 sign)
{
	int i = 0;
	u16 num1 = (u16)num;
	
	u8 temp;
	u8 flag = 0;
	u16 ten = 10000;
	if(sign)
	{
		if(num < 0)
		{
			str[i++] = '-';
			num1 = -num;
		}
	}
	
	while(ten >= 1)
	{
		temp = num1 / ten;
		num1 = num1 % ten;
		if(temp)	flag = 1;
		if(flag)	str[i++] = temp + '0';
		ten /= 10;
	}
	if(i == 0)
	{
		str[i++] = '0';
	}
	str[i++] = '\0';
}


//	if((sign)&&(num < 0))
//	{
//		if(i == 1)
//		{
//			str[i++] = '0';
//		}
//	}
//	else
//	{
//		if(i == 0)
//		{
//			str[i++] = '0';
//		}
//	}

/*****************************************************************
@函 数 名：AT_MessageSend(u8 len,u8 * mes)                 
@功    能：发送短信
@输    入：len PDU长度 mes短信内容
@输    出：成功返回1
@备    注：                           
*****************************************************************/
u8 AT_MessageSend(u8 len,u8 * mes)
{
	u8 flag;
	u8 com[12]="AT+CMGS=";
	u8 lenS[5];
	myitoa(len,lenS,0);
	com[8] = '\0';
	strcat((char *)com, (const char *)lenS);

	DebugOut("%s ......\r\n",com);
	if(AT_SendComReq(com)!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	
	flag = WaitForStr(s_smsStartOfResponse,NUM_ELEMS(s_smsStartOfResponse),WAIT_AT_TIME_MS_L1,0);

	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("%s OK\r\n",com);
		DebugOut("message send ......\r\n");
		if(AT_SendComReq(mes)!= 1)
		{
			return FLAG_SEND_FAIL;
		}
		flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);
		PrintInfo();
		
		//DebugOut("flag = [%d] \r\n",flag);
		if(flag == 1)
		{
			DebugOut("message send OK\r\n");
		}
		else
		{
			if(flag != FLAG_TIME_OUT)											//超时
			{
				DebugOut("[fail] message send: %s\r\n",s_smsEndOfResponse[flag-1]);
			}
		}
		
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//超时
		{
			DebugOut("[fail] AT+CMGS=20: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}



/*****************************************************************
@函 数 名：AT_RescueOpen()               
@功    能：紧急救生打开
@输    入：无
@输    出：成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueOpen()
{
	u8 flag;
	DebugOut("rescue open ......\r\n");
	if(AT_SendComReq("AT+CFUN=8")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);//xxx
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue open OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT) 
		{
			DebugOut("[fail] rescue open: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}
/*****************************************************************
@函 数 名：AT_RescueOpen()               
@功    能：紧急救生关闭
@输    入：无
@输    出：成功返回1
@备    注：                           
*****************************************************************/
u8 AT_RescueClose()
{
	u8 flag;
	DebugOut("rescue close] ......\r\n");

	if(AT_SendComReq("AT+CFUN=7")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue close OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)
		{
			DebugOut("[fail] rescue close: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}




/*
u8 AT_RescueOpen()
{
	u8 flag;
	DebugOut("rescue open ......\r\n");
	AT_SendComReq("AT+CRESCUE=0");
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue open OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)
		{
			DebugOut("[fail] rescue open: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}

u8 AT_RescueClose()
{
	u8 flag;
	DebugOut("rescue close] ......\r\n");
	AT_SendComReq("AT+CRESCUE=1");
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("rescue close OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT) 
		{
			DebugOut("[fail] rescue close: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}





*/
//u8 AT_BeidouOpen()
//{
//	u8 flag;
//	DebugOut("BEIDOU open] ......\r\n");

//	if(AT_SendComReq("AT^BEIDOU=1")!= 1)
//	{
//		return FLAG_SEND_FAIL;
//	}
//	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);
//	
//	PrintInfo();
//	if(flag == 1)
//	{
//		DebugOut("BEIDOU open OK\r\n");
//		return 1;
//	}
//	else
//	{
//		if(flag != FLAG_TIME_OUT) 
//		{
//			DebugOut("[fail] BEIDOU open: %s\r\n",s_smsEndOfResponse[flag-1]);
//		}
//		return 0;
//	}
//}

//u8 AT_BeidouClose()
//{
//	u8 flag;
//	DebugOut("BEIDOU close] ......\r\n");

//	if(AT_SendComReq("AT^BEIDOU=0")!= 1)
//	{
//		return FLAG_SEND_FAIL;
//	}
//	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L1,0);
//	
//	PrintInfo();
//	if(flag == 1)
//	{
//		DebugOut("BEIDOU close OK\r\n");
//		return 1;
//	}
//	else
//	{
//		if(flag != FLAG_TIME_OUT)
//		{
//			DebugOut("[fail] BEIDOU close: %s\r\n",s_smsEndOfResponse[flag-1]);
//		}
//		return 0;
//	}
//}


//u8 AT_BeidouPostion()
//{
//	u8 flag;
//	DebugOut("BEIDOU postion] ......\r\n");
//	AT_SendComReq("AT^POSITION");
//	flag = WaitForStr((const u8 **)"^POSITION",0,WAIT_AT_TIME_MS_L1,0);
//	
//	PrintInfo();
//	if(flag == 1)
//	{
//		//TODO: postion send to USB
//		DebugOut("BEIDOU postion OK\r\n");
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}
//}

u8 AT_ModeCheck()
{
	u8 flag;
	DebugOut("CFUN? ......\r\n");
	if(AT_SendComReq("AT+CFUN?")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("CFUN? OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)
		{
			DebugOut("[fail] CFUN?: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}

u8 AT_ModeNormal()
{
	u8 flag;
	DebugOut("mode normal ......\r\n");
	if(AT_SendComReq("AT+CFUN=1")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("mode normal OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//超时
		{
			DebugOut("[fail] mode normal: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}
u8 AT_ModeFlying()
{
	u8 flag;
	DebugOut("mode flying ......\r\n");

	if(AT_SendComReq("AT+CFUN=4")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);
	
	PrintInfo();
	if(flag == 1)
	{
		DebugOut("mode flying OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//超时
		{
			DebugOut("[fail] mode flying: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}

u8 AT_ModeClose()
{
	u8 flag;
	DebugOut("mode close ......\r\n");

	if(AT_SendComReq("AT+CFUN=0")!= 1)
	{
		return FLAG_SEND_FAIL;
	}
	
	flag = WaitForStr(s_smsEndOfResponse,NUM_ELEMS(s_smsEndOfResponse),WAIT_AT_TIME_MS_L2,0);

	PrintInfo();
	if(flag == 1)
	{
		DebugOut("mode close OK\r\n");
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//超时
		{
			DebugOut("[fail] mode close: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}




#ifdef	DEBUG_INPUT
/*****************************************************************
@函 数 名：DebugComConvert()               
@功    能：调试输入转换
@输    入：mes 输入的数据
@输    出：无
@备    注： 将调试串口接收的测试指令转换模拟成USB指令                          
*****************************************************************/
void DebugComConvert(u8 * mes)
{

	u8 com = mes[0];
	u8 com2 = mes[1];
	u16 len = strlen((const char *)mes);
	if(len < 2)
	{
		DebugOut("[error] input too short: %s\r\n",mes);
		return;
	}
	
	switch (com)
	{
		case '!'://特殊
		{
			USB_DataRec.USB.tos = 25;
			break;
		}
		case 'c':
		case 'C':// 电话
		{
			if(len < 3 && (com2 == '1'))
			{
				DebugOut("[error] input too short: %s\r\n",mes);
				return;
			}
			USB_DataRec.USB.tos = 1;
			break;
		}
		case 'h':
		case 'H'://紧急求救
		{
			
			USB_DataRec.USB.tos = 3;
			break;
		}
		case 'm':
		case 'M'://短信
		{
			if(len < 4)
			{
				DebugOut("[error] input too short: %s\r\n",mes);
				return;
			}
			USB_DataRec.USB.tos = 2;
			break;
		}
		case 'b':
		case 'B'://北斗3
		{
			if(len < 3)
			{
				DebugOut("[error] input too short: %s\r\n",mes);
				return;
			}
			USB_DataRec.USB.tos = 4;
			mes[2] = mes[2] - '0';
			break;
		}
		case 'r':
		case 'R':
		{
			USB_DataRec.USB.tos = 26;
			switch (com2)
			{
				case '2':PW_AT_ON;DebugOut("AT ON1\r\n");USB_DataRec.USB.tos = 0;break;
				case '3':PW_AT_OFF;DebugOut("AT OFF1\r\n");USB_DataRec.USB.tos = 0;break;
				case '4':PW_BD_ON;DebugOut("BD ON1\r\n");USB_DataRec.USB.tos = 0;break;
				case '5':PW_BD_OFF;DebugOut("BD OFF1\r\n");USB_DataRec.USB.tos = 0;break;
				case '6':BDS_TouchuanFlag = 1;DebugOut("BD touchuan ON1\r\n");USB_DataRec.USB.tos = 0;break;
				case '7':BDS_TouchuanFlag = 0;DebugOut("BD touchuan OFF1\r\n");USB_DataRec.USB.tos = 0;break;
			}
			break;
		}
		default:
		{
			DebugOut("[error] input: %c\r\n",com);
			USB_DataRec.USB.tos = 0;
			break;
		}			
	}
	USB_DataRec.USB.data[0] = com2 - '0';
}
#endif


















