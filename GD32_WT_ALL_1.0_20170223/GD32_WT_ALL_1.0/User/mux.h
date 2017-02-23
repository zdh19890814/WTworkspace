#ifndef __MUX_H
#define __MUX_H 
//---------------------------------inc-----------------------------
#include "include.h"

//#include "usart1.h"
//#include <hw_config.h>



//#define PTMASTER        "/dev/ptmx"
////#define RC_LINK_FILE    "/dev/rcpts"
//#define RIL_LINK_FILE   "/dev/rilpts"
////#define PPP_LINK_FILE   "/dev/ppppts"
//#define DEVICE_UART     "/dev/ttyS1"



////pthread_t   RcRcvThread;
//pthread_t   RilRcvThread;
////pthread_t   PppRcvThread;
//pthread_t   UartRcvThread;
//
//pthread_mutex_t        UartSendMutex = PTHREAD_MUTEX_INITIALIZER;
//
//pthread_mutex_t        MultiSend = PTHREAD_MUTEX_INITIALIZER;




/* MUX TRACE */
//#define _MUX_LOG_BY_ITSELF
//
//#ifdef _MUX_LOG_BY_ITSELF
//
//#define MUX_LOG_FILE_NAME       "HlMux.log"
////#define MUX_TRACE_OUTPUT_PATH   "/data/lc-elog/"
//#define MUX_TRACE_OUTPUT_PATH   "/data/local/log/"



//static FILE  *pLogFile = NULL;
////static void OpenLogFile();
//
//#endif

#define _MUX_FCS_CHECK_OFF


/* MUX宏定义 */
typedef enum
{
    MUX_LOG_LEVEL_1 = 0,                /* 函数在返回错误时，错误类型打印           */
    MUX_LOG_LEVEL_2,                    /* 从UART或其他接口收到消息时，解析时解析出的重要信息打印; 重要事件开始和结束时 */
    MUX_LOG_LEVEL_3,                    /* 进入、离开函数时                         */
    MUX_LOG_HIGH_LEVEL
} E_MUX_LOG_TYPE;


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
#define MUX_SERV_DATA                       0x03        /* DATA service                     *///即为上网数据
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
#define count               6

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
	ST_DLC_MNG          Dlc[MUX_DLC_NUM];       /* AT/Data/Voice DLC管理, ucDlciIndex检索   *///zl:不同的事件
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



/* add for add <cr> */




/* MUX函数声明 */
void MuxInit(void);                                                  //初始化函数
void MuxUartmain(void);                                              //接受Uart数据处理函数
void MuxRilMain(char *pBuffer, unsigned short usBufferLen);          //接受Ril数据处理函数
 unsigned char GetFrameFromDataStream(void);                          //获取帧函数
 unsigned char FrameValidityCheckAndParse(void);                      //判断帧的有效性并解析帧结构
 void FlowProcessfuncs(void);                                         //事件处理CELL函数
void UihFrameCreate(char *pBuffer, unsigned short usBufferLen);      //数据Uih帧做成
 unsigned char UihFrameStateHandle(char *pBuffer, unsigned short usBufferLen);
 unsigned char MuxUihParse(unsigned char  ucType);                    //Uih帧的解析
 void MuxDlcDataBuffer(char *pBuffer, unsigned short usBufferLen);
unsigned char SenderCreateFcs(unsigned char, unsigned char *);
unsigned char ReceiverCheckFcs(unsigned char, unsigned char, unsigned char *);
 void SendUihTestCommond(void);                                       //发送TEST命令
 void SendUihServiceNegCommond(void);                                 //发送服务协商命令
 void SendUihFcONCommond(void);                                       //发送整体流控
void SendSABMFrame(void);                                            //发送通道建立请求
 void SendUihRTRONCommond(void);                                      //发送RTR流控命令
 void SendUihRTRONResponse(void);                                     //发送RTR流控响应
void SendUihRTROFFResponse(void);                                    //发送RTR关闭相应
void SendDatatoRil(void);                                            //将串口收上来的Data进行分发。
void MuxSendUihNscResponse(void);                                    //发送不支持UIH消息类型的响应
char AtClassify(char *pUartRcvBuff, unsigned short usUartLen);



/* 接口函数 */

 void AtMuxDataReq(char *pAtcStrBuff, unsigned short usAtcStrLen);
 void AtMuxDataInd(char *pAtcStrBuff, unsigned short usAtcStrLen);
void PppMuxDataReq(char *pPppStrBuff, unsigned short usPppStrLen);
 void PppMuxDataInd(char *pPppStrBuff, unsigned short usPppStrLen);
 void UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen);
void UartMuxDataInd(char *pUartRcvBuff, unsigned short usUartLen);

/* 接口函数 */
void AtMuxStartReq(void);
void AtMuxDataReq(char *pAtcStrBuff, unsigned short usAtcStrLen);
void UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen);

void UartMuxDataInd(char *pUartRcvBuff, unsigned short usUartLen);






//  char *Testdata[count]={
//   "AT\r\n",
//   "AT+CGCLASS=\"CC\"\r\n",
//   "AT+CFUN=1\r\n",
//   "ATE1\r\n",
// //  "AT+CMEE=1\r\n",
// //  "AT+CREG=1\r\n",
// //  "AT+CGEREP=1\r\n",
// //  "AT+CGREG=1\r\n",
// //  "AT^DSCI=1\r\n",
//   "AT+CNMI=2,2,2,1,0\r\n",
// //  "AT+COPS=3,0;+COPS?;+COPS=3,1;+COPS?;+COPS=3,2;+COPS?\r\n",
// //  "AT+CGREG?\r\n",
// //  "AT+CREG?\r\n",
// //  "AT+COPS?\r\n",
// //  "AT^DEPCD=1\r\n",
// //  "AT+CGDCONT=1,\"IP\",\"cmnet\",,0,0\r\n",
// //  "AT^DAUTH=1,\"cmnet\",\"cmnet\"\r\n",
// //  "AT+CGEQREQ=1,3,0,0\r\n",
// //  "AT+CMMS=1\r\n",
//   "AT+CMGS=27\r\n"
// };  




/* 公共封装函数 */
 void MuxMemCopy(char *pDest, char *pSour, unsigned long ulen);
 void MuxMemSet(char *pDest, char val, unsigned long ulen);
 void *MuxMalloc(unsigned long ulen);
 void MuxFree(char *pStr);
 char MuxStrncmp(char *pStr1, char *pStr2, unsigned short usLen);
//static void MuxTrace(unsigned char  ucOutLevel,const char *pucOutString, ...);
//static void MuxErrorLog(const char *pucOutString, ...);
 char *MuxHex2Char(char *, unsigned short);



void at_buffer_handler(char *pAtcStrBuff, unsigned short usAtcStrLen);


#endif
