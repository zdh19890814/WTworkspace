//#define _XOPEN_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <pty.h>
#include <sys/types.h>  
#include <sys/stat.h>   
#include <termios.h>
#include <errno.h>
#include <pthread.h>
//#include <sys/stropts.h>
#include <string.h>
#include <stdarg.h> 
//#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

//#define TTL_FOR_NOT_START_EMSD 
// pty master
#define PTMASTER        "/dev/ptmx"
//#define RC_LINK_FILE    "/dev/rcpts"
#define RIL_LINK_FILE   "/dev/rilpts"
//#define PPP_LINK_FILE   "/dev/ppppts"
#define DEVICE_UART     "/dev/ttyS1"

//static int RcFd_Master = 0;
static int RilFd_Master = 0;   //zdh  chuli wenjian duqu
//static int PppFd_Master = 0; 
static int UartFd = 0; 

//pthread_t   RcRcvThread;
pthread_t   RilRcvThread;
//pthread_t   PppRcvThread;
pthread_t   UartRcvThread;

pthread_mutex_t        UartSendMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t        MultiSend = PTHREAD_MUTEX_INITIALIZER;


static int main_exit_signal = 0;

// #define MAX_RC_MSG_LEN     (8 * 1024)
#define MAX_RIL_MSG_LEN     (8 * 1024)
// #define MAX_PPP_MSG_LEN     (8 * 1024)      
#define MAX_UART_MSG_LEN    (8 * 1024)

//static char RcReadBuf[MAX_RC_MSG_LEN];
static char RilReadBuf[MAX_RIL_MSG_LEN];
// static char PppReadBuf[MAX_PPP_MSG_LEN];
static char UartReadBuf[MAX_PPP_MSG_LEN];

/* MUX TRACE */
#define _MUX_LOG_BY_ITSELF

#ifdef _MUX_LOG_BY_ITSELF

#define MUX_LOG_FILE_NAME       "HlMux.log"
//#define MUX_TRACE_OUTPUT_PATH   "/data/lc-elog/"
#define MUX_TRACE_OUTPUT_PATH   "/data/local/log/"



static FILE  *pLogFile = NULL;
static void OpenLogFile();

#endif

#define _MUX_FCS_CHECK_OFF


/* MUX�궨�� */
typedef enum
{
    MUX_LOG_LEVEL_1 = 0,                /* �����ڷ��ش���ʱ���������ʹ�ӡ           */
    MUX_LOG_LEVEL_2,                    /* ��UART�������ӿ��յ���Ϣʱ������ʱ����������Ҫ��Ϣ��ӡ; ��Ҫ�¼���ʼ�ͽ���ʱ */
    MUX_LOG_LEVEL_3,                    /* ���롢�뿪����ʱ                         */
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
/* MUX DLC����ʹ�� */                                                                       
#define MUX_FLOW_NONE                      0           /* û���¼���������frameʱ�ȸ���    */

/* MUX DLC0ʹ�� */                                                                          
#define MUX_FLOW_START_UP                  1           /* ����Mux                          */
#define MUX_FLOW_TEST                      2           /* ͨ�Ų���                         */
#define MUX_FLOW_PARAMETER_NEGOTIATION     3           /* DLC����Э��                      */
#define MUX_FLOW_CLOSE_DOWN                4           /* �ر�MUX                          */
#define MUX_FLOW_OVERALL_FC_ON             5           /* ��������ON                       */
#define MUX_FLOW_OVERALL_FC_OFF            6           /* ��������OFF                      */

/* MUX DLC1-DLC8ʹ�� */                                                                     
#define MUX_FLOW_DLC_ESTABLISH             7           /* ����DLC                          */
#define MUX_FLOW_DLC_RELEASE               8           /* �ͷ�DLC                          */
#define MUX_FLOW_SERVICE_NEGOTIATION       9           /* DLC����Э��                      */
#define MUX_FLOW_APL_FC_ON                 10          /* APL����ON                        */
#define MUX_FLOW_APL_FC_OFF                11          /* APL����OFF                       */
#define MUX_FLOW_RECEIVE_FC_ON             13          /* �յ�����ON                       */
#define MUX_FLOW_RECEIVE_FC_OFF            14          /* �յ�����OFF                      */
#define MUX_FLOW_PPP_FC_ON                 15          /* PPP����ON                        */
#define MUX_FLOW_PPP_FC_OFF                16          /* PPP����OFF                       */
#define MUX_FLOW_RECEIVE_DATA_FROM_UART    17          /* UART����֪ͨ                     */
#define MUX_FLOW_RECEIVE_DATA_FROM_ATC     18          /* ATC����֪ͨ                      */
#define MUX_FLOW_RECEIVE_DATA_FROM_PPP     19          /* PPP����֪ͨ                      */

/* MUX STATUS */                                                                            
#define MUX_STATUS_CLOSED_DOWN              0           /* MUX�ر�״̬                      */
#define MUX_STATUS_CONNECT_WAIT             1           /* MUX�ȴ�����״̬                  */
#define MUX_STATUS_DISCONNECTED             2           /* DLC��·δ����                    */
#define MUX_STATUS_CONNECTED                3           /* DLC��·���������Դ�������        */

/* MUX FLG STATUS */                                                                        
#define MUX_ON                              1           /* FLG��λ                          */
#define MUX_OFF                             0           /* FLG��λ                          */

/* MUX RETURN VALUE */                                                                      
#define MUX_FALSE                           0           /* ����ʧ��                         */
#define MUX_TRUE                            1           /* ���سɹ�                         */
#define MUX_CONTINUE                        2           /* ���سɹ�,������һ�ֲ���          */
#define MUX_TYPE_INVALID                    4           /* MUX��Ϣ������Ч                  */

/* MUX FRAME TYPE of Control field */                                                       
#define MUX_SABM_FRAME                      0x2F        /* SABM frame                       */
#define MUX_DISC_FRAME                      0x43        /* DISC frame                       */
#define MUX_UIH_FRAME                       0xEF        /* UIH frame                        */
#define MUX_UA_FRAME                        0x63        /* UA frame                         */
#define MUX_DM_FRAME                        0x0F        /* DM frame                         */

/* MUX VALUE of TYPE in UIH frame */                                                        
#define MUX_UIH_PN                          0x80        /* ����Э��                         */
#define MUX_UIH_CLD                         0xC0        /* �ر�MUX                          */
#define MUX_UIH_TEST                        0x20        /* ͨ�Ų���                         */
#define MUX_UIH_FCon                        0xA0        /* ��������ON                       */
#define MUX_UIH_FCoff                       0x60        /* ��������OFF                      */
#define MUX_UIH_SNC                         0xD0        /* ����Э��                         */
#define MUX_UIH_MSC                         0xE0        /* RTR����                          */

/* Dlc service type */                                                                      
#define MUX_SERV_AT                         0x01        /* AT service                       */
#define MUX_SERV_DATA                       0x03        /* DATA service                     */
#define MUX_SERV_VOICE                      0x05        /* VOICE service                    */

/* MUX INFORMATION OF TIMER */                                                              
#define MUX_DEFAULT_TIME_OF_T1              10          /* 0.1s  ��λ��0.01s                */
#define MUX_DEFAULT_TIME_OF_T2              200         /* 2s(default 0.3s)  ��λ��0.01s    */

/* MUX COMMON INFORMATION */                                                                
#define MUX_MAX_SIZE_OF_FRAME               31          /* FRAME����Ĭ�ϳ��ȣ�31(default)    */
#define MUX_MAX_NUM_OF_RETRANS              3           /* �ش���������3  ��ʼ��ʱ��      */
#define MUX_FLAG_SEQUENCE_FIELD             0xF9        /* frame����ʼ����Flag�̶�ֵ        */
#define MUX_MAX_CNT_OF_BUFFER               (1024 * 4)  /* MUX(UART)���յ���������ݳ���    */
#define MUX_UART_CNT_OF_BUFFER              (1024 * 4)  /* UART���յ���������ݳ���         */

/* MUX PPP STATUS */                                                                        
#define DTX_READY                           0           /* frame����ʼ����Flag�̶�ֵ        */
#define DTX_STOP                            2                                               

/* MUX BUFFER SIZE */                                                                       
#define MAX_PPP_DATA_BUFFER_SIZE            (4096*5)    /* PPP�ܷ��͵�������ݳ���          */
#define MAX_UART_BUFFER_SIZE                (3050)      /* UART�ܷ��͵�������ݳ���         */
#define MAX_UART_RX_MEM_SIZE                (1600)      /* UART���յ���������ݳ���         */

/* MUX MODE */
#define OFFLINE_AT_MODE                     0           /*                                  */
//#define ONLINE_AT_MODE                      1           /*                                */
#define ONLINE_DATA_MODE_ESTABLISHING       2           /*                                  */
#define ONLINE_DATA_MODE                    3           /* ����DLC_AT:����AT���ݣ�����DLC_DATA:����PPP���� */

/* define MAX Of Length Indicator */
#define MUX_MAX_LENGTH_INDICATOR            127         /* EA=1ʱLength Indicator��ָʾ����󳤶� */

/*         AT COMMAND                     */
#define SUPPORT_AT_NUM                      121 + 6

/* MUX�ṹ�嶨�� */
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
    unsigned char       ucDlciFlowOver;         /* MUX DLCI DLCδ����ǰ��׼��������DLCI     */
} ST_DLC0_MNG;

typedef struct {
    unsigned short      usLen;                  /* ����Ĵ����ݳ��� */
    unsigned char       *pData;                 /* ������,�򱣴����Ϣ(����Э��5.4.6.1 Message format) */
} ST_URAT_MSG;

typedef struct {
    unsigned short      usBufferCnt;
    ST_URAT_MSG         Buffer[MUX_MAX_CNT_OF_BUFFER];/* ׼�����͸�UART�Ļ�������          */
} ST_MUX_BUFFER;

typedef struct {
    unsigned char       ucFlowId;               /* MUX FLOW ID                             */
    unsigned char       ucStatus;               /* MUX STATUS                              */
    unsigned char       ucFc;                   /* ����ͨ���Ƿ���Է�����                  */
    unsigned char       ucRtrFc;                /* ��ǰ�Զ�ͨ���Ƿ����������              */
    unsigned char       ucPppFc;                /* MUX FLG STATUS                          */ 
    unsigned char       ucMode;                 /* MUX MODE                                */
    unsigned char       ucPppStatus;            /* MUX PPP STATUS                          */ 
    unsigned char       ucService;              /* Dlc service type                        */
    //unsigned char       ucCr;                 /* ���ֲ�ͬ·PPP����                       */
    ST_MUX_BUFFER        AtToUartList;          /* ����AT->URAT������                      */ 
    ST_MUX_BUFFER        PppToUartData;         /* ����PPP->URAT������                     */
    ST_PARA_INFO        ParaInfo;               /* parameter information                   */
} ST_DLC_MNG;

/* DLC����ṹ�� */
typedef struct {
    unsigned char       ucDlciIndex;            /* Current DLCI' index,DLCI - 1             */
    ST_DLC0_MNG         Dlc0;                   /* Control DLC management information       */
    ST_DLC_MNG          Dlc[MUX_DLC_NUM];       /* AT/Data/Voice DLC����, ucDlciIndex����   */
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
    unsigned char       *pData;                 /* ������Ϣʱ���棬���ݿ����������         */
} ST_TEST_INFO;

typedef struct {
    unsigned short      usLen;
    unsigned char       *pData;                 /* ������,�򱣴����Ϣ(����Э��5.4.6.1 Message format) */
} ST_UIH_INFO;

typedef struct {
    unsigned char       ucDlci;                 /* Address�е�DLCI                          */
    unsigned char       ucV24;                  /* MSC V.24 signals                         */
    unsigned char       ucInvalidType;          /* invalid type of UIH frame                */
    unsigned short      usLength;               /* Basic Option�е�Length Indicator��¼,����Ӧʱ��,UihInfo.usLen == usLength >> 1 */
    ST_TEST_INFO        TestInfo;               /* Test command information������Ӧ��Ϣʱ�Ա� */
    ST_UIH_INFO         UihInfo;                /* UIH data information Command             */
} ST_FRAME_INFO;

typedef struct {
    ST_URAT_MSG         MsgRcv;                 /* message recieved ��UART�յ�message       */
    ST_FRAME_BUFF       FrameBuff;              /* frame buffer                             */
    ST_FRAME_INFO       FrameInfo;              /* information get from frame               */
} ST_MUX_FRAME_MNG;

