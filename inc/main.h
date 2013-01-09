/**
  ******************************************************************************
  * @file    main.h 
  * @author  
  * @version 
  * @date    
  * @brief   Header for main.c
  ******************************************************************************
*/
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio_codec.h"

#include <stdio.h>
#include "stm32f4xx_it.h"
    

 /*--------------------- Defines for the Audio process------------------------ */

#define _2PI                    6.283185307f
#define _PI						3.14159265f
#define _INVPI					0.3183098861f
#define SAMPLERATE              44000
#define FREQ1                   440.0f   // default carrier frequency
#define FREQ2                   8.0f     // default modulation frequency
#define VOL                     70
#define BUFF_LEN_DIV4           96 // 2ms latency at 48kHz
#define BUFF_LEN_DIV2           192
#define BUFF_LEN                384  /* Audio buffer length : count in 16bits half-words */
#define DELAYLINE_LEN           14000  // max delay in samples
#define DELAY                   13000  // actual delay (in samples)
#define DELAY_VOLUME            0.1f   // 0.3f
#define FEEDB                   0.4f   //0.4f
#define ON                      1
#define OFF                     0

#define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)



/* Exported functions ------------------------------------------------------- */

void TimingDelay_Decrement(void);
//void Delay(__IO uint32_t nTime);
void AudioBuffer_Init(void);
void AudioBuffer_LoadWave(void);
uint32_t Codec_TIMEOUT_UserCallback(void);
uint16_t randomNum(void);


#endif /* __MAIN_H */

/************************END OF FILE****/
