#include "include.h"

ST_MUX_MNG           g_MuxDlcMng;
ST_MUX_FRAME_MNG     g_MuxFrameMng;
ST_RETRANSMIT_BUFF   g_MuxRetranBuffer;
ST_URAT_BUFF         g_UartBuffer;

int main_exit_signal = 0;

#define MAX_RC_MSG_LEN      (8 * 1024)
#define MAX_RIL_MSG_LEN     (8 * 32)
#define MAX_PPP_MSG_LEN     (8 * 1024)      
#define MAX_UART_MSG_LEN    (8 * 32)

char RcReadBuf[MAX_RC_MSG_LEN];
char RilReadBuf[MAX_RIL_MSG_LEN];
char PppReadBuf[MAX_PPP_MSG_LEN];
char UartReadBuf[MAX_UART_MSG_LEN];



















ST_ATSYMBAL AT_TABLE[SUPPORT_AT_NUM] = {
          //SS    
/* 1 */   {(unsigned char *)"+CUSD"},    
/* 2 */   {(unsigned char *)"+CCFC"},    
/* 3 */   {(unsigned char *)"+ CSSN"},    
          //BASIC                  
/* 4 */   {(unsigned char *)"+CIMI"},    
/* 5 */   {(unsigned char *)"^SPN"},    
/* 6 */   {(unsigned char *)"+CMEE"},    
/* 7 */   {(unsigned char *)"S3"},    
/* 8 */   {(unsigned char *)"S4"},    
/* 9 */   {(unsigned char *)"S5"},    
/* 10 */  {(unsigned char *)"E"},    
/* 11 */  {(unsigned char *)"V"},    
/* 12 */  {(unsigned char *)"&F"},    
/* 13 */  {(unsigned char *)"+CSQ"},    
/* 14 */  {(unsigned char *)"+CGMI"},    
/* 15 */  {(unsigned char *)"+CGMR"},    
/* 16 */  {(unsigned char *)"+CGSN"},    
/* 17 */  {(unsigned char *)"^HVER"},    
/* 18 */  {(unsigned char *)"+CPWD"},    
/* 19 */  {(unsigned char *)"+CPIN"},    
/* 20 */  {(unsigned char *)"+CLCK"},    
/* 21 */  {(unsigned char *)"+CRSM"},    
/* 22 */  {(unsigned char *)"+++"},    
/* 23 */  {(unsigned char *)"+CFUN"},    
/* 24 */  {(unsigned char *)"^SIMST"},    
/* 25 */  {(unsigned char *)"+CMUX"},    
/* 26 */  {(unsigned char *)"+CGMM"},    
/* 27 */  {(unsigned char *)"^RFSW"},    
/* 28 */  {(unsigned char *)"^CMDVER"},    
/* 29 */  {(unsigned char *)"ATQ"},    
/* 30 */  {(unsigned char *)"+CSCS"},    
/* 31 */  {(unsigned char *)"+XL1SET"},    
/* 32 */  {(unsigned char *)"^AUDPARM"},    
/* 33 */  {(unsigned char *)"^SIMTYPE"},    
/* 34 */  {(unsigned char *)"^CSQ"},    
/* 35 */  {(unsigned char *)"+CTZU"},    
/* 36 */  {(unsigned char *)"+CTZR"},    
          //MM                 
/* 37 */  {(unsigned char *)"+COPS"},    
/* 38 */  {(unsigned char *)"^MODE"},    
/* 39 */  {(unsigned char *)"^SYSINFO"},    
/* 40 */  {(unsigned char *)"+CREG"},    
/* 41 */  {(unsigned char *)"+CGREG"},    
/* 42 */  {(unsigned char *)"^SYSCONFIG"},    
/* 43 */  {(unsigned char *)"+OFF"},    
/* 44 */  {(unsigned char *)"+RESET"},    
/* 45 */  {(unsigned char *)"^CREG"},    
/* 46 */  {(unsigned char *)"^CARDMODE"},    
/* 47 */  {(unsigned char *)"^VOICEFORMA"},
/* 48 */  {(unsigned char *)"+CGCLASS"},    
/* 49 */  {(unsigned char *)"^HPAS"},    
/* 50 */  {(unsigned char *)"^ALERT"},    
/* 51 */  {(unsigned char *)"^ POSITION"},    
          //CS                  
/* 52 */  {(unsigned char *)"A"},    
/* 53 */  {(unsigned char *)"+CLIP"},    
/* 54 */  {(unsigned char *)"+CLIR"},    
/* 55 */  {(unsigned char *)"+CHLD"},    
/* 56 */  {(unsigned char *)"+COLP"},    
/* 57 */  {(unsigned char *)"+CCWA"},    
/* 58 */  {(unsigned char *)"D"},    
/* 59 */  {(unsigned char *)"+CSTA"},    
/* 60 */  {(unsigned char *)"+CHUP"},    
/* 61 */  {(unsigned char *)"+CRC"},    
/* 62 */  {(unsigned char *)"+VTS"},    
/* 63 */  {(unsigned char *)"+CLCC"},    
/* 64 */  {(unsigned char *)"+CMOD"},    
/* 65 */  {(unsigned char *)"+CPBS"},    
/* 66 */  {(unsigned char *)"^SCPBR"},    
/* 67 */  {(unsigned char *)"^ SCPBW"},    
/* 68 */  {(unsigned char *)"+CPBF"},    
/* 69 */  {(unsigned char *)"D>"},    
/* 70 */  {(unsigned char *)"S0"},    
/* 71 */  {(unsigned char *)"CBST"},    
/* 72 */  {(unsigned char *)"^DSCI"},    
/* 73 */  {(unsigned char *)"^CVOICE"},    
/* 74 */  {(unsigned char *)"^ORIG"},    
/* 75 */  {(unsigned char *)"^CONF"},    
/* 76 */  {(unsigned char *)"^CONN"},    
/* 77 */  {(unsigned char *)"^CEND"},    
/* 78 */  {(unsigned char *)"+CPBR"},    
/* 79 */  {(unsigned char *)"+CPBW"},    
/* 80 */  {(unsigned char *)"+CMUT"},    
/* 81 */  {(unsigned char *)"+CNUM"},    
/* 82 */  {(unsigned char *)"^VPIN"},    
/* 83 */  {(unsigned char *)"^VPOUT"},    
/* 84 */  {(unsigned char *)"+CEER"},    
          //SMS                
/* 85 */  {(unsigned char *)"+CSMS"},    
/* 86 */  {(unsigned char *)"+CMGF"},    
/* 87 */  {(unsigned char *)"+CMTI"},    
/* 88 */  {(unsigned char *)"+CMT"},    
/* 89 */  {(unsigned char *)"+CDSI"},    
/* 90 */  {(unsigned char *)"+CDS"},    
/* 91 */  {(unsigned char *)"+CNMI"},    
/* 92 */  {(unsigned char *)"+CMGD"},    
/* 93 */  {(unsigned char *)"+CMGC"},    
/* 94 */  {(unsigned char *)"+CNMA"},    
/* 95 */  {(unsigned char *)"+CPMS"},    
/* 96 */  {(unsigned char *)"+CSCA"},    
/* 97 */  {(unsigned char *)"+CMGS"},    
/* 98 */  {(unsigned char *)"+CMGW"},    
/* 99 */  {(unsigned char *)"+CMGL"},    
/* 100 */ {(unsigned char *)"+CMGR"},    
/* 101 */ {(unsigned char *)"+CGSMS"},    
/* 102 */ {(unsigned char *)"^CMGS"},    
/* 103 */ {(unsigned char *)"+CSAS"},    
/* 104 */ {(unsigned char *)"^SWCB"},    
/* 105 */ {(unsigned char *)"+CSCB"},    
/* 106 */ {(unsigned char *)"+CMMS"},    
/* 107 */ {(unsigned char *)"^CMFL"},    
          //PS                  
/* 108 */ {(unsigned char *)"+CGDCONT"},    
/* 109 */ {(unsigned char *)"+CGQREQ"},    
/* 110 */ {(unsigned char *)"+CGEQREQ"},       
/* 111 */ {(unsigned char *)"H"},    
/* 112 */ {(unsigned char *)"A"},    
/* 113 */ {(unsigned char *)"O"},    
/* 114 */ {(unsigned char *)"+CGATT"},    
/* 115 */ {(unsigned char *)"+CGDATA"},    
/* 116 */ {(unsigned char *)"+CGACT"},    
/* 117 */ {(unsigned char *)"+CGQMIN"},    
/* 118 */ {(unsigned char *)"+CGEQMIN"},    
/* 119 */ {(unsigned char *)"+CGEREP"},    
/* 120 */ {(unsigned char *)"$QCDGEN"},
/* 121 */ {(unsigned char *)"^DEPCD"},    
/* 122 */ {(unsigned char *)"^DAUTH"},    
/* 123 */ {(unsigned char *)"^DQDATA"},
          //ppp
/* 124 */ {(unsigned char *)"D*"},
/* 125 */ {(unsigned char *)"^DTEST"},
/* 126 */ {(unsigned char *)"^DMPCHN"},
/* 127 */ {(unsigned char *)"^DLKS"}
};

