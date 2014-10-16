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

#define MAX_VILLAGERS 64 // was 128
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
      u16 offset;
      u16 start;
      u16 wrap;
      u16 samplepos;
      u8 del,dirry;
      u8 speed, step; 
      u8 dir; // TODO: do as union or whatever for dir//flag for mirror
      u8 running;
    } villagerr;

    typedef struct {
      u16 start;
      u8 CPU;
      u16 wrap;
      u16 position;
      u8 del,dirry;
      u8 speed, step, cpu; 
      u8 dir; // TODO: do as union or whatever for dir//flag for mirror
      u8 running;
      signed char m_stack_pos;
      u8 m_stack[16];
      u16 m_reg16bit1;
      u8 m_reg8bit1,m_reg8bit2;
    } villager_generic;

void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz);

u16 xxrunleakystack(villager_generic *villager);
u16 xxrunbiota(villager_generic *villager);
u16 xxrun1(villager_generic *villager);
u16 xxrunworm(villager_generic *villager);
u16 xxrunstack(villager_generic *villager);
u16 xxrunbefunge(villager_generic *villager);
u16 xxrunlang(villager_generic *villager);
u16 xxrunbf(villager_generic *villager);
u16 xxrunturm(villager_generic *villager);
u16 xxrunca(villager_generic *villager);
u16 xxrunant(villager_generic *villager);
u16 xxrunca2(villager_generic *villager);
u16 xxrunhodge(villager_generic *villager);
u16 xxrunworm2(villager_generic *villager);
u16 xxrunleaky(villager_generic *villager);
u16 xxrunconvy(villager_generic *villager);
u16 xxrunplague(villager_generic *villager);
u16 xxrunmicro(villager_generic *villager);
u16 xxruncw(villager_generic *villager);
u16 xxrunmasque(villager_generic *villager);
#endif

