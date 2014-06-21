/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32
#define randi() (adc_buffer[9]) // 12 bits

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "simulation.h"
#include <malloc.h>
#include "audio.h"
#include "settings.h"
extern int16_t *adc_buffer;
int16_t *audio_buffer;
extern int16_t *settingsarray;
extern int16_t *villager;
#define  uint32_t int
typedef int int32_t;
#define float32_t float
int16_t	*left_buffer, *right_buffer, *mono_buffer;
#define VILLAGE_SIZE (STACK_SIZE*2) // 128
extern u8* village_effects;

void initaudio(void){
left_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
right_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
mono_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
}

#else
#include "audio.h"
#include "CPUint.h"
#include "settings.h"
#include "simulation.h"
extern __IO uint16_t adc_buffer[10];
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
extern u16 settingsarray[64];
extern u16 villager[192];
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define VILLAGE_SIZE (STACK_SIZE*2) //
extern u8 village_effects[VILLAGE_SIZE/2]; 
#define float float32_t
#endif

u16 newdirection[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768

#ifdef LACH
extern u8 EFFECTREAD;
#endif

extern signed char direction[2];
extern u8 EFFECTWRITE;
extern u8 wormdir;
extern u8 villagestackpos;
extern u8 digfilterflag;
extern u8 *datagenbuffer;

int16_t *audio_ptr;

extern const u16 SAMPLE_FREQUENCY;
extern const float Pi;
extern const float PI_2;

extern u8 www[3],freqyy[3]; // where to INIT?

const float32_t coef[5][11]= {
{ 8.11044e-06,
8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289,
-151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471 ///A
},
{4.36215e-06,
8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248, ///E
-149.6496211,87.78352223, -34.60687431, 8.282228154, -0.914150747
},
{ 3.33819e-06,
8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618,
-150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035 ///I
},
{1.13572e-06,
8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544, ///O
-154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233
},
{4.09431e-07,
8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141, ///U
-153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753
}
}; 

float memory[10]={0,0,0,0,0,0,0,0,0,0};

float formant_filter(float in, u8 vowelnum)
{
  float res= (float) ( coef[vowelnum][0]* in +
  coef[vowelnum][1] *memory[0] +
  coef[vowelnum][2] *memory[1] +  
  coef[vowelnum][3] *memory[2] +
  coef[vowelnum][4] *memory[3] +
  coef[vowelnum][5] *memory[4] +
  coef[vowelnum][6] *memory[5] +
  coef[vowelnum][7] *memory[6] +
  coef[vowelnum][8] *memory[7] +
  coef[vowelnum][9] *memory[8] +
  coef[vowelnum][10] *memory[9] );  

memory[9]= memory[8];
memory[8]= memory[7];
memory[7]= memory[6];
memory[6]= memory[5];
memory[5]= memory[4];
memory[4]= memory[3];
memory[3]= memory[2];
memory[2]= memory[1];
memory[1]= memory[0];
memory[0]=(float) res;
return res;
}

void Audio_Init(void)
{
	uint32_t i;
	
	/* clear the buffer */
	audio_ptr = audio_buffer;
	i = AUDIO_BUFSZ;
	while(i-- > 0)
		*audio_ptr++ = 0;
	
	/* init the pointer */
	audio_ptr = audio_buffer;
}

void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src++;
		//		*rdst++ = 0;
		sz--;
	}
}

void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = (*rsrc++);
		sz--;
	}
}

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  u16 tmp=0,tmper;
  int16_t tmp16,count;
  int32_t tmp32;
  u8 x,tmpp;
  static u16 start=0,startfilt=0,wrapfilt=1,wrap=1,samplepos=0,sampleposfilt=0,anyposfilt=0,anypos=0;
  u8 vill=0;
  static u8 villagerpos=0,villagefpos=0,villagewpos=0,del=0,delf=0,delread=0;
  u8 VILLAGEREAD,VILLAGEWRITE,VILLAGEFILT;
#ifndef LACH
  u8 EFFECTREAD;
  u8 EFFECTFILT;
#endif
  int16_t dirry;
  float32_t w0,w1,w2;

  static u16 anyposread=0,sampleposread=0,wrapread=1,startread=0;
  u16 wrapper; 

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else
	u16 *buf16 = (u16*) datagenbuffer;
