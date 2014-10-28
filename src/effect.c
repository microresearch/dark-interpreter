// TESTING various effects

/*0-CONV/1-phase vocoder/2-bandpass vocoder/3-FFT spectral
  exchanges/4-simple formant vowels/5-further eg envelope, formant
  synth, windowing etc.*/

#include "arm_math.h"
#include "arm_const_structs.h"
#include "effect.h"
#include "vocode.h"
#include "audio.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

extern u8 *datagenbuffer;

// sqrtf which uses FPU, the standard one apparently doesn't????
float vsqrtf(float op1) {
  float result;
  __ASM volatile ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (op1) );
  return (result);
}

float bandpass(float sample,float q, float fc, float gain){ // from OWL code - statevariable
  float f,fb,hp,bp,scale;
  static float buf0=0,buf1=0;
  f = 2.0*M_PI*fc/48000.0f;
  fb= q + q/(1.0 - f);

  hp=sample-buf0;
  bp = buf0 - buf1; 
  buf0 = buf0 + f * (hp + fb * bp); 
  buf1 = buf1 + f * (buf0 - buf1);
  return bp*gain;
}

void int_to_floot(int16_t* inbuffer, float* outbuffer){
  for (int n = 0; n < BUFF_LEN/4; n++) {
    outbuffer[n]=(float32_t)(inbuffer[n])/32768.0f;
  }
}

void intun_to_floot(int16_t* inbuffer, float* outbuffer){
  for (int n = 0; n < BUFF_LEN/4; n++) {
    outbuffer[n]=(float32_t)(inbuffer[n]-32768)/32768.0f;
  }
}


void floot_to_int(int16_t* outbuffer, float* inbuffer){
  int16_t tmp;

  for (int n = 0; n < BUFF_LEN/4; n++) {
    tmp = (int32_t)(inbuffer[n] * 32768.0f);
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    outbuffer[n] = (int16_t)tmp;
		}
}

int16_t* test_effect(int16_t* inbuffer, int16_t* outbuffer){
  u16 *buf16 = (u16*) datagenbuffer;
  extern VocoderInstance* vocoder; u8 x;
  float xx,xxx;
  float tmpbuffer[BUFF_LEN/4];
  float tmpotherbuffer[BUFF_LEN/4];
  // convert to float

  // VOCODER:
  /*  int_to_floot(inbuffer,tmpbuffer);
  intun_to_floot(buf16,tmpotherbuffer);
  runVocoder(vocoder, tmpbuffer, tmpotherbuffer, tmpbuffer, BUFF_LEN/4);
  floot_to_int(outbuffer,tmpbuffer);
  */

  int_to_floot(inbuffer,tmpbuffer);

  for (x=0;x<32;x++){

    /* formant ee: how to convert???
VOWEL SOUND "EE" GAIN (dB) Q

F1 270 0 5

F2 2300 -15 20

F3 3000 -9 50

300
870
2250
    */
    //    xxx=(float)inbuffer[x]/32768.0;
    xxx=tmpbuffer[x];
    //    xx=bandpass(xxx,0.8f,270.0f,1.0f); // q freq gain
    //    xx+=bandpass(xxx,0.8f,2300.0f,1.0f); // q freq gain
    xx=bandpass(xxx,0.7f,3000.0f,1.0f); // q freq gain
    tmpbuffer[x]=xx;
      }
  floot_to_int(outbuffer,tmpbuffer);
}


/* 1-envelope

//If you don't need different attack and decay settings, just use in->abs()->LP

// Slightly faster version of the envelope follower using one multiply form.

// attTime and relTime is in seconds

float ga = exp(-1.0f/(sampleRate*attTime));
float gr = exp(-1.0f/(sampleRate*relTime));

float envOut = 0.0f;

for( ... )
{
    // get your data into 'input'

    envIn = fabsf(input);

    if( envOut < envIn )
        envOut = envIn + ga * (envOut - envIn);
    else
        envOut = envIn + gr * (envOut - envIn);

    // envOut now contains the envelope
}

*/

// 2-convolution

//r[] as zeroed first
//max function above

void convolvee(float *convolve1, u16 n1, float *convolve2, u16 n2, float *out)
{
    for (size_t n = 0; n < n1 + n2 - 1; n++)
        for (size_t k = 0; k < MAX(n1, n2); k++)
            out[n] += (k < n1 ? convolve1[k] : 0) * (n - k < n2 ? convolve2[n - k] : 0);
}


