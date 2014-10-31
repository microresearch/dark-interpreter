#include "stm32f4xx.h"
#include "arm_math.h"

typedef struct{
	float m_y1, m_y2, m_a0, m_a1, m_a2, m_b1, m_b2;
	float m_freq, m_bw;
} BBandPass;

typedef struct{
	float m_freq, m_decayTime, m_attackTime;
	float m_y01, m_y02, m_b01, m_b02;
	float m_y11, m_y12, m_b11, m_b12;
} Formlet;

typedef struct{
      u16 offset;
      u16 start;
      u16 wrap;
      u16 counterr;
      u8 del;
      int16_t dirry;
      u8 whicheffect; // which effect and flag for datagenbuffer/audio <->
      u8 villagertwo; // for effect
      u16 villagertwopos;
      float effect,effectinv; 
      u8 running;
      float states[8];
      void* unit;
    } eff_villagerr;

/* notes: but what of effects which need to maintain state (so far
   either in static or in unit) as states in above and units just hold
   established...

other simple effects: squash, morph, mix, etc...

Formlet_init - various inited formlet units
Formlet_process

BBandPass_init - tables of bandpasses for 
BBandPass_process

convolvee - no great changes

bandpass (static) - active changes

envelopefollower (static)

mdavocoder - ???
mdavocal - ???

pvvocprocess(PV *unit, int16_t* inbuffer, int16_t* outbuffer);
hanningprocess(int16_t* inbuffer, int16_t* outbuffer);

port all above to do_effect(eff_villagerr)

*/

void test_effect(int16_t* inbuffer, int16_t* outbuffer);
void BBandPass_process(BBandPass *unit, int inNumSamples, float* inbuffer, float* outbuffer);
void BBandPass_init(BBandPass* unit);
void Formlet_init(Formlet* unit);
void Formlet_process(Formlet *unit, int inNumSamples, float* inbuffer, float* outbuffer);