#ifdef TEST_EEG
	// write buf16 into mono
	      	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos%32768]-32768;
	  samplepos++;
		}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	///	///	///	///

	// readin villager processing of left into left and right into audio_buffer

	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;
	float32_t morph_inv,fsum;
	//	settingsarray[48]=adc_buffer[2]<<4; // TESTY for FMODW!

	morph_inv = 1.0 - (float32_t)FMOD;

#ifdef LACH

	//	EFFECTREAD=(EFFECTWRITE+EFFROFFSET)%128;		
	VILLAGEREAD=(EFFECTREAD&3);	
	//	VILLAGEREAD=0; // TESTY!

      	for (x=0;x<sz/2;x++){
	  if (VILLAGEREAD==2){
	    tmpp=village_effects[vill/2]%16;
	  }
	  else tmpp=(EFFECTREAD&63)>>2;
	  //	  tmpp=0; // TESTY!

	  switch(tmpp){ 
	  case 0:
	  default:
	  src++;
	  audio_buffer[sampleposread%32768]=*src++;
	  break;
	  case 1:
	  src++;
	  buf16[sampleposread%32768]=(*src)+32768;
	  audio_buffer[sampleposread%32768]=*src++;
	  break;
	  case 2:
	  src++;
	  tmp16=buf16[sampleposread%32768]-32768;
	  buf16[sampleposread%32768]=audio_buffer[sampleposread%32768]+32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	  src++;
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  src++;
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  src++;
	  //	  tmp32=(*src++)+audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  src++;
	  //	  tmp32=(*src++)+audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  src++;
	  tmp16=(*src++)-(buf16[sampleposread%32768]-32678);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  src++;
	  tmp16=(buf16[sampleposread%32768]-32768)-(*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  src++;
	  tmp16=(buf16[sampleposread%32768]-32768) | (*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  src++;
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  src++;
	  tmp16=(*src++)-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 12:
	  src++;
	  tmp16=audio_buffer[sampleposread%32768]-(*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 13:
	  src++;
	  tmp16=audio_buffer[sampleposread%32768] | (*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 14:
	  src++;
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 15:
	  src++;
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  }
	  /////
	  if (++delread>=SAMPLESPEEDREAD){
	    if (EFFECTREAD&64 && VILLAGEREAD!=0) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
	    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD+SAMPLEREXPAND;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    tmp=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=tmp;
		    vill=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startread=villager[vill]>>1;
		    wrapread=((villager[vill+1]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){

	  ////////////////////////////////////LDST effects also...

	  EFFECTREAD=(EFFECTWRITE+EFFROFFSET)%128;
	  VILLAGEREAD=EFFECTREAD&3;
	  for (x=0;x<sz/2;x++){
	  if (VILLAGEREAD==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=(EFFECTREAD&63)>>2;
	  //	  VILLAGEREAD=0;  // TESTY!
	  //	  tmpp=0;  // TESTY!
	  switch(tmpp){ 
	  case 0:
	  default:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  audio_buffer[sampleposread%32768]=*src++;
	  break;
	  case 1:
	    audio_buffer[sampleposread%32768]=*src; // LEFT
	  *ldst++ = *src++;
	  *rdst++ = *src++; 
	  break;
	  // Effects with/without clipping *, +, -, 
	  case 2:
	    *ldst++ = *src++;
	    *rdst++ = *src; 
	    fsum=(float32_t)*src++ * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD; // shift = 32768 
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 3:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  fsum=(float32_t)*src++ * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *ldst++ = *src++;
	  fsum=(float32_t)*src++ * morph_inv * ((float32_t)buf16[sampleposread%32768]-32768) * FMOD;
	  *rdst++ = *src; 
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp16=(*src++)-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 7:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp16=audio_buffer[sampleposread%32768]-(*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  // start of *(src-1)

	  case 8:
	  tmp16=*src;
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32= *(src++) * tmp16; //right * left
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;

	  case 9:
	  *ldst++ = *src;
	  tmp16=*src++; // left
	  *rdst++ = *src; 
	  fsum=(float32_t)tmp16 * morph_inv * (float32_t)*src++ * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 10:
	  *ldst++ = *src;
	  //tmp32=audio_buffer[samplepos%32768]* *src++;
	  //	  fsum=(float32_t)*src++ * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  fsum=(float32_t)*src++ * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src++; 
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *ldst++ = *src;
	  //	  tmp32=audio_buffer[samplepos%32768]* *src++;
	  fsum=(float32_t)*src++ * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src++; 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  *ldst++ = *src;
	  //	  tmp32=*src++ +audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src++; 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  case 13:
	  *ldst++ = *src;
	  fsum=(float32_t)(buf16[sampleposread%32768]-32768) * morph_inv * (float32_t)*src++ * FMOD;
	  tmp32=fsum;
	  *rdst++ = *src++; 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	    tmper= *src;
	    *ldst++ = *src++;
	    tmp16=*src | tmper;
	    *rdst++ = *src++; 
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
 	  case 15:
	  tmp16=*src;
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  fsum=(float32_t)tmp16 * morph_inv + (float32_t)*src++ * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  }
	  	  if (++delread>=SAMPLESPEEDREAD){
		    //	    dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;

	    if (EFFECTREAD&64 && VILLAGEREAD!=0) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
		    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		      {
			sampleposread+=dirry;//)%32768;
		      }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD+SAMPLEREXPAND;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=tmp;
		    vill=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startread=villager[vill]>>1;
		    wrapread=((villager[vill+1]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	}
	else  // READIN NO DIG FILTER
	  {
	    EFFECTREAD=(EFFECTWRITE+EFFROFFSET)%128;
	    VILLAGEREAD=EFFECTREAD&3;
	    for (x=0;x<sz/2;x++){
 	  if (VILLAGEREAD==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=(EFFECTREAD&63)>>2;
	  //	  tmpp=15;
	  switch(tmpp){
	  case 0:
	  default:
	  src++;
	  audio_buffer[sampleposread%32768]=*src++;
	  break;
	  case 1:
	    src++;
	    buf16[sampleposread%32768]=(*src)+32768;
	    audio_buffer[sampleposread%32768]=*src++;
	  break;	    
	  case 2:
	    src+=2;
	  tmp16=buf16[sampleposread%32768]-32768;
	  buf16[sampleposread%32768]=audio_buffer[sampleposread%32768]+32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 3:
	    (src++);
	    fsum=(float32_t)*src++ * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	    (src++);
	    fsum=(float32_t)*src++ * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  (src++);
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  (src++);
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  (src++);
	  tmp16=(*src++)-(buf16[sampleposread%32768]-32768);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  (src++);
	  tmp16=buf16[sampleposread%32768]-(*src++)-32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  (src++);
	  tmp16=(buf16[sampleposread%32768]-32768)|(*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  (src++);
	  fsum=(float32_t)*src++ * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  (src++);
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  (src++);
	  fsum=(float32_t)*src++ * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 13:
	  (src++);
	  tmp32=(*src++)-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  (src++);
	  tmp16=audio_buffer[sampleposread%32768]-(*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 15:
	  (src++);
	  tmp16=audio_buffer[sampleposread%32768]|(*src++);
	  audio_buffer[sampleposread%32768]=tmp16;
	  }

	  if (++delread>=SAMPLESPEEDREAD){
	    if (EFFECTREAD&64 && VILLAGEREAD!=0) dirry=newdirection[wormdir];  // TESTY!
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
	    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD+SAMPLEREXPAND;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD)+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    tmp=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=tmp; //???
		    vill=((VILLAGERSTART+(villagerpos%VILLAGERWRAP))*2)%villagestackpos; //to cover all directions
		    startread=villager[vill]>>1;
		    wrapread=((villager[vill+1]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;;
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD)+SAMPLEREXPAND;;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	  }
#endif
	///!!!!!////////////////////////////////END OF READINSSS
	///!!!!!////////////////////////////////END OF READINSSS
	///!!!!!////////////////////////////////END OF READINSSS
	//	settingsarray[50]=adc_buffer[2]<<4; // TESTY for FMODW!
	morph_inv = 1.0 - (float32_t)FMODW;

#ifdef LACH

	VILLAGEWRITE=EFFECTWRITE&3;
	//	VILLAGEWRITE=0; // TESTY!

      	for (x=0;x<sz/2;x++){
 	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=(EFFECTWRITE&63)>>2;
	  //	  tmpp=0; // TESTY!
	  switch(tmpp){ 
	  case 0:
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	  break;
	  case 1:
	    tmp16=(buf16[samplepos%32768]-32768);
	    if (audio_buffer[samplepos%32768]<tmp16) tmp16=audio_buffer[samplepos%32768];
	    mono_buffer[x]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	    tmp16=audio_buffer[samplepos%32768]-(buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    tmp16=(buf16[samplepos%32768]-32768)-audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	    tmp16=(buf16[samplepos%32768]-32768)|audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	    tmp16=(buf16[samplepos%32768]-32768)&audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    tmp16=(buf16[samplepos%32768]-32768)^audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 11:
	    tmp16=(buf16[samplepos%32768]-32768)<<(audio_buffer[samplepos%32768]&7);
	  mono_buffer[x]=tmp16;
	  break;
	  case 12:
	    tmp16=(buf16[samplepos%32768]-32768)>>(audio_buffer[samplepos%32768]&7);
	  break;
	  case 13:
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*FMOD)+((float)audio_buffer[samplepos%32768]*FMODF)+((float)audio_buffer[(samplepos+1)%32768]*FMODW);
	    break;
	  case 14:
	    w0=(float32_t) (buf16[0]-32768)/32768.0f;w1=(float32_t) (buf16[1]-32768)/32768.0f;w2=(float32_t) (buf16[2]-32768)/32768.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  case 15:
	    w0=(float32_t) audio_buffer[0]/32768.0f;w1=(float32_t)audio_buffer[1]/32768.0f;w2=(float32_t)audio_buffer[2]/32768.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)(buf16[tmpp]-32768)*w0)+((float)(buf16[samplepos%32768]-32768)*w1)+((float)(buf16[(samplepos+1)%32768]-32768)*w2);
	    break;
	  }

	  	  if (++del>=SAMPLESPEED){
	    if (EFFECTWRITE&64 && VILLAGEWRITE!=0) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);// samplepos is start or start+wrap++
		    if (count<wrapread && count>0)
	      {
		    samplepos+=dirry;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP+SAMPLEEXPAND;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		    count=samplepos;
		  }
		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP)+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    tmp=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=tmp;
		    vill=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[vill]>>1;
		    wrap=((villager[vill+1]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	}
	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){
	ldst=left_buffer;
	  ////////////////////////////////////LDST effects also...

	VILLAGEWRITE=EFFECTWRITE&3;
      	for (x=0;x<sz/2;x++){

 	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=(EFFECTWRITE&63)>>2;

	  //	  VILLAGEWRITE=0; // TESTY!!!!
	  switch(tmpp){ 
	  case 0:
	    mono_buffer[x]=audio_buffer[samplepos%32768];
	  break;
	  case 1:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 2:
	    //	  tmp32=buf16[samplepos%32768] * audio_buffer[samplepos%32768];
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    //	  tmp32=audio_buffer[samplepos%32768]+buf16[samplepos%32768];
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    //	  tmp32=audio_buffer[samplepos%32768]+buf16[samplepos%32768];
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    tmp16=audio_buffer[samplepos%32768]-(buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 6:
	    tmp16=(buf16[samplepos%32768]-32768)-audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    fsum=(float32_t)*ldst++ * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 8:
	    //	  tmp32=audio_buffer[samplepos%32768]* *ldst++;
	    fsum=(float32_t)*ldst++ * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 9:
	    tmp16=audio_buffer[samplepos%32768] - *ldst++;
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    //	  tmp32=*(ldst++)+buf16[samplepos%32768];
	    fsum=(float32_t)*ldst++ * morph_inv + (float32_t)(buf16[samplepos%32768]-32768) * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 11:
	    //	  tmp32=*(ldst++)+buf16[samplepos%32768];
	    fsum=(float32_t)*ldst++ * morph_inv + (float32_t)(buf16[samplepos%32768]-32768) * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    mono_buffer[x]=((adc_buffer[9]<<3)-32768) * audio_buffer[samplepos%32768];
	  break;
	  case 13:
	    //	    tmp32=audio_buffer[samplepos%32768]+adc_buffer[9]<<3;
	    fsum=(float32_t)((adc_buffer[9]<<3)-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 14:
	    //	    tmp32=audio_buffer[samplepos%32768]*adc_buffer[9]<<3;
	    fsum=(float32_t)((adc_buffer[9]<<3)-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
 	  case 15:
	    w0=(float32_t) (buf16[0]-32768)/32768.0f;w1=(float32_t) (buf16[1]-32768)/32768.0f;w2=(float32_t) buf16[2]/32768.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  } 
	}
	  ////////////////////////--->>>>

	  	  if (++del>=SAMPLESPEED){
	    if (EFFECTWRITE&64 && VILLAGEWRITE!=0) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);
	    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP+SAMPLEEXPAND;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }

		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP)+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=tmp;
		    vill=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[vill]>>1;
		    wrap=((villager[vill+1]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
		  } // end del
	}
		else
	  { /// STRAIGHT SANS FILTEROPSSS!!!
	  VILLAGEWRITE=EFFECTWRITE&3;
	  //	  VILLAGEWRITE=0; // TESTY!!!!

	  for (x=0;x<sz/2;x++){

	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=(EFFECTWRITE&63)>>2;
	  //	     printf("%d\n",samplepos);
	  switch(tmpp){ 
	  case 0:
	     mono_buffer[x]=audio_buffer[samplepos%32768];
	    break;
	  case 1:
	    tmp16=(buf16[samplepos%32768]-32768);
	    if (audio_buffer[samplepos%32768]<tmp16) tmp16=audio_buffer[samplepos%32768];
	    mono_buffer[x]=tmp16;
	  break;
	  case 2:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	    tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv - (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	    tmp16=audio_buffer[samplepos%32768]-(buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    tmp16=(buf16[samplepos%32768]-32768)-audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	    tmp16=(buf16[samplepos%32768]-32768) | audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	    tmp16=audio_buffer[samplepos%32768] & (buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    mono_buffer[x]=((adc_buffer[9]<<3)-32768)*audio_buffer[samplepos%32768];
	  break;
	  case 11:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv + (float32_t)((adc_buffer[9]<<3)-32768) * FMODW;
 	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)((adc_buffer[9]<<3)-32768) * FMODW;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 14:
	    w0=(float32_t) (buf16[0]-32768)/32768.0f;w1=(float32_t) (buf16[1]-32768)/32768.0f;w2=(float32_t) (buf16[2]-32768)/32768.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  case 15:
	    w0=(float32_t) audio_buffer[0]/32768.0f;w1=(float32_t)audio_buffer[1]/32768.0f;w2=(float32_t)audio_buffer[2]/32768.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)(buf16[tmpp]-32768)*w0)+((float)(buf16[samplepos%32768]-32768)*w1)+((float)(buf16[(samplepos+1)%32768]-32768)*w2);
	    break;
	  }
 
	  if (++del>=SAMPLESPEED){ 
	    if (EFFECTWRITE&64 && VILLAGEWRITE!=0) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);
		    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP+SAMPLEEXPAND;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }
		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP)+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=tmp;
		    vill=((VILLAGEWSTART+(villagewpos%VILLAGEWWRAP))*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[vill]>>1;
		    wrap=((villager[vill+1]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		    #ifdef PCSIM
		    //		    printf("villager[tmp+1] %d \n",(villager[tmp+1]));
		    //		    printf("villager %d samplepos: %d wrap: %d\n",tmp,samplepos,wrap);
			   #endif
		  }
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP)+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  	  }
	  }
	  }
      	///!!!!!!////////////////////////////////END OF WRITEOUTSSS

	///	///	///	/// FILTER process

if (digfilterflag&1){ 

	ldst=left_buffer;
	rdst=right_buffer;
	//	settingsarray[49]=adc_buffer[2]<<4;// testy
	morph_inv = 1.0 - (float32_t)FMODF;

	  ////////////////////////////////////LDST effects also...
	EFFECTFILT=(EFFECTWRITE+EFFFOFFSET)%128;
	VILLAGEFILT=EFFECTFILT&3;
      	for (x=0;x<sz/2;x++){ 

 	  if (VILLAGEFILT==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=(EFFECTFILT&63)>>2;

 	  switch(tmpp){ 
	  case 0:
	  default:
	  *ldst++=audio_buffer[sampleposfilt%32768];
	  rdst++;
	  break;
	  case 1:
	  *ldst++=*rdst++;
	  break;
	  case 2:
	    *ldst++ =0;
	  rdst++;
	    break;
	  case 3:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  rdst++;
	  break;
	  case 4:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  rdst++;
	  break;
	  case 5:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  break;
	  case 6:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  break;
	  case 7:
	    fsum=(float32_t)*ldst * morph_inv * (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  break;
	  case 8:
	    fsum=(float32_t)(*ldst) * morph_inv * (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  break;
	  ////////////////////
	  case 9:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  rdst++;
	  break;
	  case 10:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  rdst++;
	  break;
	  case 11:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*rdst++) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
	  *ldst++=tmp32;
	  break;
	  case 12:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*rdst++) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  break;
	  case 13:
	    fsum=(*ldst) & *(rdst++);
	  tmp32=fsum;
	  *ldst++=tmp32;
	  break;
	  case 14:
	    fsum=(float32_t)(*ldst) * morph_inv + (float32_t)(*rdst++) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *ldst++=tmp32;
	  break;
	  case 15:
	  tmp32=audio_buffer[sampleposfilt%32768]| *ldst;
	  *ldst++=tmp32;
	  rdst++;
	  break;
	  }
	  ///	  HERE////////////////////////--->>>>

	  	  if (++delf==SAMPLESPEEDFILT){
	    if (EFFECTFILT&64 && VILLAGEFILT!=0) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRF]*SAMPLESTEPFILT;	    
	    count=((sampleposfilt-startfilt)+dirry);
		    if (count<wrapfilt && count>0)
		  {
		    sampleposfilt+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEFILT==0) {
		    startfilt=SAMPLESTARTFILT;wrapfilt=SAMPLEWRAPFILT+SAMPLEFEXPAND;
		    if (SAMPLEDIRF==1) sampleposfilt=startfilt; //forwards
		    else sampleposfilt=startfilt+wrapfilt;
		  }

		  else if (VILLAGEFILT==1) {
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  tmp=(ANYSTARTFILT+(anyposfilt%ANYWRAPFILT))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposfilt=SAMPLESTARTFILT+(tmper%SAMPLEWRAPFILT)+SAMPLEFEXPAND;
		  wrapfilt=0;startfilt=0;
		  }
		  else if (VILLAGEFILT==2) {
		    tmp=VILLAGEFSTEP*direction[VILLAGEFDIR];
		    villagefpos+=tmp;
		    vill=(VILLAGEFSTART+(villagefpos%VILLAGEFWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startfilt=villager[vill]>>1;
		    wrapfilt=((villager[vill+1]>>1)%SAMPLEWRAPFILT)+SAMPLEFEXPAND;
		    if (wrapfilt==0) wrapfilt=1;
		    if (SAMPLEDIRF==1) sampleposfilt=startfilt;
		    else sampleposfilt=startfilt+wrapfilt;
		  }
		  else {
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  wrapper=ANYWRAPFILT; 
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  startfilt=buf16[tmp]>>1;
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  wrapper=ANYWRAPFILT;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  wrapfilt=((buf16[tmp]>>1)%SAMPLEWRAPFILT)+SAMPLEFEXPAND;
		  if (wrapfilt==0) wrapfilt=1;
		  if (SAMPLEDIRF==1) sampleposfilt=startfilt;
		  else sampleposfilt=startfilt+wrapfilt;
		  }
		}
	  delf=0;
		  }
	}
 }

#endif // for LACH
 audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

#ifdef PCSIM
 //  for (x=0;x<sz/2;x++){
    //    printf("%c",mono_buffer[x]);
 //      }
#endif

#endif // for test eeg
#endif // for straight

}
