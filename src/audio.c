/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

///#define M_PI 3.14159265358979323846

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
//#include "settings.h"
extern int16_t *adc_buffer;
int16_t *audio_buffer;
#define  uint32_t int
typedef int int32_t;
#define float32_t float
int16_t	*left_buffer, *right_buffer, *mono_buffer;

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
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
int16_t	left_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define float float32_t
#endif

extern int16_t newdirection[8];//={-256,-255,1,255,256,254,-1,-257};

extern signed char direction[2];
extern u8 wormdir;
extern u8 wormflag[10];
extern u8 digfilterflag;

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


u8 fingerdir(u8 *speedmod);

extern u8 howmanywritevill,howmanyfiltinvill,howmanyfiltoutvill,howmanyreadvill;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  int32_t lasttmp=0,lasttmp16=0;
  //  u16 lp,tmpw,tmps,tmpp;
  u16 lp;
  u8 x,xx,spd;
  static u8 hdgener; 
  //  u8 whichvillager,step;
  float32_t fsum,fsumd;
  int16_t tmp,tmpl,tmp16,tmptmp,tmptmp16;
  static int16_t count=0,countf=0,countff=0,countr=0,count40106=0,counthdgener=0,countlm=0,countmaxim=0;//countr and as static is testy!
  int32_t tmp32,tmp32d;
  static u8 which40106villager=0,whichlmvillager=0,whichhdgenervillager=0,whichmaximvillager=0,whichhwvillager=1,whichwritevillager=0,whichfiltoutvillager=0,readoverlay=0;
  extern u8 howmanyhardvill,howmany40106vill,howmanylmvill,howmanyhdgenervill,howmanymaximvill;
  extern u8 hardcompress;
  static u8 whichw=0,whichr=0,delread=0,delwrite=0,delfiltin=0,delfiltout=0;
  //  extern u8 readspeed,writespeed,filtinspeed,filtoutspeed;
  //  extern int16_t dirryw,dirryr,dirryf,dirryff;

  static u16 counter=0,counterr=0,counthw=0;
  //  static u16 readbegin=0,readend=32767,readoffset=32768,writebegin=0,writeend=32767,writeoffset=32768, readstartoffset=0,writestartoffset=0;
  //  extern u8 howmanydatavill, howmanyeffectvill,howmanydatagenwalkervill;;
  //  u8 mainmode;
  extern villagerw village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  extern villagerw village_filtout[MAX_VILLAGERS+1];
  //  extern villager_datagenwalker village_datagenwalker[MAX_VILLAGERS+1];
  //  extern villager_generic village_datagen[MAX_VILLAGERS+1];
  //  extern villager_effect village_effect[17];
  extern villager_hardware village_hardware[17];
  extern villager_hardwarehaha village_40106[17];
  extern villager_hardwarehaha village_hdgener[17];
  extern villager_hardwarehaha village_lm[17];
  extern villager_hardwarehaha village_maxim[17];

  //  extern u16 databegin,dataend,counterd;
  //  extern u8 dataspeed;  
  //  extern int16_t dirryd;
  //  static u16 samplepos=0; // TESTY!

#ifdef TEST_EFFECTS
  static int16_t effect_buffer[32]; //was 32 TESTY
#endif

#ifdef TEST_EEG
  static u16 samplepos=0;
