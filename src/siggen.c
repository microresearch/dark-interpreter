#include "audio.h"
#include "simulation.h"
#include <malloc.h>
#include <math.h>
#include <audio.h>
#include "stm32f4xx.h"
#include "arm_math.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern u16 *buf16;

static uint16_t syncPhaseAcc;
static uint16_t syncPhaseInc;
static uint16_t grainPhaseAcc;
static uint16_t grainPhaseInc;
static uint16_t grainAmp;
static uint8_t grainDecay;
static uint16_t grain2PhaseAcc;
static uint16_t grain2PhaseInc;
static uint16_t grain2Amp;
static uint8_t grain2Decay;

uint16_t antilogTable[] = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};

uint16_t mapPhaseInc(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}


void runVOSIMaud(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;
  uint16_t output; u8 value;
  
  // Smooth frequency mapping
  syncPhaseInc = mapPhaseInc(buf16[0]>>6) / 4; // 10 bits /4
  
  // Stepped mapping to MIDI notes: C, Db, D, Eb, E, F...
  //syncPhaseInc = mapMidi(analogRead(SYNC_CONTROL));
  
  // Stepped pentatonic mapping: D, E, G, A, B
  //  syncPhaseInc = mapPentatonic(buf16[1]>>6);

  grainPhaseInc  = mapPhaseInc(buf16[1]>>6) / 2;
  grainDecay     = (buf16[2]>>6) / 8;
  grain2PhaseInc = mapPhaseInc(buf16[3]>>6) / 2;
  grain2Decay    = (buf16[4]>>6) / 4;
 

  // deal with step and count and so on... 
   for (u8 xx=0;xx<vill->howmany;xx++){
     count+=step;
     if (count>start+wrap) count=start;

  syncPhaseAcc += syncPhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
  }
  
  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);
  buf16[count&32767]=output; 

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;
   }
  vill->position=count;

}

static float vosim;
static 	float phase=0.f;
static 	float prevtrig=0.f;
static 	float nCycles=1.f;
static 	u16 numberCurCycle=0;
static 	float prevsine;
static 	float decay=0.5f;
static 	float amp=1.f;

const float PII = 3.1415926535f;

void runVOSIM_SC(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;
  u16 out;

  float freq = (float)(buf16[0]>>9);
  float nCycles = (float)(buf16[1]>>14);
  float nDecay = (float)(buf16[2])/65536.0f;
  float phaseinc = freq * 2.f * PII / 32000.0f;
  float numberCycles = nCycles;
  int number = numberCurCycle;


  for (u8 xx=0;xx<vill->howmany;xx++){

     count+=step;
     if (count>start+wrap) count=start;
  
     float z = vosim;
          float trigin = (float)(buf16[xx+3]-32768)/32768.0f;
     //     float trigin = (float)((rand()%65536)-32768)/32768.0f;

     if(phase > 0 && number <= numberCycles ){
       float sine = sinf(phase);
       vosim = (sine * sine) * amp;

       if(prevsine >= 0.f && sine <= 0.f){
	 number += 1;
	 amp = amp * decay;
       }

       if(prevsine <= 0.f && sine >= 0.f){
	 number += 1;
	 amp = amp * decay;
       }

       prevsine = sine;

       phase = phase + phaseinc;

     }else if(trigin > 0.f && prevtrig <= 0.f){
       numberCycles = nCycles;
       decay = nDecay;
       amp = 1.f;
       number = 0;

       float sine = sinf(phase);

       vosim = (sine * sine) * amp;

       prevsine = sine;

       phase = phase + phaseinc;
     }else if(number >= numberCycles){
       phase = 0;
       //vosim = 0.f;
     }
     prevtrig = trigin;

     // write the output
     out = (float)z*65536.0f;

     buf16[count&32767]=out+32768; 
     //        buf16[count&32767]=rand()%32768;

  }
  vill->position=count;
  nCycles = numberCycles;
  numberCurCycle = number;

}