typedef struct {
    unsigned short      usLen;
    unsigned char       pData[MUX_MAX_CNT_OF_BUFFER];                 /* ������,�򱣴����Ϣ(����Э��5.4.6.1 Message format) */
}ST_RETRANSMIT_BUFF;

typedef struct {
    unsigned char *pData;
}ST_ATSYMBAL;

typedef struct {
    unsigned short      usLen;                      /* �������յ�AT���ݳ��� */
    char                UartBuff[MUX_UART_CNT_OF_BUFFER];/* ����δ����֮ǰ����AT */
} ST_URAT_BUFF;

static ST_MUX_MNG           g_MuxDlcMng;
static ST_MUX_FRAME_MNG     g_MuxFrameMng;
static ST_RETRANSMIT_BUFF   g_MuxRetranBuffer;
static ST_URAT_BUFF         g_UartBuffer;

/* add for add <cr> */
static char g_ADD_CR = 0;



/* MUX�������� */
static void MuxInit(void);                                                  //��ʼ������
static void MuxUartmain(void);                                              //����Uart���ݴ�����
static void MuxRilMain(char *pBuffer, unsigned short usBufferLen);          //����Ril���ݴ�����
static unsigned char GetFrameFromDataStream(void);                          //��ȡ֡����
static unsigned char FrameValidityCheckAndParse(void);                      //�ж�֡����Ч�Բ�����֡�ṹ
static void FlowProcessfuncs(void);                                         //�¼�����CELL����
static void UihFrameCreate(char *pBuffer, unsigned short usBufferLen);      //����Uih֡����
static unsigned char UihFrameStateHandle(char *pBuffer, unsigned short usBufferLen);
static unsigned char MuxUihParse(unsigned char  ucType);                    //Uih֡�Ľ���
static void MuxDlcDataBuffer(char *pBuffer, unsigned short usBufferLen);
static unsigned char SenderCreateFcs(unsigned char, unsigned char *);
static unsigned char ReceiverCheckFcs(unsigned char, unsigned char, unsigned char *);
static void SendUihTestCommond(void);                                       //����TEST����
static void SendUihServiceNegCommond(void);                                 //���ͷ���Э������
static void SendUihFcONCommond(void);                                       //������������
static void SendSABMFrame(void);                                            //����ͨ����������
static void SendUihRTRONCommond(void);                                      //����RTR��������
static void SendUihRTRONResponse(void);                                     //����RTR������Ӧ
static void SendUihRTROFFResponse(void);                                    //����RTR�ر���Ӧ
static void SendDatatoRil(void);                                            //��������������Data���зַ���
static void MuxSendUihNscResponse(void);                                    //���Ͳ�֧��UIH��Ϣ���͵���Ӧ
static char AtClassify(char *pUartRcvBuff, unsigned short usUartLen);

/* �ӿں��� */
static void AtMuxStartReq(void);
static void AtMuxDataReq(char *pAtcStrBuff, unsigned short usAtcStrLen);
static void AtMuxDataInd(char *pAtcStrBuff, unsigned short usAtcStrLen);
//static void PppMuxDataReq(char *pPppStrBuff, unsigned short usPppStrLen);
//static void PppMuxDataInd(char *pPppStrBuff, unsigned short usPppStrLen);
static void UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen);
static void UartMuxDataInd(char *pUartRcvBuff, unsigned short usUartLen);

/* ������װ���� */
static void MuxMemCopy(char *pDest, char *pSour, unsigned long ulen);
static void MuxMemSet(char *pDest, char val, unsigned long ulen);
static void *MuxMalloc(unsigned long ulen);
static void MuxFree(char *pStr);
static char MuxStrncmp(char *pStr1, char *pStr2, unsigned short usLen);
static void MuxTrace(unsigned char  ucOutLevel,const char *pucOutString, ...);
static void MuxErrorLog(const char *pucOutString, ...);
static char *MuxHex2Char(char *, unsigned short);

/* ATC���ʹ�� */
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

/* FCSʹ�� */
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
	char                tstr[60];

    time_t              tCurrTime;
    struct tm           *Now;
    struct timeval      tms;
	
    
    /* ���trace�ķ�Χ */
    if (LogLevel < ucOutLevel)
    {
        return;
    }

    /* Copy Trace string */
    va_start(argument_ptr,pucOutString);
    vsprintf(cOutData,pucOutString,argument_ptr);
    va_end(argument_ptr);

#ifndef _MUX_LOG_BY_ITSELF
    /* ����RIL�еĶ�Ӧ��� */
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
    /* ��λ���ļ�ĩβ */
    fseek(pLogFile, 0, SEEK_END);   
    
    /* traceǰ�õ�ǰϵͳʱ������ */
    tCurrTime = time(NULL);
    Now = localtime(&tCurrTime);
	
	gettimeofday(&tms, NULL);
	strftime(tstr, 60, "%X", localtime(&tms.tv_sec));

    sprintf(cTimems,"%s.%d   ", tstr, tms.tv_usec/1000);
    //strftime( cTime, 128, "%Y-%m-%d  %H:%M:%S_", Now);
	strftime( cTime, 128, "%Y-%m-%d ", Now);	

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
    /* ��λ���ļ�ĩβ */
    fseek(pLogFile, 0, SEEK_END);
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
    char *AtResponse[20] = {0};

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
    pTmpData = pTmpData + 2;                                   /* ָ���ƶ�����AT��֮���λ�� */
    pCrPosition = MuxStrChr(pTmpData, 0x0D);                   /* �ҵ�CR                  */

    for(i = 0; i < SUPPORT_AT_NUM; i++)
    {
        /* �ҵ���Ӧ��AT���� */
        ucSynLength = strlen(AT_TABLE[i].pData);
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

static void AtMuxStartReq()
{
    /* AT+CMUX=0,0,5,500 */
    unsigned char i;
    char *pAtCmux = NULL;

    MuxTrace(MUX_LOG_LEVEL_3, "Function AtMuxStartReq");

    pAtCmux = (char *)MuxMalloc(18 + 1);                                         /* At commond length          */
    MuxMemCopy(pAtCmux, "AT+CMUX=0,0,5,1600\r", 19);
    for (i = 0; i < MUX_DLC_NUM; i++)
    {
        g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci = i + 1;                          /* ͨ����                      */
        g_MuxDlcMng.Dlc[i].ParaInfo.ucT1 = 10;                               /* default value is 10 (100ms) */
        g_MuxDlcMng.Dlc[i].ParaInfo.usN1 = 1600;                             /* maximum frame size 1600B     */
        g_MuxDlcMng.Dlc[i].ParaInfo.ucT2 = 30;                               /* default value is 30 (300ms) */
        g_MuxDlcMng.Dlc[i].ParaInfo.ucN2 = MUX_MAX_NUM_OF_RETRANS;           /* maximum number of re-transmissions(default value)*/
    }

    g_MuxFrameMng.FrameBuff.usBuffLen = 1600 * 2;
    g_MuxFrameMng.FrameBuff.pRcvData = MuxMalloc(g_MuxFrameMng.FrameBuff.usBuffLen);

    g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_START_UP;

    MuxTrace(MUX_LOG_LEVEL_2, "Mux start: AT+CMUX=0,0,5,1600\r");
    UartMuxDataReq(pAtCmux, 18 + 1);
    MuxFree(pAtCmux);
    return;
}

static void AtMuxDataReq(char *pAtcStrBuff, unsigned short usAtcStrLen)
{
    unsigned char  ucDlci = 0;
    int i;

    MuxTrace(MUX_LOG_LEVEL_3, "Function AtMuxDataReq");
    if(0x1A != pAtcStrBuff[usAtcStrLen-1])
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT data  ---->  %s", pAtcStrBuff);
    }

    /* �ж�MUX�Ƿ��� */
    if ((MUX_STATUS_CLOSED_DOWN == g_MuxDlcMng.Dlc0.ucStatus) || (MUX_STATUS_CONNECT_WAIT == g_MuxDlcMng.Dlc0.ucStatus)) 
    {
        /* ���������Ƿ���� */
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
        pthread_mutex_lock(&MultiSend);
        g_MuxDlcMng.ucDlciIndex = ucDlci - 1;
        MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d receive data from AT", ucDlci);
        g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId = MUX_FLOW_RECEIVE_DATA_FROM_ATC;
        MuxRilMain(pAtcStrBuff, usAtcStrLen + g_ADD_CR);
        pthread_mutex_unlock(&MultiSend);
    }

    return;
}

// static void PppMuxDataReq(char *pPppStrBuff, unsigned short usPppStrLen)
// {
    // unsigned char  i;

    // MuxTrace(MUX_LOG_LEVEL_3, "Function PppMuxDataReq");
    
    // pPppStrBuff[usPppStrLen] = 0x00;
    // MuxTrace(MUX_LOG_LEVEL_2, "PPP AT data  ---->  %s", pPppStrBuff);

    // if(NULL != MuxStrStr(pPppStrBuff,"ATD*"))
    // {/* ATD* */
        // MuxTrace(MUX_LOG_LEVEL_2, "ATD* COMMAND, sleep 5s!");
        // sleep(5);
    // }
   
    // /* �ж�MUX�Ƿ��� */
    // if (MUX_STATUS_CLOSED_DOWN == g_MuxDlcMng.Dlc0.ucStatus) 
    // {
        // MuxTrace(MUX_LOG_LEVEL_1, "MUX is not start");
        // UartMuxDataReq(pPppStrBuff, usPppStrLen);
        // return;
    // }

    // for (i = 0; i < MUX_DLC_NUM; i ++)
    // {
        // /* �ҵ�PPP��ص�DLC,�˴��̶�ΪMUX_DLC_DATA_1 */
        // if (MUX_DLC_DATA_2 == g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci)
        // {
            ////g_DLC4Mode= PPPMODE;
            // MuxTrace(MUX_LOG_LEVEL_1, "Dlci %d receive data from Ppp", g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci);
            // pthread_mutex_lock(&MultiSend);
            // g_MuxDlcMng.Dlc[i].ucFlowId = MUX_FLOW_RECEIVE_DATA_FROM_PPP;
            // g_MuxDlcMng.ucDlciIndex = i;
            // MuxRilMain(pPppStrBuff, usPppStrLen);
            // pthread_mutex_unlock(&MultiSend);
            // break;
        // }
    // }

    // return;
// }

