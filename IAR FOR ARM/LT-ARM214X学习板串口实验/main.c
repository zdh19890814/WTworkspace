/******************************************************************
  Copyright (C), 2008-2009, 力天电子，LiTian Tech.Co.Ltd.
  Module Name   : uart              
  File Name     : uart.c	            
  Author   	: HE CHENG         
  Create Date  	: 2009/10/19         
  Version   	: 1.0                
  Function      : 串口实验                        
  Description  	: 无                 
  Support     	: www.LT430.com  QQ:330508762                  
******************************************************************/
#include <NXP/iolpc2148.h>
#include <intrinsics.h>
#include "uart.h"
#include "bsp.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdarg.h> 
#include <time.h>
#include <errno.h>



//延时函数
void delayn(unsigned long n)
{
  while(n--);
}
//系统时钟宏定义
#define XTALFREQ 12000000         //XTAL frequency in Hz
#define PCLKFREQ (XTALFREQ/4)     //pclk must always be XTALFREQ/4?





#define PTMASTER        "/dev/ptmx"
//#define RC_LINK_FILE    "/dev/rcpts"
#define RIL_LINK_FILE   "/dev/rilpts"
//#define PPP_LINK_FILE   "/dev/ppppts"
#define DEVICE_UART     "/dev/ttyS1"

//static int RcFd_Master = 0;
static int RilFd_Master = 0;   //zdh  chuli wenjian duqu
//static int PppFd_Master = 0; 
static int UartFd = 0; 

////pthread_t   RcRcvThread;
//pthread_t   RilRcvThread;
////pthread_t   PppRcvThread;
//pthread_t   UartRcvThread;
//
//pthread_mutex_t        UartSendMutex = PTHREAD_MUTEX_INITIALIZER;
//
//pthread_mutex_t        MultiSend = PTHREAD_MUTEX_INITIALIZER;


static int main_exit_signal = 0;

// #define MAX_RC_MSG_LEN     (8 * 1024)
#define MAX_RIL_MSG_LEN     (8 * 32)
// #define MAX_PPP_MSG_LEN     (8 * 1024)      
#define MAX_UART_MSG_LEN    (8 * 32)

//static char RcReadBuf[MAX_RC_MSG_LEN];
static char RilReadBuf[MAX_RIL_MSG_LEN];
// static char PppReadBuf[MAX_PPP_MSG_LEN];
static char UartReadBuf[MAX_UART_MSG_LEN];

/* MUX TRACE */
#define _MUX_LOG_BY_ITSELF

#ifdef _MUX_LOG_BY_ITSELF

#define MUX_LOG_FILE_NAME       "HlMux.log"
//#define MUX_TRACE_OUTPUT_PATH   "/data/lc-elog/"
#define MUX_TRACE_OUTPUT_PATH   "/data/local/log/"



static FILE  *pLogFile = NULL;
//static void OpenLogFile();

#endif

#define _MUX_FCS_CHECK_OFF


/* MUX宏定义 */
typedef enum
{
    MUX_LOG_LEVEL_1 = 0,                /* 函数在返回错误时，错误类型打印           */
    MUX_LOG_LEVEL_2,                    /* 从UART或其他接口收到消息时，解析时解析出的重要信息打印; 重要事件开始和结束时 */
    MUX_LOG_LEVEL_3,                    /* 进入、离开函数时                         */
    MUX_LOG_HIGH_LEVEL
} E_MUX_LOG_TYPE;

static E_MUX_LOG_TYPE LogLevel = MUX_LOG_LEVEL_3;

/* MUX DLCI */                     //zdh   DLC_AT keyi he zai yiqi       keyi delete DLC_DATA_3      DLC_VOICE keyi he zai yiqi                                                      
#define MUX_DLC0                            0           /* DLC0                             */
#define MUX_DLC_AT_1                        1           /* DLC1 for general AT include SS AT */
#define MUX_DLC_AT_2                        2           /* DLC2 for voice call AT           */
#define MUX_DLC_AT_3                        3           /* DLC3 SMS & STK AT                */
#define MUX_DLC_DATA_1                      4           /* DLC4 for PDP                     */   //AT
#define MUX_DLC_DATA_2                      5           /* DLC5 for PDP                     */   //PPP
#define MUX_DLC_DATA_3                      6           /* DLC6 for PDP                     */
#define MUX_DLC_VOICE_1                     7           /* DLC7                             */
#define MUX_DLC_VOICE_2                     8           /* DLC8                             */
#define MUX_DLC_NUM                         8           /* max dlc num                      */

/* MUX FLOW ID */                                                                          
/* MUX DLC整体使用 */                                                                       
#define MUX_FLOW_NONE                      0           /* 没有事件发生，解frame时先附空    */

/* MUX DLC0使用 */                                                                          
#define MUX_FLOW_START_UP                  1           /* 启动Mux                          */
#define MUX_FLOW_TEST                      2           /* 通信测试                         */
#define MUX_FLOW_PARAMETER_NEGOTIATION     3           /* DLC参数协商                      */
#define MUX_FLOW_CLOSE_DOWN                4           /* 关闭MUX                          */
#define MUX_FLOW_OVERALL_FC_ON             5           /* 整体流控ON                       */
#define MUX_FLOW_OVERALL_FC_OFF            6           /* 整体流控OFF                      */

/* MUX DLC1-DLC8使用 */                                                                     
#define MUX_FLOW_DLC_ESTABLISH             7           /* 建立DLC                          */
#define MUX_FLOW_DLC_RELEASE               8           /* 释放DLC                          */
#define MUX_FLOW_SERVICE_NEGOTIATION       9           /* DLC服务协商                      */
#define MUX_FLOW_APL_FC_ON                 10          /* APL流控ON                        */
#define MUX_FLOW_APL_FC_OFF                11          /* APL流控OFF                       */
#define MUX_FLOW_RECEIVE_FC_ON             13          /* 收到流控ON                       */
#define MUX_FLOW_RECEIVE_FC_OFF            14          /* 收到流控OFF                      */
#define MUX_FLOW_PPP_FC_ON                 15          /* PPP流控ON                        */
#define MUX_FLOW_PPP_FC_OFF                16          /* PPP流控OFF                       */
#define MUX_FLOW_RECEIVE_DATA_FROM_UART    17          /* UART收信通知                     */
#define MUX_FLOW_RECEIVE_DATA_FROM_ATC     18          /* ATC送信通知                      */
#define MUX_FLOW_RECEIVE_DATA_FROM_PPP     19          /* PPP送信通知                      */

