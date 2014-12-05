/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#ifdef TENE
#define FIRST 2
#define SECOND 0
#define THIRD 3
#define FOURTH 4
#define FIFTH 1
#define UP 6
#define DOWN 8
#define LEFT 5
#define RIGHT 7
#else
#define FIRST 3
#define SECOND 0
#define THIRD 2
#define FOURTH 4
#define FIFTH 1
#define UP 5
#define DOWN 6
#define LEFT 8
#define RIGHT 7
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
//#include "settings.h"
extern int16_t *adc_buffer;
int16_t *audio_buffer;
#define  uint32_t int
typedef int int32_t;
#define float32_t float
int16_t	*left_buffer, *mono_buffer;

void initaudio(void){
left_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
mono_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
}

#else

#include "audio.h"
#include "CPUint.h"
#include "effect.h"
//#include "settings.h"
#include "hardware.h"
#include "simulation.h"
extern __IO uint16_t adc_buffer[10];
extern int16_t* buf16;
extern u8* datagenbuffer;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
int16_t	left_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define float float32_t
#endif

static const int16_t newdir[8]={-256,-255,1,255,256,254,-1,-257};
static const signed char dir[2]={-1,1};
extern u8 wormdir;
extern u8 digfilterflag;

void Audio_Init(void)
{
	uint32_t i;
	int16_t *audio_ptr;
	
	/* clear the buffer */
	audio_ptr = audio_buffer;
	i = AUDIO_BUFSZ;
	while(i-- > 0)
		*audio_ptr++ = 0;
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

inline void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
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
  int16_t dirry;
  int32_t lasttmp=0,lasttmp16=0;
  register int32_t lp,samplepos;
  register int32_t tmp,tmpl,tmp16,tmp32d;

  u8 x,xx;
  u16 overlay;
  extern villagerw village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  static u8 whichwritevillager=0;
#ifndef LACH
  static u8 hdgener; 
  static u16 counthw=0;
  static int16_t count40106=0,counthdgener=0,countlm=0,countmaxim=0;
  static u8 which40106villager=0,whichlmvillager=0,whichhdgenervillager=0,whichmaximvillager=0,whichhwvillager=1,whichfiltoutvillager=0;
  extern u8 howmanyhardvill,howmany40106vill,howmanylmvill,howmanyhdgenervill,howmanymaximvill;
  extern u8 howmanywritevill,howmanyfiltoutvill,howmanyreadvill;
  extern u8 hardcompress;
  extern villagerw village_filtout[MAX_VILLAGERS+1];
  extern villager_hardware village_hardware[17];
  extern villager_hardwarehaha village_40106[17];
  extern villager_hardwarehaha village_hdgener[17];
  extern villager_hardwarehaha village_lm[17];
  extern villager_hardwarehaha village_maxim[17];
#endif

#ifdef TEST_EFFECTS
  static int16_t effect_buffer[32]; //was 32 TESTY
#endif

#ifdef TEST_EEG
  static u16 samplepos=0;
#endif

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, mono_buffer);
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

#ifdef TEST_EEG
	// write buf16 into mono
	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos&32767];//-32768;
	  samplepos++;
	}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