static void UartMuxDataInd(char *pUartRcvBuff, unsigned short usUartLen)
{
    char *pTmpBuffStart = NULL;
    char *pTmpBuffStop = NULL;
    char *pTmpBuff = NULL;
    unsigned short usTmpBuffLen;
    char *pCharstring = NULL;              // zdh  delete

    MuxTrace(MUX_LOG_LEVEL_3, "Function UartMuxDataInd");

    pCharstring = MuxHex2Char(pUartRcvBuff, usUartLen);   // zdh  delete
    //MuxTrace(MUX_LOG_LEVEL_1, "Uart recieve data:%s", pCharstring);   
    MuxFree(pCharstring);   // zdh  delete
    
    if ((MUX_STATUS_CLOSED_DOWN == g_MuxDlcMng.Dlc0.ucStatus) || (MUX_STATUS_CONNECT_WAIT == g_MuxDlcMng.Dlc0.ucStatus)) 
    {
        MuxMemCopy(g_UartBuffer.UartBuff + g_UartBuffer.usLen, pUartRcvBuff, usUartLen);
        g_UartBuffer.usLen += usUartLen;
        if (MUX_UART_CNT_OF_BUFFER < g_UartBuffer.usLen)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Buffer is full");
        }
        g_UartBuffer.UartBuff[g_UartBuffer.usLen] = 0x00;

        for (; ;)
        {
            pTmpBuffStart = MuxStrStr(g_UartBuffer.UartBuff, "\r\n");
            if (NULL != pTmpBuffStart)
            {
                pTmpBuffStop = MuxStrStr(pTmpBuffStart + 1, "\r\n");
                if (NULL != pTmpBuffStop)
                {
                    pTmpBuffStop += 2;

                    pTmpBuff = (char *)MuxMalloc(pTmpBuffStop - pTmpBuffStart + 1);
                    /* ����malloc��Ϊ���LOGʱ��Ҫ���\0������ */
                    MuxMemCopy(pTmpBuff, pTmpBuffStart, pTmpBuffStop - pTmpBuffStart);


                    /* �ж�MUX�Ƿ����������� */
                    if(MUX_FLOW_START_UP == g_MuxDlcMng.Dlc0.ucFlowId)
                    {
                        if(NULL != MuxStrStr(pTmpBuff, "OK"))
                        {
                            MuxTrace(MUX_LOG_LEVEL_2, "Mux start complete: OK");
                            g_MuxDlcMng.Dlc0.ucStatus = MUX_STATUS_DISCONNECTED;
                            g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_TEST;
                            FlowProcessfuncs();
                        }
                        else
                        {
                            /* ��ΪMux���ڴ��ڿ�������״̬ʱ��PS��Ӧ���Է��ķ���AT���� */
                            MuxTrace(MUX_LOG_LEVEL_1, "AT commod from PS is error!");
                            /* �ط�AT+CMUX=0,0,5,1600���� */
                            UartMuxDataReq("AT+CMUX=0,0,5,1600\r", 19);
                        }
                        MuxFree(pTmpBuff);
                        return;
                    }

                    if (MUX_STATUS_CONNECT_WAIT == g_MuxDlcMng.Dlc0.ucStatus)
                    {
                        if (NULL !=  MuxStrStr(pTmpBuff + 2, "OK"))
                        {
                            AtMuxStartReq();
                        }
                    }

                    /* ������������ֱ�ӷ��͸�leadcore */
                    AtMuxDataInd(pTmpBuff, pTmpBuffStop - pTmpBuffStart);

                    MuxFree(pTmpBuff);

                    usTmpBuffLen = g_UartBuffer.usLen - (pTmpBuffStop - g_UartBuffer.UartBuff);
                    if (0 != usTmpBuffLen)
                    {
                        pTmpBuff = (char *)MuxMalloc(usTmpBuffLen);
                        MuxMemCopy(pTmpBuff, pTmpBuffStop, usTmpBuffLen);
                        MuxMemSet((char *)&g_UartBuffer, 0, sizeof(ST_URAT_BUFF));
                        MuxMemCopy(g_UartBuffer.UartBuff, pTmpBuff, usTmpBuffLen);
                        g_UartBuffer.usLen = usTmpBuffLen;
                        MuxFree(pTmpBuff);
                    }
                    else
                    {
                        MuxMemSet((char *)&g_UartBuffer, 0, sizeof(ST_URAT_BUFF));
                    }

                    if (g_UartBuffer.usLen > 0)
                    {
                        continue;
                    }
                }
            }
            break;
        }
        MuxTrace(MUX_LOG_LEVEL_2, "Uart need recieve another!");
        return;
    }

    g_MuxFrameMng.MsgRcv.pData = MuxMalloc(usUartLen);
    MuxMemCopy(g_MuxFrameMng.MsgRcv.pData, pUartRcvBuff, usUartLen);
    g_MuxFrameMng.MsgRcv.usLen = usUartLen;

    MuxUartmain();
    return;  

}

static void AtMuxDataInd(char *pAtcStrBuff, unsigned short usAtcStrLen)
{
    unsigned short  SendLen = 0;
    unsigned short  SendOffset = 0;

    pAtcStrBuff[usAtcStrLen] = 0x00;
    if (0 == MuxStrncmp(pAtcStrBuff, "\r\n", 2))
    {
        MuxTrace(MUX_LOG_LEVEL_2, "AT data  <----  <CR><LF>%s", (pAtcStrBuff + 2));
    }
    else
    {
        MuxTrace(MUX_LOG_LEVEL_2, " AT data  <----  %s", pAtcStrBuff);
    }

    while(usAtcStrLen > 0)
    {
        SendLen = write(RilFd_Master, pAtcStrBuff + SendOffset, usAtcStrLen);
        SendOffset  += SendLen;
        usAtcStrLen -= SendLen;
        if(0 == SendLen)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Send data to RIL error, length = 0");
            break;
        }
    }
    return;
}

// static void PppMuxDataInd(char *pPppStrBuff, unsigned short usPppStrLen)
// {
    // unsigned short  SendLen = 0;
    // unsigned short  SendOffset = 0;

    // pPppStrBuff[usPppStrLen] = 0x00;
    // if (0 == MuxStrncmp(pPppStrBuff, "\r\n", 2))
    // {
        // MuxTrace(MUX_LOG_LEVEL_2, "PPP AT data  <----  <CR><LF>%s", (pPppStrBuff + 2));
    // }
    // else
    // {
        ////MuxTrace(MUX_LOG_LEVEL_2, "PPP AT data  <----  %s", pPppStrBuff);
        // MuxTrace(MUX_LOG_LEVEL_2, "PPP AT data  <----  usPppStrLen = %d", usPppStrLen);
    // }

    // while(usPppStrLen > 0)
    // {
        // SendLen = write(PppFd_Master, pPppStrBuff + SendOffset, usPppStrLen);
        // SendOffset  += SendLen;
        // usPppStrLen -= SendLen;
        // if(0 == SendLen)
        // {
            // MuxTrace(MUX_LOG_LEVEL_1, "Send data to PPP error, length = 0");
            // break;
        // }
    // }

    // SendLen = write(PppFd_Master, "\r", 1);
    // if(0 == SendLen)
    // {
        // MuxTrace(MUX_LOG_LEVEL_1, "Send '\r' to PPP error, length = 0");
    // }    

    // return;
// }

static void UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen)
{
    unsigned short  SendLen = 0;
    unsigned short  SendOffset = 0;
    int i;
    
    MuxTrace(MUX_LOG_LEVEL_3, "Function UartMuxDataReq");
    pthread_mutex_lock(&UartSendMutex);
    MuxTrace(MUX_LOG_LEVEL_3, ">>>>>>>>>lock uart port");
    while(usUartLen > 0)
    {
        SendLen = write(UartFd, pUartSndBuff + SendOffset, usUartLen);  //fasong shuju
        SendOffset  += SendLen;
        usUartLen -= SendLen;
        if(0 == SendLen)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Send data to UART error, length = 0");
            break;
        }
    }

    MuxTrace(MUX_LOG_LEVEL_3,"Send lenth is %d", SendOffset);
    
    MuxTrace(MUX_LOG_LEVEL_3, "<<<<<<<<<unlock uart port");
    pthread_mutex_unlock(&UartSendMutex);

    return;
}

