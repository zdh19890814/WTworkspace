#include "hw_config.h"
#include <string.h>
#include "AT_app.h"
unsigned char	endBuff[2] = { 0x0D, 0x0A };

//---------------------开机命令数-----------------------


#define true  1
#define false  0

#define Init_count  24

int TelReceiveAP = true;   // ZDH TODO: 手机端回复
int NetReceiveAP = true;

int voice_data_to_cp_status = 0;
int voice_data_to_ap_status = 0;


//-----------------------audio data---------------------
int j;
extern unsigned char channel;
extern unsigned short int *iis1;
extern unsigned short int *iis2;

//zdh  TODO :UartMuxDataInd应修改成上报的函数（USB）


/*******************************基本命令*************************************/
char getCOPS()
{
	AtMuxDataReq("AT+COPS?\r\n", 11);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

char get_model_version()
{
	AtMuxDataReq("AT+CGMR\r\n", 10);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

char get_hardware_version()
{
	AtMuxDataReq("AT^HVER\r\n", 10);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

char get_sim_status()
{
	AtMuxDataReq("AT^CARDMODE?\r\n", 15);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

char gei_imsi()
{
	AtMuxDataReq("AT+CIMI\r\n", 10);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

char gei_imei()
{
	AtMuxDataReq("AT+CGSN\r\n", 10);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

int set_muti_channel()
{
	AtMuxDataReq("AT+CMUX=0,0,2,31,10,10,30,10,22\r\n", 34);
	if (ReceiveAns == "OK")
	{
		return true;
	}
}

char check_phonenumber()
{
	AtMuxDataReq("AT+CNUM\r\n", 10);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}

//zdh TODO 自动接听时间参数  信号强度 厂商信息 高渗透振铃？



//------------------------------end basic command---------------------------/




//--------------------------------开机初始化---------------------------------
/***********************同一行输入对应同一行输出****************************/
//---------------------------------send--------------------------------------
char *OpenCommandSend[] =
{
	"AT+CMUX=0,0,5,1600" ,  //delay 100ms

	"AT\r\n",
	"AT+DELAY=0",
	"AT+CGCLASS=\"A\"\r\n",
	"AT+CFUN=1\r\n",
	"AT+CMEE=1\r\n",
	"AT+CREG=1\r\n",
	"AT+CGREG=1\r\n",
	"AT+CGEREP=1\r\n",
	"AT^DSCI\r\n",
	

	"AT+CNMI=2,2,2,1,0\r\n",


	//	"ATE1\r\n",
	"AT+COPS=3,0;+COPS?;+COPS=3,1;+COPS?;+COPS=3,2;+COPS?\r\n",

	"AT+CGREG?\r\n",
	"AT+CREG?\r\n",
	"AT+CGEQREQ=1,3,0,0\r\n",
	//话音途中关闭PS域
	"AT^DEPCD=0\r\n",

	//text PDU command   (	ACK PDU?)
	"AT+CSMS =0\r\n",
	"AT+CNMI = 2,1,0,1,0\r\n",
	"AT+CSCS = \"HEX\"\r\n",
	"AT+CMGF = 0\r\n",
	"AT+CGSMS =1\r\n",
	"AT+CSCA = \"+8613888888877\",146\r\n",
	"AT+CMMS = 1\r\n",
	//phone
	"AT^DEPCD=0\r\n"
	



};


int open_init()
{
	int i = 0;
	if (NULL != strstr(ReceiveAns[3], "^SIMST:1"))
	{
		for (i = 0; i < Init_count; i++)
		{
			AtMuxDataReq(OpenCommandSend[i], strlen(OpenCommandSend[i]));
		}
		if (ReceiveAns[3 + i] == OpenCommandRec[i])
		{
			i++;
		}
		else
		{
			return false;//  zdh TODO: print i
		}
		return true;
	}
	else
		return false;
	
}

//--------------------------------receive------------------------------------

char *OpenCommandRec[] =
{
	"\r\nOK\r\n"

	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",

	"\r\nOK\r\n",
	"\r\n+CGREG:1\r\n",
	"\r\n+CREG:1\r\n",
	"\r\n^MODE:15,8\r\n",



//	"\r\nOK\r\n",
	"\r\nOK\r\n",  //zdh  TODO 运营商选择 未定

	"\r\n+CGREG:1,1\r\n",
	"\r\n+CREG:1,1\r\n",
	"\r\nOK\r\n",
	//话音途中关闭PS域
	"\r\nOK\r\n",

//text PDU receive
	"\r\n+CSMS:1,1,0\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	"\r\nOK\r\n",
	//phone
	"\r\nOK\r\n"
	


};

//---------------------------北斗信息相关流程--------------------------------
//---------------------------------send--------------------------------------
char *BDCommandSend[] =
{
	"AT^BEIDOU=1\r\n",//kaiqi
	"AT^POSITION\r\n",
	"AT^BEIDOU=0\r\n"//guanbi

};

int open_BD_model()
{
	AtMuxDataReq("AT^BEIDOU=1\r\n", 14);
	if (ReceiveAns == "OK")
		return true;
	else
		return false;
}


int close_BD_model()
{
	AtMuxDataReq("AT^BEIDOU=0\r\n", 14);
	if (ReceiveAns == "OK")
		return true;
	else
		return false;
}



//--------------------------------receive------------------------------------
char *BDCommandRec[] =
{
	"\r\nOK\r\n",
	"\r\n^POSITION:1,90,180,1387584000\r\n",//坐标可能改变
	"\r\nOK\r\n"
};

char BD_message()
{
	AtMuxDataReq("AT^POSITION\r\n", 14);
	UartMuxDataInd(ReceiveAns, strlen(ReceiveAns));
}


//---------------------------语音业务流程------------------------------------
//---------------------------------send--------------------------------------
int voice_at_send(char *number)
{

	char *temp = (char*)malloc(17);
	memset(temp, 0, 17);
	temp = "ATD";
	strcat(temp, number);
	strcat(temp, endBuff);
	AtMuxDataReq((char *)temp, strlen(temp));
	int i = 0;
	if (ReceiveAns[i] == "BUSY")
	{
		return false;
	}
	else if (ReceiveAns[i] == "NO ANSWER")
	{
		return false;
	}
	else if (ReceiveAns[i] == "OK")
	{
		if (ReceiveAns[++i] == "^ORIG:1，0")
		{
			if (ReceiveAns[++i] == "NO CARRIER")
			{
				return false;
			}
			else if (ReceiveAns[++i] == "^CONN:1,0")
				return true;// zdh TODO: 主动挂断

		}

		else
			return false;

	}
}


int voice_emec_call(char *num)
{
	char *temp = (char*)malloc(12);
	memset(temp, 0, 12);
	temp = "ATD";
	strcat(temp, num);
	strcat(temp, endBuff);
	AtMuxDataReq(temp, strlen(temp));
	int i = 0;
	if (ReceiveAns[i] == "OK")
	{
		return true;
	}
	else
	{
		return false;
	}
}

int voice_message_send(unsigned short int *temp, int count)
{
	if (voice_data_to_cp_status)
	{
		if ((channel++) & 1)
		{
			for (j = 0; j < count; j++)
			{
				*iis1++ == *temp++;
			}
		}
		else
		{
			for (j = 0; j < count; j++)
			{
				*iis2++ == *temp++;
			}
		}
		voice_data_to_cp_status = 0;
	}

	return true;	
}


//--------------------------------receive------------------------------------
int voice_at_rec_cp()
{
	int i = 0;
	if (ReceiveAns[i] == "+CRING:VOICE")
	{
		i++;
		if (NULL != strstr(ReceiveAns[i], "^ORIG"))
		{
			UartMuxDataInd("Receive call!", 14);  //zdh  直接USB中断发送  TODO
			if (TelReceiveAP == 1)          
			{
				AtMuxDataReq("ATA\r\n", 6);
				i++;
				if (ReceiveAns[i] == "OK")
				{
					return true;
				}
				else
				{
					return false;
				}
					
			}
			else if (TelReceiveAP == 0)
			{
				AtMuxDataReq("AT+CHUP\r\n", 10);
				i += 2;
			}


		}
		else if (NULL != strstr(ReceiveAns[i], "+CLIP"))
		{
			UartMuxDataInd("Receive call!", 14);  //zdh  直接USB中断发送   TODO
			if (TelReceiveAP == 1)
			{
				AtMuxDataReq("ATA\r\n", 6);
				i++;
				if (ReceiveAns[i] == "NO CARRIER")
				{
					return false;
				}
				else if (ReceiveAns[i] == "OK")
					return true;
			}
			else if (TelReceiveAP == 0)
			{
				AtMuxDataReq("AT+CHUP\r\n", 10);
				i += 2;
			}

		}

	}
	else if (NULL != strstr(ReceiveAns[i], "^DSCI"))
	{
		int cancelnum;
		char numtemp[13];
		char *tmp = strstr(ReceiveAns[i], "^DSCI");
		tmp += 17;                           //point to number
		strncpy(numtemp, tmp, 13);

		UartMuxDataInd(numtemp, 13);  //zdh  直接USB中断发送 TODO
		UartMuxDataInd("be canceled", 12);
	}

}


int voice_message_receive(unsigned short int *temp, int count)
{
	if (voice_data_to_ap_status)
	{
		if ((channel++) & 1)
		{
			for (j = 0; j < count; j++)
			{
				*temp++ == *iis1++;
			}
		}
		else
		{
			for (j = 0; j < count; j++)
			{
				*temp++ == *iis2++;
			}
		}
		voice_data_to_ap_status = 0;
	}
}



//----------------------------开关机操作-----------------------------------
int closedown()
{
	AtMuxDataReq("AT+CFUN=0\r\n", 12);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
	{
		return false;
	}
}
int enter_fly_mode()
{
	AtMuxDataReq("AT+CFUN=4\r\n", 12);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
	{
		return false;
	}

}
int quit_fly_mode()
{
	AtMuxDataReq("AT+CFUN=1\r\n", 12);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
	{
		return false;
	}

}
//---------------------------- END 开关机操作--------------------------------

//-----------------------------开机网络选择----------------------------------

int auto_search_network()
{
	AtMuxDataReq("AT+COPS=0\r\n", 12);
	int i = 0;
	if (NULL != strstr(ReceiveAns[i], "OK"))
	{
		return true;
	}
	else if(NULL != strstr(ReceiveAns[i], "+CME ERROR:4"))
	{
		return false;  //zdh  TODO 少两条
	}
}


//zdh TODO :确定手动搜网参数


//----------------------------END-开机网络选择--------------------------------

//---------------------------分组（上网）流程--------------------------------
//---------------------------------send--------------------------------------
int online_at_send(char *APN)
{
	char *temp1;
	char *temp2;
	char *tempans;
	int  *ans;
	AtMuxDataReq("AT^DTEST\r\n", 11);
	int i = 1;// zdh   不管回复是OK还是ERROR都可以 从1开始
	AtMuxDataReq("AT^DMPCHN=1\r\n", 14);
	if (ReceiveAns[i] == "OK")
	{
		++i;
		AtMuxDataReq("AT+CGDCONT=1,\"IP\"", 18);
		AtMuxDataReq(APN, strlen(APN));
		AtMuxDataReq(endBuff, 2);
		if (ReceiveAns[i] == "OK")
		{
			++i;
			AtMuxDataReq("AT+CGACT=1,1\r\n", 15);
			if (ReceiveAns[i] == "OK")
			{
				++i;
				AtMuxDataReq("ATD*98*1#\r\n", 12);
				if (ReceiveAns[i] == "CONNECT")
				{
					++i;
					if (NULL != strstr(ReceiveAns[i], "+CGEV"))
					{
						temp1 = strstr(ReceiveAns[i], "\"IP\"");
						if (NULL == temp1)
							return -1;
						temp1 += 6;
						temp2 = (strstr(temp1, "\""));
						if (NULL == temp2)
							return -1;
						memset(tempans, 0, strlen(temp1) - strlen(temp2));
						strncpy(tempans, temp1, trlen(temp1) - strlen(temp2));
						ans = atoi(tempans);
						return ans;
					}
					else
					{
						return false;
					}

				}
				else if (NULL != strstr(ReceiveAns[i], "+CME ERROR"))
				{
					return false;
				}
			}
		}
	}
}

int end_online()
{
	AtMuxDataReq("AT+CGACT=0,1\r\n", 15);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
	{
		return false;
	}

}


//--------------------------------receive------------------------------------
int online_ap_receive()
{
	int i = 0;
	if (ReceiveAns[i] == "RING")
	{
		UartMuxDataInd("NetCmdRec", 10);
		if (NetReceiveAP == 1)
		{
			AtMuxDataReq("ATA\r\n", 6);
			i++;
			if (ReceiveAns[i] == "CONNECT")
				return true;
		}
		else if (NetReceiveAP == 0)
		{
			AtMuxDataReq("ATH\r\n", 6);
			i++;
			if (ReceiveAns[i] == "OK")
				return true;
		}
	}
	
}







//---------------------------短消息相关流程----------------------------------
//---------------------------------send--------------------------------------
int sendMessage(char * text)
{
	AtMuxDataReq("AT+CMGS=20\r\n", 13);
	int i = 0;
	if (ReceiveAns[i] == ">")
	{
		AtMuxDataReq(text, strlen(text));
		++i;
		if (ReceiveAns[i] == "OK")//zdh  TODO queding duanxin fanhui
		{
			return true;
		}
		else if (NULL != strstr(ReceiveAns[i], "+CME ERROR"))
		{
			return false;
		}
	}

}

//--------------------------------receive------------------------------------
int recMessage(char * text)
{
	UartMuxDataInd(text, 80);
	AtMuxDataReq("AT+CNMA=1\r\n", 12);
	if (ReceiveAns == "OK")
	{
		return true;
	}
}

//---------------------------应急救生相关流程--------------------------------
int set_emergency()
{
	AtMuxDataReq("AT+CRCFN=20,50\r\n", 17);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
		return false;
}
//---------------------------------send--------------------------------------
int open_emergency()
{
	AtMuxDataReq("AT+CRESCUE=0\r\n", 15);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
		return false;
}

int close_emergency()
{
	AtMuxDataReq("AT+CRESCUE=1\r\n", 15);
	if (ReceiveAns == "OK")
	{
		return true;
	}
	else
		return false;
}


//--------------------------------receive------------------------------------

//----------------------------集群业务流程-----------------------------------
//---------------------------------send--------------------------------------
//--------------------------------receive------------------------------------

//----------------------------secret card------------------------------------
int check_status()
{
	char *temp1;
	char *temp2;
	char *temp3;
	char *id;
	int  userid;
	AtMuxDataReq("AT+CCMPQR?\r\n", 13);
	if (NULL != strstr(ReceiveAns, "+CCMPQR"))
	{
		temp1 = strstr(ReceiveAns, "+CCMPQR");
		if (NULL == temp1)
			return -1;
		temp2 = temp1 + 9;//  to ->n
		if (*temp2 == "0")
		{
			temp2 += 4;
			temp3 = strstr(temp2, ">");
			if (NULL == temp3)
				return -1;
			strncpy(id, temp2, strlen(temp2) - strlen(temp3));
			userid = atoi(id);
			return userid;  //zdh  TODO deside ID/success

		}
		else if (*temp2 == "1")
		{
			UartMuxDataInd("error", 6);
		}
		else if (*temp2 == "2")
		{
			UartMuxDataInd("be distoried", 13);
		}
		else if (*temp2 == "3")
		{
			UartMuxDataInd("need open bar", 14);
		}
	}

}


int code_distroy(int level)//dengji
{
	if (0 == level)
	{
		AtMuxDataReq("AT+CCMLDSTR=<0>\r\n",18);
		return true;
		//zdh   TODO:需要一个实例
	}
	else if (1 == level)
	{
		AtMuxDataReq("AT+CCMLDSTR=<1>\r\n",18);
		return true;
	}
}


int code_config()
{
	AtMuxDataReq("AT+CCMCIQ?\r\n", 13);
	if (NULL != strstr(ReceiveAns, "+CCMCIQ:"))
		return true;
	//zdh TODO : xuyao yige shili
}
/******************************end secret************************************/