#ifdef TEST_EFFECTS
	for (x=0;x<sz/2;x++){
	  src++;
	  tmp=*(src++); 
	  audio_buffer[x]=tmp;
	}

	test_effect(audio_buffer, effect_buffer);

	for (x=0;x<sz/2;x++){
	  	  mono_buffer[x]=effect_buffer[x];//-32768;
	  }
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else
	//////////////////////////////////////////////////////////	

	// READ! 

	  for (xx=0;xx<sz/2;xx++){
#ifndef LACH
	    tmpl=*(src++);
	    tmp=*(src++); 
	    if (digfilterflag&1 && village_read[x].overlay&32) tmp=tmpl;
#else
	    src++;
	    tmp=*(src++); 
#endif

	    //	    	    for (x=0;x<howmanyreadvill;x++){ // process other way round:
	    for(x=howmanyreadvill; x--; ){
	      overlay=village_read[x].overlay;

	    village_read[x].counterr+=village_read[x].dirryr;

	    if (village_read[x].counterr>=village_read[x].compress) {// whether still makes sense as ??? guess so!!!
	      village_read[x].counterr=0;
	      village_read[x].running=1;
	    }

	    if (village_read[x].offset<=village_read[x].counterr && village_read[x].running==1){
	      samplepos=village_read[x].samplepos;
	      lp=(samplepos+village_read[x].start)&32767;

	      if (overlay&16){ // datagen business readin! - top bit=32
		// 32 is swop datagen/16 could be leftIN rather than ssat//rest is overlay and effect
	      switch(overlay&15){
	      case 0: // overlay=all,effect=straight
	      tmp16=buf16[lp]-32768;
		buf16[lp]=tmp+32768; // TESTY commented out for datagens
		audio_buffer[lp]=tmp16;
	      break;
	      case 1://or
	      tmp16=buf16[lp]-32768;
	      buf16[lp]|=tmp+32768;
	      audio_buffer[lp]|=tmp16;
	      break;
	      case 2:///+
	      tmp16=buf16[lp];
	      tmp32d=tmp16+tmp;
	      tmp16+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d;// 
		audio_buffer[lp]=tmp16;
		break;
	      case 3://last
	      tmp16=buf16[lp]-32768;
	      if (tmp>lasttmp) buf16[lp]=tmp+32768;
	      if (tmp16>lasttmp16) audio_buffer[lp]=tmp16;
	      lasttmp=tmp; lasttmp16=tmp16;
	      break;

	      case 4: // // overlay=all,effect=&
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		buf16[lp]=tmp16+32768;
		audio_buffer[lp]=tmp16;
	      break;
	      case 5:// overlay or
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		buf16[lp]|=tmp16+32768;
		audio_buffer[lp]|=tmp16;
		break;
	      case 6: // overlay +
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		tmp16+=audio_buffer[lp];
		tmp32d=buf16[lp]+tmp16+32768;
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d;// 
		audio_buffer[lp]=tmp16;
		break;
	      case 7: // overlay last
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		if (tmp16>lasttmp) {
		  buf16[lp]=tmp16+32768;
		  audio_buffer[lp]=tmp16;
	      }
		lasttmp=tmp16;
		break;

	      case 8: // // overlay=all,effect=+
		tmp16=buf16[lp];
		tmp32d=tmp+tmp16; 
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d;// 
		audio_buffer[lp]=tmp32d-32768;
	      break;
	      case 9:// overlay or
		tmp16=buf16[lp];
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]|=tmp32d;
		audio_buffer[lp]|=tmp32d-32768;
		break;
	      case 10: // overlay +
		tmp16=buf16[lp];
		tmp32d=tmp+tmp16; 
		tmp16+=tmp32d;
		tmp32d+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		buf16[lp]=tmp16;
		audio_buffer[lp]=tmp32d-32768;
		break;
	      case 11: // overlay last
		tmp16=buf16[lp];
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		if (tmp32d>lasttmp) {
		  buf16[lp]=tmp32d;
		  audio_buffer[lp]=tmp32d-32768;
		}
		lasttmp=tmp32d;
		break;

	      case 12: // // overlay=all,effect=*
		tmp16=buf16[lp]-32768;
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d-32768;// 
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:// overlay or
		tmp16=buf16[lp]-32768;
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]|=tmp32d+32768;
		audio_buffer[lp]|=tmp32d;
		break;
	      case 14: // overlay +
		tmp16=buf16[lp]-32768;
		tmp32d=tmp*tmp16; 
		tmp16=buf16[lp]+tmp32d;
		tmp32d+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		buf16[lp]=tmp16+32768;
		audio_buffer[lp]=tmp32d;
		break;
	      case 15: // overlay last
		tmp16=buf16[lp]-32768;
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		if (tmp32d>lasttmp) {
		  buf16[lp]=tmp32d+32768;
		audio_buffer[lp]=tmp32d;
		}
		lasttmp=tmp32d;
		break;
	      }// switch
	      }
	      else // straight UP
		{
	      switch(overlay&15){
	      case 0: // overlay=all,effect=straight
		audio_buffer[lp]=tmp;
	      break;
	      case 1://or
		audio_buffer[lp]|=tmp;
	      break;
	      case 2:///+
		tmp32d=audio_buffer[lp]+tmp;
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp32d;
		break;
	      case 3://last
		if (tmp>lasttmp) audio_buffer[lp]=tmp;
		lasttmp=tmp;
	      break;

	      case 4: // // overlay=all,effect=&
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		audio_buffer[lp]=tmp16;
	      break;
	      case 5:// overlay or
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		audio_buffer[lp]|=tmp16;
		break;
	      case 6: // overlay +
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		tmp16+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		  //		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp16;
		break;
	      case 7: // overlay last
		tmp16=buf16[lp]-32768;
		tmp16&=tmp;
		if (tmp16>lasttmp) {
		  audio_buffer[lp]=tmp16;
	      }
		lasttmp=tmp16;
		break;
	      case 8: // // overlay=all,effect=+
		tmp16=buf16[lp]-32768;
		tmp16+=tmp; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		audio_buffer[lp]=tmp16;
	      break;
	      case 9:// overlay or
		tmp16=buf16[lp]-32768;
		tmp16+=tmp; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		audio_buffer[lp]|=tmp16;
		break;
	      case 10: // overlay +
		tmp16=buf16[lp]-32768;
		tmp16+=tmp+audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		audio_buffer[lp]=tmp16;
		break;
	      case 11: // overlay last
		tmp16=buf16[lp]-32768;
		tmp16+=tmp;
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		if (tmp16>lasttmp) {
		audio_buffer[lp]=tmp16;
		}
		lasttmp=tmp16;
		break;

	      case 12: // // overlay=all,effect=*
		tmp16=buf16[lp]-32768;
		tmp16*=tmp; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		audio_buffer[lp]=tmp16;
	      break;
	      case 13:// overlay or
		tmp16=buf16[lp]-32768;
		tmp16*=tmp; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		audio_buffer[lp]|=tmp16;
		break;
	      case 14: // overlay +
		tmp16=buf16[lp]-32768;
		tmp16*=tmp; 
		tmp16+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		audio_buffer[lp]=tmp16;
		break;
	      case 15: // overlay last
		tmp16=buf16[lp]-32768;
		tmp16*=tmp; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		if (tmp16>lasttmp) {
		audio_buffer[lp]=tmp16;
		}
		lasttmp=tmp16;
		break;
	      } ///end last switch 
		} //

	      if (++village_read[x].del>=village_read[x].step){
		dirry=village_read[x].dirry;
		samplepos+=dirry;//)&32767;
		if (samplepos>=village_read[x].wrap || samplepos<0){
		  village_read[x].running==0;
		if (dirry>0) samplepos=0;
		  else samplepos=village_read[x].wrap;		
		}
		village_read[x].samplepos=samplepos;// only need update here
		village_read[x].del=0;
	      }
	    }//running
	    } // howmanyreadvill
	  }//sz

	// WRITE! simplified to consecutive!! DONE- to test!
	
	  for (xx=0;xx<sz/2;xx++){
	  samplepos=village_write[whichwritevillager].samplepos;//)&32767;
    	    lp=(samplepos+village_write[whichwritevillager].start)&32767;
	    //	    lp=(samplepos)&32767; // TESTY!
	    mono_buffer[xx]=audio_buffer[lp];
	    if (++village_write[whichwritevillager].del>=village_write[whichwritevillager].step){
	      village_write[whichwritevillager].del=0;

		////try this// TESTY!
	      if (village_write[whichwritevillager].dir==2) dirry=newdir[wormdir];
	      else if (village_write[whichwritevillager].dir==3) dirry=dir[adc_buffer[DOWN]&1]*village_write[whichwritevillager].speed;
	      else dirry=village_write[whichwritevillager].dirry;

	      samplepos+=dirry;//)&32767;
	      if (samplepos>=village_write[whichwritevillager].wrap || samplepos<0){
		  /// next villager now
		if (dirry>0) samplepos=0;
		  else samplepos=village_write[whichwritevillager].wrap;
		village_write[whichwritevillager].samplepos=samplepos;
		whichwritevillager++; 
		whichwritevillager=whichwritevillager%howmanywritevill;		  //u8 /// move on to next
	      }
		else village_write[whichwritevillager].samplepos=samplepos;
	    }
	  }/// end of write!

