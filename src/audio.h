/*
 * audio.h - audio processing routines
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#ifdef PCSIM
#define int16_t signed short int
#define u16 unsigned short int
#else
#include "stm32f4xx.h"
#include "arm_math.h"
#endif

#define MAX_VILLAGERS 64 // was 128
#define BUFF_LEN 128 // TEST! reduce to 16 
#define AUDIO_BUFSZ 32768 // was 32768

    typedef struct {
      u8 whicheffect,speed,step;
      u16 instart,modstart,outstart;
      u16 inpos,modpos,outpos;// various counters
      u16 inwrap,modwrap,outwrap;
      u16  modifier;
      //      u8 running;
    } villager_effect;

    typedef struct {
      u16 length;
      u8 setting,inp;
    } villager_hardware;

    typedef struct {
      u16 length;
      u16 dataoffset;
      u16 knoboffset;
      int16_t samplepos;
      int16_t dirry;
      u8 speed, step;
      u8 dir;
    } villager_hardwarehaha;

    typedef struct {
      u16 length;
      u16 dataoffset;
      u16 knoboffset;
      int16_t samplepos;
      int16_t dirry;
      u8 speed, step;
      u8 dir;
    } villager_datagenwalker;

/// TODO: mirrors for above!

typedef struct {
  u16 kstart,kcompress,kwrap;
  u16 mstart,mcompress,mwrap;
  u8 mirrormod,mirrordel,mirrorspeed; // how mirror effects mainline start/wrap and samplepos
  u8 infected;
  u8 fingered; // what is input here as modifier

  u16 start,offset,wrap;
  u16 counterr;
  u16 samplepos;
  u8 del;
  int16_t dirry;
  u16 compress;
  u8 speed, step;
  float effect,effectinv; // now as value of effect! TODO!
  u8 dir,overlay; // TODO: do as union or whatever for dir//flag for mirror
  u8 running;
    } villagerr;

/*typedef struct {
  u16 start,wrap,compress;
  u8 mirrormod,mirrorspeed,mirrorstep; // how mirror effects mainline start/wrap and samplepos
  u8 infectedstart,infectedwrap;
  u8 fingered; // what is input here as modifier
  } mirror;*/

    typedef struct {
      u16 start;
      u8 CPU;
      u16 wrap;
      u16 position;
      u8 del;
      int16_t dirry;
      u8 speed, step; 
      u8 dir; // TODO: do as union or whatever for dir//flag for mirror
      u8 running;
      signed char m_stack_pos;
      u8 m_stack[16];
      u16 m_reg16bit1;
      u8 m_reg8bit1,m_reg8bit2;
    } villager_generic;//TODO: and  mirror?

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