const unsigned char  crctable[256] = {    //reversed, 8-bit, poly=0x07
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,  0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
    0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,  0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
    0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,  0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
    0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,  0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,

    0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,  0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
    0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,  0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
    0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,  0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
    0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,  0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,

    0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,  0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
    0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,  0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
    0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,  0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,  0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,

    0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,  0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
    0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,  0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
    0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,  0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
    0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,  0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

E_MUX_LOG_TYPE LogLevel = MUX_LOG_LEVEL_3;




unsigned char SenderCreateFcs(unsigned char len, unsigned char *p)
{
    /* Init */
    unsigned char ucCalFcs=0xFF;

   // printf(MUX_LOG_LEVEL_3, "Function SenderCreateFcs");

    /* len is the number of bytes in the message, p points to message */
    while (len--)
    {
        ucCalFcs = crctable[ucCalFcs^*p++];
    }
    /* Ones complement */
    ucCalFcs = 0xFF - ucCalFcs;

    return (ucCalFcs);
}

char MuxStrncmp(char *pStr1, char *pStr2, unsigned short usLen)
{
    if((NULL == pStr1) || (NULL == pStr2))
    {
     //   printf(MUX_LOG_LEVEL_1, "String Strncmp Error");
    }
    return strncmp(pStr1, pStr2, usLen);
}
int checkDLC(char *sendbuffer, int len)
{
   char cRet = 0;
	 char *data=0;
	 int i=0;
	 unsigned char length = 0;
	
	 if(len<=2) 
   {
		 cRet =1;//MUX_DLC0
     return cRet;
	 }
	 
	 data = (char *)MuxMalloc(len + 1);
   MuxMemCopy(data,sendbuffer, len);
	 
	 data = data + 2;     
	 len-=2;
	 
    for(i = 0; i < SUPPORT_AT_NUM; i++)
    {
      length =(unsigned char)strlen((const char *)AT_TABLE[i].pData);
      if(len>=length)        
			{
				if(0 == MuxStrncmp(data, (char *)AT_TABLE[i].pData, length))
         {
                break;
         }
			}
    }
	 
    if (i >= 0 && i <= 50)
    {
        //printf(MUX_LOG_LEVEL_2, "AT command is general AT");
        cRet =1;// MUX_DLC_AT_1;                                            /* for general AT include SS AT */
    }
    else if (i >= 51 && i <= 83)
    {
      //  printf(MUX_LOG_LEVEL_2, "AT command is voice call AT");
        cRet =2;// MUX_DLC_AT_2;                                            /* for voice call AT           */
    }
    else if (i >= 84 && i <= 106)
    {
      //  printf(MUX_LOG_LEVEL_2, "AT command is SMS or STK AT");
        cRet = 3;//MUX_DLC_AT_3;                                            /* for SMS & STK AT            */
    }

    else if (i >= 107 && i <= 122)
    {
      //  printf(MUX_LOG_LEVEL_2, "AT command is PDP AT");
        cRet =4;// MUX_DLC_DATA_1;                                           /* for PDP                */

    }
	else if(i >= 123 && i <= SUPPORT_AT_NUM - 1)
	{
		//printf(MUX_LOG_LEVEL_2, "AT command is PDP AT");
        cRet =5;// MUX_DLC_DATA_2;
	}
    else
    {
        if(0x1A == data[len - 1 - 2] ||
           0x1B == data[len - 1 - 2])
        {
            // This is SMS PDU
         //   printf(MUX_LOG_LEVEL_2, "SMS PDU");
            cRet =3;// MUX_DLC_AT_3;
        }  
        else
        {
          //  printf(MUX_LOG_LEVEL_2, "UNKONWN AT COMMAND!");
            //cRet = MUX_DLC0;
            cRet =1;// MUX_DLC_AT_1;
        }
    }
    MuxFree(data - 2);
    return cRet;  

}
//void MuxSend(char *sendbuffer, int len)
//{
//  int uslen;
//	int DLCi;
//	unsigned char  ucFcs = 0; 
//	u8 *ussendbuffer;
//	
//	DLCi=checkDLC(sendbuffer,len);
//	uslen=len+8;
//	ussendbuffer=(u8 *)MuxMalloc((unsigned long)uslen);
//	ussendbuffer[0]=0xF9;
//	ussendbuffer[1]=0x01|(DLCi << 2);
//	ussendbuffer[2] = 0xEF; 
//	ussendbuffer[3] =0x01 | ((uslen - 6) << 1);
//	
//	ucFcs  = SenderCreateFcs(3, &ussendbuffer[1]);
//	
//	MuxMemCopy((char *)ussendbuffer+4,(char *)sendbuffer,uslen-8);
//	ussendbuffer[uslen-4] ='\r';
//	ussendbuffer[uslen-3] ='\n';
//	ussendbuffer[uslen-2] =ucFcs; 
//	ussendbuffer[uslen-1] =0xF9;
//	
//  UartMuxDataReq(ussendbuffer, uslen);
//  MuxFree((char*)ussendbuffer);
//}




u8 AT_SendMux(u8 *sendbuffer)
{
	u8 flag;
	int uslen;
	int DLCi;
	unsigned char  ucFcs = 0; 
	u8 *ussendbuffer;
	u16 len = strlen((const char *)sendbuffer);
	
	DLCi=checkDLC((char *)sendbuffer,len);
	uslen=len+8;
	ussendbuffer=(unsigned char *)MuxMalloc((unsigned long)uslen);
	ussendbuffer[0]=0xF9;
	ussendbuffer[1]=0x01|(DLCi << 2);
	ussendbuffer[2] = 0xEF; 
	ussendbuffer[3] =0x01 | ((uslen - 6) << 1);
	
	ucFcs  = SenderCreateFcs(3, &ussendbuffer[1]);
	
	MuxMemCopy((char *)ussendbuffer+4,(char *)sendbuffer,uslen-8);
	ussendbuffer[uslen-4] ='\r';
	ussendbuffer[uslen-3] ='\n';
	ussendbuffer[uslen-2] =ucFcs; 
	ussendbuffer[uslen-1] =0xF9;
	
	flag = AT_SendData(ussendbuffer,uslen);
  MuxFree((char*)ussendbuffer);
	return flag;
}

u8 AT_SendDataMux(u8 *sendbuffer, u16 len)
{
	u8 flag;
	int uslen;
	int DLCi;
	unsigned char  ucFcs = 0; 
	u8 *ussendbuffer;
	DLCi=checkDLC((char *)sendbuffer,len);
	uslen=len+8;
	if(uslen  <= 127)
	{
		
		ussendbuffer=(unsigned char *)MuxMalloc((unsigned long)uslen);
		if(0 == ussendbuffer)
		{
			return FLAG_SEND_FAIL;
		}
		ussendbuffer[0]=0xF9;
		ussendbuffer[1]=0x01|(DLCi << 2);
		ussendbuffer[2] = 0xEF; 
		ussendbuffer[3] =0x01 | ((uslen - 6) << 1);
		
		ucFcs  = SenderCreateFcs(3, &ussendbuffer[1]);
		
		MuxMemCopy((char *)ussendbuffer+4,(char *)sendbuffer,uslen-8);
		ussendbuffer[uslen-4] ='\r';
		ussendbuffer[uslen-3] ='\n';
		ussendbuffer[uslen-2] =ucFcs; 
		ussendbuffer[uslen-1] =0xF9;
		
		flag = AT_SendData(ussendbuffer,uslen);
		MuxFree((char*)ussendbuffer);
	}
	else
	{
		ussendbuffer=(unsigned char *)MuxMalloc((unsigned long)uslen + 1);
		if(0 == ussendbuffer)
		{
			return FLAG_SEND_FAIL;
		}
		ussendbuffer[0]=0xF9;
		ussendbuffer[1]=0x01|(DLCi << 2);
		ussendbuffer[2] = 0xEF; 
		
		ussendbuffer[3] = ((uslen - 7) & 0x7f)<<1;
		ussendbuffer[4] = ((uslen - 7) >> 7) & 0xff;
		
//		ussendbuffer[3] = ((uslen - 7)<<1) & 0xff;
//		ussendbuffer[4] = (uslen - 7)<<1 >> 8;
		
		ucFcs  = SenderCreateFcs(3, &ussendbuffer[1]);
		
		MuxMemCopy((char *)ussendbuffer+4,(char *)sendbuffer,uslen-8);
		ussendbuffer[uslen-4] ='\r';
		ussendbuffer[uslen-3] ='\n';
		ussendbuffer[uslen-2] =ucFcs; 
		ussendbuffer[uslen-1] =0xF9;
		
		flag = AT_SendData(ussendbuffer,uslen);
		MuxFree((char*)ussendbuffer);
	}
	return flag;
}




void MuxInit()
{
    unsigned char  i;

   // printf(MUX_LOG_LEVEL_3, "Function MuxInit");


    MuxMemSet((char *)&g_MuxDlcMng, 0, sizeof(ST_MUX_MNG));
    MuxMemSet((char *)&g_MuxFrameMng, 0, sizeof(ST_MUX_FRAME_MNG));
    MuxMemSet((char *)&g_MuxRetranBuffer, 0, sizeof(ST_RETRANSMIT_BUFF));
    MuxMemSet((char *)&g_UartBuffer, 0, sizeof(ST_URAT_BUFF));

    g_MuxDlcMng.Dlc0.ucStatus               = MUX_STATUS_CLOSED_DOWN;
    g_MuxDlcMng.Dlc0.ucFlowId               = MUX_FLOW_NONE;

    for (i = 0; i < MUX_DLC_NUM; i++)
    {
        g_MuxDlcMng.Dlc[i].ucStatus         = MUX_STATUS_DISCONNECTED;
        g_MuxDlcMng.Dlc[i].ucFlowId         = MUX_FLOW_NONE;
        g_MuxDlcMng.Dlc[i].ucFc             = MUX_OFF;
        g_MuxDlcMng.Dlc[i].ucRtrFc          = MUX_OFF;
        g_MuxDlcMng.Dlc[i].ucMode           = OFFLINE_AT_MODE;
        g_MuxDlcMng.Dlc[i].ucPppStatus      = DTX_READY;
        g_MuxDlcMng.Dlc[i].ucPppFc          = MUX_OFF;
        //g_MuxDlcMng.Dlc[i].ucCr             = 0xff;
        //g_MuxDlcMng.Dlc[i].ucFcDelayFlg     = MUX_OFF;
        g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci  = i + 1;
        g_MuxDlcMng.Dlc[i].ParaInfo.ucN2    = MUX_MAX_NUM_OF_RETRANS;
        g_MuxDlcMng.Dlc[i].ParaInfo.ucT1    = MUX_DEFAULT_TIME_OF_T1;
        g_MuxDlcMng.Dlc[i].ParaInfo.ucT2    = MUX_DEFAULT_TIME_OF_T2;
        g_MuxDlcMng.Dlc[i].ParaInfo.usN1    = MUX_MAX_SIZE_OF_FRAME;
    }
    g_MuxDlcMng.Dlc[0].ucService            = MUX_SERV_AT;
    g_MuxDlcMng.Dlc[1].ucService            = MUX_SERV_AT;
    g_MuxDlcMng.Dlc[2].ucService            = MUX_SERV_AT;
    g_MuxDlcMng.Dlc[3].ucService            = MUX_SERV_DATA;
    g_MuxDlcMng.Dlc[4].ucService            = MUX_SERV_DATA;
    g_MuxDlcMng.Dlc[5].ucService            = MUX_SERV_DATA;
    g_MuxDlcMng.Dlc[6].ucService            = MUX_SERV_VOICE;
    g_MuxDlcMng.Dlc[7].ucService            = MUX_SERV_VOICE;

    return;
}


///*
//void AtMuxStartReq(void)
//{

//   unsigned char i;
////   char *pAtCmux = NULL;

//   // printf(MUX_LOG_LEVEL_3, "Function AtMuxStartReq");

//   // pAtCmux = (char *)MuxMalloc(18 + 1);                                         /* At commond length          */
//  //  MuxMemCopy(pAtCmux, "AT+CMUX=0,0,5,1600\r", 19);
//    for (i = 0; i < MUX_DLC_NUM; i++)
//    {
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci = i + 1;                          /* 通道号                      */
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucT1 = 10;                               /* default value is 10 (100ms) */
//       g_MuxDlcMng.Dlc[i].ParaInfo.usN1 = 1600;                             /* maximum frame size 1600B     */
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucT2 = 30;                               /* default value is 30 (300ms) */
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucN2 = MUX_MAX_NUM_OF_RETRANS;           /* maximum number of re-transmissions(default value)*/
//    }

//	g_MuxFrameMng.FrameBuff.usBuffLen = 1600 * 2;
//    g_MuxFrameMng.FrameBuff.pRcvData =(char *)malloc(g_MuxFrameMng.FrameBuff.usBuffLen);

//    g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_START_UP;
//g_MuxFrameMng.FrameBuff.usRcvDataLen=0;//zl
//  // printf(MUX_LOG_LEVEL_2, "Mux start: AT+CMUX=0,0,5,1600\r");
//    //UartMuxDataReq(pAtCmux, 18 + 1);
//   // MuxFree(pAtCmux);
//   return;
//}

void SendUihTestCommond()
{
    unsigned char *pTestFrame = NULL;
    char *pAddr = NULL;
    unsigned char ucLen = 12;                                         /* 帧长度                         */

  //  printf(MUX_LOG_LEVEL_3, "Function SendUihTestCommond");       /* trace output                  */



    pTestFrame = (unsigned char *)MuxMalloc(ucLen);                   /*F9|ADDR|CLR|LEN|TYPE|LEN|VALUE0|VALUE1|VALUE2|VALUE3|FSC|F9*/
    if (NULL == pTestFrame)
    {
        //异常处理
 //       printf(MUX_LOG_LEVEL_1, "SendUihTestCommond: No memory");
        return;
    }

    pTestFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */
    pTestFrame[1] = 0x03;                                             /* Address Field (EA=1 CR=1 DLCI=0)*/
    pTestFrame[2] = MUX_UIH_FRAME | 0x10;                             /* Control Field (UIH Frame P=1)   */ 
    pTestFrame[3] = 0x0D;                                             /* Length Field (EA=1 length=6)    */
    pTestFrame[4] = MUX_UIH_TEST | 0x03;                              /* Type Field (EA=1 CR=1)          */
    pTestFrame[5] = 0x09;                                             /* Length Field (EA=1 length=4)    */

    pAddr = (char *)MuxMalloc(1);
    /*用随机申请的指针地址作为TEST验证值*/
    pTestFrame[6] = (unsigned char)((unsigned int)pAddr);                             
    pTestFrame[7] = (unsigned char)((unsigned int)pAddr >> 8);
    pTestFrame[8] = (unsigned char)((unsigned int)pAddr >> 16);
    pTestFrame[9] = (unsigned char)((unsigned int)pAddr >> 24); 
    MuxFree(pAddr);
    pAddr = NULL;

    pTestFrame[10] = SenderCreateFcs(3, &pTestFrame[1]);               /* Fcs Field                       */
    pTestFrame[11] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */

    /* 保存TEST值与接收到的响应值对比 */
    if (NULL == g_MuxFrameMng.FrameInfo.TestInfo.pData)
    {
        g_MuxFrameMng.FrameInfo.TestInfo.pData = (unsigned char *)MuxMalloc(4);
        if (NULL == g_MuxFrameMng.FrameInfo.TestInfo.pData)
        {
   //         printf(MUX_LOG_LEVEL_1, "SendUihTestCommond: No memory 2");
            MuxFree((char *)pTestFrame);
            return;
        }
    }
    MuxMemCopy((char *)g_MuxFrameMng.FrameInfo.TestInfo.pData, (char *)&pTestFrame[6], 4);
    g_MuxFrameMng.FrameInfo.TestInfo.usLen = 4;

    /* 备份帧，在重传的时候使用 */
    MuxMemSet((char *)g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy((char *)g_MuxRetranBuffer.pData, (char *)pTestFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    AT_SendData(pTestFrame, ucLen);

    MuxFree((char *)pTestFrame);
    return;
}

void SendUihFcONCommond()
{
    unsigned char *pFcONFrame = NULL;
    unsigned char ucLen = 8;                                           /* 帧长度                         */
    unsigned char i;


    pFcONFrame = (unsigned char *)MuxMalloc(ucLen);                     /* F9|ADDR|CLR|LEN|TYPE|LEN|FSC|F9 */
    pFcONFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */
    pFcONFrame[1] = 0x03;                                               /* Address Field (EA=1 CR=1 DLCI=0)*/
    pFcONFrame[2] = MUX_UIH_FRAME & 0xEF;                               /* Control Field (UIH Frame P=0)   */
    pFcONFrame[3] = 0x05;                                               /* Length Field (EA=1 length=2)    */
    pFcONFrame[4] = MUX_UIH_FCon | 0x03;                                /* Type Field (EA=1 CR=1)          */
    pFcONFrame[5] = 0x01;                                               /* Length Field (EA=1 length=0)    */
    pFcONFrame[6] = SenderCreateFcs(3, &pFcONFrame[1]);                 /* Fcs Field                       */
    pFcONFrame[7] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */

    /* 打开整体流控 */
    for(i = 0; i < MUX_DLC_NUM; i++)
    {
        g_MuxDlcMng.Dlc[i].ucFc = MUX_ON;
    }

    AT_SendData(pFcONFrame, ucLen);

    MuxFree((char *)pFcONFrame);

    return;
}
void SendSABMFrame()
{
    unsigned char *pSABMFrame = NULL;
    unsigned char ucLen = 6;                                          /* 帧长度                           */
    unsigned char ucDlci = g_MuxDlcMng.Dlc0.ucDlciFlowOver;           /* 待建立的通道号                   */

    pSABMFrame = (unsigned char *)MuxMalloc(ucLen);
    if (NULL == pSABMFrame)
    {
       
        return;
    }

    pSABMFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */
    pSABMFrame[1] = (ucDlci << 2) | 0x03;                             /* Address Field (EA=1 CR=1 DLCI=ucDlci)*/
    pSABMFrame[2] = MUX_SABM_FRAME | 0x10;                            /* Control Field (UIH Frame P=1)   */
    pSABMFrame[3] = 0x01;                                             /* Length Field (EA=1 length=0)    */
    pSABMFrame[4] = SenderCreateFcs(3, &pSABMFrame[1]);               /* Fcs Field                       */
    pSABMFrame[5] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */

    /* 备份帧，在重传的时候使用 */
    MuxMemSet((char *)g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy((char *)g_MuxRetranBuffer.pData, (char *)pSABMFrame, ucLen);
     // .usLen = ucLen;

    AT_SendData(pSABMFrame, ucLen);

    MuxFree((char *)pSABMFrame);
    return;
}


void SendUihServiceNegCommond()
{
    unsigned char *pServiceFrame = NULL;
    unsigned char ucDlci = g_MuxDlcMng.Dlc0.ucDlciFlowOver;              /* 需要协商的通道号                */
    unsigned char ucLen = 11;                                            /* 帧长度                         */ 


    pServiceFrame = (unsigned char *)MuxMalloc(11);                      /*F9|ADDR|CLR|LEN|TYPE|LEN|VALUE0|VALUE1|VALUE2|FSC|F9*/
    if (NULL == pServiceFrame)                                 
    {
        return;
    }
    pServiceFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */                    
    pServiceFrame[1] = 0x03;                                             /* Address Field (EA=1 CR=1 DLCI=0)*/
    pServiceFrame[2] = MUX_UIH_FRAME | 0x10;                             /* Control Field (UIH Frame P=1)   */
    pServiceFrame[3] = 0x0B;                                             /* Length Field (EA=1 length=5)    */
    pServiceFrame[4] = MUX_UIH_SNC | 0x03;                               /* Type Field (EA=1 CR=1)          */
    pServiceFrame[5] = 0x07;                                             /* Length Field (EA=1 length=3)    */
    pServiceFrame[6] = (ucDlci << 2) | 0x03;                             /* DLCI (EA=1 CR=1)                */

    switch (ucDlci)
    {
    case MUX_DLC_AT_1:
    case MUX_DLC_AT_2:
    case MUX_DLC_AT_3:
        pServiceFrame[7] = 0x01;                                         /* Service value (EA=1 AT mode)    */
        pServiceFrame[8] = 0x01;                                         /* Voice codec value (EA=1 None)   */
        break;
    case MUX_DLC_DATA_1:   
    case MUX_DLC_DATA_2:   
    case MUX_DLC_DATA_3:
        pServiceFrame[7] = 0x03;                                         /* Service value (EA=1 Data mode)  */
        pServiceFrame[8] = 0x01;                                         /* Voice codec value (EA=1 None)   */
        break;
    case MUX_DLC_VOICE_1:  
    case MUX_DLC_VOICE_2:
        pServiceFrame[7] = 0x05;                                         /* Service value (EA=1 Voice mode) */
        pServiceFrame[8] = 0x01;                                         /* Voice codec value (EA=1 None)   */
    default:
//        printf(MUX_LOG_LEVEL_1, "Dlci out of range!");
        break;
    }

    pServiceFrame[9] = SenderCreateFcs(3, &pServiceFrame[1]);           /* Fcs Field                       */
    pServiceFrame[10] = MUX_FLAG_SEQUENCE_FIELD;                        /* Flag Sequence Field(0xF9)       */

    /*备份帧，在重传的时候使用*/
    MuxMemSet((char *)g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy((char *)g_MuxRetranBuffer.pData, (char *)pServiceFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    AT_SendData(pServiceFrame, ucLen);

    MuxFree((char *)pServiceFrame);

    return;
}


void SendUihRTRONCommond()
{
    unsigned char *pRTRonFrame = NULL;
    unsigned char ucDlci = g_MuxDlcMng.Dlc0.ucDlciFlowOver;              /* 需要协商的通道号                */
    unsigned char ucLen = 10;                                            /* 帧长度                         */



    pRTRonFrame = (unsigned char *)MuxMalloc(ucLen);                     /* F9|ADDR|CLR|LEN|TYPE|LEN|DLCI|V.24|FSC|F9 */
    if (NULL == pRTRonFrame)
    {
 //       printf(MUX_LOG_LEVEL_1, "SendUihRTRONCommond: No memory!");
        return;
    }
    pRTRonFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */
    pRTRonFrame[1] = 0x03;                                               /* Address Field (EA=1 CR=1 DLCI=0)*/
    pRTRonFrame[2] = MUX_UIH_FRAME | 0x10;                               /* Control Field (UIH Frame P=1)   */
    pRTRonFrame[3] = 0x09;                                               /* Length Field (EA=1 length=4)    */
    pRTRonFrame[4] = MUX_UIH_MSC | 0x03;                                 /* Type Field (EA=1 CR=1)          */
    pRTRonFrame[5] = 0x05;                                               /* Length Field (EA=1 length=2)    */
    pRTRonFrame[6] = (ucDlci << 2) | 0x03;                               /* DLCI Field (EA=1 CR=1)          */

    pRTRonFrame[7] = 0x00;                                               /* V.24控制字节初值                 */
    pRTRonFrame[7] |= 0x01;                                              /* EA bit  no Break Signals        */                                      
    pRTRonFrame[7] |= 0x04;                                              /* RTC bit                         */
    pRTRonFrame[7] |= 0x08;                                              /* RTR bit                         */
    pRTRonFrame[7] |= 0x80;                                              /* DV bit                          */

    pRTRonFrame[8] = SenderCreateFcs(3, &pRTRonFrame[1]);                /* Fcs Field                       */
    pRTRonFrame[9] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */

    /* 备份帧，重传的时候调用 */
    MuxMemSet((char *)g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy((char *)g_MuxRetranBuffer.pData, (char *)pRTRonFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    AT_SendData(pRTRonFrame, ucLen);

    MuxFree((char *)pRTRonFrame);                       
    return;
}


void SendUihRTRONResponse()
{
    unsigned char *pRTRonFrame = NULL;
    unsigned char ucDlci;                                                /* 需要协商的通道号                */
    unsigned char ucLen = 10;                                            /* 帧长度                         */

 //   printf(MUX_LOG_LEVEL_3, "Function SendUihRTRONResponse");        /* trace output                  */

    ucDlci = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci;
    pRTRonFrame = (unsigned char *)MuxMalloc(ucLen);                     /* F9|ADDR|CLR|LEN|TYPE|LEN|DLCI|V.24|FSC|F9 */
    if (NULL == pRTRonFrame)
    {
//        printf(MUX_LOG_LEVEL_1, "SendUihRTRONResponse: No memory!");
    }
    pRTRonFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */
    pRTRonFrame[1] = 0x03;                                               /* Address Field (EA=1 CR=1 DLCI=0)*/
    pRTRonFrame[2] = MUX_UIH_FRAME | 0x10;                               /* Control Field (UIH Frame P=1)   */
    pRTRonFrame[3] = 0x09;                                               /* Length Field (EA=1 length=4)    */
    pRTRonFrame[4] = MUX_UIH_MSC | 0x01;                                 /* Type Field (EA=1 CR=0)          */
    pRTRonFrame[5] = 0x05;                                               /* Length Field (EA=1 length=2)    */
    pRTRonFrame[6] = (ucDlci << 2) | 0x03;                               /* DLCI Field (EA=1 CR=1)          */
    pRTRonFrame[7] = g_MuxFrameMng.FrameInfo.ucV24;                      /* V.24 signal                     */                        
    pRTRonFrame[8] = SenderCreateFcs(3, &pRTRonFrame[1]);                /* Fcs Field                       */
    pRTRonFrame[9] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */

    AT_SendData(pRTRonFrame, ucLen);

    /* 打开RTR流控 */
    g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucRtrFc = MUX_ON;

    MuxFree((char *)pRTRonFrame);                       
    return;
}

void MuxFree(char *pStr)
{
    if(NULL != pStr)
    {
        free(pStr);
    }
}

void *MuxMalloc(unsigned long ulen)//void MuxFree(char *pStr)
{
    void *pTmp;

    pTmp = malloc(ulen);

    if(NULL == pTmp)
    {
     //   printf(MUX_LOG_LEVEL_1,"Memory malloc Error\r\n");
    }

    memset(pTmp, 0, ulen);

    return pTmp;
}

void MuxMemSet(char *pDest, char val, unsigned long ulen)//void *MuxMalloc(unsigned long ulen)//void MuxFree(char *pStr)
{
    if(NULL == pDest)
    {
      //  printf(MUX_LOG_LEVEL_1, "Memory Set Error");
    }

    memset(pDest, val, ulen);
}
void MuxMemCopy(char *pDest, char *pSour, unsigned long ulen)//void MuxMemSet(char *pDest, char val, unsigned long ulen)//void *MuxMalloc(unsigned long ulen)//void MuxFree(char *pStr)
{
	//printf("yeah004");
    memcpy(pDest, pSour, ulen);
	//printf("yeah005");
	
}


void UihFrameCreate(char *pBuffer, unsigned short usBufferLen)
{
    unsigned short usFrameLen = 0;                      /* 发送帧长度 */
    unsigned char  *pDataFrame = NULL;                  /* 帧的数据 */
	  //char  *pDataFrame = NULL; 
    unsigned char  ucFcs = 0;                           /* 校验位 */
    unsigned char  i = 0;  
    unsigned char  n = 0;                               /* 发送帧的个数 */
    unsigned char  *pUihData = NULL;
    // char  *pUihData = NULL;

   // printf(MUX_LOG_LEVEL_3, "Function UihFrameCreate");

    if ((0 == usBufferLen) || (NULL == pBuffer))
    {/* 收到的数据为空 */
      //  printf(MUX_LOG_LEVEL_1, "data of pBuffer is empty Or usBufferLen is 0");
        return;
    }
    /* 1byte Length Indicator */
    if (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 <= (MUX_MAX_LENGTH_INDICATOR + 6))
    {
        /* Frame length(Length Indicator 1 Oct) without Information */
        n = (usBufferLen - 1) / (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 6) + 1;
    }
    /* 2byte Length Indicator */
    else
    {
        /* Frame l ength(Length Indicator 2 Oct) without Information */
        n = (usBufferLen - 1) / (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7) + 1;
    }

    /* 做n次FRAME，并发送 */
    for (i = 0; i < n; i++)
    {
        /* 帧长度为N1时 */
        if (i < (n - 1))
        {
            usFrameLen = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1;
            if (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 <= (MUX_MAX_LENGTH_INDICATOR + 6))
            {
                usBufferLen   = usBufferLen - (usFrameLen - 6);
                pUihData   = (u8 *)&pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 6)];
            }
            else
            {
                usBufferLen   = usBufferLen - (usFrameLen - 7);
                pUihData   = (u8 *)&pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7)];
            }
        }
        /* 帧长度小于N1时 */
        else
        {
            if(usBufferLen <= MUX_MAX_LENGTH_INDICATOR)
            {
                usFrameLen = usBufferLen + 6;
                if (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 <= (MUX_MAX_LENGTH_INDICATOR + 6))
                {
                    //usFrameLen = usBufferLen + 6;
                    pUihData   = (u8 *)&pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 6)];
                }
                else
                {
                    //usFrameLen = usBufferLen + 7;
                    pUihData   = (u8 *)&pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7)];
                }         
            }
            else
            {
                usFrameLen = usBufferLen + 7;
                pUihData   = (u8 *)&pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7)];
            }
        }
        //pDataFrame =(unsigned char *)MuxMalloc((unsigned long)usFrameLen);
				pDataFrame =(unsigned char *)MuxMalloc((unsigned long)usFrameLen);
        /* 做成FRAME */
        pDataFrame[0]  = MUX_FLAG_SEQUENCE_FIELD;                                            /* Flag Sequence Field(0xF9) */
        pDataFrame[1]  = 0x01 | (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci << 2);/* Address Field (EA=1 CR=0 DLCI = g_MuxDlcMng.ucDlciIndex) */
        pDataFrame[2]  = MUX_UIH_FRAME;                                                      /* Control Field (UIH Frame F=0) */
        /* 1byte Length Indicator */
        //printf("yes");
				if (usFrameLen <= (MUX_MAX_LENGTH_INDICATOR + 6))
        {
            pDataFrame[3]  = 0x01 | ((usFrameLen - 6) << 1);           /* Length Field (EA=1) */
            ucFcs          = SenderCreateFcs(3, &pDataFrame[1]);
            MuxMemCopy((char *)&pDataFrame[4], (char *)pUihData, (usFrameLen - 6));
        }
        /* 2byte Length Indicator */
        else
        {
            pDataFrame[3]  = ((usFrameLen - 7) << 1) & 0xFE;            /* Length Field (EA=0) */
            pDataFrame[4]  = (usFrameLen - 7) >> 7;
            ucFcs          = SenderCreateFcs(4, &pDataFrame[1]);
            MuxMemCopy((char *)&pDataFrame[5], (char *)pUihData, (usFrameLen - 7));
        }
        pDataFrame[usFrameLen - 2] = ucFcs;                                                   /* FCS */
        pDataFrame[usFrameLen - 1] = MUX_FLAG_SEQUENCE_FIELD;                                 /* Flag Sequence Field(0xF9) */
        /* 发送 */
      //  printf( MUX_LOG_LEVEL_2, "send a Frame whose length is %d to USB", usFrameLen);
        AT_SendData(pDataFrame, usFrameLen);
        MuxFree((char*)pDataFrame);
    }

    return;
}

