/*
 * audio.h - audio processing routines
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#include "stm32f4xx.h"
#include "arm_math.h"

#define BUFF_LEN 128
#define AUDIO_BUFSZ 32768 // was 32768

extern int16_t audio_buffer[AUDIO_BUFSZ];
//extern int16_t writeloc[BUFF_LEN/2];
//extern int16_t readloc[BUFF_LEN/2];

void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz, uint16_t ht);

#endif