/* MUX STATUS */                                                                            
#define MUX_STATUS_CLOSED_DOWN              0           /* MUX关闭状态                      */
#define MUX_STATUS_CONNECT_WAIT             1           /* MUX等待建立状态                  */
#define MUX_STATUS_DISCONNECTED             2           /* DLC链路未建立                    */
#define MUX_STATUS_CONNECTED                3           /* DLC链路建立，可以传输数据        */

/* MUX FLG STATUS */                                                                        
#define MUX_ON                              1           /* FLG置位                          */
#define MUX_OFF                             0           /* FLG清位                          */

/* MUX RETURN VALUE */                                                                      
#define MUX_FALSE                           0           /* 返回失败                         */
#define MUX_TRUE                            1           /* 返回成功                         */
#define MUX_CONTINUE                        2           /* 返回成功,进行下一轮操作          */
#define MUX_TYPE_INVALID                    4           /* MUX消息类型无效                  */

/* MUX FRAME TYPE of Control field */                                                       
#define MUX_SABM_FRAME                      0x2F        /* SABM frame                       */
#define MUX_DISC_FRAME                      0x43        /* DISC frame                       */
#define MUX_UIH_FRAME                       0xEF        /* UIH frame                        */
#define MUX_UA_FRAME                        0x63        /* UA frame                         */
#define MUX_DM_FRAME                        0x0F        /* DM frame                         */

/* MUX VALUE of TYPE in UIH frame */                                                        
#define MUX_UIH_PN                          0x80        /* 参数协商                         */
#define MUX_UIH_CLD                         0xC0        /* 关闭MUX                          */
#define MUX_UIH_TEST                        0x20        /* 通信测试                         */
#define MUX_UIH_FCon                        0xA0        /* 整体流控ON                       */
#define MUX_UIH_FCoff                       0x60        /* 整体流控OFF                      */
#define MUX_UIH_SNC                         0xD0        /* 服务协商                         */
#define MUX_UIH_MSC                         0xE0        /* RTR流控                          */

/* Dlc service type */                                                                      
#define MUX_SERV_AT                         0x01        /* AT service                       */
#define MUX_SERV_DATA                       0x03        /* DATA service                     */
#define MUX_SERV_VOICE                      0x05        /* VOICE service                    */

/* MUX INFORMATION OF TIMER */                                                              
#define MUX_DEFAULT_TIME_OF_T1              10          /* 0.1s  单位：0.01s                */
#define MUX_DEFAULT_TIME_OF_T2              200         /* 2s(default 0.3s)  单位：0.01s    */

/* MUX COMMON INFORMATION */                                                                
#define MUX_MAX_SIZE_OF_FRAME               31          /* FRAME数据默认长度：31(default)    */
#define MUX_MAX_NUM_OF_RETRANS              3           /* 重传最大次数：3  初始化时用      */
#define MUX_FLAG_SEQUENCE_FIELD             0xF9        /* frame的起始终了Flag固定值        */
#define MUX_MAX_CNT_OF_BUFFER               (16 * 4)  /* MUX(UART)能收到的最大数据长度    */
#define MUX_UART_CNT_OF_BUFFER              (16 * 4)  /* UART能收到的最大数据长度         */

/* MUX PPP STATUS */                                                                        
#define DTX_READY                           0           /* frame的起始终了Flag固定值        */
#define DTX_STOP                            2                                               

/* MUX BUFFER SIZE */                                                                       
//#define MAX_PPP_DATA_BUFFER_SIZE            (4096*5)    /* PPP能发送的最大数据长度          */
#define MAX_UART_BUFFER_SIZE                (128)      /* UART能发送的最大数据长度         */
#define MAX_UART_RX_MEM_SIZE                (128)      /* UART能收到的最大数据长度         */

/* MUX MODE */
#define OFFLINE_AT_MODE                     0           /*                                  */
//#define ONLINE_AT_MODE                      1           /*                                */
#define ONLINE_DATA_MODE_ESTABLISHING       2           /*                                  */
#define ONLINE_DATA_MODE                    3           /* 对于DLC_AT:缓存AT数据；对于DLC_DATA:发送PPP数据 */

/* define MAX Of Length Indicator */
#define MUX_MAX_LENGTH_INDICATOR            127         /* EA=1时Length Indicator可指示的最大长度 */

/*         AT COMMAND                     */
#define SUPPORT_AT_NUM                      121 + 6


/*         test AT command lenth  */
#define count               20

/* MUX结构体定义 */
/* parameter information */
typedef struct {
    unsigned char       ucDlci;                 /* DLCI                                     */
    unsigned char       ucI;                    /* the type of frames                       */
    unsigned char       ucCl;                   /* the type of convergence layer            */
    unsigned char       ucPriority;             /* priority                                 */
    unsigned char       ucT1;                   /* the value of the acknowledgment timer    */
    unsigned char       ucN2;                   /* the maximum number of retransmissions    */
    unsigned char       ucT2;                   /* the value of the response timer          */
    unsigned short      usN1;                   /* the maximum frame size                   */
} ST_PARA_INFO;

typedef struct {
    unsigned char       ucFlowId;               /* MUX FLOW ID                              */
    unsigned char       ucStatus;               /* MUX STATUS                               */
    unsigned char       ucDlciFlowOver;         /* MUX DLCI DLC未建立前，准备操作的DLCI     */
} ST_DLC0_MNG;