//u8 UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen)
//{
////	unsigned short  SendOffset = 0;
//  int i;
//	TimeTemp = SysTime;
//	if(usUartLen > 0)
//  {
//		for(i=0; i<usUartLen; i++)
//		{
//			USART_SendData(USART2,pUartSndBuff[i]);
//			while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
//			WaitTimeOut(10000);
//		}
//  }
//   return 1;
//}








u8 ChannelOpen()
{
	int i;
	g_MuxDlcMng.Dlc0.ucDlciFlowOver=0;
	SendUihTestCommond();
	delay_ms(10);
	SendUihFcONCommond();
	delay_ms(10);
	
	for(i=0;i<5;i++)
	{
		g_MuxDlcMng.Dlc0.ucDlciFlowOver++;
  	SendSABMFrame();
	  delay_ms(10);
	  SendUihServiceNegCommond();
	  delay_ms(10);	
  	SendUihRTRONResponse();
  	delay_ms(10);
  	SendUihRTRONCommond();
  	delay_ms(10);
	}
	//TODO:clear rec
	DebugOut("channel open finished.\r\n");
	return 1;
}



































int   receivelen=0;

char  *pTmpFrame=0;
char  *pFrameData=0;
char  receivebuffer[1600];
char  message1[4]="CMT:";
char  message2[4]="CDS:";
char  message3[4]="CBM:";