#endif

  //  howmanywritevill=64; // TESTY!
  //  howmanyreadvill=16; // TESTY!

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, mono_buffer);
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	/// HARDWARE at start

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

	// write to mono_buffer
	for (x=0;x<sz/2;x++){
	  	  mono_buffer[x]=effect_buffer[x];//-32768;
	  //	  mono_buffer[x]=audio_buffer[x];//-32768;
	  //	  countr++; if (countr==32) countr=0;
	  }
	// out!
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else
	//////////////////////////////////////////////////////////	

	// READ! 

	  for (xx=0;xx<sz/2;xx++){
#ifndef LACH
	    tmpl=*(src++);
#else
	    src++:
#endif
	    tmp=*(src++); 
	    lasttmp=0,lasttmp16=0;

	    for (x=0;x<howmanyreadvill;x++){
	      //	      village_read[x].overlay|=16; //TESTY!
#ifndef LACH
	    if (digfilterflag && village_read[x].overlay&16) tmp=tmpl;
#endif
	    village_read[x].counterr+=village_read[x].dirryr;
	    if (village_read[x].counterr>=village_read[x].compress) {// whether still makes sense as ??? guess so!!!
	      village_read[x].counterr=0;
	      village_read[x].running=1;
	    }
	    if (village_read[x].offset<=village_read[x].counterr && village_read[x].running==1){

	      lp=village_read[x].samplepos&32767;
	    tmp16=buf16[lp]-32768;

	    //	    village_read[x].overlay=32; // TESTY for datagens!
	      if (village_read[x].overlay&32){ // datagen business readin! - top bit=32
		// 32 is swop datagen/16 could be leftIN rather than ssat//rest is overlay and effect
	      switch(village_read[x].overlay&15){
	      case 0: // overlay=all,effect=straight
		buf16[lp]=tmp+32768; // TESTY!
		audio_buffer[lp]=tmp16;
	      break;
	      case 1://or
		buf16[lp]|=tmp+32768;
		audio_buffer[lp]|=tmp16;
	      break;
	      case 2:///+
		tmp32d=buf16[lp]+tmp;
		tmp32=audio_buffer[lp]+tmp16;
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 3://last
		if (tmp>lasttmp) buf16[lp]=tmp+32768;
		if (tmp16>lasttmp16) audio_buffer[lp]=tmp16;
		lasttmp=tmp; lasttmp16=tmp16;
	      break;
	      case 4: // // overlay=all,effect=&
		tmp16&=tmp;
		buf16[lp]=tmp16+32768;
		audio_buffer[lp]=tmp16;
	      break;
	      case 5:// overlay or
		tmp16&=tmp;
		buf16[lp]|=tmp16+32768;
		audio_buffer[lp]|=tmp16;
		break;
	      case 6: // overlay +
		tmp16&=tmp;
		tmp32d=buf16[lp]+tmp16;
		tmp32=audio_buffer[lp]+tmp16;
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 7: // overlay last
		tmp16&=tmp;
		if (tmp16>lasttmp) {
		  buf16[lp]=tmp16+32768;
		  audio_buffer[lp]=tmp16;
	      }
		lasttmp=tmp16;
		break;
	      case 8: // // overlay=all,effect=+
		tmp32d=tmp+tmp16; 
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
	      break;
	      case 9:// overlay or
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]|=tmp32d+32768;
		audio_buffer[lp]|=tmp32d;
		break;
	      case 10: // overlay +
		tmp32d=tmp+tmp16; 
		tmp32=buf16[lp]+tmp32d;
		tmp32d+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
		break;
	      case 11: // overlay last
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		if (tmp32d>lasttmp) {
		  buf16[lp]=tmp32d+32768;
		audio_buffer[lp]=tmp32d;
		}
		lasttmp=tmp32d;
		break;
	      case 12: // // overlay=all,effect=*
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:// overlay or
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]|=tmp32d+32768;
		audio_buffer[lp]|=tmp32d;
		break;
	      case 14: // overlay +
		tmp32d=tmp*tmp16; 
		tmp32=buf16[lp]+tmp32d;
		tmp32d+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
		break;
	      case 15: // overlay last
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
	      switch(village_read[x].overlay&15){
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
		tmp16&=tmp;
		audio_buffer[lp]=tmp16;
	      break;
	      case 5:// overlay or
		tmp16&=tmp;
		audio_buffer[lp]|=tmp16;
		break;
	      case 6: // overlay +
		tmp16&=tmp;
		tmp32=audio_buffer[lp]+tmp16;
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp32;
		break;
	      case 7: // overlay last
		tmp16&=tmp;
		if (tmp16>lasttmp) {
		  audio_buffer[lp]=tmp16;
	      }
		lasttmp=tmp16;
		break;
	      case 8: // // overlay=all,effect=+
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp32d;
	      break;
	      case 9:// overlay or
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]|=tmp32d;
		break;
	      case 10: // overlay +
		tmp32d=tmp+tmp16+audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp32d;
		break;
	      case 11: // overlay last
		tmp32d=tmp+tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		if (tmp32d>lasttmp) {
		audio_buffer[lp]=tmp32d;
		}
		lasttmp=tmp32d;
		break;
	      case 12: // // overlay=all,effect=*
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:// overlay or
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]|=tmp32d;
		break;
	      case 14: // overlay +
		tmp32d=tmp*tmp16; 
		tmp32d+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		audio_buffer[lp]=tmp32d;
		break;
	      case 15: // overlay last
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		if (tmp32d>lasttmp) {
		audio_buffer[lp]=tmp32d;
		}
		lasttmp=tmp32d;
		break;
	      } ///end last switch 
		} //
	      ////
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)&32767;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	  }
	// WRITE! simplified to consecutive!! DONE- to test!
	  for (xx=0;xx<sz/2;xx++){
	    lp=village_write[whichwritevillager].samplepos&32767;
	    mono_buffer[xx]=audio_buffer[lp];
	    if (++village_write[whichwritevillager].del>=village_write[whichwritevillager].step){
	      village_write[whichwritevillager].del=0;

	       count=((village_write[whichwritevillager].samplepos-village_write[whichwritevillager].start)+village_write[whichwritevillager].dirry);
	      if (count<village_write[whichwritevillager].wrap && count>0)
		{
		  village_write[whichwritevillager].samplepos+=village_write[whichwritevillager].dirry;//)&32767;
		}
	      else
		{
		  //		village_write[whichwritevillager].running==0;
		  if (village_write[whichwritevillager].dir==2) village_write[whichwritevillager].dirry=newdirection[wormdir];
		  else if (village_write[whichwritevillager].dir==3) village_write[whichwritevillager].dirry=direction[adc_buffer[DOWN]&1]*village_write[whichwritevillager].speed;
		  else village_write[whichwritevillager].dirry=direction[village_write[whichwritevillager].dir]*village_write[whichwritevillager].speed;
		  if (village_write[whichwritevillager].dirry>0) village_write[whichwritevillager].samplepos=village_write[whichwritevillager].start;
		  else village_write[whichwritevillager].samplepos=village_write[whichwritevillager].start+village_write[whichwritevillager].wrap;
		  whichwritevillager++; 
		  whichwritevillager=whichwritevillager%howmanywritevill;		  //u8 /// move on to next
		}
		}
	    }/// end of write!

	  // TODO: filtout as above!
