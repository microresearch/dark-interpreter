/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#ifdef LACH
#define SETSIZE 36
#define INFECTSIZE 740 
#define SETSHIFT 11
#define SHIFTY 7
#else
#define SETSIZE 66
#define INFECTSIZE 770 
#define SETSHIFT 10
#define SHIFTY 6
#endif

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
extern u16 settingsarray[SETSIZE];
extern u16 villager[128];
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define VILLAGE_SIZE (STACK_SIZE*2) //
extern u8 village_effects[VILLAGE_SIZE/2]; 
#define float float32_t
#endif

int16_t newdirection[8]={-256,-255,1,255,256,254,-1,-257};

extern signed char direction[2];
extern u8 EFFECTWRITE, EFFECTREAD,EFFECTFILTER;
extern u8 wormdir;
extern u8 wormflag[10];
//extern u8 villagestackpos;
extern u8 digfilterflag;
extern u8 *datagenbuffer;

int16_t *audio_ptr;

extern const u16 SAMPLE_FREQUENCY;
extern const float Pi;
extern const float PI_2;

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
		*ldst++ = *(src++);
		sz--;
		*(rdst++) = *(src++);
		//		*(rdst++) = 0;
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
  u16 tmp=0; u16 tmper;
  int16_t tmp16,count;
  int32_t tmp32;
  u8 x,tmpp;
  static u16 start=0,startfilt=0,wrapfilt=1,wrap=1,samplepos=0,sampleposfilt=0,anyposfilt=0,anypos=0;
  static u8 vill=0;
  static u8 villagerpos=0,villagefpos=0,villagewpos=0,del=0,delf=0,delread=0;
  u8 VILLAGEREAD,VILLAGEWRITE,VILLAGEFILT;

  int16_t dirry,dirryy=0;
  float32_t w0,w1,w2;

  static u16 anyposread=0,sampleposread=0,wrapread=1,startread=0;
  u16 wrapper; 

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

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
	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;
	float32_t morph_inv,fsum;

	morph_inv = 1.0f - (float32_t)FMOD;