typedef struct {
    unsigned short      usLen;                  /* 缓存的纯数据长度 */
    unsigned char       *pData;                 /* 纯数据,或保存的消息(参照协议5.4.6.1 Message format) */
} ST_URAT_MSG;

typedef struct {
    unsigned short      usBufferCnt;
    ST_URAT_MSG         Buffer[MUX_MAX_CNT_OF_BUFFER];/* 准备发送给UART的缓存数据          */
} ST_MUX_BUFFER;

typedef struct {
    unsigned char       ucFlowId;               /* MUX FLOW ID                             */
    unsigned char       ucStatus;               /* MUX STATUS                              */
    unsigned char       ucFc;                   /* 所有通道是否可以发数据                  */
    unsigned char       ucRtrFc;                /* 当前对端通道是否可以收数据              */
    unsigned char       ucPppFc;                /* MUX FLG STATUS                          */ 
    unsigned char       ucMode;                 /* MUX MODE                                */
    unsigned char       ucPppStatus;            /* MUX PPP STATUS                          */ 
    unsigned char       ucService;              /* Dlc service type                        */
    //unsigned char       ucCr;                 /* 区分不同路PPP数据                       */
    ST_MUX_BUFFER        AtToUartList;          /* 缓存AT->URAT的数据                      */ 
    ST_MUX_BUFFER        PppToUartData;         /* 缓存PPP->URAT的数据                     */
    ST_PARA_INFO        ParaInfo;               /* parameter information                   */
} ST_DLC_MNG;

/* DLC管理结构体 */
typedef struct {
    unsigned char       ucDlciIndex;            /* Current DLCI' index,DLCI - 1             */
    ST_DLC0_MNG         Dlc0;                   /* Control DLC management information       */
    ST_DLC_MNG          Dlc[MUX_DLC_NUM];       /* AT/Data/Voice DLC管理, ucDlciIndex检索   */
} ST_MUX_MNG;

typedef struct {
    unsigned short      usBuffLen;              /* max length of usb data buffer = 2 * max N1 of all dlc    */
    unsigned short      usRcvDataLen;           /* the length of data received from USB     */
    unsigned char       *pRcvData;              /* store data received from USB             */
    unsigned short      usFrameDataLen;         /* the length of current frame              */
    unsigned char       *pFrameData;            /* store a whole frame                      */
} ST_FRAME_BUFF;

typedef struct {
    unsigned short      usLen;
    unsigned char       *pData;                 /* 发送消息时保存，数据可以随机产生         */
} ST_TEST_INFO;

typedef struct {
    unsigned short      usLen;
    unsigned char       *pData;                 /* 纯数据,或保存的消息(参照协议5.4.6.1 Message format) */
} ST_UIH_INFO;

typedef struct {
    unsigned char       ucDlci;                 /* Address中的DLCI                          */
    unsigned char       ucV24;                  /* MSC V.24 signals                         */
    unsigned char       ucInvalidType;          /* invalid type of UIH frame                */
    unsigned short      usLength;               /* Basic Option中的Length Indicator记录,做响应时用,UihInfo.usLen == usLength >> 1 */
    ST_TEST_INFO        TestInfo;               /* Test command information用于响应消息时对比 */
    ST_UIH_INFO         UihInfo;                /* UIH data information Command             */
} ST_FRAME_INFO;

typedef struct {
    ST_URAT_MSG         MsgRcv;                 /* message recieved 从UART收的message       */
    ST_FRAME_BUFF       FrameBuff;              /* frame buffer                             */
    ST_FRAME_INFO       FrameInfo;              /* information get from frame               */
} ST_MUX_FRAME_MNG;

typedef struct {
    unsigned short      usLen;
    unsigned char       pData[MUX_MAX_CNT_OF_BUFFER];                 /* 纯数据,或保存的消息(参照协议5.4.6.1 Message format) */
}ST_RETRANSMIT_BUFF;

typedef struct {
    unsigned char *pData;
}ST_ATSYMBAL;

typedef struct {
    unsigned short      usLen;                      /* 缓存已收到AT数据长度 */
    char                UartBuff[MUX_UART_CNT_OF_BUFFER];/* 复用未启动之前缓存AT */
} ST_URAT_BUFF;

static ST_MUX_MNG           g_MuxDlcMng;
static ST_MUX_FRAME_MNG     g_MuxFrameMng;
static ST_RETRANSMIT_BUFF   g_MuxRetranBuffer;
static ST_URAT_BUFF         g_UartBuffer;

/* add for add <cr> */
static char g_ADD_CR = 0;



/* MUX函数声明 */
static void MuxInit(void);                                                  //初始化函数
// static void MuxUartmain(void);                                              //接受Uart数据处理函数
static void MuxRilMain(char *pBuffer, unsigned short usBufferLen);          //接受Ril数据处理函数
// static unsigned char GetFrameFromDataStream(void);                          //获取帧函数
// static unsigned char FrameValidityCheckAndParse(void);                      //判断帧的有效性并解析帧结构
// static void FlowProcessfuncs(void);                                         //事件处理CELL函数
static void UihFrameCreate(char *pBuffer, unsigned short usBufferLen);      //数据Uih帧做成
static unsigned char UihFrameStateHandle(char *pBuffer, unsigned short usBufferLen);
// static unsigned char MuxUihParse(unsigned char  ucType);                    //Uih帧的解析
static void MuxDlcDataBuffer(char *pBuffer, unsigned short usBufferLen);
static unsigned char SenderCreateFcs(unsigned char, unsigned char *);
static unsigned char ReceiverCheckFcs(unsigned char, unsigned char, unsigned char *);
// static void SendUihTestCommond(void);                                       //发送TEST命令
// static void SendUihServiceNegCommond(void);                                 //发送服务协商命令
// static void SendUihFcONCommond(void);                                       //发送整体流控
// static void SendSABMFrame(void);                                            //发送通道建立请求
// static void SendUihRTRONCommond(void);                                      //发送RTR流控命令
// static void SendUihRTRONResponse(void);                                     //发送RTR流控响应
// static void SendUihRTROFFResponse(void);                                    //发送RTR关闭相应
// static void SendDatatoRil(void);                                            //将串口收上来的Data进行分发。
// static void MuxSendUihNscResponse(void);                                    //发送不支持UIH消息类型的响应
static char AtClassify(char *pUartRcvBuff, unsigned short usUartLen);