static void MuxInit()
{
    unsigned char  i;

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxInit");

    /* ��ʼ��ȫ���� */
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

static void MuxUartmain()
{
    unsigned char ucRet = 0;                /* ���ز������ */

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxUartmain");

    for (; ;)
    {
        /* ������������frame֡ */
        ucRet = GetFrameFromDataStream();

        /* �ɹ��ҳ�һ��frame֡ */
        if (MUX_TRUE == ucRet)
        {
            /* ���DLC��ʶ */
            g_MuxDlcMng.ucDlciIndex = 0;

            ucRet = FrameValidityCheckAndParse();
            MuxTrace(MUX_LOG_LEVEL_3, "FrameValidityCheckAndParse = %d", ucRet);
            switch (ucRet)
            {
            case MUX_TYPE_INVALID:
                MuxSendUihNscResponse();
                break;
            case MUX_TRUE:
                FlowProcessfuncs();
                break;
            default:
                break;
            }
            MuxFree((char *)(g_MuxFrameMng.FrameBuff.pFrameData));
            g_MuxFrameMng.FrameBuff.pFrameData = NULL;
            g_MuxFrameMng.FrameBuff.usFrameDataLen = 0;
        }
        if (MUX_FALSE == ucRet)
        {
            break;
        }

    }
    return;
}

static void MuxRilMain(char *pBuffer, unsigned short usBufferLen)
{
    unsigned char ucRet = 0;            //���ز������

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxRilMain");

    /* DLC״̬��������Ϸ���Ҫ�󣬻������� */
    ucRet = UihFrameStateHandle(pBuffer, usBufferLen);

    /* �������������� */
    if(MUX_TRUE == ucRet)
    {
        /* ����FRAME�������� */
        UihFrameCreate(pBuffer, usBufferLen);
    }

    return;
}//added by zdh ���ĸ�����

static void FlowProcessfuncs()
{
    ST_URAT_MSG *pStTmp = NULL;

    MuxTrace(MUX_LOG_LEVEL_3, "Function FlowProcessfuncs");                    /* trace output               */

    switch (g_MuxDlcMng.Dlc0.ucFlowId)
    {
    case MUX_FLOW_TEST:
        SendUihTestCommond();
        break;
   case MUX_FLOW_NONE:
        switch (g_MuxDlcMng.Dlc[g_MuxDlcMng.Dlc0.ucDlciFlowOver - 1].ucFlowId)
        {
        case MUX_FLOW_DLC_ESTABLISH :
            if (MUX_DLC_AT_1 == g_MuxDlcMng.Dlc0.ucDlciFlowOver)
            {
                SendUihFcONCommond();
            }
            SendSABMFrame();
            break;
        case MUX_FLOW_SERVICE_NEGOTIATION:
            SendUihServiceNegCommond();
            break;
        case MUX_FLOW_APL_FC_ON :
            SendUihRTRONCommond();
            break;
        case MUX_FLOW_RECEIVE_DATA_FROM_UART :
            SendDatatoRil();
            break;
        default:
            break;
        }
    default:
        break;
    }
    return;
}

static void SendSABMFrame()
{
    unsigned char *pSABMFrame = NULL;
    unsigned char ucLen = 6;                                          /* ֡����                           */
    unsigned char ucDlci = g_MuxDlcMng.Dlc0.ucDlciFlowOver;           /* ��������ͨ����                   */

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendSABMFrame");            /* trace output                   */

    pSABMFrame = (unsigned char *)MuxMalloc(ucLen);
    if (NULL == pSABMFrame)
    {
        /* �쳣���� */
        MuxTrace(MUX_LOG_LEVEL_1, "SendSABMFrame: No memory");      /* trace output                   */
        return;
    }

    pSABMFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */
    pSABMFrame[1] = (ucDlci << 2) | 0x03;                             /* Address Field (EA=1 CR=1 DLCI=ucDlci)*/
    pSABMFrame[2] = MUX_SABM_FRAME | 0x10;                            /* Control Field (UIH Frame P=1)   */
    pSABMFrame[3] = 0x01;                                             /* Length Field (EA=1 length=0)    */
    pSABMFrame[4] = SenderCreateFcs(3, &pSABMFrame[1]);               /* Fcs Field                       */
    pSABMFrame[5] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */

    /* ����֡�����ش���ʱ��ʹ�� */
    MuxMemSet(g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy(g_MuxRetranBuffer.pData, pSABMFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    UartMuxDataReq(pSABMFrame, ucLen);

    MuxFree((char *)pSABMFrame);
    return;
}

static void SendUihTestCommond()
{
    unsigned char *pTestFrame = NULL;
    char *pAddr = NULL;
    unsigned char ucLen = 12;                                         /* ֡����                         */

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendUihTestCommond");       /* trace output                  */

    if (MUX_STATUS_CLOSED_DOWN == g_MuxDlcMng.Dlc0.ucStatus)
    {
        //�쳣����
        MuxTrace(MUX_LOG_LEVEL_1, "Mux is not opened!");
        return;
    }

    pTestFrame = (unsigned char *)MuxMalloc(ucLen);                   /*F9|ADDR|CLR|LEN|TYPE|LEN|VALUE0|VALUE1|VALUE2|VALUE3|FSC|F9*/
    if (NULL == pTestFrame)
    {
        //�쳣����
        MuxTrace(MUX_LOG_LEVEL_1, "SendUihTestCommond: No memory");
        return;
    }

    pTestFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */
    pTestFrame[1] = 0x03;                                             /* Address Field (EA=1 CR=1 DLCI=0)*/
    pTestFrame[2] = MUX_UIH_FRAME | 0x10;                             /* Control Field (UIH Frame P=1)   */ 
    pTestFrame[3] = 0x0D;                                             /* Length Field (EA=1 length=6)    */
    pTestFrame[4] = MUX_UIH_TEST | 0x03;                              /* Type Field (EA=1 CR=1)          */
    pTestFrame[5] = 0x09;                                             /* Length Field (EA=1 length=4)    */

    pAddr = (char *)MuxMalloc(1);
    /*����������ָ���ַ��ΪTEST��ֵ֤*/
    pTestFrame[6] = (unsigned char)((unsigned int)pAddr);                             
    pTestFrame[7] = (unsigned char)((unsigned int)pAddr >> 8);
    pTestFrame[8] = (unsigned char)((unsigned int)pAddr >> 16);
    pTestFrame[9] = (unsigned char)((unsigned int)pAddr >> 24); 
    MuxFree(pAddr);
    pAddr = NULL;

    pTestFrame[10] = SenderCreateFcs(3, &pTestFrame[1]);               /* Fcs Field                       */
    pTestFrame[11] = MUX_FLAG_SEQUENCE_FIELD;                          /* Flag Sequence Field(0xF9)       */

    /* ����TESTֵ����յ�����Ӧֵ�Ա� */
    if (NULL == g_MuxFrameMng.FrameInfo.TestInfo.pData)
    {
        g_MuxFrameMng.FrameInfo.TestInfo.pData = (unsigned char *)MuxMalloc(4);
        if (NULL == g_MuxFrameMng.FrameInfo.TestInfo.pData)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "SendUihTestCommond: No memory 2");
            MuxFree(pTestFrame);
            return;
        }
    }
    MuxMemCopy(g_MuxFrameMng.FrameInfo.TestInfo.pData, &pTestFrame[6], 4);
    g_MuxFrameMng.FrameInfo.TestInfo.usLen = 4;

    /* ����֡�����ش���ʱ��ʹ�� */
    MuxMemSet(g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy(g_MuxRetranBuffer.pData, pTestFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    UartMuxDataReq(pTestFrame, ucLen);

    MuxFree((char *)pTestFrame);
    return;
}

static void SendUihServiceNegCommond()
{
    unsigned char *pServiceFrame = NULL;
    unsigned char ucDlci = g_MuxDlcMng.Dlc0.ucDlciFlowOver;              /* ��ҪЭ�̵�ͨ����                */
    unsigned char ucLen = 11;                                            /* ֡����                         */ 

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendUihServiceNegCommond");    /* trace output                  */

    if (MUX_STATUS_DISCONNECTED == g_MuxDlcMng.Dlc[ucDlci - 1].ucStatus)
    {
        //�쳣����
        MuxTrace(MUX_LOG_LEVEL_3, "DLC%d is not established", ucDlci);
        return;
    }
    pServiceFrame = (unsigned char *)MuxMalloc(11);                      /*F9|ADDR|CLR|LEN|TYPE|LEN|VALUE0|VALUE1|VALUE2|FSC|F9*/
    if (NULL == pServiceFrame)                                 
    {
        MuxTrace(MUX_LOG_LEVEL_1, "SendUihServiceNegCommond: No memory");
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
        MuxTrace(MUX_LOG_LEVEL_1, "Dlci out of range!");
        break;
    }

    pServiceFrame[9] = SenderCreateFcs(3, &pServiceFrame[1]);           /* Fcs Field                       */
    pServiceFrame[10] = MUX_FLAG_SEQUENCE_FIELD;                        /* Flag Sequence Field(0xF9)       */

    /*����֡�����ش���ʱ��ʹ��*/
    MuxMemSet(g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy(g_MuxRetranBuffer.pData, pServiceFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    UartMuxDataReq(pServiceFrame, ucLen);

    MuxFree((char *)pServiceFrame);

    return;
}

static void SendUihFcONCommond()
{
    unsigned char *pFcONFrame = NULL;
    unsigned char ucLen = 8;                                           /* ֡����                         */
    unsigned char i;

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendUihFcONCommond");          /* trace output                  */

    if (MUX_STATUS_CLOSED_DOWN == g_MuxDlcMng.Dlc0.ucStatus)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "Mux is not opened!");
        return;
    }

    pFcONFrame = (unsigned char *)MuxMalloc(ucLen);                     /* F9|ADDR|CLR|LEN|TYPE|LEN|FSC|F9 */
    pFcONFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */
    pFcONFrame[1] = 0x03;                                               /* Address Field (EA=1 CR=1 DLCI=0)*/
    pFcONFrame[2] = MUX_UIH_FRAME & 0xEF;                               /* Control Field (UIH Frame P=0)   */
    pFcONFrame[3] = 0x05;                                               /* Length Field (EA=1 length=2)    */
    pFcONFrame[4] = MUX_UIH_FCon | 0x03;                                /* Type Field (EA=1 CR=1)          */
    pFcONFrame[5] = 0x01;                                               /* Length Field (EA=1 length=0)    */
    pFcONFrame[6] = SenderCreateFcs(3, &pFcONFrame[1]);                 /* Fcs Field                       */
    pFcONFrame[7] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */

    /* ���������� */
    for(i = 0; i < MUX_DLC_NUM; i++)
    {
        g_MuxDlcMng.Dlc[i].ucFc = MUX_ON;
    }

    UartMuxDataReq(pFcONFrame, ucLen);

    MuxFree((char *)pFcONFrame);

    return;
}

static void SendUihRTRONCommond()
{
    unsigned char *pRTRonFrame = NULL;
    unsigned char ucDlci = g_MuxDlcMng.Dlc0.ucDlciFlowOver;              /* ��ҪЭ�̵�ͨ����                */
    unsigned char ucLen = 10;                                            /* ֡����                         */

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendUihRTRONCommond");         /* trace output                  */

    if (MUX_STATUS_DISCONNECTED == g_MuxDlcMng.Dlc[ucDlci - 1].ucStatus)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "DLC%d is not estabilished!", ucDlci);
        return;
    }

    pRTRonFrame = (unsigned char *)MuxMalloc(ucLen);                     /* F9|ADDR|CLR|LEN|TYPE|LEN|DLCI|V.24|FSC|F9 */
    if (NULL == pRTRonFrame)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "SendUihRTRONCommond: No memory!");
        return;
    }
    pRTRonFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */
    pRTRonFrame[1] = 0x03;                                               /* Address Field (EA=1 CR=1 DLCI=0)*/
    pRTRonFrame[2] = MUX_UIH_FRAME | 0x10;                               /* Control Field (UIH Frame P=1)   */
    pRTRonFrame[3] = 0x09;                                               /* Length Field (EA=1 length=4)    */
    pRTRonFrame[4] = MUX_UIH_MSC | 0x03;                                 /* Type Field (EA=1 CR=1)          */
    pRTRonFrame[5] = 0x05;                                               /* Length Field (EA=1 length=2)    */
    pRTRonFrame[6] = (ucDlci << 2) | 0x03;                               /* DLCI Field (EA=1 CR=1)          */

    pRTRonFrame[7] = 0x00;                                               /* V.24�����ֽڳ�ֵ                 */
    pRTRonFrame[7] |= 0x01;                                              /* EA bit  no Break Signals        */                                      
    pRTRonFrame[7] |= 0x04;                                              /* RTC bit                         */
    pRTRonFrame[7] |= 0x08;                                              /* RTR bit                         */
    pRTRonFrame[7] |= 0x80;                                              /* DV bit                          */

    pRTRonFrame[8] = SenderCreateFcs(3, &pRTRonFrame[1]);                /* Fcs Field                       */
    pRTRonFrame[9] = MUX_FLAG_SEQUENCE_FIELD;                            /* Flag Sequence Field(0xF9)       */

    /* ����֡���ش���ʱ����� */
    MuxMemSet(g_MuxRetranBuffer.pData, 0, MUX_MAX_CNT_OF_BUFFER);
    MuxMemCopy(g_MuxRetranBuffer.pData, pRTRonFrame, ucLen);
    g_MuxRetranBuffer.usLen = ucLen;

    UartMuxDataReq(pRTRonFrame, ucLen);

    MuxFree((char *)pRTRonFrame);                       
    return;
}

static void SendUihRTRONResponse()
{
    unsigned char *pRTRonFrame = NULL;
    unsigned char ucDlci;                                                /* ��ҪЭ�̵�ͨ����                */
    unsigned char ucLen = 10;                                            /* ֡����                         */

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendUihRTRONResponse");        /* trace output                  */

    ucDlci = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci;
    pRTRonFrame = (unsigned char *)MuxMalloc(ucLen);                     /* F9|ADDR|CLR|LEN|TYPE|LEN|DLCI|V.24|FSC|F9 */
    if (NULL == pRTRonFrame)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "SendUihRTRONResponse: No memory!");
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

    UartMuxDataReq(pRTRonFrame, ucLen);

    /* ��RTR���� */
    g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucRtrFc = MUX_ON;

    MuxFree((char *)pRTRonFrame);                       
    return;
}

static void SendUihRTROFFResponse()
{
    unsigned char *pRTRoffFrame = NULL;
    unsigned char ucDlci;                                                /* ��ҪЭ�̵�ͨ����                    */
    unsigned char ucLen = 10;                                            /* ֡����                             */

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendUihRTROFFResponse");       /* trace output                      */

    ucDlci = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci;
    pRTRoffFrame = (unsigned char *)MuxMalloc(ucLen);                    /* F9|ADDR|CLR|LEN|TYPE|LEN|DLCI|V.24|FSC|F9 */
    if (NULL == pRTRoffFrame)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "SendUihRTROFFResponse: No memory!");
    }
    pRTRoffFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                           /* Flag Sequence Field(0xF9)       */
    pRTRoffFrame[1] = 0x03;                                              /* Address Field (EA=1 CR=1 DLCI=0)*/
    pRTRoffFrame[2] = MUX_UIH_FRAME | 0x10;                              /* Control Field (UIH Frame P=1)   */
    pRTRoffFrame[3] = 0x09;                                              /* Length Field (EA=1 length=4)    */
    pRTRoffFrame[4] = MUX_UIH_MSC | 0x01;                                /* Type Field (EA=1 CR=0)          */
    pRTRoffFrame[5] = 0x05;                                              /* Length Field (EA=1 length=2)    */
    pRTRoffFrame[6] = (ucDlci << 2) | 0x03;                              /* DLCI Field (EA=1 CR=1)          */
    pRTRoffFrame[7] = g_MuxFrameMng.FrameInfo.ucV24;                     /* V.24 signal                     */                        
    pRTRoffFrame[8] = SenderCreateFcs(3, &pRTRoffFrame[1]);              /* Fcs Field                       */
    pRTRoffFrame[9] = MUX_FLAG_SEQUENCE_FIELD;                           /* Flag Sequence Field(0xF9)       */

    UartMuxDataReq(pRTRoffFrame, ucLen);

    /* �ر�RTR���� */
    g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucRtrFc = MUX_OFF;

    MuxFree((char *)pRTRoffFrame);                       
    return;
}

static void MuxSendUihNscResponse(void)
{
    unsigned char *pNscFrame;
    unsigned char ucFcs;
    unsigned char ucLen;                                                /* ֡����                         */

    MuxTrace(MUX_LOG_LEVEL_3, "MuxSendUihNscResponse");

    ucLen = 9;
    pNscFrame = (unsigned char *)MuxMalloc(ucLen);                      /* F9|ADDR|CLR|LEN|TYPE|LEN|DLCI|V.24|FSC|F9 */
    pNscFrame[0] = MUX_FLAG_SEQUENCE_FIELD;                             /* Flag Sequence Field(0xF9)        */
    pNscFrame[1] = 0x03;                                                /* Address Field (EA=1 CR=1 DLCI=0) */
    pNscFrame[2] = MUX_UIH_FRAME | 0x10;                                /* Control Field (UIH Frame F=1)    */
    pNscFrame[3] = 0x07;                                                /* Length Field (EA=1 Value=3)      */
    pNscFrame[4] = 0x11;                                                /* Type Field (EA=1 CR=0)           */
    pNscFrame[5] = 0x03;                                                /* Length Field (EA=1 Value=1)      */
    pNscFrame[6] = g_MuxFrameMng.FrameInfo.ucInvalidType;
    ucFcs        = SenderCreateFcs(3, &pNscFrame[1]);
    pNscFrame[7] = ucFcs;                                               /* FCS                              */
    pNscFrame[8] = MUX_FLAG_SEQUENCE_FIELD;                             /* Flag Sequence Field(0xF9)        */

    UartMuxDataReq(pNscFrame, ucLen);

    MuxFree((char *)pNscFrame);

    return;
}


