#include "arm_math.h"
#include "arm_const_structs.h"
#include "effect.h"
#include "vocode.h"
#include "audio.h"

extern u8 *datagenbuffer;

// sqrtf which uses FPU, the standard one apparently doesn't
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
  extern VocoderInstance* vocoder;  //  u16 x,xxxxx;
  //  float xx,xxx,xxxx;
  float tmpbuffer[BUFF_LEN/4];
  float tmpotherbuffer[BUFF_LEN/4];
  // convert to float
  int_to_floot(inbuffer,tmpbuffer);
  intun_to_floot(buf16,tmpotherbuffer);

  // do vocoder
  runVocoder(vocoder, tmpbuffer, tmpotherbuffer, tmpbuffer, BUFF_LEN/4);

  //void runVocoder(VocoderInstance *vocoder, float *formant=voice, float *carrier, float *out, unsigned int SampleCount)


  // convert back to int
  floot_to_int(outbuffer,tmpbuffer);

  //  for (x=0;x<howmany;x++){

    /* formant ee: how to convert???
VOWEL SOUND "EE" GAIN (dB) Q

F1 270 0 5

F2 2300 -15 20

F3 3000 -9 50

300
870
2250
    */

    /*
    xxx=(float)inbuffer[(x+orfset)%1024]/32768.0;
    xx=bandpass(xxx,0.8f,270.0f,1.0f); // q freq gain
    xx+=bandpass(xxx,0.8f,2300.0f,1.0f); // q freq gain
    xx+=bandpass(xxx,0.8f,3000.0f,1.0f); // q freq gain
    xxxxx=xx*32768.0;
    outbuffer[(x+orfset)%1024]=xxxxx;
    */
  //  }
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

/* 2-convolution

//r[] as zeroed first
//max function?
// #define MAX(a,b) (((a)>(b))?(a):(b))

void conv(const float v1[], size_t n1, const float v2[], size_t n2, float r[])
{
    for (size_t n = 0; n < n1 + n2 - 1; n++)
        for (size_t k = 0; k < max(n1, n2); k++)
            r[n] += (k < n1 ? v1[k] : 0) * (n - k < n2 ? v2[n - k] : 0);
}

*/


/*0-CONV/1-phase vocoder/2-bandpass vocoder/3-FFT spectral
  exchanges/4-simple formant vowels/5-further eg envelope*/