#ifdef LACH

	VILLAGEREAD=(EFFECTREAD&3);	
	//VILLAGEREAD=2; // TESTY!
      	for (x=0;x<sz/2;x++){
	  if (VILLAGEREAD==2){
	    tmpp=village_effects[vill/2]%16;
	  }
	  else tmpp=EFFECTREAD>>2;
	  //	  tmpp=0; // TESTY!

	  switch(tmpp){ 
	  case 0:
	  default:
	  src++;
	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 1:
	  src++;
	  buf16[sampleposread%32768]=(*src)+32768;
	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 2:
	  src++;
	  tmp16=buf16[sampleposread%32768]-32768;
	  buf16[sampleposread%32768]=*(src++)+32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  src++;
	  //	  tmp32=(*(src++))+audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  src++;
	  //	  tmp32=(*(src++))+audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  src++;
	  tmp16=(*(src++))-(buf16[sampleposread%32768]-32678);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  src++;
	  tmp16=(buf16[sampleposread%32768]-32768)-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  src++;
	  tmp16=(buf16[sampleposread%32768]-32768) | (*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  src++;
	  tmp16=(*(src++))-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 12:
	  src++;
	  tmp16=audio_buffer[sampleposread%32768]-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 13:
	  src++;
	  tmp16=audio_buffer[sampleposread%32768] | (*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 14:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 15:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  }
	  /////

	  if (++delread>=SAMPLESPEEDREAD){
	    if (wormflag[2]) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
	    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;//+SAMPLEREXPAND;
		    if (dirry>0) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		    if (wormflag[0]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    if (wormflag[4]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=dirryy;
		    //		    vill=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2
		    tmp=VILLAGERSTART;tmper=VILLAGERWRAP;
		    vill=(tmp+(villagerpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;

		    startread=villager[vill]>>1;
		    wrapread=((villager[vill+1]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		    if (wrapread==0) wrapread=1;
		    if (dirry>0) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		    if (wormflag[0]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  if (wormflag[0]) dirryy=newdirection[wormdir]; 
		  else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	/////////////////////////////NO____LACH!!!!!!!!!
#else // end LACH

	if (digfilterflag&32 || digfilterflag&1){

	  VILLAGEREAD=EFFECTREAD&3;
	  for (x=0;x<sz/2;x++){
	  if (VILLAGEREAD==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=EFFECTREAD>>2;
	  switch(tmpp){ 
	  case 0:
	  default:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 1:
	    audio_buffer[sampleposread%32768]=*src; // LEFT
	  *(ldst++) = *(src++);
	  *(rdst++) = *(src++); 
	  break;
	  // Effects with/without clipping *, +, -, 
	  case 2:
	    *(ldst++) = *(src++);
	    *(rdst++) = *src; 
	    fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD; // shift = 32768 
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 3:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *(ldst++) = *(src++);
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)buf16[sampleposread%32768]-32768) * FMOD;
	  *(rdst++) = *src; 
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  tmp16=(*(src++))-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 7:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  tmp16=audio_buffer[sampleposread%32768]-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;

	  case 8:
	  tmp16=*src;
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  tmp32= *(src++) * tmp16; //right * left
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;

	  case 9:
	  *(ldst++) = *src;
	  tmp16=*(src++); // left
	  *(rdst++) = *src; 
	  fsum=(float32_t)tmp16 * morph_inv * (float32_t)*(src++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 10:
	  *(ldst++) = *src;
	  //tmp32=audio_buffer[samplepos%32768]* *(src++);
	  //	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *(ldst++) = *src;
	  //	  tmp32=audio_buffer[samplepos%32768]* *(src++);
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  *(ldst++) = *src;
	  //	  tmp32=*(src++) +audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  case 13:
	  *(ldst++) = *src;
	  fsum=(float32_t)(buf16[sampleposread%32768]-32768) * morph_inv * (float32_t)*(src++) * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	    tmper= *src;
	    *(ldst++) = *(src++);
	    tmp16=*src | tmper;
	    *(rdst++) = *(src++); 
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
 	  case 15:
	  tmp16=*src;
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  fsum=(float32_t)tmp16 * morph_inv + (float32_t)*(src++) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  }
	  	  if (++delread>=SAMPLESPEEDREAD){
		    if (wormflag[4]) dirry=newdirection[wormdir]; 
		    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
		    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		      {
			sampleposread+=dirry;//)%32768;
		      }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;//+SAMPLEREXPAND;
		    if (dirry>0) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    if (wormflag[7]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=dirryy;
		    //		    vill=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2
		    tmp=VILLAGERSTART;tmper=VILLAGERWRAP;
		    vill=(tmp+(villagerpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;

		    startread=villager[vill]>>1;
		    wrapread=((villager[vill+1]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		    if (wrapread==0) wrapread=1;
		    if (dirry>0) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
		  }
	  }
	  /// INSERT writeDONE

	ldst=left_buffer;
	morph_inv = 1.0f - (float32_t)FMODW;
	VILLAGEWRITE=EFFECTWRITE&3;

      	for (x=0;x<sz/2;x++){
 	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=EFFECTWRITE>>2;

	  //	  VILLAGEWRITE=0; // TESTY!!!!
	  //	  tmpp=11; // TESTY!
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
	    fsum=(float32_t)*(ldst++) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 8:
	    //	  tmp32=audio_buffer[samplepos%32768]* *(ldst++);
	    fsum=(float32_t)*(ldst++) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 9:
	    tmp16=audio_buffer[samplepos%32768] - *(ldst++);
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    //	  tmp32=*(ldst++)+buf16[samplepos%32768];
	    fsum=(float32_t)*(ldst++) * morph_inv + (float32_t)(buf16[samplepos%32768]-32768) * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 11:
	    //	  tmp32=*(ldst++)+buf16[samplepos%32768];
	    fsum=(float32_t)*(ldst++) * morph_inv + (float32_t)(buf16[samplepos%32768]-32768) * FMODW;
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
	    w0=(float32_t) (buf16[0]-32768)/16384.0f;w1=(float32_t) (buf16[1]-32768)/16384.0f;w2=(float32_t) buf16[2]/16384.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  } 
	  ////////////////////////--->>>>

	  
	if (++del>=SAMPLESPEED){
	  if (wormflag[5]) dirry=newdirection[wormdir]; 
	  else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);
	    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;//+SAMPLEEXPAND;
		    if (dirry>0) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }
		  else if (VILLAGEWRITE==1) {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);//+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    if (wormflag[8]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=dirryy;
		    //		    vill=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2
		    tmp=VILLAGEWSTART;tmper=VILLAGEWWRAP;
		    vill=(tmp+(villagewpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;

		    start=villager[vill]>>1;
		    wrap=((villager[vill+1]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (dirry>0) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  if (wormflag[2]) dirryy=newdirection[wormdir]; 
		  else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (dirry>0) samplepos=start;
		  else samplepos=start+wrap;
		  }
		  }
	  del=0;
	} // end del
	}
	}
	else  // READIN NO DIG FILTER
	  {
	    VILLAGEREAD=EFFECTREAD&3;
	    //	    VILLAGEREAD=2; // TESTY!	    

	    for (x=0;x<sz/2;x++){
 	  if (VILLAGEREAD==2){
	    tmpp=village_effects[vill/2];
	    //	    tmpp=0;//TESTY!
	  }
	  else tmpp=EFFECTREAD>>2;
	  //	  tmpp=15;
	  switch(tmpp){
	  case 0:
	  default:
	  src++;
	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 1:
	    src++;
	    buf16[sampleposread%32768]=(*src)+32768;
	    audio_buffer[sampleposread%32768]=*(src++);
	  break;	    
	  case 2:
	  src++;
	  tmp16=buf16[sampleposread%32768]-32768;
	  buf16[sampleposread%32768]=*(src++)+32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 3:
	    (src++);
	    fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	    (src++);
	    fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  (src++);
	  tmp16=(*(src++))-(buf16[sampleposread%32768]-32768);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  (src++);
	  tmp16=buf16[sampleposread%32768]-(*(src++))-32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  (src++);
	  tmp16=(buf16[sampleposread%32768]-32768)|(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 13:
	  (src++);
	  tmp32=(*(src++))-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  (src++);
	  tmp16=audio_buffer[sampleposread%32768]-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 15:
	  (src++);
	  tmp16=audio_buffer[sampleposread%32768]|(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  }

	  if (++delread>=SAMPLESPEEDREAD){
	    if (wormflag[4]) dirry=newdirection[wormdir];  
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
	    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;//+SAMPLEREXPAND;
		    if (dirry>0) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    if (wormflag[7]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=dirryy; 
		    //		    vill=((VILLAGERSTART+(villagerpos%VILLAGERWRAP))*2)%VILLAGESTACKPOS; //to cover all directions
		    tmp=VILLAGERSTART;tmper=VILLAGERWRAP;
		    vill=(tmp+(villagerpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;

		    startread=villager[vill]>>1;
		    wrapread=((villager[vill+1]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;;
		    if (wrapread==0) wrapread=1;
		    if (dirry>0) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;;
		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	    }

/// STRAIGHT SANS FILTEROPSSS!!!
	    VILLAGEWRITE=EFFECTWRITE&3;
	    morph_inv = 1.0f - (float32_t)FMODW;

	    //	    VILLAGEWRITE=2;//testy!
	    //settingsarray[15]=1024;//testy! SAMPLEWRAP	  
	  for (x=0;x<sz/2;x++){

	  if (VILLAGEWRITE==2){
	    	    tmpp=village_effects[vill/2]; // TESTY!
	  }
	  else tmpp=EFFECTWRITE>>2;
	  switch(tmpp){ 
	  case 0:
	    mono_buffer[x]=audio_buffer[samplepos%32768];
	    //mono_buffer[x]=buf16[samplepos%32768]; // TESTY!!!!
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
	  case 13:
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*FMOD)+((float)audio_buffer[samplepos%32768]*FMODF)+((float)audio_buffer[(samplepos+1)%32768]*FMODW);
	    break;
	  case 14:
	    w0=(float32_t) (buf16[0]-32768)/16384.0f;w1=(float32_t) (buf16[1]-32768)/16384.0f;w2=(float32_t) (buf16[2]-32768)/16384.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  case 15:
	    w0=(float32_t) audio_buffer[0]/16384.0f;w1=(float32_t)audio_buffer[1]/16384.0f;w2=(float32_t)audio_buffer[2]/16384.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)(buf16[tmpp]-32768)*w0)+((float)(buf16[samplepos%32768]-32768)*w1)+((float)(buf16[(samplepos+1)%32768]-32768)*w2);
	    //	    mono_buffer[x]=((float)(buf16[tmpp])*w0)+((float)(buf16[samplepos%32768])*w1)+((float)(buf16[(samplepos+1)%32768])*w2);
	    break;
	  }
 
	  if (++del>=SAMPLESPEED){ 
	    if (wormflag[5]) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);
	    //	    VILLAGEWRITE=2; // TESTY!!!!
		    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;//+SAMPLEEXPAND;
		    if (dirry>0) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }
		  else if (VILLAGEWRITE==1) {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);//+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    if (wormflag[8]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=dirryy;
		    tmp=VILLAGEWSTART;tmper=VILLAGEWWRAP;
		    vill=(tmp+(villagewpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;
		    start=villager[vill]>>1;
		    wrap=((villager[vill+1]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		    //		    printf("vill %d stackpos %d\n",vill,VILLAGESTACKPOS);
		    if (wrap==0) wrap=1;
		    if (dirry>0) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (dirry>0) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	  }
	  }
#endif

#ifdef LACH

	morph_inv = 1.0f - (float32_t)FMODW;

	VILLAGEWRITE=EFFECTWRITE&3;
	//	VILLAGEWRITE=2; // TESTY!

      	for (x=0;x<sz/2;x++){
 	  if (VILLAGEWRITE==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=EFFECTWRITE>>2;
	  //	  tmpp=0; // TESTY!
	  switch(tmpp){ 
	  case 0:
	      mono_buffer[x]=audio_buffer[samplepos%32768];
	    //	    mono_buffer[x]=buf16[samplepos%32768]-32768; // TESTY!!!!
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
	    w0=(float32_t) (buf16[0]-32768)/16384.0f;w1=(float32_t) (buf16[1]-32768)/16384.0f;w2=(float32_t) (buf16[2]-32768)/16384.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpp]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  case 15:
	    w0=(float32_t) audio_buffer[0]/16384.0f;w1=(float32_t)audio_buffer[1]/16384.0f;w2=(float32_t)audio_buffer[2]/16384.0f;
	    tmpp=samplepos-1;
	    tmpp=tmpp%32768;
	    //	    mono_buffer[x]=((float)(buf16[tmpp]-32768)*w0)+((float)(buf16[samplepos%32768]-32768)*w1)+((float)(buf16[(samplepos+1)%32768]-32768)*w2);
	    mono_buffer[x]=((float)(buf16[tmpp])*w0)+((float)(buf16[samplepos%32768])*w1)+((float)(buf16[(samplepos+1)%32768])*w2);

	    break;
	  }

	  	  if (++del>=SAMPLESPEED){
		    if (wormflag[3]) dirry=newdirection[wormdir]; 
	  	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);// samplepos is start or start+wrap++
		    if (count<wrap && count>0)
	      {
		    samplepos+=dirry;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;//+SAMPLEEXPAND;
		     if (dirry>0) samplepos=start; //forwards
		    else samplepos=start+wrap;
		    count=samplepos;
		  }
		  else if (VILLAGEWRITE==1) {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);//+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    if (wormflag[5]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=dirryy;
		    //		    vill=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2

		    tmp=VILLAGEWSTART;tmper=VILLAGEWWRAP;
		    vill=(tmp+(villagewpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;

		    start=villager[vill]>>1;
		    wrap=((villager[vill+1]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (dirry>0) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (dirry>0) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  	  }
	}
	/////////////////////////////NO____LACH!!!!!!!!!
#else // end LACH

	///	///	///	/// FILTER process

if (digfilterflag&1){ 

	ldst=left_buffer;
	rdst=right_buffer;
	morph_inv = 1.0f - (float32_t)FMODF;

	  ////////////////////////////////////LDST effects also...
	//	EFFECTFILT=(EFFECTWRITE+EFFFOFFSET)%64;
	VILLAGEFILT=EFFECTFILTER&3;
      	for (x=0;x<sz/2;x++){ 

 	  if (VILLAGEFILT==2){
	    tmpp=village_effects[vill/2];
	  }
	  else tmpp=EFFECTFILTER>>2;
	  //	  tmpp=0; // TESTY!
 	  switch(tmpp){ 
	  case 0:
	  default:
	  *(ldst++)=audio_buffer[sampleposfilt%32768];
	  rdst++;
	  break;
	  case 1:
	  *(ldst++)=*(rdst++);
	  break;
	  case 2:
	    *(ldst++) =0;
	  rdst++;
	    break;
	  case 3:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 4:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 5:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 6:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  case 7:
	    fsum=(float32_t)*ldst * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 8:
	    fsum=(float32_t)(*ldst) * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  ////////////////////
	  case 9:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 10:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 11:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 12:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  case 13:
	    fsum=(*ldst) & *(rdst++);
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 14:
	    fsum=(float32_t)(*ldst) * morph_inv + (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  case 15:
	  tmp32=audio_buffer[sampleposfilt%32768]| *ldst;
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  }
	  ///	  HERE////////////////////////--->>>>

	  	  if (++delf==SAMPLESPEEDFILT){
		    if (wormflag[6]) dirry=newdirection[wormdir]; 
		    else dirry=direction[SAMPLEDIRF]*SAMPLESTEPFILT;	    
	    count=((sampleposfilt-startfilt)+dirry);
		    if (count<wrapfilt && count>0)
		  {
		    sampleposfilt+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEFILT==0) {
		    startfilt=SAMPLESTARTFILT;wrapfilt=SAMPLEWRAPFILT;//+SAMPLEFEXPAND;
		    if (dirry>0) sampleposfilt=startfilt; //forwards
		    else sampleposfilt=startfilt+wrapfilt;
		  }

		  else if (VILLAGEFILT==1) {
		    if (wormflag[3]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPFILT*direction[DATADIRR];
		  anyposfilt+=dirryy;
		  tmp=(ANYSTARTFILT+(anyposfilt%ANYWRAPFILT))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposfilt=SAMPLESTARTFILT+(tmper%SAMPLEWRAPFILT);//+SAMPLEFEXPAND;
		  wrapfilt=0;startfilt=0;
		  }
		  else if (VILLAGEFILT==2) {
		    if (wormflag[9]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagefpos+=dirryy;
		    //		    vill=(VILLAGEFSTART+(villagefpos%VILLAGEFWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2

		    tmp=VILLAGEFSTART;tmper=VILLAGEFWRAP;
		    vill=(tmp+(villagefpos%tmper))*2;
		    vill=vill%VILLAGESTACKPOS;

		    startfilt=villager[vill]>>1;
		    wrapfilt=((villager[vill+1]>>1)%SAMPLEWRAPFILT);//+SAMPLEFEXPAND;
		    if (wrapfilt==0) wrapfilt=1;
		    if (dirry>0) sampleposfilt=startfilt;
		    else sampleposfilt=startfilt+wrapfilt;
		  }
		  else {
		    if (wormflag[3]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPFILT*direction[DATADIRF];

		  anyposfilt+=dirryy;
		  wrapper=ANYWRAPFILT; 
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  startfilt=buf16[tmp]>>1;
		    if (wormflag[3]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPFILT*direction[DATADIRF];

		  anyposfilt+=dirryy;
		  wrapper=ANYWRAPFILT;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  wrapfilt=((buf16[tmp]>>1)%SAMPLEWRAPFILT);//+SAMPLEFEXPAND;
		  if (wrapfilt==0) wrapfilt=1;
		  if (dirry>0) sampleposfilt=startfilt;
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
 //   for (x=0;x<sz/2;x++){
     //      printf("%c",mono_buffer[x]);
 //    }
#endif

#endif // for test eeg
#endif // for straight

}
