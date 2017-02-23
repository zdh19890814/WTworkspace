/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_desc.c
* Author             : LKZ
* Version            : V1.0
* Date               : 07/05/2016
* Description        : 各个描述符配置
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"


/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/



/* USB Standard Device Descriptor */
const uint8_t CustomHID_DeviceDescriptor[CUSTOMHID_SIZ_DEVICE_DESC] =
{
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, //设备描述符的类型编号
    0x00,                       /*bcdUSB */
    0x02,
    0x00,                       /*bDeviceClass*/
    0x00,                       /*bDeviceSubClass*/
    0x00,                       /*bDeviceProtocol*/
    0x40,                       /*bMaxPacketSize40*/
    0x71, 0x04,					// 设备ID,VID
    0x98, 0x09,					// 设备ID,PID   卫通模块的PID
    0x00, 0x02,					// 设备系列号
    0, 0, 0,					// 索引    
    0x01                        /*bNumConfigurations*/
}; /* CustomHID_DeviceDescriptor */

const uint8_t CustomHID_ConfigDescriptor[CUSTOMHID_SIZ_CONFIG_DESC] =
{
    0x09, /* bLength: Configuation Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, //配置描述符类型的编码
    CUSTOMHID_SIZ_CONFIG_DESC,     
    /* wTotalLength: Bytes returned */
    0x00,
    0x01,         /* bNumInterfaces: 1 interface */
    0x01,         /* bConfigurationValue: Configuration value */
    0x00,         /* iConfiguration: Index of string descriptor describing
                                 the configuration*/
    0xC0,         /* bmAttributes: Bus powered */
                  /*Bus powered: 7th bit, Self Powered: 6th bit, Remote wakeup: 5th bit, reserved: 4..0 bits */
    0x32,         // MaxPower 500 mA: 从总线获得最大流量
    
    
    /************** Descriptor of Custom HID interface ****************/
    /* 09 */
    0x09,         /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,//接口描述符类型的编码
    0x00,         /* bInterfaceNumber: Number of Interface */
    0x00,         /* bAlternateSetting: Alternate setting */
    0x04,         /* bNumEndpoints */
    0xdc,         /* bInterfaceClass: Class code = 0DCH */
    0xa0,         /* bInterfaceSubClass : Subclass code = 0A0H */
    0xb0,         /* nInterfaceProtocol : Protocol code = 0B0H */
    4,            //接口的字符串的索引值
    
    
    /******************** endpoint descriptor ********************/
    /* 18 */
    0x07,         /* endpoint descriptor length = 07H */
    USB_ENDPOINT_DESCRIPTOR_TYPE, //端点描述符类型的编码
    0x81,         //端点1 IN
    0x02,					//批量传输类型
    0x40,0x00,    //传输长度64字节
    0x00,         //传输时间间隔


		
    0x07,         /* endpoint descriptor length = 07H */
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* endpoint descriptor type = 05H */
    0x02,         //端点2 OUT
    0x02,					//批量传输类型
    0x40,0x00,    //传输长度
    0x00,         //传输时间间隔
		
		 0x07,         /* endpoint descriptor length = 07H */
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* endpoint descriptor type = 05H */
    0x83,         //端点3 IN
    0x03,					//中断传输类型
    0x40,0x00,    //最大传输长度
    0x01,         //中断传输查询时间间隔1ms


		
    0x07,         /* endpoint descriptor length = 07H */
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* endpoint descriptor type = 05H */
    0x03,         //端点3 OUT
    0x03,					//中断传输类型
    0x40,0x00,    //最大传输长度
    0x01,         //中断传输查询时间间隔1ms

}; /* CustomHID_ConfigDescriptor */



/* 字符串描述符（可选）*/
const uint8_t CustomHID_StringLangID[CUSTOMHID_SIZ_STRING_LANGID] =
{
    CUSTOMHID_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
}; /* LangID = 0x0409: U.S. English */


const uint8_t CustomHID_StringVendor[CUSTOMHID_SIZ_STRING_VENDOR] =
{
    CUSTOMHID_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    // Manufacturer: "STMicroelectronics" 
    'M', 0, 'y', 0, 'U', 0,'S', 0,'B', 0, '_', 0, 'H', 0,'I',0,'D',0
};


const uint8_t CustomHID_StringProduct[CUSTOMHID_SIZ_STRING_PRODUCT] =
{
    CUSTOMHID_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'B', 0, 'y', 0, ' ', 0, 'e', 0, 'm', 0, 'b', 0,'e',0,'d',0,'-',0,'n',0,'e',0,'t',0
};


uint8_t CustomHID_StringSerial[CUSTOMHID_SIZ_STRING_SERIAL] =
{
    CUSTOMHID_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'x', 0, 'x', 0, 'x', 0,'x', 0,'x', 0, 'x', 0, 'x', 0
};

const uint8_t MASS_StringInterface[16] =
  {
    MASS_SIZ_STRING_INTERFACE,
    0x03,
    /* Interface 0: "ST Mass" */
    'S', 0, 'T', 0, ' ', 0, 'M', 0, 'a', 0, 's', 0, 's', 0
  };
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