//receivebuffer = MuxMalloc(maxbuffer);
  
void analyze(char *buffer, unsigned short len)
{
   char ucAddress;
	 char DLCi;	 
	 int uslen=0,startlen=0; 
	 int lenflag;
	 int j=0;
	
	 ucAddress = *(buffer);
	 DLCi = ucAddress>>2;	 
	  
	 if(1 == (0x01 & buffer[2])) 
	 {
	    lenflag=1;
	 }
	 else
	 {
	    lenflag=2;
	 } 
	 
	 startlen=2+lenflag;
	 
	 if(DLCi<5)
	 {

		 while(startlen<len)
		 { 

        if(buffer[startlen++]=='\r'&&buffer[startlen++]=='\n')
		    {
				  
          if(buffer[startlen]=='>'&&buffer[startlen+1]==' ')
					{
	            uslen=2;
						  j=2;
					}
					else
					{
						if(buffer[startlen]=='+')
					  {
						  if((!strncmp(buffer+startlen+1,message1,4))||(!strncmp(buffer+startlen+1,message2,4))||(!strncmp(buffer+startlen+1,message3,4)))
	  					{
								startlen+=5;
								while(buffer[startlen++]!='\r');
								startlen++;
							}								
					  }
						for(j=0;startlen+j<len-1;j++)
					  {
					     if(buffer[startlen+j]=='\r'&&buffer[startlen+j+1]=='\n')
			         {
			           uslen=j;
                 break;
						   }						
					  }
            if(startlen+j==len-1)	return;
				  }	

					ComBuf[ComBufIndexEnd][uslen] = 0;
	        MuxMemCopy((char *)ComBuf[ComBufIndexEnd],buffer+startlen,uslen);

			    startlen+=j+2;									  			 

					ComBufIndexPlusPlus(ComBufIndexEnd);
			}
		 }
	 }
	 else
	 {
	    uslen=len-3-lenflag;
	 }
}

