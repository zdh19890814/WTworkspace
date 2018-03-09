//*************************receive***********************************//
#define error 0
#define ok 1
#define maxbuffer 3200;

int   receivelen=0;

char  *pTmpFrame=NULL;
char  *pFrameData=NULL;
char  *receivebuffer=NULL;
  
receivebuffer= MuxMalloc(maxbuffer);

void analyze(char *buffer, unsigned short len)
{
   char ucAddress;
	 char DLCi;	 
	 char  *AT_databuffer=NULL;
	 char  *PPP_databuffer=NULL;
	 int uslen,startlen; 
	 int lenflag;
	
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
	 uslen=len-3-lenflag;
	 
	 if(DLCi<5)
	 {
		 if(buffer[startlen]=='\r'&&buffer[startlen+1]=='\n')
		 {
		   startlen+=2;
			 uslen-=4;
		 }

		 AT_databuffer = MuxMalloc(uslen);
	   MuxMemCopy(AT_databuffer,buffer+startlen,uslen); 
	 }
	 else
	 {
	    PPP_databuffer = MuxMalloc(len-3-lenflag);
	    MuxMemCopy(PPP_databuffer+2+lenflag,buffer,uslen); 
	 }
}
	
int get(char *data, unsigned short len)
{

	  unsigned char   *pData = NULL;
	  unsigned short  i, j,length,FrameDataLen,uslen; ;
	  
    if(len!=0)
	  {
    	 uslen=len;
       MuxMemCopy(receivebuffer+receivelen,data,uslen);
       receivelen+=len; 
			 len=0;
	  }		
 
	  pTmpFrame=receivebuffer;
	
    while(receivelen!= 0)
    {
		    for (i = 0; i < len; i++)
        {
            if (MUX_FLAG_SEQUENCE_FIELD == pTmpFrame[i])
            {
                 break;
            }
        }

       if (i == receivelen)
       {
          if ((NULL != pTmpFrame) && (0 !=len))
          {
            MuxMemSet(pTmpFrame, 0, len);
          }
          receivelen= 0;
				  return error;
       }

       if (5 > (receivelen - i))//总长小于5的情况(待写)
       {
          if (0 != i)
          {
            pData = MuxMalloc(receivelen - i);
            MuxMemCopy(pData, pTmpFrame + i, receivelen - i);
            MuxMemSet(pTmpFrame, 0, len);
            MuxMemCopy(pTmpFrame, pData, receivelen - i);
            receivelen -= i;
            MuxFree((void*)pData);
           }
       }
			 else
			 {			 
					 if (1 == (0x01 & pTmpFrame[i + 3]))//得出在读帧的标致长
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
			
					 if (receivelen < (i + uslen + 2))//数据总长小于一帧标致长的情况
					 {
							 if (0 != i)
							 {
									pData = MuxMalloc(receivelen - i);
									MuxMemCopy(pData, pTmpFrame + i, receivelen - i);
									MuxMemSet(pTmpFrame, 0, receivelen);
									MuxMemCopy(pTmpFrame, pData, receivelen - i);
									MuxFree((void*)pData);
									receivelen-=i;
							 }					    
							 return ok;
					 } 
		 
					 if (MUX_FLAG_SEQUENCE_FIELD != pTmpFrame[i + uslen + 1])//没对应到F9尾
					 {
							 for (j = i + 1; j < receivelen; j++)//找F9
							 {
									if (MUX_FLAG_SEQUENCE_FIELD == pTmpFrame[j+tmplen])
									{
											break;		
									}
							 }
							 
							 if (j == receivelen)//没有找到F9
							 {
								 MuxMemSet(pTmpFrame, 0, receivelen); 
								 receivelen=0;					 
								 return error;
							 }

							 receivelen -= j;
							 pData = MuxMalloc(receivelen);
							 MuxMemCopy(pData, pTmpFrame + j , receivelen);
							 MuxMemSet(pTmpFrame, 0, len + j);
							 MuxMemCopy(pTmpFrame, pData,receivelen);
							 MuxFree((void*)pData);
						}
						else
						{
							 FrameDataLen  = uslen;
							 pFrameData = MuxMalloc(FrameDataLen);
							 MuxMemCopy(pFrameData, pTmpFrame + i + 1, FrameDataLen);
							 analyze(pFrameData,FrameDataLen );
							 
							
							 if (receivelen != (i + uslen + 2))
							 {
									receivelen -= (i + uslen + 2);
									pData = MuxMalloc(len);
									MuxMemCopy(pData, pTmpFrame + i + uslen, receivelen);
									MuxMemSet(pTmpFrame, 0, receivelen + i + uslen);
									MuxMemCopy(pTmpFrame, pData, receivelen);
									MuxFree((void*)pData);
							 }
							 else
							 {
									MuxMemSet(pTmpFrame, 0, receivelen);
									receivelen = 0;
									return ok;
							 }
						 }
			 }
     }
}

//*************************send***********************************//
typedef struct {
    unsigned char *pData;
}ST_ATSYMBAL;

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



void send(char *sendbuffer, int len)
{
  int uslen;
	int DLCi;
	unsigned char  ucFcs = 0; 
	char *ussendbuffer;
	
	DLCi=1;
	uslen=len+8;
	ussendbuffer=(unsigned char *)MuxMalloc((unsigned long)uslen);
	ussendbuffer[0]=0xF9;
	ussendbuffer[1]=0x01|(DLCi << 2);
	ussendbuffer[2] = 0xEF; 
	ussendbuffer[3] =0x01 | ((uslen - 6) << 1);
	
	ucFcs  = SenderCreateFcs(3, &ussendbuffer[1]);
	
	MuxMemCopy(ussendbuffer+4,sendbuffer,uslen-8);
	ussendbuffer[uslen-4] ='\r';
	ussendbuffer[uslen-3] ='\n';
	ussendbuffer[uslen-2] =ucFcs; 
	ussendbuffer[uslen-1] =0xF9;
	
  UartMuxDataReq(ussendbuffer, uslen);
  MuxFree((char*)ussendbuffer);
}