/* 接口函数 */
static void AtMuxStartReq(void);
static void AtMuxDataReq(char *pAtcStrBuff, unsigned short usAtcStrLen);
//static void AtMuxDataInd(char *pAtcStrBuff, unsigned short usAtcStrLen);
//static void PppMuxDataReq(char *pPppStrBuff, unsigned short usPppStrLen);
//static void PppMuxDataInd(char *pPppStrBuff, unsigned short usPppStrLen);
static void UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen);
//static void UartMuxDataInd(char *pUartRcvBuff, unsigned short usUartLen);

/* 公共封装函数 */
static void MuxMemCopy(char *pDest, char *pSour, unsigned long ulen);
static void MuxMemSet(char *pDest, char val, unsigned long ulen);
static void *MuxMalloc(unsigned long ulen);
static void MuxFree(char *pStr);
static char MuxStrncmp(char *pStr1, char *pStr2, unsigned short usLen);
static void MuxTrace(unsigned char  ucOutLevel,const char *pucOutString, ...);
static void MuxErrorLog(const char *pucOutString, ...);
static char *MuxHex2Char(char *, unsigned short);

/* ATC检查使用 */
static const ST_ATSYMBAL AT_TABLE[SUPPORT_AT_NUM] = {
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

/*  AT test array */

static char *Testdata[count]={
  "AT\r\n",
  "AT+CGCLASS=\"A\"\r\n",
  "AT+CFUN=1\r\n",
  "ATE1\r\n",
  "AT+CMEE=1\r\n",
  "AT+CREG=1\r\n",
  "AT+CGEREP=1\r\n",
  "AT+CGREG=1\r\n",
  "AT^DSCI=1\r\n",
  "AT+CNMI=2,2,2,1,0\r\n",
  "AT+COPS=3,0;+COPS?;+COPS=3,1;+COPS?;+COPS=3,2;+COPS?\r\n",
  "AT+CGREG?\r\n",
  "AT+CREG?\r\n",
  "AT+COPS?\r\n",
  "AT^DEPCD=1\r\n",
  "AT+CGDCONT=1,\"IP\",\"cmnet\",,0,0\r\n",
  "AT^DAUTH=1,\"cmnet\",\"cmnet\"\r\n",
  "AT+CGEQREQ=1,3,0,0\r\n",
  "AT+CMMS=1\r\n",
  "AT+CMGS=31\r\n"
};  


/* FCS使用 */
static const unsigned char  crctable[256] = {    //reversed, 8-bit, poly=0x07
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


static void MuxMemCopy(char *pDest, char *pSour, unsigned long ulen)
{
    memcpy(pDest, pSour, ulen);
}

static void MuxMemSet(char *pDest, char val, unsigned long ulen)
{
    if(NULL == pDest)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "Memory Set Error");
    }

    memset(pDest, val, ulen);
}

static void *MuxMalloc(unsigned long ulen)
{
    void *pTmp;

    pTmp = malloc(ulen);

    if(NULL == pTmp)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "Memory malloc Error");
    }

    memset(pTmp, 0, ulen);

    return pTmp;
}

static void MuxFree(char *pStr)
{
    if(NULL != pStr)
    {
        free(pStr);
    }
}

static char MuxStrncmp(char *pStr1, char *pStr2, unsigned short usLen)
{
    if((NULL == pStr1) || (NULL == pStr2))
    {
        MuxTrace(MUX_LOG_LEVEL_1, "String Strncmp Error");
    }
    return strncmp(pStr1, pStr2, usLen);
}

static char* MuxStrStr(char *pStr1, char *pStr2)
{
    if((NULL == pStr1) || (NULL == pStr2))
    {
        MuxTrace(MUX_LOG_LEVEL_1, "String search Error");
    }

    for(;;)
    {
        if(0x00 == *pStr1)
        {
            pStr1++;
        }
        else
        {
            break;
        }
    }
    return strstr(pStr1, pStr2);
}

static char* MuxStrChr(char *pStr1, char cValue)
{
    if((NULL == pStr1))
    {
        MuxTrace(MUX_LOG_LEVEL_1, "String search Error");
    }
    
    for(;;)
    {
        if(0x00 == *pStr1)
        {
            pStr1++;
        }
        else
        {
            break;
        }
    }
    
    return strchr(pStr1, cValue);
}

static char *MuxHex2Char(char *pString, unsigned short len)
{
    char *pTmpData = NULL;
    unsigned short i = 0;
    char cHex = 0;

    if (NULL == pString)
    {
        return NULL;
    }
    pTmpData = (char *)MuxMalloc(len * 2 + 1);
    for (i = 0; i < len * 2; i++)
    {
        if (i % 2 == 0)
        {
            cHex = (pString[i / 2] & 0xF0) >> 4;
            cHex = cHex & 0x0F;
        }
        else
        {
            cHex = pString[i / 2] & 0x0F;
        }

        if (cHex <= 0x09)
        {
            cHex = cHex + 0x30;
        }
        else if ((cHex <= 0x0F) && (cHex >= 0x0A))
        {
            cHex = cHex + 0x37;
        }

        pTmpData[i] = cHex;
    }

    return pTmpData;
}

