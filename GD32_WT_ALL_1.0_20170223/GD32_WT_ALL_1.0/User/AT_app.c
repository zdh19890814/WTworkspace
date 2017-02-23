#include "include.h"

//uint8_t data[USB_DATA_SIZE + USB_HEAD_NUM];
//uint8_t ep1_data[100];



//usb_ProType USB_Send;
//usb_ProType USB_DataRec.USB_Rec;




u8 MuxOpenFlag = 0;


volatile SYSTIME SysTime = 0;//ʱ�����ÿ100us���ж� SysTick_Handler() ���Լ�һ
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
	//����;�йر�PS��
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
1.���ַ��� line ��Ѱ�� lelms �������ַ���
	opt = 0�����俪ͷ
	opt = 1���������
2.����
	0��û��
	x��lelms�еĵ� (x-1)���ַ���

*/




/*****************************************************************
@�� �� ����SeekStr(const u8 **lelms,size_t num, u8 *line,u8 opt)                           
@��    �ܣ����ַ��� line ��Ѱ�� lelms �������ַ���
@��    �룺lelms ָ��Ԥ�ڿ��ܻظ������ݣ�numΪ0ʱ��lelmsΪһ��ָ�룩
					 num Ԥ�ڻظ����ܵĸ���
					 opt �Ƚ����� 	
							opt = 0�����俪ͷ
							opt = 1���������
					 timeout_ms ��ʱʱ��
@��    ��������Ϊi��ʾ�ȵ���lelms[i-1]������ 0��ʾû��
@��    ע����                                       
*****************************************************************/
size_t SeekStr(const u8 **lelms,size_t num, u8 *line,u8 opt)
{
	size_t i;
	if(opt == 0)//flag = 0�����俪ͷ
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
@�� �� ����Gpio_Init(void)                                 
@��    �ܣ�Gpio ��ʼ��
@��    �룺��
@��    ������
@��    ע������USBʹ�ܣ�AT��BDʹ�ܣ�USBʹ��ֱ�Ӵ�                                                          
*****************************************************************/
void Gpio_Init(void)
{
#ifdef GD32
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);	 //ʹ�ܶ˿�ʱ��

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
@�� �� ����UnsolicitedHandle(u8 *com)                               
@��    �ܣ�������ͨģ���ָ��
@��    �룺���������\r\n F9��
@��    ��������ɹ�����1��ʧ�ܷ���0
@��    ע����                                       
*****************************************************************/
u8 UnsolicitedHandle(u8 *com)
{
	u8 * temp;
	if(strStartsWith(com,"^ORIG"))//�������
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
	else if(strStartsWith(com,"^CONF"))//�������
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
	else if(strStartsWith(com,"^CONN"))//�������
	{
		if(VoiceHandle[0].state == VOICE_CALL_STATE_CALL_CONNECT)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_CALL_ACCEPT;//��ͨ
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
	else if(strStartsWith(com,"NO CARRIER"))//�������
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
	else if(strStartsWith(com,"RING"))//�������
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
	else if(strStartsWith(com,"^CEND"))//�������
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
	else if(strStartsWith(com,"^SIMST: 1"))//�����ظ����ϵ���յ��λظ����ܽ��г�ʼ��
	{
		AT_OpenInitState = AT_OPEN_INIT_STATE_READY;
	}
	
	else if(strStartsWith(com,"^SIMST: 0"))//�����ظ�
	{
		AT_OpenInitState = AT_OPEN_INIT_STATE_0;
		AT_OpenSucceed = 0;
	}
	else if(strStartsWith(com,"^DSCI"))//ͨ��״̬�ϱ���Ϣ
	{
		DebugOut("DSCI: %s\r\n",com);
		DSCI_Handler(com);//������ָ�Ŀǰֻ�������루��δ���ԣ�
	}
	else if(strStartsWith(com,"+SSENDSTATUS="))//��������״̬����Ӧ��
	{
		if(com[13] == '0')
		{
			DebugOut("rescue back secceed\r\n");//Ӧ��ɹ�
		}
		else if(com[13] == '1')
		{
			DebugOut("[fail] rescue back: %s\r\n",com);//Ӧ��ʧ��
		}
		else if(com[13] == '2')
		{
			DebugOut("[fail] rescue back\r\n");//��������������
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
	else if(strStartsWith(com,"MODE:"))//�����ɹ�
	{
		USB_DataSend.USB.tos = 1;
		USB_DataSend.USB.data[0] = 5;
		USB_DataSend.USB.data[1] = 1;
		USB_SendCom();
	}
	//else if(SeekStr(s_Message,NUM_ELEMS(s_Message), com,0))
	else if(isSMSUnsolicited(com))//�յ����ţ�����USB�ֻ�
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
@�� �� ����DSCI_Handler(u8* com)                               
@��    �ܣ�����DSCI ָ��
@��    �룺��
@��    ������
@��    ע����                                       
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
@�� �� ����WaitForStr(const u8 **lelms,size_t num,u16 timeout_ms,u8 opt)                               
@��    �ܣ�������ָ��ȴ���ͨģ��ظ�
@��    �룺
					lelms ָ��Ԥ�ڿ��ܻظ������ݣ�numΪ0ʱ��lelmsΪһ��ָ�룩
					num Ԥ�ڻظ����ܵĸ���
					opt �Ƚ����� 	
						opt = 0�����俪ͷ
						opt = 1���������
					timeout_ms ��ʱʱ��
@��    ����
					����Ϊi��ʾ�ȵ���lelms[i-1]�����ݣ����᷵��0��Ĭ�Ϸ���1��ʾ�ɹ�
					��ʱ���� FLAG_TIME_OUT
@��    ע����                                       
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
		}//�õ���Ҫ�Ľ���ͷ���
		
		//}\
		if(flag)\
		{\
			return flag;\
		}//�����굱ǰ������Ϣ���ٷ���
	}
}

void MuxHandle()
{	
#ifdef MUX_MAIN
	int sum;
	for (; MuxBufIndexEnd != MuxBufIndexStart;)//�������յ�������
	{
		sum = AT_MuxBuff[MuxBufIndexStart][0]*256 + AT_MuxBuff[MuxBufIndexStart][1];//��ȡ���ݳ���
		get((char *)AT_MuxBuff[MuxBufIndexStart] + 2,sum);//����
		//AT_MuxBuffHandler(AT_MuxBuff[MuxBufIndexStart] + 2,sum);
		MuxBufIndexPlusPlus(MuxBufIndexStart);
	}
	#endif
}


/*****************************************************************
@�� �� ����AT_ComHandle(void)                                 
@��    �ܣ�������ͨģ�鷢����ATָ��
@��    �룺��
@��    ������
@��    ע����                                       
*****************************************************************/
void AT_ComHandle()
{
	MuxHandle();//��Muxʱ�����ѡ��������������MUX����ָ����ڴ˴����жϽ��յ������ݣ����������ȴ�����
	
	for (;ComBufIndexEnd != ComBufIndexStart;)//����������ָ��
	{
		UnsolicitedHandle(ComBuf[ComBufIndexStart]);//�����ָ��
		ComBufIndexPlusPlus(ComBufIndexStart);//ָ����һ��ָ��
	}
}

/*****************************************************************
@�� �� ����SendCallState(void)                                 
@��    �ܣ����ֻ�USB����ͨ��״̬
@��    �룺��
@��    ������
@��    ע����                                       
*****************************************************************/
void SendCallState(void)
{
	USB_DataSend.USB.tos = 1;
	USB_DataSend.USB.length = strlen((const char *)VoiceHandle[0].num) + 2;
	USB_DataSend.USB.data[0] = 4;
	USB_DataSend.USB.data[1] = VoiceHandle[0].state;
	memcpy(USB_DataSend.USB.data + 2, VoiceHandle[0].num, strlen((const char *)VoiceHandle[0].num) + 1);//����
	USB_SendCom();//���͸�USB
}

/*****************************************************************
@�� �� ����CallStateHandle(void)                                 
@��    �ܣ�����ͨ��״̬�����仯
@��    �룺��
@��    ������
@��    ע����                                       
*****************************************************************/
void CallStateHandle(void)
{
	static u8 call_state_last = 0;
	static u8 voice_call_last = 0;
		
	if(call_state_last != VoiceHandle[0].state)//ͨ��״̬�����仯����Ҫ�ϱ�USB
	{
		call_state_last = VoiceHandle[0].state;
		DebugOut("call state %d\r\n",VoiceHandle[0].state);

		SendCallState();

		if((VoiceHandle[0].state == VOICE_CALL_STATE_CALL_ACCEPT)||(VoiceHandle[0].state == VOICE_CALL_STATE_RECEIVE_CONNECT))//ͨ�����
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
			 memset(I2S_DMA_DoubleBufferSend,0,I2S_SEND_SIZE * 2);//������������
	
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
	
		
		
		
		
		
		
		
		
		
		
		
		
		
		if(voice_call_last != VoiceCallFlag)//ͨ�����仯
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
@�� �� ����AT_SendDataReq(u8 * data,u16 len)           
@��    �ܣ���ͨģ�鷢������
@��    �룺data ���������ݣ�len ���ݳ���
@��    �������ͳɹ�����1��ʧ�ܷ���0��һ��������������
@��    ע��
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
@�� �� ����PrintInfo(void)                                 
@��    �ܣ��ȴ����ؽ���������յ���������Ϣ
@��    �룺��
@��    ������
@��    ע��Ŀǰ�������ڴ�ӡ                                        
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
@�� �� ����PowerOnInit(void)                                 
@��    �ܣ��ϵ��ATģ���ʼ��
@��    �룺��
@��    ������
@��    ע�������յ��ϵ�ɹ����ص�����ָ������AT_Init()���г�ʼ��                                          
*****************************************************************/
void PowerOnInit()
{
	DebugOut("wait for AT model power on ...\r\n");
	TimeTemp = SysTime;
	while(AT_OpenInitState == AT_OPEN_INIT_STATE_0)//�ȴ�ģ���ϵ���������ָ�һ��30s�ڻ����
	{
		if((SysTime - TimeTemp) > WAIT_INIT_WAIT)
		{
			break;
		}
		AT_ComHandle();
	}
	
	if((SysTime - TimeTemp) <= WAIT_INIT_WAIT)//�յ�����ָ��
	{
		AT_OpenSucceed = AT_Init();
	}
	else//��ʱ
	{
		DebugOut("[fail] AT init\r\n");
	}
}


/*****************************************************************
@�� �� ����AT_Init(void)                                 
@��    �ܣ���ʼ��ģ��
@��    �룺��
@��    �����ɹ�����1
@��    ע�����η���OpenCommandSend������ָ����г�ʼ��                                       
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
		
		if(i == 3)//CFUN ��Ҫ��ʱ��ϳ�
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
	if(AT_SendComReq("AT+CMUX=0,0,5,1600")!= 1)//��MUX
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
			ChannelOpen();//��ͨ��
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
@�� �� ����AT_VoiceCall(u8 *number)                                 
@��    �ܣ�����绰
@��    �룺number �����ַ���ָ�� '\0'��β
@��    �����ɹ�����1
@��    ע��                                       
*****************************************************************/
u8 AT_VoiceCall(u8 *number)
{
	u8 flag;
	u8 CallSend[LEN_NUM + 5]="ATD\0";
	if(strlen((const char *) number) >= LEN_NUM)//���볤�ȹ���
	{
		DebugOut("number is too long:%s\r\n",number);
		return 20;
	}
	memcpy(VoiceHandle[0].num, (const char *)number,1 + strlen((const char *) number));
	if(VoiceHandle[0].state == VOICE_CALL_STATE_IDLE)//����״̬ʱ���ܲ���
	{
		strcat((char *)CallSend, (const char *)number);//ƴ�Ӻ���
		strcat((char *)CallSend, ";\0");
		DebugOut("voice call ......\r\n");
		
		if(AT_SendComReq(CallSend) != 1)//���ʹ�绰ָ�����"ATD10086;"��
		{
			return FLAG_SEND_FAIL;
		}			
		flag = WaitForStr(s_VoiceCall,NUM_ELEMS(s_VoiceCall),WAIT_AT_TIME_MS_L1,0);//�ȴ�����
		PrintInfo();//�ȴ����ؽ���������յ���������Ϣ
		if(flag == 1)
		{
			VoiceHandle[0].state = VOICE_CALL_STATE_CALL_SEND;
			DebugOut("voice call OK\r\n");
		}
		else 
		{
			if(flag != FLAG_TIME_OUT)											//��ʱ
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
@�� �� ����AT_VoiceReceive()                               
@��    �ܣ���ͨ��ǰ����
@��    �룺��
@��    �����ɹ�����1
@��    ע��                           
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
			if(flag != FLAG_TIME_OUT)											//��ʱ 
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
@�� �� ����AT_VoiceHangup()                               
@��    �ܣ��Ҷϵ�ǰ����ͨ��
@��    �룺��
@��    �����ɹ�����1
@��    ע��                           
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
			if(flag != FLAG_TIME_OUT)											//��ʱ
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
@�� �� ����AT_RescueStateSend()                               
@��    �ܣ���������״̬����
@��    �룺��
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueTestSendOpen()                               
@��    �ܣ����������Բ��
@��    �룺��
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueTestSendClose()                               
@��    �ܣ����������Բ�ر�
@��    �룺��
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueMesSend()                               
@��    �ܣ�����������Ϣ����
@��    �룺��
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueSetFre(u16 sendfre,u16 recfre)                             
@��    �ܣ�������������Ƶ������
@��    �룺sendfre ����Ƶ�㣬recfre����Ƶ��
@��    �������ͳɹ�����1
@��    ע��                           
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
		if(flag != FLAG_TIME_OUT)											//3?����
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
@�� �� ����AT_RescueSetGain(u8 gain)                          
@��    �ܣ���������������������
@��    �룺gain ����
@��    �������ͳɹ�����1
@��    ע��                           
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
		if(flag != FLAG_TIME_OUT)											//3?����
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
@�� �� ����AT_RescueSetCount(u16 count)                        
@��    �ܣ������������ʹ�������
@��    �룺count ����
@��    �������ͳɹ�����1
@��    ע��                           
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
		if(flag != FLAG_TIME_OUT)											//3?����
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
@�� �� ����AT_RescueSetInterval(u16 interval)                      
@��    �ܣ������������ͼ������
@��    �룺interval ���
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueSetArmy(u8 army)                    
@��    �ܣ�����������������
@��    �룺army ����
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueSetOffset(s16 offset)                   
@��    �ܣ���������Ƶƫ����
@��    �룺offset Ƶƫ
@��    �������ͳɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueSet(u8 *par)                  
@��    �ܣ��������� ����
@��    �룺par ����������
@��    �������ͳɹ�����1
@��    ע��                           
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
	if(AT_RescueSetFre(temp1,temp2) != 1)	//����Ƶ��
	{
		flag |= 0x02;
	}	
	temp0 = par[4];
	if(AT_RescueSetGain(temp0) != 1)	//��������
	{
		flag |= 0x04;
	}
	temp1 = par[5]*256 + par[6];
	if(AT_RescueSetCount(temp1) != 1)	//���÷��ʹ���
	{
		flag |= 0x08;
	}
	temp1 = par[7]*256 + par[8];
	if(AT_RescueSetInterval(temp1) != 1)	//���÷��ͼ��
	{
		flag |= 0x10;
	}	
	temp1 = par[9];
	if(AT_RescueSetArmy(temp1) != 1)	//���þ���
	{
		flag |= 0x20;
	}
	temp3 = par[7]*256 + par[8];
	if(AT_RescueSetOffset(temp3) != 1)	//����Ƶƫ
	{
		flag |= 0x40;
	}
	if(flag == 0)
		flag = 1;

	return flag;
}


/*****************************************************************
@�� �� ����myitoa(s16 num,u8 * str,u8 sign)                  
@��    �ܣ�����ת�ַ���
@��    �룺num��ת�����֣�strת����洢λ�õ�ָ�룬sign�Ƿ��з���
@��    ������
@��    ע��                           
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
@�� �� ����AT_MessageSend(u8 len,u8 * mes)                 
@��    �ܣ����Ͷ���
@��    �룺len PDU���� mes��������
@��    �����ɹ�����1
@��    ע��                           
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
			if(flag != FLAG_TIME_OUT)											//��ʱ
			{
				DebugOut("[fail] message send: %s\r\n",s_smsEndOfResponse[flag-1]);
			}
		}
		
	}
	else
	{
		if(flag != FLAG_TIME_OUT)											//��ʱ
		{
			DebugOut("[fail] AT+CMGS=20: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}



/*****************************************************************
@�� �� ����AT_RescueOpen()               
@��    �ܣ�����������
@��    �룺��
@��    �����ɹ�����1
@��    ע��                           
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
@�� �� ����AT_RescueOpen()               
@��    �ܣ����������ر�
@��    �룺��
@��    �����ɹ�����1
@��    ע��                           
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
		if(flag != FLAG_TIME_OUT)											//��ʱ
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
		if(flag != FLAG_TIME_OUT)											//��ʱ
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
		if(flag != FLAG_TIME_OUT)											//��ʱ
		{
			DebugOut("[fail] mode close: %s\r\n",s_smsEndOfResponse[flag-1]);
		}
	}
	return flag;
}




#ifdef	DEBUG_INPUT
/*****************************************************************
@�� �� ����DebugComConvert()               
@��    �ܣ���������ת��
@��    �룺mes ���������
@��    ������
@��    ע�� �����Դ��ڽ��յĲ���ָ��ת��ģ���USBָ��                          
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
		case '!'://����
		{
			USB_DataRec.USB.tos = 25;
			break;
		}
		case 'c':
		case 'C':// �绰
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
		case 'H'://�������
		{
			
			USB_DataRec.USB.tos = 3;
			break;
		}
		case 'm':
		case 'M'://����
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
		case 'B'://����3
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


















