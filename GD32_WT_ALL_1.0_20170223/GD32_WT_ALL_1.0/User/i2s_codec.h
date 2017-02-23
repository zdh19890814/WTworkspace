
#ifndef __I2S_CODEC_H
#define __I2S_CODEC_H
#include "include.h"

#define DMA_Channel_IIS_Rec DMA1_Channel4
#define DMA_Channel_IIS_Send DMA2_Channel2


extern uint16_t I2S_DMA_BuffRec1[I2S_REC_SIZE];
extern uint16_t I2S_DMA_BuffRec2[I2S_REC_SIZE];
extern uint16_t I2S_DMA_BuffSend1[I2S_SEND_SIZE];
extern uint16_t I2S_DMA_BuffSend2[I2S_SEND_SIZE];



extern uint16_t I2S_DMA_DoubleBufferSend[I2S_SEND_SIZE * 2];
extern uint16_t I2S_DMA_DoubleBufferRec[I2S_REC_SIZE * 2];

extern uint16_t *I2S_DMA_BufferRec1;
extern uint16_t *I2S_DMA_BufferRec2;
extern uint16_t *I2S_DMA_BufferSend1;
extern uint16_t *I2S_DMA_BufferSend2;






extern uint8_t I2S_DMA_BuffRec_NO;
extern volatile u8 I2S_DMA_BuffRec_OK;
extern volatile u8 I2S_DMA_Buff1Rec_OK;
extern volatile u8 I2S_DMA_Buff2Rec_OK;

extern uint8_t I2S_DMA_BuffSend_NO;
extern volatile u8 I2S_DMA_Buff1Send_OK;
extern volatile u8 I2S_DMA_Buff2Send_OK;
extern u16 BufSendIndex;
extern  DMA_InitTypeDef DMA2_InitStructure;
extern  DMA_InitTypeDef DMA1_InitStructure;






typedef enum
{
  Valid_WAVE_File = 0,
  Unvalid_RIFF_ID,
  Unvalid_WAVE_Format,
  Unvalid_FormatChunk_ID,
  Unsupporetd_FormatTag,
  Unsupporetd_Number_Of_Channel,
  Unsupporetd_Sample_Rate,
  Unsupporetd_Bits_Per_Sample,
  Unvalid_DataChunk_ID,
  Unsupporetd_ExtraFormatBytes,
  Unvalid_FactChunk_ID
} ErrorCode;
 
/* Exported constants --------------------------------------------------------*/

/* Codec output DEVICE */
#define OutputDevice_SPEAKER          1
#define OutputDevice_HEADPHONE        2  
#define OutputDevice_BOTH             3

/* VOLUME control constants */
// #define DEFAULT_VOL                   0x52
// #define VolumeDirection_HIGH          0xF
// #define VolumeDirection_LOW           0xA
// #define VolumeDirection_LEVEL         0x0
// #define VOLStep                       4 

// /* Forward and Rewind constants */
// #define STEP_FORWARD                  2 /* 2% of wave file data length*/
// #define STEP_BACK                     6 /* 6% of wave file data length*/

/* Codec POWER DOWN modes */
// #define CodecPowerDown_HW             1
// #define CodecPowerDown_SW 	          2

/* Audio Play STATUS */
// #define AudioPlayStatus_STOPPED       0
// #define AudioPlayStatus_PLAYING	      1
// #define AudioPlayStatus_PAUSED		    2	 

// /* MUTE commands */ 
// #define MUTE_ON                       1
// #define MUTE_OFF							        0

/* I2S configuration parameters */
#define I2S_STANDARD                  I2S_Standard_PCMShort
#define I2S_MCLKOUTPUT                I2S_MCLKOutput_Enable

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
 
  /* CONFIGURATION */
uint32_t I2S_CODEC_Init(uint32_t OutputDevice, uint32_t Address);
//void I2S_CODEC_ReplayConfig(uint32_t Repetions);

	/* AUDIO CONTROL */
// uint32_t I2S_CODEC_Play(uint32_t AudioStartPosition);
// uint32_t I2S_CODEC_Pause(void);
// uint32_t I2S_CODEC_Stop(void);	
// uint32_t I2S_CODEC_ControlVolume(uint32_t direction, uint8_t volume);
//void I2S_CODEC_Mute(uint32_t Command);
//void I2S_CODEC_ForwardPlay(uint32_t Step);
//void I2S_CODEC_RewindPlay(uint32_t Step);

   /* EXTRA CONTROLS */								 
//void I2S_CODEC_PowerDown(uint32_t CodecPowerDown_Mode);
//void I2S_CODEC_Reset(void);
//uint32_t I2S_CODEC_SpeakerHeadphoneSwap(uint32_t CODEC_AudioOutput, uint32_t Address);
//uint8_t GetVar_CurrentVolume(void);

/* Medium Layer Codec Functions ----------------------------------------------*/						
//void I2S_CODEC_DataTransfer(void);
//void I2S_CODEC_UpdateStatus(void);
//uint32_t GetVar_DataStartAddr(void);	
//ErrorCode I2S_CODEC_WaveParsing(uint8_t* HeaderTab);
// uint32_t GetVar_CurrentOutputDevice(void);

// uint32_t GetVar_AudioDataIndex(void);
// void SetVar_AudioDataIndex(uint32_t value);
// void ResetVar_AudioDataIndex(void);
//void IncrementVar_AudioDataIndex(uint32_t IncrementNumber);
//void DecrementVar_AudioDataIndex(uint32_t DecrementNumber);

// uint32_t GetVar_AudioReplay(void);
// void Decrement_AudioReplay(void);

// uint32_t GetVar_AudioPlayStatus(void);
// uint32_t SetVar_AudioPlayStatus(uint32_t Status);

// uint16_t GetVar_i2saudiofreq(void);
// uint32_t GetVar_AudioDataLength(void);

/* Low Layer Codec Function --------------------------------------------------*/
// uint32_t SetVar_SendDummyData(void);
// uint32_t ResetVar_SendDummyData(void);
// uint32_t GetVar_SendDummyData(void);
//uint32_t AudioFile_Init(void);

void I2S_GPIO_Config(void); 
void I2S_Config(uint16_t Standard, uint16_t MCLKOutput, uint16_t AudioFreq);
//uint32_t CODEC_Config(uint16_t AudioOutput, uint16_t I2S_Standard, uint16_t I2S_MCLKOutput, uint8_t Volume);

//uint32_t Media_Init(void);
//void Media_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
//void Media_StartReadSequence(uint32_t ReadAddr);
//uint16_t Media_ReadHalfWord(uint32_t Offset);
//uint8_t Media_ReadByte(uint32_t Offset);

void delay(__IO uint32_t nCount); 

void Speaker_Config(void);
void Audio_Config(void);
void I2S3DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void I2S3DMA_Enable(DMA_Channel_TypeDef*DMA_CHx);

void I2S2DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void I2S2DMA_Enable(DMA_Channel_TypeDef*DMA_CHx);

void I2S_ConfigS(uint16_t Standard, uint16_t MCLKOutput, uint16_t AudioFreq);

void I2S_DMA_Init(void);
void I2S_DMA_Init_D(void);
#endif /* __I2S_CODEC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