static void SendDatatoRil()
{
    unsigned short usLen = g_MuxFrameMng.FrameInfo.UihInfo.usLen;

    MuxTrace(MUX_LOG_LEVEL_3, "Function SendDatatoRil");

    /* add for AT ECHO DISPLAY */
    if(usLen <= 1)
    {
        MuxTrace(MUX_LOG_LEVEL_3, "AT ECHO DISPLAY!");
        return;
    }
    
    if (MUX_SERV_AT == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucService)
    {  
        /* AT����������߼��жϣ�ֱ�ӷ��� */
        AtMuxDataInd(g_MuxFrameMng.FrameInfo.UihInfo.pData, usLen);
        g_MuxFrameMng.FrameInfo.UihInfo.pData = NULL;

    }
    else if (MUX_SERV_DATA == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucService)
    {
        if(g_MuxFrameMng.FrameInfo.ucDlci == MUX_DLC_DATA_1)
        {
			AtMuxDataInd(g_MuxFrameMng.FrameInfo.UihInfo.pData, usLen);
            g_MuxFrameMng.FrameInfo.UihInfo.pData = NULL;
		}
        // else if(g_MuxFrameMng.FrameInfo.ucDlci == MUX_DLC_DATA_2)
        // {
			// PppMuxDataInd(g_MuxFrameMng.FrameInfo.UihInfo.pData, usLen);
            // g_MuxFrameMng.FrameInfo.UihInfo.pData = NULL;
		// }			
    }
    else
    {
        MuxTrace(MUX_LOG_LEVEL_1, "SendDatatoRil: Service type error!");
    }
    return;
}

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

static unsigned char GetFrameFromDataStream()
{
    unsigned char   *pData = NULL;
    unsigned char   *pTmpFrame = NULL;     /* ��ȡ�յ������� */
    unsigned char   *pMsg = NULL;
    unsigned short  i, j, usLen;

    MuxTrace(MUX_LOG_LEVEL_3, "Function GetFrameFromDataStream");

    /* ��ȡ���buff�������� */
    if (NULL != g_MuxFrameMng.MsgRcv.pData)
    {
        /* ��ȡ���buff�������� */
        if ((g_MuxFrameMng.FrameBuff.usBuffLen - g_MuxFrameMng.FrameBuff.usRcvDataLen) < g_MuxFrameMng.MsgRcv.usLen)
        {
            usLen = g_MuxFrameMng.FrameBuff.usBuffLen - g_MuxFrameMng.FrameBuff.usRcvDataLen;
        }
        else
        {
            usLen = g_MuxFrameMng.MsgRcv.usLen;
        }

        /* ���ν��յ������ݣ����ӵ��Ѿ������֮�� */
        MuxMemCopy(g_MuxFrameMng.FrameBuff.pRcvData + g_MuxFrameMng.FrameBuff.usRcvDataLen, g_MuxFrameMng.MsgRcv.pData, usLen);
        g_MuxFrameMng.FrameBuff.usRcvDataLen += usLen;

        /* buffװ���µ����ݼ�������g_MuxFrameMng.MsgRcv�� */
        g_MuxFrameMng.MsgRcv.usLen -= usLen;
        if (0 != g_MuxFrameMng.MsgRcv.usLen)
        {
            pMsg = MuxMalloc(g_MuxFrameMng.MsgRcv.usLen);
            MuxMemCopy(pMsg, g_MuxFrameMng.MsgRcv.pData + usLen, g_MuxFrameMng.MsgRcv.usLen);
            MuxFree((void*)g_MuxFrameMng.MsgRcv.pData);
            g_MuxFrameMng.MsgRcv.pData = pMsg;
        }
        else
        {
            MuxFree(g_MuxFrameMng.MsgRcv.pData);
            g_MuxFrameMng.MsgRcv.pData = NULL;
        }
    }

    pTmpFrame = g_MuxFrameMng.FrameBuff.pRcvData;

    for (i = 0; i < g_MuxFrameMng.FrameBuff.usRcvDataLen; i++)
    {
        /* �ҳ�frame����ʼ��� */
        if (MUX_FLAG_SEQUENCE_FIELD == pTmpFrame[i])
        {
            break;
        }
    }

    /* û���ҵ�frame����ʼ��� */
    if (i == g_MuxFrameMng.FrameBuff.usRcvDataLen)
    {
        /* ���pRcvData */
        if ((NULL != pTmpFrame) && (0 != g_MuxFrameMng.FrameBuff.usRcvDataLen))
        {
            MuxTrace(MUX_LOG_LEVEL_2, "Not frame!. discard data len:%d", g_MuxFrameMng.FrameBuff.usRcvDataLen);
            MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen);
        }
        g_MuxFrameMng.FrameBuff.usRcvDataLen = 0;

        /* pMsg�����н�ȡ�����ݣ�����һ��GetFrameFromDataStream */
        if (NULL != g_MuxFrameMng.MsgRcv.pData)
        {
            MuxTrace(MUX_LOG_LEVEL_3, "Function GetFrameFromDataStream finish");
            return (MUX_CONTINUE);
        }
        else
        {
            return (MUX_FALSE);
        }
    }

    /* frame����ʼ��Ǻ�С��һ��frame����С����5���ֽ� */
    if (5 > (g_MuxFrameMng.FrameBuff.usRcvDataLen - i))
    {
        /* ��ʼ���ǰ������Ч���� */
        if (0 != i)
        {
            pData = MuxMalloc(g_MuxFrameMng.FrameBuff.usRcvDataLen - i);
            /* ��ʼ���ָ�����pData������pDataǰ������ */
            MuxMemCopy(pData, pTmpFrame + i, g_MuxFrameMng.FrameBuff.usRcvDataLen - i);
            MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen);
            MuxMemCopy(pTmpFrame, pData, g_MuxFrameMng.FrameBuff.usRcvDataLen - i);
            g_MuxFrameMng.FrameBuff.usRcvDataLen -= i;

            MuxFree((void*)pData);
            MuxTrace(MUX_LOG_LEVEL_2, "discard data before F9!. len:%d", i);
        }

        /* pMsg�����н�ȡ�����ݣ�����һ��GetFrameFromDataStream */
        if (NULL != g_MuxFrameMng.MsgRcv.pData)
        {
            return (MUX_CONTINUE);
        }
        else
        {
            return (MUX_FALSE);
        }
    }

    /* �ж�Length Indicator�ֽ�û��E/A */
    if (1 == (0x01 & pTmpFrame[i + 3]))
    {
        /* ��FrameInfo�и��³��� */
        g_MuxFrameMng.FrameInfo.usLength = pTmpFrame[i + 3];
        /* ȥ��E/Aռ��bitλ */
        usLen = g_MuxFrameMng.FrameInfo.usLength >> 1;
        /* ����Address Control Length FCS��ռ��4�ֽڣ�����ȥ��ʼ���˱�־�ĳ��� */
        usLen += 4;
    }
    else
    {
        /* Length�����ֽڱ�ʾʱ����FrameInfo�и��³��� */
        g_MuxFrameMng.FrameInfo.usLength = pTmpFrame[i + 4];
        g_MuxFrameMng.FrameInfo.usLength <<= 8;
        g_MuxFrameMng.FrameInfo.usLength |= pTmpFrame[i + 3];
        /* ȥ��E/Aռ��bitλ */
        usLen = g_MuxFrameMng.FrameInfo.usLength >> 1;
        /* ����Address Control Length FCS��ռ��5�ֽڣ�����ȥ��ʼ���˱�־�ĳ��� */
        usLen += 5;
    }

    /* һ��Frameռ�������ֽ��ǲ��Ƕ���FrameBuff�У�2��ʾFCS Flag */
    if (g_MuxFrameMng.FrameBuff.usRcvDataLen < (i + usLen + 2))
    {
        /* ��ʼ���ǰ������Ч���� */
        if (0 != i)
        {
            pData = MuxMalloc(g_MuxFrameMng.FrameBuff.usRcvDataLen - i);
            /* ��ʼ���ָ�����pData������pDataǰ������ */
            MuxMemCopy(pData, pTmpFrame + i, g_MuxFrameMng.FrameBuff.usRcvDataLen - i);
            MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen);
            MuxMemCopy(pTmpFrame, pData, g_MuxFrameMng.FrameBuff.usRcvDataLen - i);
            g_MuxFrameMng.FrameBuff.usRcvDataLen -= i;

            MuxFree((void*)pData);
            MuxTrace(MUX_LOG_LEVEL_2, "discard data before F9!. len:%d", i);
        }
        /* usDataLen�Ѿ������buff�� */
        else if (g_MuxFrameMng.FrameBuff.usRcvDataLen == g_MuxFrameMng.FrameBuff.usBuffLen)
        {
            MuxTrace(MUX_LOG_LEVEL_2, "USB data overflow!. discard data len:%d", g_MuxFrameMng.FrameBuff.usRcvDataLen);
            /* �ⲻ��������������� */
            g_MuxFrameMng.FrameBuff.usRcvDataLen = 0;
        }
        else
        {
        }

        /* pMsg�����н�ȡ�����ݣ�����һ��GetFrameFromDataStream */
        if (NULL != g_MuxFrameMng.MsgRcv.pData)
        {
            return (MUX_CONTINUE);
        }
        else
        {
            return (MUX_FALSE);
        }
    }

    /* frame�����˱���볤��ָʾ�Ĳ�һ�� */
    if (MUX_FLAG_SEQUENCE_FIELD != pTmpFrame[i + usLen + 1])
    {
        /* �ҳ�frame�����˱�� */
        for (j = i + 1; j < g_MuxFrameMng.FrameBuff.usRcvDataLen; j++)
        {
            if (MUX_FLAG_SEQUENCE_FIELD == pTmpFrame[j])
            {
                break;
            }
        }
        /* ���˱��û�ҵ� */
        if (j == g_MuxFrameMng.FrameBuff.usRcvDataLen)
        {
            /* ���FrameBuff.pRcvData */
            MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen);
            g_MuxFrameMng.FrameBuff.usRcvDataLen = 0;
            MuxTrace(MUX_LOG_LEVEL_2, "no F9 match!. discard all data, len:%d", g_MuxFrameMng.FrameBuff.usRcvDataLen);

            /* pMsg�����н�ȡ�����ݣ�����һ��GetFrameFromDataStream */
            if (NULL != g_MuxFrameMng.MsgRcv.pData)
            {
                return (MUX_CONTINUE);
            }
            else
            {
                return (MUX_FALSE);
            }
        }

        /* ���˱���ҵ��ˣ������˶�frame  */
        MuxTrace(MUX_LOG_LEVEL_2, "no F9 match!. discard data before next F9, len:%d", j);
        g_MuxFrameMng.FrameBuff.usRcvDataLen -= j;
        pData = MuxMalloc(g_MuxFrameMng.FrameBuff.usRcvDataLen);
        /* ���˱��ָ�����pData������pDataǰ������ */
        MuxMemCopy(pData, pTmpFrame + j, g_MuxFrameMng.FrameBuff.usRcvDataLen);
        MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen + j);
        MuxMemCopy(pTmpFrame, pData, g_MuxFrameMng.FrameBuff.usRcvDataLen);
        MuxFree((void*)pData);
        return (MUX_CONTINUE);
    }

    /* frame���˱�����¼�ĳ���һ�� */
    /* ��ȥ��ʼ���˱�־�ĳ��ȸ�usFrameLen */
    g_MuxFrameMng.FrameBuff.usFrameDataLen  = usLen;
    g_MuxFrameMng.FrameBuff.pFrameData = MuxMalloc(g_MuxFrameMng.FrameBuff.usFrameDataLen);

    /* pFrameData�н����һ��frame֡����Ч���ݲ���Address Control Length Information FCS */
    MuxMemCopy(g_MuxFrameMng.FrameBuff.pFrameData,
        pTmpFrame + i + 1,
        g_MuxFrameMng.FrameBuff.usFrameDataLen);

    /* �س��˶�frame����δ������ */
    if (g_MuxFrameMng.FrameBuff.usRcvDataLen != (i + usLen + 2))
    {
        g_MuxFrameMng.FrameBuff.usRcvDataLen -= (i + usLen + 2);
        pData = MuxMalloc(g_MuxFrameMng.FrameBuff.usRcvDataLen);
        MuxMemCopy(pData, pTmpFrame + i + usLen + 2, g_MuxFrameMng.FrameBuff.usRcvDataLen);
        MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen + i + usLen + 2);

        /* pTmpFrame == FrameBuff.pRcvData,ʣ�µ�������һ�ν� */
        MuxMemCopy(pTmpFrame, pData, g_MuxFrameMng.FrameBuff.usRcvDataLen);
        MuxFree((void*)pData);
    }
    else
    {
        /* ���FrameBuff.pRcvData */
        MuxMemSet(pTmpFrame, 0, g_MuxFrameMng.FrameBuff.usRcvDataLen);
        g_MuxFrameMng.FrameBuff.usRcvDataLen = 0;
    }

    /* FrameBuff.pFrameData�д����һ������frame֡������ */
    return (MUX_TRUE);
}