static void MuxTrace(unsigned char ucOutLevel,const char *pucOutString, ...)
{
    va_list             argument_ptr;
    char                cOutData[4096] = {0};
    char                cTime[4096] = {0};
    char                cTimems[20] = {0};
  //  char                tstr[60];

    time_t              tCurrTime;
    struct tm           *Now;
//    struct timeval      tms;    <sys/time.h>
	
    
    /* 输出trace的范围 */
    if (LogLevel < ucOutLevel)
    {
        return;
    }

    /* Copy Trace string */
    va_start(argument_ptr,pucOutString);
    vsprintf(cOutData,pucOutString,argument_ptr);
    va_end(argument_ptr);

#ifndef _MUX_LOG_BY_ITSELF
    /* 调用RIL中的对应输出 */
    if (MUX_LOG_LEVEL_1 == ucOutLevel)
    {
//        RLOGE("%s\n", cOutData);
    }
    else if (MUX_LOG_LEVEL_2 == ucOutLevel)
    {
//        RLOGI("%s\n", cOutData);
    }
    else
    {
//        RLOGD("%s\n", cOutData);
    }

#else
    /* 定位到文件末尾 */
    fseek(pLogFile, 0, SEEK_END);   
    
    /* trace前用当前系统时间命名 */
    tCurrTime = time(NULL);
    Now = localtime(&tCurrTime);
	
//	gettimeofday(&tms, NULL);
//	strftime(tstr, 60, "%X", localtime(&tms.tv_sec));
//
//    sprintf(cTimems,"%s.%d   ", tstr, tms.tv_usec/1000);
//    strftime( cTime, 128, "%Y-%m-%d  %H:%M:%S_", Now);
//	strftime( cTime, 128, "%Y-%m-%d ", Now);	

    strcat(cOutData, "\r\n" );
	strcat(cTime, cTimems);
    strcat( cTime, cOutData);
    fwrite(cTime, strlen(cTime), 1, pLogFile);
#endif

    return;
}

static void MuxErrorLog(const char *pucOutString, ...)
{
    va_list             argument_ptr;
    char                cOutData[4096] = {0};
    char                cOutData1[4096] = {0};

    /* Copy Trace string */
    va_start(argument_ptr,pucOutString);
    vsprintf(cOutData,pucOutString,argument_ptr);
    va_end(argument_ptr);

#ifndef _MUX_LOG_BY_ITSELF
    //TBD

#else
    /* 定位到文件末尾 */
 //   fseek(pLogFile, 0, SEEK_END);
    sprintf(cOutData1, ", errno = %d, %s\n", errno, strerror(errno));
    strcat(cOutData, cOutData1 );
    fwrite(cOutData, strlen(cOutData), 1, pLogFile);
#endif

    return;
}




static char AtClassify(char *pUartRcvBuff, unsigned short usUartLen)
{
    char cRet = 0;
    unsigned char i;
    unsigned char ucSynLength = 0;
    char *pTmpData = NULL;
    char *pCrPosition = NULL;
  //  char *AtResponse[20] = {0};

    MuxTrace(MUX_LOG_LEVEL_2, "Function AtClassify");
    //MuxMemCopy(AtResponse,"\r\nOK\r\n",6);
    pTmpData = (char *)MuxMalloc(usUartLen + 1);
    MuxMemCopy(pTmpData, pUartRcvBuff, usUartLen);

    if(usUartLen <= 2)
    {
        cRet = MUX_DLC0;
        return cRet;
    }
    
    /* add for \r before AT command */
    for(i = 0;; i++)
    {
        if('\r' == *pTmpData || '\n' == *pTmpData)
        {
            pTmpData = pTmpData + 1;
        }
        else
        {
            if(0 != i)
            {
                MuxTrace(MUX_LOG_LEVEL_2, "Function AtClassify: discard <cr>");
                MuxMemSet(pUartRcvBuff, 0, usUartLen);
                MuxMemCopy(pUartRcvBuff, pTmpData, usUartLen - i);
            }
            
            if(0x0D == pUartRcvBuff[usUartLen - i - 1])
            {
                g_ADD_CR = 0;
            }
            else
            {
                pUartRcvBuff[usUartLen - i] = 0x0D;
                g_ADD_CR = 1;
            }
            MuxTrace(MUX_LOG_LEVEL_2, "g_ADD_CR = %d", g_ADD_CR);
            break;
        }
    }
    pTmpData = pTmpData + 2;                                   /* 指针移动到“AT”之后的位置 */
    pCrPosition = MuxStrChr(pTmpData, 0x0D);                   /* 找到CR                  */

    for(i = 0; i < SUPPORT_AT_NUM; i++)
    {
        /* 找到相应的AT命令 */
        ucSynLength =(unsigned char)strlen(AT_TABLE[i].pData);
        if (pCrPosition - pTmpData >= ucSynLength)
        {
            if(0 == MuxStrncmp(pTmpData, AT_TABLE[i].pData, ucSynLength))
            {
                break;
            }
        }
    }

    if (i >= 0 && i <= 50)
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT command is general AT");
        cRet = MUX_DLC_AT_1;                                            /* for general AT include SS AT */
    }
    else if (i >= 51 && i <= 83)
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT command is voice call AT");
        cRet = MUX_DLC_AT_2;                                            /* for voice call AT           */
    }
    else if (i >= 84 && i <= 106)
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT command is SMS or STK AT");
        cRet = MUX_DLC_AT_3;                                            /* for SMS & STK AT            */
    }

    else if (i >= 107 && i <= 122)
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT command is PDP AT");
        cRet = MUX_DLC_DATA_1;                                           /* for PDP                */

    }
	else if(i >= 123 && i <= SUPPORT_AT_NUM - 1)
	{
		MuxTrace(MUX_LOG_LEVEL_2, "AT command is PDP AT");
        cRet = MUX_DLC_DATA_2;
	}
    else
    {
        if(0x1A == pTmpData[usUartLen - 1 - 2] ||
           0x1B == pTmpData[usUartLen - 1 - 2])
        {
            // This is SMS PDU
            MuxTrace(MUX_LOG_LEVEL_2, "SMS PDU");
            cRet = MUX_DLC_AT_3;
        }  
        else
        {
            MuxTrace(MUX_LOG_LEVEL_2, "UNKONWN AT COMMAND!");
            //cRet = MUX_DLC0;
            cRet = MUX_DLC_AT_1;
        }
    }
    MuxFree(pTmpData);
    return cRet;
}