int get(char *data, unsigned short len)
{

	  unsigned char   *pData = 0;
	  unsigned short  i,j,length,FrameDataLen,uslen;
	
	  if(len!=0&&data!=0)
	  {
    	 uslen=len;	
       //receivebuffer = MuxMalloc(uslen+receivelen);				
       MuxMemCopy(receivebuffer+receivelen,data,uslen);
       receivelen+=uslen; 
			 len=0;
	  }		
		
	  pTmpFrame=receivebuffer;
	
    while(receivelen!= 0)
    {
		    for (i = 0; i < receivelen; i++)
        {
            if (MUX_FLAG_SEQUENCE_FIELD == (u8)pTmpFrame[i])//zl?
            {
                 break;
            }
        }

       if (i == receivelen)
       {
          if ((0!= pTmpFrame) && (0 !=receivelen))
          {
            MuxMemSet(pTmpFrame, 0, receivelen);
          }
          receivelen= 0;
				  return 0;
       }

       if (5 > (receivelen - i))//×Ü³¤Ð¡ÓÚ5µÄÇé¿ö
       {
          if (0 != i)
          {
            pData = MuxMalloc(receivelen - i);
            MuxMemCopy((char *)pData, pTmpFrame + i, receivelen - i);
            MuxMemSet(pTmpFrame, 0, receivelen);
            MuxMemCopy(pTmpFrame, (char *)pData, receivelen - i);
            receivelen -= i;
            MuxFree((void*)pData);
           }
       }
			 else
			 {			 
					 if (1 == (0x01 & pTmpFrame[i + 3]))//µÃ³öÔÚ¶ÁÖ¡µÄ¶¨Òå³¤£¨È¥µôÍ·Î²F9£©
					 {
							 length = pTmpFrame[i + 3];
							 uslen = length >> 1;
							 uslen += 4;
					 }
					 else
					 {
							 length = pTmpFrame[i + 4];
							 length <<= 8;
							 length |= pTmpFrame[i + 3];
							 uslen =length >> 1;
							 uslen += 5;
					 }
			
					 if (receivelen < (i + uslen + 2))//Êý¾Ý×Ü³¤Ð¡ÓÚ±ê×¼Ò»Ö¡³¤µÄÇé¿ö
					 {
							 if (0 != i)
							 {
									pData = MuxMalloc(receivelen - i);
									MuxMemCopy((char *)pData, pTmpFrame + i, receivelen - i);
									MuxMemSet(pTmpFrame, 0, receivelen);
									MuxMemCopy(pTmpFrame, (char *)pData, receivelen - i);
									MuxFree((void*)pData);
									receivelen-=i;
							 }
               //for(k=0;k<receivelen;k++)  printf("%c",pTmpFrame[k]);							 
							 return 2;//ÊÇ·ñÓÃ²»Í¬µÄ±êÖ¾
					 } 
		 
					 if (MUX_FLAG_SEQUENCE_FIELD != (u8)pTmpFrame[i + uslen + 1])//zl?
					 {
							 for (j = i + 1; j < receivelen; j++)//ÕÒF9
							 {
									if (MUX_FLAG_SEQUENCE_FIELD == (u8)pTmpFrame[j])//zl?
									{
											break;		
									}
							 }
							 
							 if (j == receivelen)//meizhaodao
							 {
								 MuxMemSet(pTmpFrame, 0, receivelen); 
								 receivelen=0;					 
								 return 0;
							 }

							 receivelen -= j;
							 pData = MuxMalloc(receivelen);
							 MuxMemCopy((char *)pData, pTmpFrame + j , receivelen);
							 MuxMemSet(pTmpFrame, 0, receivelen + j);
							 MuxMemCopy(pTmpFrame, (char *)pData,receivelen);
							 MuxFree((void*)pData);
					 }
					 else
					 {
							 FrameDataLen  = uslen;
							 pFrameData = MuxMalloc(FrameDataLen);
							 MuxMemCopy(pFrameData, pTmpFrame + i + 1, FrameDataLen);
							 analyze(pFrameData,FrameDataLen );
						   MuxFree((void*)pFrameData);
						 						 
							 if (receivelen != (i + uslen + 2))
							 {
									receivelen -= (i + uslen + 2);
									pData = MuxMalloc(receivelen);
									MuxMemCopy((char *)pData, pTmpFrame + i + uslen+2, receivelen);
									MuxMemSet(pTmpFrame, 0, receivelen + i + uslen+2);
									MuxMemCopy((char *)pTmpFrame, (char *)pData, receivelen);
									MuxFree((void*)pData);
							 }
							 else
							 {
									MuxMemSet(pTmpFrame, 0, receivelen);
									receivelen = 0;
									return 1;
							 }
					 }
			 }
     }
		return 1;
}




