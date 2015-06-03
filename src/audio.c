/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#ifdef TENE
#define UP 6
#define DOWN 8
#define LEFT 5
#define RIGHT 7
#else
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
  int16_t dirry,laststart;
  int32_t lasttmp=0,lasttmp16=0;
  register int32_t lp,samplepos;
  register int32_t tmp,tmpl,tmp16,tmp32d;

  u8 x,xx;
  u16 overlay;
  extern villagerw village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  static u8 whichwritevillager=0;
  extern u8 howmanywritevill,howmanyreadvill;

#ifndef LACH
  static u8 hdgener; 
  static u16 counthw=0;
  static int16_t count40106=0,counthdgener=0,countlm=0,countmaxim=0;
  //  static u8 which40106villager=0,whichlmvillager=0,whichhdgenervillager=0,whichmaximvillager=0,whichhwvillager=0,whichfiltoutvillager=0;
static u8 whichfiltoutvillager=0;
  //  extern u8 howmanyhardvill,howmany40106vill,howmanylmvill,howmanyhdgenervill,howmanymaximvill;
  extern u8 hardcompress;
  extern u8 howmanyfiltoutvill;
    extern villagerw village_filtout[MAX_VILLAGERS+1];
    /*  extern villager_hardware village_hardware[17];
  extern villager_hardwarehaha village_40106[17];
  extern villager_hardwarehaha village_hdgener[17];
  extern villager_hardwarehaha village_lm[17];
  extern villager_hardwarehaha village_maxim[17];*/
#endif

#ifdef TEST_EFFECTS
  static int16_t effect_buffer[32]; //was 32 TESTY
#endif

#ifdef TEST_EEG
  static u16 sampleposss=0;
#endif

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, mono_buffer);
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

#ifdef TEST_EEG
	// write buf16 into mono
	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[sampleposss&32767];//-32768;
	  sampleposss++;
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
#else
	    src++;
	    tmp=*(src++); 
#endif
	    laststart=0;
	    	    for (x=0;x<howmanyreadvill;x++){ // process other way round:
	    //   for(x=howmanyreadvill; x--; ){
		      overlay=village_read[x].overlay;
#ifndef LACH
	    if ((digfilterflag&1) && (overlay&32)) tmp=tmpl;
#endif
		   //	   overlay=16; // testy!
	    village_read[x].counterr+=village_read[x].dirryr;

	    if (village_read[x].counterr>=village_read[x].compress) {// whether still makes sense as ??? guess so!!!
	      village_read[x].counterr=0;
	      village_read[x].running=1;
	    }
	    if (village_read[x].compress==0) village_read[x].compress=1;
	    if ((village_read[x].offset%village_read[x].compress)<=village_read[x].counterr && village_read[x].running==1){
	      samplepos=village_read[x].samplepos;
	      //	      lp=(samplepos+village_read[x].start)&32767;

	      //	      tmpr=x-1;x
	      	      laststart+=village_read[x].start;
	      	      lp=(samplepos+laststart)&32767; // start is now added to the last start // rather than WRAP!
	      //	      lp=(samplepos+village_read[x].start)&32767; // start is now added to the last start // rather than WRAP!

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
	      buf16[lp]|=(tmp+32768);
	      audio_buffer[lp]|=tmp16;
	      break;
	      case 2:///+
	      tmp16=buf16[lp];
	      tmp32d=tmp16+tmp+32768;
	      tmp16+=audio_buffer[lp];
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp16) : [src] "r" (tmp16));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		buf16[lp]=tmp32d;// 
		audio_buffer[lp]=tmp16-32768;
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
		buf16[lp]|=(tmp16+32768);
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
		audio_buffer[lp]|=(tmp32d-32768);
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
		buf16[lp]=tmp32d+32768;// 
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:// overlay or
		tmp16=buf16[lp]-32768;
		tmp32d=tmp*tmp16; 
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  buf16[lp]|=(tmp32d+32768);
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
		  village_read[x].running=0;
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

	  samplepos=village_write[whichwritevillager].samplepos;//)&32767;
	  for (xx=0;xx<sz/2;xx++){
	  //    	    lp=(samplepos+village_write[whichwritevillager].start)&32767;
	    //	    lp=(samplepos)&32767; // TESTY!
	    //	      tmpr=whichwritevillager-1;
	      lp=(samplepos+village_write[whichwritevillager].start)&32767; 

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
		laststart=village_write[whichwritevillager].start;
		whichwritevillager++; 
		whichwritevillager=whichwritevillager%howmanywritevill;		  //u8 /// move on to next
		samplepos=village_write[whichwritevillager].samplepos;
		village_write[whichwritevillager].start+=laststart;
	      }
	      //		else village_write[whichwritevillager].samplepos=samplepos;
	    }
	  }/// end of write!
	  village_write[whichwritevillager].samplepos=samplepos;

#ifndef LACH
	  if (digfilterflag&1){

	    samplepos=village_filtout[whichfiltoutvillager].samplepos;

	  for (xx=0;xx<sz/2;xx++){

	    //	      tmpr=whichfiltoutvillager-1;
	      lp=(samplepos+village_filtout[whichfiltoutvillager].start)&32767; 

	    //	    lp=(samplepos+village_filtout[whichfiltoutvillager].start)&32767;
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
		laststart=village_filtout[whichfiltoutvillager].start;
		whichfiltoutvillager++; 
		whichfiltoutvillager=whichfiltoutvillager%howmanyfiltoutvill;		  //u8 /// move on to next
		samplepos=village_filtout[whichfiltoutvillager].samplepos;
		village_filtout[whichfiltoutvillager].start+=laststart;
		}
		//		else village_filtout[whichfiltoutvillager].samplepos=samplepos;
	    }
	  }/// end of FILTwrite!
	  village_filtout[whichfiltoutvillager].samplepos=samplepos;

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

	   
#endif // for test effects
#endif // for test eeg
#endif // for straight

	  }