//static void AtMuxStartReq()
//{
//    /* AT+CMUX=0,0,5,500 */
//    unsigned char i;
//    char *pAtCmux = NULL;
//
//    MuxTrace(MUX_LOG_LEVEL_3, "Function AtMuxStartReq");
//
//    pAtCmux = (char *)MuxMalloc(18 + 1);                                         /* At commond length          */
//    MuxMemCopy(pAtCmux, "AT+CMUX=0,0,5,1600\r", 19);
//    for (i = 0; i < MUX_DLC_NUM; i++)
//    {
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci = i + 1;                          /* 通道号                      */
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucT1 = 10;                               /* default value is 10 (100ms) */
//        g_MuxDlcMng.Dlc[i].ParaInfo.usN1 = 1600;                             /* maximum frame size 1600B     */
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucT2 = 30;                               /* default value is 30 (300ms) */
//        g_MuxDlcMng.Dlc[i].ParaInfo.ucN2 = MUX_MAX_NUM_OF_RETRANS;           /* maximum number of re-transmissions(default value)*/
//    }
//
//    g_MuxFrameMng.FrameBuff.usBuffLen = 1600 * 2;
//    g_MuxFrameMng.FrameBuff.pRcvData = MuxMalloc(g_MuxFrameMng.FrameBuff.usBuffLen);
//
//    g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_START_UP;
//
//    MuxTrace(MUX_LOG_LEVEL_2, "Mux start: AT+CMUX=0,0,5,1600\r");
//    UartMuxDataReq(pAtCmux, 18 + 1);
//    MuxFree(pAtCmux);
//    return;
//}

static void AtMuxDataReq(char *pAtcStrBuff, unsigned short usAtcStrLen)
{
    unsigned char  ucDlci = 0;
  //  int i;

    MuxTrace(MUX_LOG_LEVEL_3, "Function AtMuxDataReq");
    if(0x1A != pAtcStrBuff[usAtcStrLen-1])
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT data  ---->  %s", pAtcStrBuff);
    }

    /* 判断MUX是否开启 */
    if ((MUX_STATUS_CLOSED_DOWN == g_MuxDlcMng.Dlc0.ucStatus) || (MUX_STATUS_CONNECT_WAIT == g_MuxDlcMng.Dlc0.ucStatus)) 
    {
        /* 开机流程是否结束 */
        #if 0
        if(NULL != MuxStrStr(pAtcStrBuff, "AT\r"))
        {
            pAtcStrBuff[usAtcStrLen] = '\n';
            usAtcStrLen +=1;
        }
        #endif

        if(NULL != MuxStrStr(pAtcStrBuff, "AT+CNMI="))
        {
            g_MuxDlcMng.Dlc0.ucStatus = MUX_STATUS_CONNECT_WAIT;
        }

        if (MUX_FLOW_START_UP == g_MuxDlcMng.Dlc0.ucFlowId)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "MUX is not start finished");
            MuxDlcDataBuffer(pAtcStrBuff, usAtcStrLen);
        }
        else
        {
            MuxTrace(MUX_LOG_LEVEL_1, "MUX is not start");
            UartMuxDataReq(pAtcStrBuff, usAtcStrLen);
        }
        return;
    }

    ucDlci = AtClassify(pAtcStrBuff, usAtcStrLen);
    if (MUX_DLC0 == ucDlci)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "AT command type is erro, discard this command!");
        return;
    }
    else
    {
   //     pthread_mutex_lock(&MultiSend);
        g_MuxDlcMng.ucDlciIndex = ucDlci - 1;
        MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d receive data from AT", ucDlci);
        g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId = MUX_FLOW_RECEIVE_DATA_FROM_ATC;
        MuxRilMain(pAtcStrBuff, usAtcStrLen + g_ADD_CR);
   //     pthread_mutex_unlock(&MultiSend);
    }

    return;
}

static void UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen)
{
//    unsigned short  SendLen = 0;
    unsigned short  SendOffset = 0;
//    int i;
    
    MuxTrace(MUX_LOG_LEVEL_3, "Function UartMuxDataReq");
  //  pthread_mutex_lock(&UartSendMutex);
    MuxTrace(MUX_LOG_LEVEL_3, ">>>>>>>>>lock uart port");
    while(usUartLen > 0)
    {
        //SendLen = write(UartFd, pUartSndBuff + SendOffset, usUartLen);  //fasong shuju
        sendStr(1, pUartSndBuff + SendOffset, usUartLen);
//        SendOffset  += SendLen;
//        usUartLen -= SendLen;
//        if(0 == SendLen)
//        {
//            MuxTrace(MUX_LOG_LEVEL_1, "Send data to UART error, length = 0");
//            break;
//        }
    }

    MuxTrace(MUX_LOG_LEVEL_3,"Send lenth is %d", SendOffset);
    
    MuxTrace(MUX_LOG_LEVEL_3, "<<<<<<<<<unlock uart port");
    //pthread_mutex_unlock(&UartSendMutex);

    return;
}