#ifndef LACH
	  if (digfilterflag){

	  for (xx=0;xx<sz/2;xx++){
	    lp=village_filtout[whichfiltoutvillager].samplepos&32767;
	    left_buffer[xx]=audio_buffer[lp];
	  
	    if (++village_filtout[whichfiltoutvillager].del>=village_filtout[whichfiltoutvillager].step){
	      village_filtout[whichfiltoutvillager].del=0;
	      count=((village_filtout[whichfiltoutvillager].samplepos-village_filtout[whichfiltoutvillager].start)+village_filtout[whichfiltoutvillager].dirry);
	      if (count<village_filtout[whichfiltoutvillager].wrap && count>0)
		{
		  village_filtout[whichfiltoutvillager].samplepos+=village_filtout[whichfiltoutvillager].dirry;//)&32767;
		}
	      else
		{
		  //		village_filtout[whichfiltoutvillager].running==0;
		  if (village_filtout[whichfiltoutvillager].dir==2) village_filtout[whichfiltoutvillager].dirry=newdirection[wormdir];
		  else if (village_filtout[whichfiltoutvillager].dir==3) village_filtout[whichfiltoutvillager].dirry=direction[adc_buffer[DOWN]&1]*village_filtout[whichfiltoutvillager].speed;
		  else village_filtout[whichfiltoutvillager].dirry=direction[village_filtout[whichfiltoutvillager].dir]*village_filtout[whichfiltoutvillager].speed;
		  if (village_filtout[whichfiltoutvillager].dirry>0) village_filtout[whichfiltoutvillager].samplepos=village_filtout[whichfiltoutvillager].start;
		  else village_filtout[whichfiltoutvillager].samplepos=village_filtout[whichfiltoutvillager].start+village_filtout[whichfiltoutvillager].wrap;
		  whichfiltoutvillager++; 
		  whichfiltoutvillager=whichfiltoutvillager%howmanyfiltoutvill;		  //u8 /// move on to next
		}
	    }
	  }/// end of FILTwrite!
}
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
	  	  
	  x=which40106villager%howmany40106vill;
	  count40106+=village_40106[x].step;
	  tmp=village_40106[x].knoboffset>>4; // 11 bits=2048 for 40106 as we have 15 bits from loggy! 32768 MAX NOTE!

	  set40106pwm(tmp+(buf16[(village_40106[x].dataoffset+village_40106[x].samplepos)&32767])%(2048-tmp));

	  village_40106[x].samplepos+=village_40106[x].dirry;
	  if (village_40106[x].samplepos>=village_40106[x].length) village_40106[x].samplepos=0;
	  else if (village_40106[x].samplepos<0) village_40106[x].samplepos=village_40106[x].length;

	  if (count40106>=village_40106[x].length){
	    count40106=0;
	    which40106villager++; //u8
	  }

	  // hdgener=16// note hdgener is 8 bits
	  if (digfilterflag&16){
	  x=whichhdgenervillager%howmanyhdgenervill;
	  counthdgener+=village_hdgener[x].step;
	  tmp=village_hdgener[x].knoboffset>>7; // 8 bits

	  hdgener=(tmp+(buf16[(village_hdgener[x].dataoffset+village_hdgener[x].samplepos)&32767])%(255-tmp));

	  village_hdgener[x].samplepos+=village_hdgener[x].dirry;
	  if (village_hdgener[x].samplepos>=village_hdgener[x].length) village_hdgener[x].samplepos=0;
	  else if (village_hdgener[x].samplepos<0) village_hdgener[x].samplepos=village_hdgener[x].length;

	  if (counthdgener>=village_hdgener[x].length){
	    counthdgener=0;
	    whichhdgenervillager++; //u8
	  }
	  }

	  // maxim=8 depth is 13 bits 8192
	  if (digfilterflag&8){
	  x=whichmaximvillager%howmanymaximvill;
	  countmaxim+=village_maxim[x].step;
	  tmp=village_maxim[x].knoboffset>>2; // 13 bits
	  setmaximpwm(tmp+(buf16[(village_maxim[x].dataoffset+village_maxim[x].samplepos)&32767])%(8192-tmp));
	  village_maxim[x].samplepos+=village_maxim[x].dirry;
	  if (village_maxim[x].samplepos>=village_maxim[x].length) village_maxim[x].samplepos=0;
	  else if (village_maxim[x].samplepos<0) village_maxim[x].samplepos=village_maxim[x].length;

	  if (countmaxim>=village_maxim[x].length){
	    countmaxim=0;
	    whichmaximvillager++; //u8
	  }
	  }
	  
	  // lm=4
	  if (digfilterflag&4){
	    // copy from above - change depth/2048
	  x=whichlmvillager%howmanylmvill;
	  countlm+=village_lm[x].step;
	  tmp=village_lm[x].knoboffset>>3; // 12 bits=4096 for lm

	  setlmpwm(tmp+(buf16[(village_lm[x].dataoffset+village_lm[x].samplepos)&32767])%(4096-tmp));

	  village_lm[x].samplepos+=village_lm[x].dirry;
	  if (village_lm[x].samplepos>=village_lm[x].length) village_lm[x].samplepos=0;
	  else if (village_lm[x].samplepos<0) village_lm[x].samplepos=village_lm[x].length;

	  if (countlm>=village_lm[x].length){
	    countlm=0;
	    whichlmvillager++; //u8
	  }
	  }
	  
	  // process village_hardware[whichhwvillager%howmany]	  
	  // max length is 4096 =2.7 seconds

	  //	  dohardwareswitch(5, 0,hdgener);// TESTY for 40106

	  counthw+=hardcompress; // with compression
	  x=whichhwvillager%howmanyhardvill;
	  dohardwareswitch(village_hardware[x].setting, village_hardware[x].inp,hdgener);
	  if (counthw>=village_hardware[x].length){
	    counthw=0;
	    whichhwvillager++; //u8
	    }
	   
#endif // for test effects
#endif // for test eeg
#endif // for straight

}