static unsigned char FrameValidityCheckAndParse(void)
{
    unsigned char  i, ucRet;
    unsigned char  ucFieldLen;                              /* ��Ҫ��FCS��Field���� */
    unsigned char  ucType;                                  /* UIH֡��Message Type */
    unsigned char  ucAddress, ucControl, ucPFbit, ucFcs;    /* frame�и�Field���� */
    unsigned short usLength;                                /* frame�и�Field���� */

    MuxTrace(MUX_LOG_LEVEL_3, "Function FrameValidityCheckAndParse");

    ucAddress = g_MuxFrameMng.FrameBuff.pFrameData[0];
    ucControl = g_MuxFrameMng.FrameBuff.pFrameData[1] & 0xEF;
    ucPFbit   = g_MuxFrameMng.FrameBuff.pFrameData[1] & 0x10;
    usLength  = g_MuxFrameMng.FrameInfo.usLength >> 1;
    ucFcs     = g_MuxFrameMng.FrameBuff.pFrameData[g_MuxFrameMng.FrameBuff.usFrameDataLen - 1];

    /* Address Check */
    if (!(ucAddress & 0x01))
    {
        /* Addressֻ������һ���ֽ� */
        MuxTrace(MUX_LOG_LEVEL_1, "Address Check Fail");
        return (MUX_CONTINUE);
    }

    /* ����DLCI */
    g_MuxFrameMng.FrameInfo.ucDlci = ucAddress >> 2;
    MuxTrace(MUX_LOG_LEVEL_2, "Address->DLCI = %d", g_MuxFrameMng.FrameInfo.ucDlci);

    /* ֡���ͷ��ദ�� */
    switch (ucControl)
    {
    case MUX_DM_FRAME:
        MuxTrace(MUX_LOG_LEVEL_2, "Control->FRAME TYPE = MUX_DM_FRAME");

        if (0 == ucPFbit)
        {
            /* F bit == 0,���Ը���Ϣ */
            MuxTrace(MUX_LOG_LEVEL_1, "F bit in MUX_DM_FRAME is 0");
            return (MUX_CONTINUE);
        }
        
        /* ������ʧ����Ϣ��ucAddress�е�DLCI�����ж� */
        for (i = 0; i < MUX_DLC_NUM; i++)
        {
            /* ������DLC������frame */
            if (MUX_FLOW_DLC_ESTABLISH == g_MuxDlcMng.Dlc[i].ucFlowId)
            {
                MuxTrace(MUX_LOG_LEVEL_2, "Establish Dlci %d Again", g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci);
                /* ���DLC���½��� */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci;
                break;
            }

            /* ������DLC�ͷŵ�frame */
            if (MUX_FLOW_DLC_RELEASE == g_MuxDlcMng.Dlc[i].ucFlowId)
            {
                MuxTrace(MUX_LOG_LEVEL_2, "Release Dlci %d Again", g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci);
                /* ���DLC�����ͷ� */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver =  g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci;
                break;
            }
        }
        if (MUX_DLC_NUM == i)
        {
            MuxTrace(MUX_LOG_LEVEL_2, "There is no DLC establish or release, ignore this frame");
            return (MUX_CONTINUE);
        }
        break;
    case MUX_UA_FRAME:
        MuxTrace(MUX_LOG_LEVEL_2, "Control->FRAME TYPE = MUX_UA_FRAME");

        if (0 == ucPFbit)
        {
            /* F bit == 0,���Ը���Ϣ */
            MuxTrace(MUX_LOG_LEVEL_1, "F bit in MUX_UA_FRAME is 0");
            return (MUX_CONTINUE);
        }

        /* Dlci�����֧�ַ�Χ */
        if (MUX_DLC_NUM < g_MuxFrameMng.FrameInfo.ucDlci)
        {
            /* ��������frame */
            MuxTrace(MUX_LOG_LEVEL_1, "Dlci out of range,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* Address field + Control field + length field */
        ucFieldLen = 3;
        /* FCS��� */
        if (MUX_FALSE ==  ReceiverCheckFcs(ucFcs, ucFieldLen, g_MuxFrameMng.FrameBuff.pFrameData))
        {
            MuxTrace(MUX_LOG_LEVEL_1, "FCS check fail");
            return (MUX_CONTINUE);
        }

        /* Information���Ȳ��������ֽ�,�ҳ���Ӧ��Ϊ0 */
        if ((0 == (g_MuxFrameMng.FrameInfo.usLength & 0x0001)) || (0 != (g_MuxFrameMng.FrameInfo.usLength >> 1)))
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Length check fail");
            return (MUX_CONTINUE);
        }

        for (i = 0;i < MUX_DLC_NUM; i ++)
        {
            /* �����ڽ�����DLC */
            if ((g_MuxFrameMng.FrameInfo.ucDlci == g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci) && 
                (g_MuxDlcMng.Dlc[i].ucFlowId == MUX_FLOW_DLC_ESTABLISH))
            {
                MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d establish successfully", g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci);
                /* �յ�UA��Ӧ��DLC�����ɹ�����ʼ����Э������ */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = g_MuxFrameMng.FrameInfo.ucDlci;
                g_MuxDlcMng.Dlc[i].ucStatus = MUX_STATUS_CONNECTED;
                g_MuxDlcMng.Dlc[i].ucFlowId = MUX_FLOW_SERVICE_NEGOTIATION;
                break;
            }

            /* �������ͷŵ�DLC */
            if ((g_MuxFrameMng.FrameInfo.ucDlci == g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci) && 
                (g_MuxDlcMng.Dlc[i].ucFlowId == MUX_FLOW_DLC_RELEASE))
            {
                MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d release successfully", g_MuxDlcMng.Dlc[i].ParaInfo.ucDlci);
                /* �յ�UA��Ӧ��DLC�ͷųɹ����������̴��� */
                g_MuxDlcMng.Dlc[i].ucStatus = MUX_STATUS_DISCONNECTED;
                g_MuxDlcMng.Dlc[i].ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = MUX_DLC0;
                break;
            }
        }

        if (MUX_DLC_NUM == i)
        {
            MuxTrace(MUX_LOG_LEVEL_2, "There is no DLC establish or release, ignore this frame");
            return (MUX_CONTINUE);
        }
        break;
    case MUX_UIH_FRAME:
        MuxTrace(MUX_LOG_LEVEL_2, "Control->FRAME TYPE = MUX_UIH_FRAME");

        /* Dlci�����֧�ַ�Χ */
        if (MUX_DLC_NUM < g_MuxFrameMng.FrameInfo.ucDlci)
        {
            /* ��������frame */
            MuxTrace(MUX_LOG_LEVEL_1, "Dlci out of range");
            return (MUX_CONTINUE);
        }
        /* Address field + Control field + Length field */
        if (g_MuxFrameMng.FrameInfo.usLength & 0x0001)
        {
            ucFieldLen = 3;
        }
        /* length field�����ֽ� */
        else
        {
            ucFieldLen = 4;
        }

        /* FCS��� */
        if (MUX_FALSE == ReceiverCheckFcs(ucFcs, ucFieldLen, g_MuxFrameMng.FrameBuff.pFrameData))
        {
            MuxTrace(MUX_LOG_LEVEL_1, "FCS check fail");
            return (MUX_CONTINUE);
        }

        /* ����Information��ʵ�ʳ��Ⱥ�ʵ�����ݣ���UihInfo */
        g_MuxFrameMng.FrameInfo.UihInfo.usLen = g_MuxFrameMng.FrameInfo.usLength >> 1;
        g_MuxFrameMng.FrameInfo.UihInfo.pData = g_MuxFrameMng.FrameBuff.pFrameData + ucFieldLen;

        /* UIH֡���ǿ�����Ϣ */
        if (MUX_DLC0 == g_MuxFrameMng.FrameInfo.ucDlci)
        {
            ucType = g_MuxFrameMng.FrameInfo.UihInfo.pData[0];

            /* Message Type���������ֽ� */
            if (1 != (ucType & 0x01))
            {
                MuxTrace(MUX_LOG_LEVEL_2, "UIH_FRAME Type check fail");
                return (MUX_CONTINUE);
            }

            /* ����UIH���ͽ��н��� */
            ucRet = MuxUihParse(ucType);
            return ucRet;
        }
        /* UIH֡�������� */
        else
        {
            MuxTrace(MUX_LOG_LEVEL_2, "Receive data from uart");
            g_MuxDlcMng.ucDlciIndex = g_MuxFrameMng.FrameInfo.ucDlci - 1;
            g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
            g_MuxDlcMng.Dlc0.ucDlciFlowOver = g_MuxFrameMng.FrameInfo.ucDlci;
            g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId = MUX_FLOW_RECEIVE_DATA_FROM_UART;
        }
        break;
    default:
        /* ����֡���ͺ��� */
        MuxTrace(MUX_LOG_LEVEL_1, "Control->FRAME TYPE = %X, Ignore this frame", ucControl);
        return (MUX_CONTINUE);
    }

    return (MUX_TRUE);
}

static unsigned char MuxUihParse(unsigned char  ucType)
{
    unsigned short  usLen, usCnt;
    ST_URAT_MSG *pStTmp;
    int i;

    MuxTrace(MUX_LOG_LEVEL_3, "Function MuxUihParse");

    /* ȥ��EA�����ͷַ� */
    switch (ucType & 0xFC)
    {
        /* ͨ�Ų��� */
    case MUX_UIH_TEST:
        MuxTrace(MUX_LOG_LEVEL_2, "UIH frame Type = MUX_UIH_TEST");

        /* C/R���,��Ϊ1,���Դ���Ϣ */
        if (1 == (ucType & 0x02))
        {
            MuxTrace(MUX_LOG_LEVEL_1, "C/R check fail,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* ��ǰû�н���ͨ�Ų������̣����Դ���Ϣ */
        if (MUX_FLOW_TEST != g_MuxDlcMng.Dlc0.ucFlowId)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Unsuitable Flow,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* Length��һ���ֽڱ�ʾ */
        if (g_MuxFrameMng.FrameInfo.UihInfo.pData[1] & 0x01)
        {
            /* �յ�����ӦTestInfo�б�����������Ա� */
            if ((g_MuxFrameMng.FrameInfo.TestInfo.usLen == (g_MuxFrameMng.FrameInfo.UihInfo.pData[1] >> 1)) && 
                (0 == MuxStrncmp(g_MuxFrameMng.FrameInfo.TestInfo.pData, 
                g_MuxFrameMng.FrameInfo.UihInfo.pData + 2, 
                g_MuxFrameMng.FrameInfo.TestInfo.usLen)))
            {
                /* ����һ�£�ͨ�Ų��Գɹ�������MUX_DLC_AT_1�Ľ������� */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = MUX_DLC_AT_1;
                g_MuxDlcMng.Dlc[MUX_DLC_AT_1 - 1].ucFlowId = MUX_FLOW_DLC_ESTABLISH;
                MuxTrace(MUX_LOG_LEVEL_2, "Test Command successfully");
            } 
            else
            {
                /* ͨ�Ų���ʧ�ܣ���������������*/
                MuxTrace(MUX_LOG_LEVEL_1, "Test Command fail��restart mux");
                AtMuxStartReq();
            }
        }
        else
        {
            usLen = g_MuxFrameMng.FrameInfo.UihInfo.pData[2];
            usLen <<= 8;
            usLen |= g_MuxFrameMng.FrameInfo.UihInfo.pData[1];

            /* �յ�����ӦTestInfo�б�����������Ա� */
            if ((g_MuxFrameMng.FrameInfo.TestInfo.usLen == (usLen >> 1)) && 
                (0 == MuxStrncmp(g_MuxFrameMng.FrameInfo.TestInfo.pData, 
                g_MuxFrameMng.FrameInfo.UihInfo.pData + 3, 
                g_MuxFrameMng.FrameInfo.TestInfo.usLen)))
            {
                /* ����һ�£�ͨ�Ų��Գɹ�������MUX_DLC_AT_1�Ľ������� */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = MUX_DLC_AT_1;
                g_MuxDlcMng.Dlc[MUX_DLC_AT_1 - 1].ucFlowId = MUX_FLOW_DLC_ESTABLISH;
                MuxTrace(MUX_LOG_LEVEL_2, "Test Command successfully");
            }
            else
            {
                /* ͨ�Ų���ʧ�ܣ���������������*/
                MuxTrace(MUX_LOG_LEVEL_1, "Test Command fail��restart mux");
                AtMuxStartReq();
            }
        }
        break;
        /* ����Э�� */
    case MUX_UIH_SNC:
        MuxTrace(MUX_LOG_LEVEL_2, "UIH frame Type = MUX_UIH_SNC");

        /* ��UIH��ȡ��DLC��ʶ */
        g_MuxDlcMng.ucDlciIndex = (g_MuxFrameMng.FrameInfo.UihInfo.pData[2] >> 2) - 1;
        if (MUX_DLC_NUM <= g_MuxDlcMng.ucDlciIndex)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Dlci out of range,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* C/R���,��Ϊ1,���Դ���Ϣ */
        if (1 == (ucType & 0x02))
        {
            MuxTrace(MUX_LOG_LEVEL_1, "C/R check fail,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* ��ǰû�н��з���Э�����̣����Դ���Ϣ */
        if (MUX_FLOW_SERVICE_NEGOTIATION != g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Unsuitable Flow,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* ��·DLC����Э��������ɣ�����MUX_DLC_DATA_1�����ؿ������� */
        g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
        g_MuxDlcMng.Dlc0.ucDlciFlowOver = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci;
        g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId = MUX_FLOW_APL_FC_ON;
        MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d Service Negotiation Command successfully", g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci);
        break;
        /* RTR���� */
    case MUX_UIH_MSC:
        MuxTrace(MUX_LOG_LEVEL_2, "UIH frame Type = MUX_UIH_MSC");

        g_MuxDlcMng.ucDlciIndex = (g_MuxFrameMng.FrameInfo.UihInfo.pData[2] >> 2) - 1;
        if (MUX_DLC_NUM <= g_MuxDlcMng.ucDlciIndex)
        {
            MuxTrace(MUX_LOG_LEVEL_1, "Dlci out of range,Ignore this frame");
            return (MUX_CONTINUE);
        }

        /* C/R���,��Ϊ0,�յ�����ResponseMsc */
        if (0 == (ucType & 0x02))
        {
            /* ��ǰû�н������ؿ������̣����Դ���Ϣ */
            if ((MUX_FLOW_APL_FC_ON != g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId) && 
                (MUX_FLOW_APL_FC_OFF != g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId))
            {
                MuxTrace(MUX_LOG_LEVEL_1, "Unsuitable Flow,Ignore this frame");
                return (MUX_CONTINUE);
            }

            MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d Modem Status Command successfully", g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci);

            /* RTR���ؿ����ɹ����������һ·DLC������� */
            if (g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci < MUX_DLC_DATA_2)
            {
                /* ����DLC2��DLC5�Ľ������� */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci + 1;
                g_MuxDlcMng.Dlc[g_MuxDlcMng.Dlc0.ucDlciFlowOver - 1].ucFlowId = MUX_FLOW_DLC_ESTABLISH;
            }
            else if (MUX_DLC_DATA_2 == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci)
            {
                /* MUX_DLC_DATA_2������ɣ�DLC��ʼ������ */
                g_MuxDlcMng.Dlc0.ucFlowId = MUX_FLOW_NONE;
                g_MuxDlcMng.Dlc0.ucDlciFlowOver = MUX_DLC0;
            }
            else
            {
                /* �յ����ڴ���ResponseMsc��������Ϣ */
                MuxTrace(MUX_LOG_LEVEL_1, "Unsuitable Flow,Ignore this frame");
                return (MUX_CONTINUE);
            }
        }
        /* C/R���,��Ϊ1,�յ�����CommandMsc */
        else
        {
            MuxTrace(MUX_LOG_LEVEL_2, "Dlci %d receive Modem Status Command", g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ParaInfo.ucDlci);

            /* ����V24 */
            g_MuxFrameMng.FrameInfo.ucV24 = g_MuxFrameMng.FrameInfo.UihInfo.pData[3];

            /* ���������ж� */
            if ((0 != (g_MuxFrameMng.FrameInfo.ucV24 & 0x02))
                || (0 == (g_MuxFrameMng.FrameInfo.ucV24 & 0x04))
                || (0 == (g_MuxFrameMng.FrameInfo.ucV24 & 0x08)))
            {
                SendUihRTROFFResponse();
                return (MUX_CONTINUE);
            }
            else
            {
                SendUihRTRONResponse();
                if (0 != g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].AtToUartList.usBufferCnt)
                {
                    /* �л������ݣ���֡���� */
                    usCnt = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].AtToUartList.usBufferCnt;
                    pStTmp = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].AtToUartList.Buffer;
                    for (i = 0; i < usCnt; i++)
                    {
                        UihFrameCreate(pStTmp[i].pData, pStTmp[i].usLen);
                        /* ɾ������ */
                        MuxFree(pStTmp[i].pData);
                        pStTmp[i].pData = NULL;
                        pStTmp[i].usLen = 0;
                    }
                    MuxMemSet((char *)&g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].AtToUartList, 
                        0, sizeof(ST_MUX_BUFFER));
                }
                else if (0 != g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].PppToUartData.usBufferCnt)
                {
                    /* PPP���������Ƿ�ֱ�ӷ��ͣ��������� */
                    /* �ж�PPP����״̬ */
                    /* .............. */
                    usCnt = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].PppToUartData.usBufferCnt;
                    pStTmp = g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].PppToUartData.Buffer;
                    for (i = 0; i < usCnt; i++)
                    {
                        UihFrameCreate(pStTmp[i].pData, pStTmp[i].usLen);
                        /* ɾ������ */
                        MuxFree(pStTmp[i].pData);
                        pStTmp[i].pData = NULL;
                        pStTmp[i].usLen = 0;
                    }
                    MuxMemSet((char *)&g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].PppToUartData, 
                        0, sizeof(ST_MUX_BUFFER));
                }
                return (MUX_CONTINUE);
            }
        }
        break;
    case MUX_UIH_PN:      /* ����Э�� */
    case MUX_UIH_CLD:     /* �ر�MUX */
    case MUX_UIH_FCon:    /* ��������ON */
    case MUX_UIH_FCoff:   /* ��������OFF */
        /* ����Ҫ�����UIH���� */
        MuxTrace(MUX_LOG_LEVEL_1, "Receive an unexpected UIH_FRAME Type,Ignore this frame");
        return (MUX_CONTINUE);
    default:
        /* C/R���Ϊ1,�յ�����Command��Ϣ����¼�����Ч���� */
        if (1 == (ucType & 0x02))
        {
            g_MuxFrameMng.FrameInfo.ucInvalidType = ucType;
            MuxTrace(MUX_LOG_LEVEL_1, "Receive a Command message with invalid UIH frame Type");
            return (MUX_TYPE_INVALID);
        }
        /* �յ��Ĳ���Command��Ϣ�����Դ���Ϣ */
        MuxTrace(MUX_LOG_LEVEL_1, "Receive an invalid UIH_FRAME Type,Ignore this frame");
        return (MUX_CONTINUE);
    }

    return (MUX_TRUE);
}