#ifndef LACH
	  if (digfilterflag&1){

	  for (xx=0;xx<sz/2;xx++){
	    samplepos=village_filtout[whichfiltoutvillager].samplepos;
	    lp=(samplepos+village_filtout[whichfiltoutvillager].start)&32767;
	    left_buffer[xx]=audio_buffer[lp];
	    //	    left_buffer[xx]=0;
	  
	    if (++village_filtout[whichfiltoutvillager].del>=village_filtout[whichfiltoutvillager].step){
	      village_filtout[whichfiltoutvillager].del=0;

		////try this// TESTY!
		if (village_filtout[whichfiltoutvillager].dir==2) dirry=newdir[wormdir];
		else if (village_filtout[whichfiltoutvillager].dir==3) dirry=dir[adc_buffer[DOWN]&1]*village_filtout[whichfiltoutvillager].speed;
		else dirry=village_filtout[whichfiltoutvillager].dirry;
		samplepos+=dirry;//)&32767;
		if (samplepos>=village_filtout[whichfiltoutvillager].wrap || samplepos<0){
		  /// next villager now

		if (dirry>0) samplepos=0;
		  else samplepos=village_filtout[whichfiltoutvillager].wrap;
		village_filtout[whichfiltoutvillager].samplepos=samplepos;
		  whichfiltoutvillager++; 
		  whichfiltoutvillager=whichfiltoutvillager%howmanyfiltoutvill;		  //u8 /// move on to next
		}
		else village_filtout[whichfiltoutvillager].samplepos=samplepos;
	    }
	  }/// end of FILTwrite!
	  	  }//digfilterflag
