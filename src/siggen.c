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

void runVOSIMaud(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;
  uint16_t output; u8 value;

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
