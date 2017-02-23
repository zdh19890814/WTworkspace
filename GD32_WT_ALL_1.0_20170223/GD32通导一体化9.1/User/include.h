#ifndef  __INCLUDE_H__
#include "app_cfg.h"

#define DEBUG_PRINT//使能调试打印
#define DEBUG_INPUT//使能调试输入
#define GD32//GD32、STM32选择
//#define MUX_MAIN//MUX放主函数解析还是中断
//#define MUX_ON1//打开MUX
#define I2S_ON_OFF//I2S根据通话状态打开关闭还是一直打开
#define I2S_HALF_BUFFER//I2S使用双buffer手动切换还是使用单buffer半满中断


#include "main.h"
#include "usart.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usbio.h"
#include "delay.h"
#include "app_cfg.h"
#include "string.h"
#include "wtp.h"
#include "AT_app.h"
#include "i2s_codec.h"
#include "led.h"
#include <string.h>
#include "AT_app.h"
#include "usart.h"
#include "wtp.h"
#include "usbio.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "i2s_codec.h"

#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "usart.h"
#include "AT_app.h"
#include "wtp.h"
#include "main.h"

#include "bds.h"

#include "delay.h"

#include "stm32f10x.h"
#include "wtp.h"
#include "usbio.h"
#include "usb_lib.h"
#include "media_process.h"
#include "i2s_codec.h"
#include "usart.h"
#include "main.h"
#include "AT_app.h"


#include "wtp.h"
#include "usart.h"
#include "AT_app.h"

#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "wtp_mux.h"
#include "tcpip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#define  __INCLUDE_H__
#endif