static void MuxInit()
{
    unsigned char  i;

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxInit");

    /* 初始化全局量 */
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

static void MuxRilMain(char *pBuffer, unsigned short usBufferLen)
{
    unsigned char ucRet = 0;            //返回操作结果

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxRilMain");

    /* DLC状态如果不符合发送要求，缓存数据 */
    ucRet = UihFrameStateHandle(pBuffer, usBufferLen);

    /* 能立即发送数据 */
    if(MUX_TRUE == ucRet)
    {
        /* 做成FRAME，并发送 */
        UihFrameCreate(pBuffer, usBufferLen);
    }

    return;
}//added by zdh 向哪个方向发

static unsigned char ReceiverCheckFcs(unsigned char ucFcs, unsigned char len, unsigned char *p)
{
#ifndef _MUX_FCS_CHECK_OFF
    /* Init */
    unsigned char ucCalFcs=0xFF;

    MuxTrace(MUX_LOG_LEVEL_3, "Function ReceiverCheckFcs");

    /* len is the number of bytes in the message, p points to message */
    while (len--)
    {
        ucCalFcs = crctable[ucCalFcs^*p++];
    }

    /* Ones complement */
    ucCalFcs = crctable[ucCalFcs^ucFcs];

    /* 0xCF is the reversed order of 11110011. */
    if (ucCalFcs == 0xCF)
    {
        return (MUX_TRUE); /* FCS is OK */
    }
    else
    {
        return (MUX_FALSE);/* FCS is not OK */
    }
#else
    return (MUX_TRUE); /* FCS is OK */
#endif
}

static unsigned char SenderCreateFcs(unsigned char len, unsigned char *p)
{
    /* Init */
    unsigned char ucCalFcs=0xFF;

    MuxTrace(MUX_LOG_LEVEL_3, "Function SenderCreateFcs");

    /* len is the number of bytes in the message, p points to message */
    while (len--)
    {
        ucCalFcs = crctable[ucCalFcs^*p++];
    }
    /* Ones complement */
    ucCalFcs = 0xFF - ucCalFcs;

    return (ucCalFcs);
}


static void UihFrameCreate(char *pBuffer, unsigned short usBufferLen)
{
    unsigned short usFrameLen = 0;                      /* 发送帧长度 */
    unsigned char  *pDataFrame = NULL;                  /* 帧的数据 */
    unsigned char  ucFcs = 0;                           /* 校验位 */
    unsigned char  i = 0;  
    unsigned char  n = 0;                               /* 发送帧的个数 */
    unsigned char  *pUihData = NULL;

    MuxTrace(MUX_LOG_LEVEL_3, "Function UihFrameCreate");

    if ((0 == usBufferLen) || (NULL == pBuffer))
    {/* 收到的数据为空 */
        MuxTrace(MUX_LOG_LEVEL_1, "data of pBuffer is empty Or usBufferLen is 0");
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
        /* Frame length(Length Indicator 2 Oct) without Information */
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
                pUihData   = &pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 6)];
            }
            else
            {
                usBufferLen   = usBufferLen - (usFrameLen - 7);
                pUihData   = &pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7)];
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
                    pUihData   = &pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 6)];
                }
                else
                {
                    //usFrameLen = usBufferLen + 7;
                    pUihData   = &pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7)];
                }         
            }
            else
            {
                usFrameLen = usBufferLen + 7;
                pUihData   = &pBuffer[i * (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.usN1 - 7)];
            }
        }
        pDataFrame =(unsigned char *)MuxMalloc((unsigned long)usFrameLen);
        /* 做成FRAME */
        pDataFrame[0]  = MUX_FLAG_SEQUENCE_FIELD;                                            /* Flag Sequence Field(0xF9) */
        pDataFrame[1]  = 0x01 | (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci << 2);/* Address Field (EA=1 CR=0 DLCI = g_MuxDlcMng.ucDlciIndex) */
        pDataFrame[2]  = MUX_UIH_FRAME;                                                      /* Control Field (UIH Frame F=0) */
        /* 1byte Length Indicator */
        if (usFrameLen <= (MUX_MAX_LENGTH_INDICATOR + 6))
        {
            pDataFrame[3]  = 0x01 | ((usFrameLen - 6) << 1);           /* Length Field (EA=1) */
            ucFcs          = SenderCreateFcs(3, &pDataFrame[1]);
            MuxMemCopy(&pDataFrame[4], pUihData, (usFrameLen - 6));
        }
        /* 2byte Length Indicator */
        else
        {
            pDataFrame[3]  = ((usFrameLen - 7) << 1) & 0xFE;            /* Length Field (EA=0) */
            pDataFrame[4]  = (usFrameLen - 7) >> 7;
            ucFcs          = SenderCreateFcs(4, &pDataFrame[1]);
            MuxMemCopy(&pDataFrame[5], pUihData, (usFrameLen - 7));
        }
        pDataFrame[usFrameLen - 2] = ucFcs;                                                   /* FCS */
        pDataFrame[usFrameLen - 1] = MUX_FLAG_SEQUENCE_FIELD;                                 /* Flag Sequence Field(0xF9) */
        /* 发送 */
        MuxTrace( MUX_LOG_LEVEL_2, "send a Frame whose length is %d to USB", usFrameLen);
        UartMuxDataReq(pDataFrame, usFrameLen);
        MuxFree((char*)pDataFrame);
    }

    return;
}