#endif
	  /////////////////////////
	  // final combine

	  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

#ifdef PCSIM
//    for (x=0;x<sz/2;x++){
//         printf("%c",mono_buffer[x]);
// 	   }
#endif

      /////////////////////////////////////
	  // process 40106, hdhgener maximer and lmer

	  // 40106 always
	  //digfilterflag= 32.16.8.4.2.1=filterfeedin,switch_hardware,maxim,lm,40106,digfilter_process
#ifndef LACH
	  // always running!	  	  
	  x=which40106villager%howmany40106vill;

	  if (++village_40106[x].del>=village_40106[x].speed){
	    village_40106[x].del=0;
	  count40106+=village_40106[x].step;

	  if (village_40106[x].dir==2) dirry=newdir[wormdir];
	  else if (village_40106[x].dir==3) dirry=dir[adc_buffer[DOWN]&1]*village_40106[x].speed;
	  else dirry=village_40106[x].dirry;

	  tmp=village_40106[x].knoboffset>>4; // 11 bits=2048 for 40106 as we have 15 bits from loggy! 32768 MAX NOTE!
	  samplepos=village_40106[x].samplepos;
	  set40106pwm(tmp+(buf16[(village_40106[x].dataoffset+samplepos)&32767])%(2049-tmp));
	  samplepos+=dirry;
	  if (samplepos>=village_40106[x].length) samplepos=0;
	  else if (samplepos<0) samplepos=village_40106[x].length;
	  village_40106[x].samplepos=samplepos;

	  if (count40106>=village_40106[x].length){
	    count40106=0;
	    which40106villager++; //u8
	  }
	  }

	  // how looks without step:
	  /*	  if (++village_40106[x].del>=village_40106[x].speed){
	      village_40106[x].del=0;
	  tmp=village_40106[x].knoboffset>>4; // 11 bits=2048 for 40106 as we have 15 bits from loggy! 32768 MAX NOTE!
	  samplepos=village_40106[x].samplepos;
	  set40106pwm(tmp+(buf16[(village_40106[x].dataoffset+samplepos)&32767])%(2048-tmp));
	  samplepos+=village_40106[x].dirry;
	  if (samplepos>=village_40106[x].length) {
	    samplepos=0;
	    which40106villager++; //u8
	  }
	  else if (samplepos<0) {
	    samplepos=village_40106[x].length;
	    which40106villager++; //u8
	  }
	  village_40106[x].samplepos=samplepos;
	  }*/


	  // hdgener=16// note hdgener is 8 bits

	  
	  if (digfilterflag&16){
	  x=whichhdgenervillager%howmanyhdgenervill;
	    if (++village_hdgener[x].del>=village_hdgener[x].speed){
	      village_hdgener[x].del=0;

	    if (village_hdgener[x].dir==2) dirry=newdir[wormdir];
	    else if (village_hdgener[x].dir==3) dirry=dir[adc_buffer[DOWN]&1]*village_hdgener[x].speed;
	    else dirry=village_hdgener[x].dirry;


	      samplepos=village_hdgener[x].samplepos;
	      counthdgener+=village_hdgener[x].step;
	      tmp=village_hdgener[x].knoboffset>>7; // 8 bits
	      hdgener=(tmp+(datagenbuffer[(village_hdgener[x].dataoffset+samplepos)])%(256-tmp));
	      samplepos+=dirry;
	      if (samplepos>=village_hdgener[x].length) samplepos=0;
	      else if (samplepos<0) samplepos=village_hdgener[x].length;
	      village_hdgener[x].samplepos=samplepos;
	      if (counthdgener>=village_hdgener[x].length){
		counthdgener=0;
		whichhdgenervillager++; //u8
	      }
	    }
	  }
	  //	  hdgener=village_hdgener[x].knoboffset>>7;//TESTY!


	  // maxim=8 depth is 13 bits 8192

	  	  if (digfilterflag&8){ // TESTY!
	  x=whichmaximvillager%howmanymaximvill;
	    if (++village_maxim[x].del>=village_maxim[x].speed){
	      village_maxim[x].del=0;

	    if (village_maxim[x].dir==2) dirry=newdir[wormdir];
	    else if (village_maxim[x].dir==3) dirry=dir[adc_buffer[DOWN]&1]*village_maxim[x].speed;
	    else dirry=village_maxim[x].dirry;

	  countmaxim+=village_maxim[x].step;
	  tmp=village_maxim[x].knoboffset>>2; // 13 bits
	  samplepos=village_maxim[x].samplepos;
	  setmaximpwm(tmp+(buf16[(village_maxim[x].dataoffset+samplepos)&32767])%(8193-tmp));
	  samplepos+=dirry;
	  if (samplepos>=village_maxim[x].length) samplepos=0;
	  else if (samplepos<0) samplepos=village_maxim[x].length;
	  village_maxim[x].samplepos=samplepos;
	  if (countmaxim>=village_maxim[x].length){
	    countmaxim=0;
	    whichmaximvillager++; //u8
	  }
	    }
	    }
	  //	  setmaximpwm(128); // TESTY!
	  // lm=4
	  if (digfilterflag&4){
	  x=whichlmvillager%howmanylmvill;
	    // copy from above - change depth/2048

	    if (++village_lm[x].del>=village_lm[x].speed){
	      village_lm[x].del=0;

	      if (village_lm[x].dir==2) dirry=newdir[wormdir];
	      else if (village_lm[x].dir==3) dirry=dir[adc_buffer[DOWN]&1]*village_lm[x].speed;
	      else dirry=village_lm[x].dirry;

	  countlm+=village_lm[x].step;
	  tmp=village_lm[x].knoboffset>>3; // 12 bits=4096 for lm
	  samplepos=village_lm[x].samplepos;
	  setlmpwm(tmp+(buf16[(village_lm[x].dataoffset+samplepos)&32767])%(4097-tmp));
	  samplepos+=dirry;
	  if (samplepos>=village_lm[x].length) samplepos=0;
	  else if (samplepos<0) samplepos=village_lm[x].length;
	  village_lm[x].samplepos=samplepos;
	  if (countlm>=village_lm[x].length){
	    countlm=0;
	    whichlmvillager++; //u8
	  }
	  }
	  }
	  
	  // process village_hardware[whichhwvillager%howmany]	  
	  // max length is 4096 =5 seconds+depending on speed
	  //	  dohardwareswitch(village_hardware[x].setting, village_hardware[x].inp,hdgener);//TESTY

	  //	  dohardwareswitch(5, 0,hdgener);// TESTY for 40106
	  x=whichhwvillager%howmanyhardvill;
	  if (++village_hardware[x].del>=village_hardware[x].speed){
	    village_hardware[x].del=0;
	  counthw+=hardcompress; // with compression
	  dohardwareswitch(village_hardware[x].setting, village_hardware[x].inp,hdgener);
	  if (counthw>=village_hardware[x].length){
	    counthw=0;
	    whichhwvillager++; //u8
	    }
	    }
	   
#endif // LACH
#endif // for test effects
#endif // for test eeg
#endif // for straight

	  }
