#ifndef __AT_APP_H
#define __AT_APP_H
#include "stm32f10x.h"
#include <string.h>

#define FLOW_CONTROL_TIME 1000
#define BD_REC_TIME 30000

#define WAIT_AT_TIME_MS_L1 500
#define WAIT_AT_TIME_MS_L2 8000

#define WaitTimeOut(x) {\
	if ((SysTime - TimeTemp) > x)\
	{\
		return FLAG_TIME_OUT;\
	}\
}\
TimeTemp = SysTime;




#define GPIO_Pin_PW_BD GPIO_Pin_4
#define GPIO_PW_BD GPIOC
#define PW_BD_ON GPIO_SetBits(GPIO_PW_BD,GPIO_Pin_PW_BD)
#define PW_BD_OFF GPIO_ResetBits(GPIO_PW_BD,GPIO_Pin_PW_BD)

#define GPIO_Pin_PW_AT GPIO_Pin_5
#define GPIO_PW_AT GPIOC
#define PW_AT_ON GPIO_SetBits(GPIO_PW_AT,GPIO_Pin_PW_AT)
#define PW_AT_OFF GPIO_ResetBits(GPIO_PW_AT,GPIO_Pin_PW_AT)


#define GPIO_Pin_USB_EN GPIO_Pin_1
#define GPIO_USB_EN GPIOB
#define USB_EN_OFF GPIO_SetBits(GPIO_USB_EN,GPIO_Pin_USB_EN)
#define USB_EN_ON GPIO_ResetBits(GPIO_USB_EN,GPIO_Pin_USB_EN)







typedef u32 SYSTIME;
extern volatile SYSTIME SysTime;
extern SYSTIME TimeTemp;


extern u8 MuxOpenFlag;
volatile extern u8 AT_OpenInitState;
extern u8 AT_OpenSucceed;
#define AT_OPEN_INIT_STATE_0 0
#define AT_OPEN_INIT_STATE_READY 1
//#define AT_OPEN_INIT_STATE_FINISH 2



#define FLAG_TIME_OUT 99
#define FLAG_SEND_FAIL 98

#define WAIT_INIT_WAIT (1000*10*30)//30s



#define USB_EP2_REC_NUM 640
#define USB_EP1_SEND_NUM USB_EP2_REC_NUM

#define USB_DATA_SIZE USB_EP2_REC_NUM


#define USB_EP3_REC_NUM 50
#define USB_EP3_SEND_NUM USB_EP3_REC_NUM

#define I2S_REC_SIZE	(USB_EP1_SEND_NUM / 2 * 1)
#define I2S_SEND_SIZE	(USB_EP2_REC_NUM / 2 * 1)

#define USB_HEAD_NUM 6

#define VOICE_CALL_STATE_IDLE 0
#define VOICE_CALL_STATE_CALL_SEND 1
#define VOICE_CALL_STATE_CALL_START 2
#define VOICE_CALL_STATE_CALL_CONNECT 3
#define VOICE_CALL_STATE_CALL_ACCEPT 4
#define VOICE_CALL_STATE_RECEIVE_GET 11
#define VOICE_CALL_STATE_RECEIVE_START 12
#define VOICE_CALL_STATE_RECEIVE_AGREE 13
#define VOICE_CALL_STATE_RECEIVE_CONNECT 14

#define LEN_NUM 20

struct voice 
{
	u8 num[LEN_NUM + 1];	//对方号码
	u8 id;			//通话标识ID
	u8 dir;			//通话方向（发起方）
	u8 state;		//通话状态
};
extern struct voice VoiceHandle[1];

void Gpio_Init(void);

u8 UnsolicitedHandle(u8 *com);
void UnexpectedHandle(u8 *com);
u8 isUnsolicited(const u8 *line);
void PrintAllComAndMsm(void);

void PowerOnInit(void);

size_t SeekStr(const u8 **lelms,size_t num, u8 *line,u8 opt);
u8 WaitForStr(const u8 **lelms,size_t num,u16 timeout_ms,u8 opt);
u8 AT_VoiceCall(u8 *number);
u8 AT_VoiceReceive(void);
u8 AT_VoiceHangup(void);
u8 AT_Voice_112(void);
u8 AT_Init(void);
void AT_ComHandle(void);

u8 AT_SendComReq(u8 * str);
u8 AT_SendDataReq(u8 * data,u16 len);
u8 AT_MessageSend(u8 len,u8 * mes);

u8 AT_RescueSet(u8 *par);
u8 AT_RescueOpen(void);
u8 AT_RescueClose(void);
u8 AT_BeidouOpen(void);
u8 AT_BeidouClose(void);
u8 AT_BeidouPostion(void);
u8 AT_ModeCheck(void);
u8 AT_ModeNormal(void);
u8 AT_ModeFlying(void);
u8 AT_ModeClose(void);

u8 AT_TaskHandle(void);
void TestComConvert(u8 * mes);
void MuxHandle(void);

u8 AT_RescueStateSend(void);

void DebugComConvert(u8 * mes);
void myitoa(s16 num,u8 * str,u8 sign);

u8 AT_RescueSetFre(u16 sendfre,u16 recfre);	//设置频点
u8 AT_RescueSetGain(u8 gain);								//设置增益
u8 AT_RescueSetCount(u16 count);						//设置发送次数
u8 AT_RescueSetInterval(u16 interval);			//设置发送间隔
u8 AT_RescueSetArmy(u8 army);								//设置军种
u8 AT_RescueSetOffset(s16 offset);					//设置频偏

u8 AT_RescueTestSendOpen(void);
u8 AT_RescueTestSendClose(void);
u8 AT_RescueMesSend(void);
	
	
void USB_MessageTest(void);
void USB_VoiceTest(void);
void CallStateHandle(void);
void DSCI_Handler(u8* com);
void SendCallState(void);
#endif
