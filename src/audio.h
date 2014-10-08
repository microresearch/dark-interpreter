/*
 * audio.h - audio processing routines
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#ifdef PCSIM
#define int16_t unsigned short int
#else
#include "stm32f4xx.h"
#include "arm_math.h"
#endif

#define MAX_VILLAGERS 128
#define BUFF_LEN 128 // TEST! reduce to 16 
#define AUDIO_BUFSZ 32768 // was 32768

/*    typedef struct {
      u16 start;
      u16 wrap;
      u8 effect;
      u8 speed_step; // TODO: how do as fractional?
      u8 dir; // TODO: do as union or whatever for dir//flag for mirror
    } villagerr;*/

// test struct for multiple grains/villagers

    typedef struct {
      u16 start;
      u16 wrap;
      u16 sampleposwrite;
      u8 effect,delw,dirryw;
      u8 speed_step; // TODO: how do as fractional?
      u8 dir; // TODO: do as union or whatever for dir//flag for mirror
    } villagerr;


void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz);

#endif