static unsigned char UihFrameStateHandle(char *pBuffer, unsigned short usBufferLen)
{
    MuxTrace(MUX_LOG_LEVEL_3, "Function UihFrameStateHandle");

    if ((MUX_STATUS_CONNECTED == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucStatus)
        && (MUX_ON == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFc)
        && (MUX_ON == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucRtrFc))
    {
        /* AT数据传输 */
        if (MUX_FLOW_RECEIVE_DATA_FROM_ATC == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
        {
            if (ONLINE_DATA_MODE == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucMode)        /* 流控没开缓存 */
            {
                MuxDlcDataBuffer(pBuffer, usBufferLen);
                return MUX_FALSE;
            }
            else
            {
                return MUX_TRUE;
            }
        }
        /* PPP数据传输 */
        else if (MUX_FLOW_RECEIVE_DATA_FROM_PPP == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
        {

           return MUX_TRUE;
    
        }
        else
        {
            ;
        }
    }
    else                                                                                    /* 流控没开缓存 */
    {
        MuxDlcDataBuffer(pBuffer, usBufferLen);
        return MUX_FALSE;
    }
    return MUX_TRUE;
}

static void MuxDlcDataBuffer(char *pBuffer, unsigned short usBufferLen)
{
    ST_MUX_BUFFER   *AtDataBuffer;
    ST_MUX_BUFFER   *PppDataBuffer;

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxDlcDataBuffer");

    AtDataBuffer = &g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].AtToUartList;
    PppDataBuffer = &g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].PppToUartData;

    if (MUX_FLOW_RECEIVE_DATA_FROM_ATC == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
    {
        if (MUX_MAX_CNT_OF_BUFFER == AtDataBuffer->usBufferCnt)     /* 判断缓存区是否已满 */
        {
            MuxTrace(MUX_LOG_LEVEL_1, "the buffer of AT full, discard this data!");
            return;/* 这里怎么操作 */
        }
        /* AT数据存入缓存区 */
        else
        {
            AtDataBuffer->Buffer[AtDataBuffer->usBufferCnt].usLen = usBufferLen;
            AtDataBuffer->Buffer[AtDataBuffer->usBufferCnt].pData = (unsigned char *)MuxMalloc(usBufferLen);
            MuxMemCopy(AtDataBuffer->Buffer[AtDataBuffer->usBufferCnt].pData, pBuffer, usBufferLen);
            AtDataBuffer->usBufferCnt++;
        }
    }
    else if (MUX_FLOW_RECEIVE_DATA_FROM_PPP == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
    {
        if (MUX_MAX_CNT_OF_BUFFER == PppDataBuffer->usBufferCnt)    /* 判断缓存区是否已满 */
        {
            MuxTrace(MUX_LOG_LEVEL_1, "the buffer of PPP full, discard this data!");
            return;/* 这里怎么操作 */
        }
        /* PPP数据存入缓存区 */
        else
        {
            PppDataBuffer->Buffer[PppDataBuffer->usBufferCnt].usLen = usBufferLen;
            PppDataBuffer->Buffer[PppDataBuffer->usBufferCnt].pData = (unsigned char *)MuxMalloc(usBufferLen);
            MuxMemCopy(PppDataBuffer->Buffer[PppDataBuffer->usBufferCnt].pData, pBuffer, usBufferLen);
            PppDataBuffer->usBufferCnt++;
        }
    }
    else
    {
        ;
    }
    return;
}





static void ReadRilLoop (unsigned short usBufferLen)
{
    int     nRead = 0;
 //  void*   pData = NULL;
//    int     i;
    char *temp =NULL;
    MuxTrace(MUX_LOG_LEVEL_1, "Thread ReadRilLoop is running");
    
 
        //nRead = read(RilFd_Master, RilReadBuf, MAX_RIL_MSG_LEN);//shuru ATmingying yu shuju
//        sendStr(1,"AT\r\n");
        
//        
//        
//        if(nRead > 0)
//        {
//            MuxTrace(MUX_LOG_LEVEL_2, "nRead = %d", nRead);
//            RilReadBuf[nRead] = 0;
//            AtMuxDataReq(RilReadBuf, nRead);    
//        }
//         else
//        {
//            MuxErrorLog("In ReadRilLoop read error");
//            sleep(2);
    for(int i=0; i<usBufferLen; i++)
    {
      nRead =strlen(Testdata[i]);
        if(0==nRead)
          break;
     
      temp=(char *)malloc(nRead +1);
      
      MuxMemCopy(temp,Testdata[i], nRead);
      
    
      AtMuxDataReq(temp, nRead);     
      
      delayn(10000000);
           
        
    }  
    return;
}




//static void signal_treatment(	int param)
//{
//	switch (param)
//	{
//	case SIGINT:
//        MuxTrace(MUX_LOG_LEVEL_1, "Signal SIGINT received");
//        main_exit_signal = 1;
//	    break;
//	case SIGTERM:
//        MuxTrace(MUX_LOG_LEVEL_1, "Signal SIGTERM received");
//        main_exit_signal = 1;
//	    break;
//	default:
//    	break;
//	}
//    return;
//}
//
//static void CloseDown()
//{
//    
//#ifdef _MUX_LOG_BY_ITSELF
//    fclose(pLogFile);
//#endif
//    //TBD;
//    //Close MUX, all devices, etc.
//}

int main(int argc,char *argv[])
{
    

  // char*       pRilSlaveName = NULL;
   MuxTrace(MUX_LOG_LEVEL_1, "MUX version 2015.12.01 17:50", RilFd_Master);

   


    //Creat pseudo-terminal for RIL
//    if(0 > (RilFd_Master = open(PTMASTER, O_RDWR)))   // zdh  shebei dakai
//    {
//        MuxErrorLog("open /dev/ptmx error");
//        perror("open /dev/ptmx error");
//        exit(EXIT_FAILURE);
//    }
// 

     MuxInit();    //MUX init   //zdh  bixu xian zhixing 
     BSP_Init();


    ReadRilLoop(count);
    
    
    
    while(main_exit_signal == 0)
    {
    	//sleep(2);
        delayn(10000000);
    }

    MuxTrace(MUX_LOG_LEVEL_1, "MUX main exit");

//    CloseDown();
//   
    return 0;
}

 

 




//void main()
//{
//     BSP_Init();
//     while(1)
//     {
//     sendStr(1,"AT\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CGCLASS=\"A\"\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CFUN=1\r\n");
//     delayn(10000000);
//     sendStr(1,"ATE1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CMEE=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CREG=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CGEREP=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CGREG=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT^DSCI=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CNMI=2,2,2,1,0\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+COPS=3,0;+COPS?;+COPS=3,1;+COPS?;+COPS=3,2;+COPS?\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CGREG?\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CREG?\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+COPS?\r\n");
//     delayn(10000000);
//     sendStr(1,"AT^DEPCD=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CGDCONT=1,\"IP\",\"cmnet\",,0,0\r\n");
//     delayn(10000000);
//     sendStr(1,"AT^DAUTH=1,\"cmnet\",\"cmnet\"\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CGEQREQ=1,3,0,0\r\n");
//     delayn(10000000);
//     //发送接收短信业务
//     sendStr(1,"AT+CMMS=1\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CMGS=27\r\n");
//     delayn(10000000);
//     sendStr(1,"AT+CNMA=1\r\n");
//     delayn(10000000);
//     }
//}