static void UihFrameCreate(char *pBuffer, unsigned short usBufferLen)
{
    unsigned short usFrameLen = 0;                      /* ����֡���� */
    unsigned char  *pDataFrame = NULL;                  /* ֡������ */
    unsigned char  ucFcs = 0;                           /* У��λ */
    unsigned char  i = 0;  
    unsigned char  n = 0;                               /* ����֡�ĸ��� */
    unsigned char  *pUihData = NULL;

    MuxTrace(MUX_LOG_LEVEL_3, "Function UihFrameCreate");

    if ((0 == usBufferLen) || (NULL == pBuffer))
    {/* �յ�������Ϊ�� */
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

    /* ��n��FRAME�������� */
    for (i = 0; i < n; i++)
    {
        /* ֡����ΪN1ʱ */
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
        /* ֡����С��N1ʱ */
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
        /* ����FRAME */
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
        /* ���� */
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
        /* AT���ݴ��� */
        if (MUX_FLOW_RECEIVE_DATA_FROM_ATC == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
        {
            if (ONLINE_DATA_MODE == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucMode)        /* ����û������ */
            {
                MuxDlcDataBuffer(pBuffer, usBufferLen);
                return MUX_FALSE;
            }
            else
            {
                return MUX_TRUE;
            }
        }
        /* PPP���ݴ��� */
        else if (MUX_FLOW_RECEIVE_DATA_FROM_PPP == g_MuxDlcMng.Dlc[g_MuxDlcMng.ucDlciIndex].ucFlowId)
        {

           return MUX_TRUE;
    
        }
        else
        {
            ;
        }
    }
    else                                                                                    /* ����û������ */
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
        if (MUX_MAX_CNT_OF_BUFFER == AtDataBuffer->usBufferCnt)     /* �жϻ������Ƿ����� */
        {
            MuxTrace(MUX_LOG_LEVEL_1, "the buffer of AT full, discard this data!");
            return;/* ������ô���� */
        }
        /* AT���ݴ��뻺���� */
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
        if (MUX_MAX_CNT_OF_BUFFER == PppDataBuffer->usBufferCnt)    /* �жϻ������Ƿ����� */
        {
            MuxTrace(MUX_LOG_LEVEL_1, "the buffer of PPP full, discard this data!");
            return;/* ������ô���� */
        }
        /* PPP���ݴ��뻺���� */
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

#ifdef _MUX_LOG_BY_ITSELF
static void OpenLogFile()
{
    char        cName[260] = {0};
    char        cTimeName[260] = {0};
    time_t      tCurrTime;
    struct tm   *Now;
    
    
    /* �ļ��õ�ǰϵͳʱ������ */
    tCurrTime = time(NULL);
    Now = localtime(&tCurrTime);

    strcat( cName, MUX_TRACE_OUTPUT_PATH);
    strftime( cTimeName, 128, "%Y%m%d_%H%M%S_", Now);
    strcat( cName, cTimeName);
    strcat( cName, MUX_LOG_FILE_NAME);

    /* ����һ������д����ı��ļ� */
    pLogFile = fopen(cName, "w+");
    //pLogFile = fopen(MUX_LOG_FILE_NAME, "w+");

    /* �ļ�û�гɹ���                 */
    if (NULL == pLogFile)
    {
//        printf("cannot open log file!\n");
        //MuxTrace(MUX_LOG_LEVEL_1, "cannot open log file!\n");
        //exit(0);
    }

    return;
}
#endif

// static void ReadRcLoop()
// {
    // int     nRead = 0;
    // void*   pData = NULL;
    // int     i;
    // MuxTrace(MUX_LOG_LEVEL_1, "Thread ReadRcLoop is running");
    // while(1)
    // {
        // nRead = read(RcFd_Master, RcReadBuf, MAX_RC_MSG_LEN);
        // if(nRead > 0)
        // {
            // MuxTrace(MUX_LOG_LEVEL_2, "nRead = %d", nRead);
            // RcReadBuf[nRead] = 0;
            // AtMuxDataReq(RcReadBuf, nRead);    
        // }
        // else
        // {
            // MuxErrorLog("In RcReadBuf read error");
            // sleep(2);
        // }
    // }
    // return;
// }



static void ReadRilLoop()
{
    int     nRead = 0;
    void*   pData = NULL;
    int     i;
    MuxTrace(MUX_LOG_LEVEL_1, "Thread ReadRilLoop is running");
    while(1)
    {
        nRead = read(RilFd_Master, RilReadBuf, MAX_RIL_MSG_LEN);
        if(nRead > 0)
        {
            MuxTrace(MUX_LOG_LEVEL_2, "nRead = %d", nRead);
            RilReadBuf[nRead] = 0;
            AtMuxDataReq(RilReadBuf, nRead);    
        }
        else
        {
            MuxErrorLog("In ReadRilLoop read error");
            sleep(2);
        }
    }
    return;
}

// static void ReadPppLoop()
// {
    // int     nRead = 0;
    // void*   pData = NULL;
    // char*   pPppSlaveName = NULL;
    // struct  termios options;
    // int     i = 0;
    
    // MuxTrace(MUX_LOG_LEVEL_1, "Thread ReadPppLoop is running");
    // while(1)
    // {
        // nRead = read(PppFd_Master, PppReadBuf, MAX_PPP_MSG_LEN);
        // if(nRead > 0)
        // {
            // i = 0;
            // PppMuxDataReq(PppReadBuf, nRead);    
        // }
        // else
        // {
            // MuxErrorLog("In ReadPppLoop read error");

            // sleep(2);
            // i++;
            // if(i == 3)
            // {
                // unlink(PPP_LINK_FILE);
                ////close(PppFd_Master);
                // PppFd_Master = -1;
                // MuxTrace(MUX_LOG_LEVEL_2, "Close PppFd_Master = %d!", PppFd_Master);
                // usleep(100);
                
                ////system("rm /dev/ppppts");
                ////unlink(PPP_LINK_FILE);
                // if(0 > (PppFd_Master = open(PTMASTER, O_RDWR)))
                // {
                    // MuxErrorLog("open /dev/ptmx error");
                    // perror("open /dev/ptmx error");
                    // exit(EXIT_FAILURE);
                // }
                // MuxTrace(MUX_LOG_LEVEL_2, "open /dev/ptmx return ID = %d", PppFd_Master);

                // tcgetattr(PppFd_Master, &options); //get the parameters
                // options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
                // options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
                // options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
                // tcsetattr(PppFd_Master, TCSANOW, &options); //set the parameters
                    
                // grantpt(PppFd_Master);  /* change permission of slave */
                // unlockpt(PppFd_Master); /* unlock slave */

                // pPppSlaveName = ptsname(PppFd_Master); //get slave name
                // if (pPppSlaveName == NULL) 
                // {
                    // MuxTrace(MUX_LOG_LEVEL_1, "Get pts name failed");
                    // exit (EXIT_FAILURE);
                // }
                // MuxTrace(MUX_LOG_LEVEL_2, "Slave name for PPP is %s", pPppSlaveName);
                                
                // unlink(PPP_LINK_FILE);
                // if(symlink(pPppSlaveName, PPP_LINK_FILE) < 0)
                // {
                    // MuxErrorLog("symlink error");
                    // perror("symlink error");
                    // exit(EXIT_FAILURE);
                // }
                // i = 0;
                // MuxTrace(MUX_LOG_LEVEL_2, "PPP restart complete!");
            // }
        // }
    // }    
    // return;
// }

static void set_serial_dev_attr( int fd )
{
        int ret_val = 0;
        struct termios  ios;
        speed_t speed = B115200;
    
        ret_val = tcgetattr( fd, &ios );
        //CHECK_SYS_ERR( ret_val, "tcgetattr" );
    
        /* Set the baudrate. */
        if( !ret_val )
        {
            ret_val = cfsetispeed( &ios, speed );
            //CHECK_SYS_ERR( ret_val, "cfsetispeed" );
            //RLOGD("set_serial_dev_attr(),cfsetispeed OK\n");
        }
        if( !ret_val)
        {
            ret_val = cfsetospeed( &ios, speed );
            //CHECK_SYS_ERR( ret_val, "cfsetospeed" );
            //RLOGD("set_serial_dev_attr(), cfsetospeed OK\n");
        }
    
        ios.c_iflag &= ~INLCR;  /* do not change '\n' to '\r' */
        ios.c_iflag &= ~ICRNL;  /* do not change '\r' to '\n' */
        ios.c_iflag &= ~IXON;   /* disable start/stop output control */
        ios.c_lflag = 0;        /* disable ECHO, ICANON, etc... */
        ios.c_oflag &= ~ONLCR;  /* do not change '\n' to '\r' '\n' */
        ios.c_oflag &= ~OCRNL;  /* do not change '\n' to '\r' */
        /* BEGIN: Added for PN:h_project */
        ios.c_cflag &= ~CSIZE;
        ios.c_cflag |= CS8;     
        ios.c_cflag &= ~PARENB;    /* Clear parity enable */
        ios.c_iflag &= ~INPCK;    /* Enable parity checking */     
        ios.c_cflag &= ~CSTOPB;     
#if 1//xqf
        ios.c_cflag |= CRTSCTS;                     /* enable the RTS/CTS flow control */
#else
        ios.c_cflag &= ~CRTSCTS;                    /* disable the RTS/CTS flow control */
#endif
        tcflush(fd, TCIFLUSH);
        /* END:  Added for PN:h_project */
        if( !ret_val)
        {
            ret_val = tcsetattr( fd, TCSANOW, &ios );
            //RLOGD("set_serial_dev_attr() ,tcsetattr OK\n");
            //CHECK_SYS_ERR( ret_val, "tcsetattr" );
        }
    
        //SetSpeed(fd, 115200);
        //SetParity(fd, 8, 1, 'N',0);
        //DisableEcho(fd);
	#if 0 
        struct termios  ios;
        tcgetattr( fd, &ios );
        ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
        ios.c_cflag = CRTSCTS| CS8|CREAD|CLOCAL;
    
        /* add by zhaohehui 2015.10.20 start*/
        ios.c_iflag &= ~ICRNL;  /* disable ''/r' to '/n' */
        ios.c_iflag &= ~INLCR;   /* disable  '/n''to'/r' */ 
        /* add by zhaohehui 2015.10.20 end*/
                    
        cfsetispeed(&ios, B115200);
        cfsetospeed(&ios, B115200);
        tcsetattr( fd, TCSANOW, &ios ); 
	#endif
}
static void ReadUartLoop()
{
    int     nRead = 0;
    void*   pData = NULL;
    struct termios  options;
    
    MuxTrace(MUX_LOG_LEVEL_1, "Thread ReadUartLoop is running");

    //UART open
    if(0 > (UartFd = open(DEVICE_UART, O_RDWR)))
    {
        MuxErrorLog("open /dev/ttyS2 error");
        perror("open /dev/ttyS2 error");
        exit(EXIT_FAILURE);
    }
    MuxTrace(MUX_LOG_LEVEL_1, "Open device %s success", DEVICE_UART);
    set_serial_dev_attr(UartFd);
    //UART read
    while(1)
    {
        nRead = read(UartFd, UartReadBuf, MAX_UART_MSG_LEN);
        if(nRead > 0)
        {
            UartMuxDataInd(UartReadBuf, nRead);
        }
        else
        {
            MuxErrorLog("In ReadUartLoop read error");
            sleep(1);
        }
    }    
    return;
}

static void signal_treatment(	int param)
{
	switch (param)
	{
	case SIGINT:
        MuxTrace(MUX_LOG_LEVEL_1, "Signal SIGINT received");
        main_exit_signal = 1;
	    break;
	case SIGTERM:
        MuxTrace(MUX_LOG_LEVEL_1, "Signal SIGTERM received");
        main_exit_signal = 1;
	    break;
	default:
    	break;
	}
    return;
}

static void CloseDown()
{
    
#ifdef _MUX_LOG_BY_ITSELF
    fclose(pLogFile);
#endif
    //TBD;
    //Close MUX, all devices, etc.
}

int main(int argc,char *argv[])
{
  //  char*       pRcSlaveName = NULL;
    char*       pRilSlaveName = NULL;
  //  char*       pPppSlaveName = NULL;

    struct termios options;

  //  system("rm /data/local/log/*HlMux.log");
    
#ifdef _MUX_LOG_BY_ITSELF
    OpenLogFile();
#endif
    
    MuxTrace(MUX_LOG_LEVEL_1, "MUX version 2015.12.01 17:50", RilFd_Master);

    //delete link file
    system("rm /dev/rcpts");
    system("rm /dev/rilpts");
    system("rm /dev/ppppts");
      
    //Creat pseudo-terminal for RC
    // if(0 > (RcFd_Master = open(PTMASTER, O_RDWR)))
    // {
        // MuxErrorLog("open /dev/ptmx error");
        // perror("open /dev/ptmx error");
        // exit(EXIT_FAILURE);
    // }
    // MuxTrace(MUX_LOG_LEVEL_2, "open /dev/ptmx return ID = %d", RcFd_Master);

    // tcgetattr(RcFd_Master, &options); //get the parameters
    // options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
    // options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    // options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
    // tcsetattr(RcFd_Master, TCSANOW, &options); //set the parameters
        
    // grantpt(RcFd_Master);  /* change permission of slave */
    // unlockpt(RcFd_Master); /* unlock slave */

    // pRcSlaveName = ptsname(RcFd_Master); //get slave name
    // if (pRcSlaveName == NULL) 
    // {
        // MuxTrace(MUX_LOG_LEVEL_1, "Get pts name failed");
        // exit (EXIT_FAILURE);
    // }
    // MuxTrace(MUX_LOG_LEVEL_2, "Slave name for RIL is %s", pRcSlaveName);
    
    // if(symlink(pRcSlaveName, RC_LINK_FILE) < 0)
    // {
        // MuxErrorLog("symlink error");
        // perror("symlink error");
        // exit(EXIT_FAILURE);
    // }


    //Creat pseudo-terminal for RIL
    if(0 > (RilFd_Master = open(PTMASTER, O_RDWR)))   // zdh  shebei dakai
    {
        MuxErrorLog("open /dev/ptmx error");
        perror("open /dev/ptmx error");
        exit(EXIT_FAILURE);
    }
    MuxTrace(MUX_LOG_LEVEL_2, "open /dev/ptmx return ID = %d", RilFd_Master);

    tcgetattr(RilFd_Master, &options); //get the parameters
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
    options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
    tcsetattr(RilFd_Master, TCSANOW, &options); //set the parameters
        
    grantpt(RilFd_Master);  /* change permission of slave */
    unlockpt(RilFd_Master); /* unlock slave */

    pRilSlaveName = ptsname(RilFd_Master); //get slave name
    if (pRilSlaveName == NULL) 
    {
        MuxTrace(MUX_LOG_LEVEL_1, "Get pts name failed");
        exit (EXIT_FAILURE);
    }
    MuxTrace(MUX_LOG_LEVEL_2, "Slave name for RIL is %s", pRilSlaveName);
   
    if(symlink(pRilSlaveName, RIL_LINK_FILE) < 0)
    {
        MuxErrorLog("symlink error");
        perror("symlink error");
        exit(EXIT_FAILURE);
    }

    //Creat pseudo-terminal for PPP
    // if(0 > (PppFd_Master = open(PTMASTER, O_RDWR)))
    // {
        // MuxErrorLog("open /dev/ptmx error");
        // perror("open /dev/ptmx error");
        // exit(EXIT_FAILURE);
    // }
    // MuxTrace(MUX_LOG_LEVEL_2, "open /dev/ptmx return ID = %d", PppFd_Master);

    // tcgetattr(PppFd_Master, &options); //get the parameters
    // options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
    // options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    // options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
    // tcsetattr(PppFd_Master, TCSANOW, &options); //set the parameters
        
    // grantpt(PppFd_Master);  /* change permission of slave */
    // unlockpt(PppFd_Master); /* unlock slave */

    // pPppSlaveName = ptsname(PppFd_Master); //get slave name
    // if (pPppSlaveName == NULL) 
    // {
        // MuxTrace(MUX_LOG_LEVEL_1, "Get pts name failed");
        // exit (EXIT_FAILURE);
    // }
    // MuxTrace(MUX_LOG_LEVEL_2, "Slave name for PPP is %s", pPppSlaveName);
    
    // if(symlink(pPppSlaveName, PPP_LINK_FILE) < 0)
    // {
        // MuxErrorLog("symlink error");
        // perror("symlink error");
        // exit(EXIT_FAILURE);
    // }    

    MuxInit();    //MUX init   //zdh  bixu xian zhixing 
    
    // Creat reveive thread
   // if(pthread_create(&RcRcvThread, NULL, (void*)ReadRcLoop, NULL) == -1)
   // {
       // MuxTrace(MUX_LOG_LEVEL_1, "pthread_create Rc receive thread failed");
       // perror("pthread_create Rc receive thread failed");
       // exit(EXIT_FAILURE);
   // }
    
    if(pthread_create(&RilRcvThread, NULL, (void*)ReadRilLoop, NULL) == -1)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "pthread_create RIL receive thread failed");
        perror("pthread_create RIL receive thread failed");
        exit(EXIT_FAILURE);
    }

    // if(pthread_create(&PppRcvThread, NULL, (void*)ReadPppLoop, NULL) == -1)
    // {
        // MuxTrace(MUX_LOG_LEVEL_1, "pthread_create PPP receive thread failed");
        // perror("pthread_create PPP receive thread failed");
        // exit(EXIT_FAILURE);
    // }

    if(pthread_create(&UartRcvThread, NULL, (void*)ReadUartLoop, NULL) == -1)
    {
        MuxTrace(MUX_LOG_LEVEL_1, "pthread_create UART receive thread failed");
        perror("pthread_create UART receive thread failed");
        exit(EXIT_FAILURE);
    }

    usleep(200);

    pthread_mutex_init(&UartSendMutex, NULL);
    pthread_mutex_init(&MultiSend, NULL);

    //signals treatment
	signal(SIGINT, signal_treatment);
	signal(SIGTERM, signal_treatment);

    while(main_exit_signal == 0)
    {
    	sleep(2);
    }

    MuxTrace(MUX_LOG_LEVEL_1, "MUX main exit");

    CloseDown();
   
    return 0;
}

 

 
