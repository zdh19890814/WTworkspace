#include "include.h"
int main(void)
{
	Set_System();
	TickInit();
	MuxInit();
	LED_Init();
	Gpio_Init();
	USART2_DMA_Init();
	DebugUartInit();
	NVIC_AllInit();
	BD_UartInit();
	Set_USBClock();
#ifdef I2S_HALF_BUFFER
	I2S_DMA_Init_D();
#else
	I2S_DMA_Init();
#endif
	USB_Init();
	DebugOut("program is running\r\n");
	
	/*延时两秒再开启卫通模块使能，并初始化卫通模块，设置北斗模式*/
	delay_ms(2000);\
	PW_AT_ON;\
	PW_BD_ON;\
	DebugOut("modem power on\r\n");\
	PowerOnInit();\
	BDS_SET_Pattern(0);
	
	/*主循环*/
	while(1)
	{
		AT_ComHandle();			
		TaskHandle();	  	
		USB_RecData();  	
		BDS_Handle();	
	}
}
