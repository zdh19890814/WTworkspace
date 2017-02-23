#ifndef  __WTP_MUX_H__
#define  __WTP_MUX_H__
#include "include.h"

#define _MUX_FCS_CHECK_OFF


/* MUX�궨�� */
typedef enum
{
    MUX_LOG_LEVEL_1 = 0,                /* �����ڷ��ش���ʱ���������ʹ�ӡ           */
    MUX_LOG_LEVEL_2,                    /* ��UART�������ӿ��յ���Ϣʱ������ʱ����������Ҫ��Ϣ��ӡ; ��Ҫ�¼���ʼ�ͽ���ʱ */
    MUX_LOG_LEVEL_3,                    /* ���롢�뿪����ʱ                         */
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
#define MUX_SERV_DATA                       0x03        /* DATA service                     *///��Ϊ��������
#define MUX_SERV_VOICE                      0x05        /* VOICE service                    */

/* MUX INFORMATION OF TIMER */                                                              
#define MUX_DEFAULT_TIME_OF_T1              10          /* 0.1s  ��λ��0.01s                */
#define MUX_DEFAULT_TIME_OF_T2              200         /* 2s(default 0.3s)  ��λ��0.01s    */

/* MUX COMMON INFORMATION */                                                                
#define MUX_MAX_SIZE_OF_FRAME               31          /* FRAME����Ĭ�ϳ��ȣ�31(default)    */
#define MUX_MAX_NUM_OF_RETRANS              3           /* �ش���������3  ��ʼ��ʱ��      */
#define MUX_FLAG_SEQUENCE_FIELD             0xF9        /* frame����ʼ����Flag�̶�ֵ        */
#define MUX_MAX_CNT_OF_BUFFER               (16 * 4)  /* MUX(UART)���յ���������ݳ���    */
#define MUX_UART_CNT_OF_BUFFER              (16 * 4)  /* UART���յ���������ݳ���         */

/* MUX PPP STATUS */                                                                        
#define DTX_READY                           0           /* frame����ʼ����Flag�̶�ֵ        */
#define DTX_STOP                            2                                               

/* MUX BUFFER SIZE */                                                                       
//#define MAX_PPP_DATA_BUFFER_SIZE            (4096*5)    /* PPP�ܷ��͵�������ݳ���          */
#define MAX_UART_BUFFER_SIZE                (128)      /* UART�ܷ��͵�������ݳ���         */
#define MAX_UART_RX_MEM_SIZE                (128)      /* UART���յ���������ݳ���         */

/* MUX MODE */
#define OFFLINE_AT_MODE                     0           /*                                  */
//#define ONLINE_AT_MODE                      1           /*                                */
#define ONLINE_DATA_MODE_ESTABLISHING       2           /*                                  */
#define ONLINE_DATA_MODE                    3           /* ����DLC_AT:����AT���ݣ�����DLC_DATA:����PPP���� */

/* define MAX Of Length Indicator */
#define MUX_MAX_LENGTH_INDICATOR            127         /* EA=1ʱLength Indicator��ָʾ����󳤶� */

/*         AT COMMAND                     */
#define SUPPORT_AT_NUM                      121 + 6


/*         test AT command lenth  */
//#define count               6

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
	ST_DLC_MNG          Dlc[MUX_DLC_NUM];       /* AT/Data/Voice DLC����, ucDlciIndex����   *///zl:��ͬ���¼�
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









void MuxInit(void);
void SendUihTestCommond(void);
void SendUihFcONCommond(void);
void SendSABMFrame(void);
void SendUihServiceNegCommond(void);
void SendUihRTRONCommond(void);
void MuxFree(char *pStr);
void *MuxMalloc(unsigned long ulen);
void MuxMemSet(char *pDest, char val, unsigned long ulen);
void MuxMemCopy(char *pDest, char *pSour, unsigned long ulen);
unsigned char SenderCreateFcs(unsigned char len, unsigned char *p);
u8 UartMuxDataReq(char *pUartSndBuff, unsigned short usUartLen);
void MuxSendUihNscResponse(void);
void UihFrameCreate(char *pBuffer, unsigned short usBufferLen);
void SendUihRTRONResponse(void);
u8 ChannelOpen(void);
unsigned char SenderCreateFcs(unsigned char len, unsigned char *p);
void MuxSend(char *sendbuffer, int len);
u8 AT_SendMux(u8 *sendbuffer);
u8 AT_SendDataMux(u8 *sendbuffer, u16 len);

int get(char *data, unsigned short len);
void analyze(char *buffer, unsigned short len);


extern int   receivelen;
extern char  *pTmpFrame;
extern char  *pFrameData;
//extern char  *receivebuffer;
extern char  message1[4];
extern char  message2[4];
extern char  message3[4];



#endif
