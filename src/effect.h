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

int16_t* test_effect(int16_t* inbuffer, int16_t* outbuffer);
void BBandPass_process(BBandPass *unit, int inNumSamples, float* inbuffer, float* outbuffer);
void BBandPass_init(BBandPass* unit);
void Formlet_init(Formlet* unit);
void Formlet_process(Formlet *unit, int inNumSamples, float* inbuffer, float* outbuffer);
