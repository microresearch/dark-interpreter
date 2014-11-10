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
right_buffer=malloc(MONO_BUFSZ*sizeof(int16_t));
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
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define float float32_t
#endif

extern int16_t newdirection[8];//={-256,-255,1,255,256,254,-1,-257};

extern signed char direction[2];
extern u8 wormdir;
extern u8 wormflag[10];
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


u8 fingerdir(u8 *speedmod);

extern u8 howmanywritevill,howmanyfiltinvill,howmanyfiltoutvill,howmanyreadvill;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  int lasttmp=0,lasttmp16=0;
  //  u16 lp,tmpw,tmps,tmpp;
  u16 lp, tmpp;
  u8 x,xx,spd;
  static u8 hdgener; 
  //  u8 whichvillager,step;
  float32_t fsum,fsumd;
  int16_t tmp,tmp16;
  static int16_t count=0,countf=0,countff=0,countr=0,count40106=0,counthdgener=0,countlm=0,countmaxim=0;//countr and as static is testy!
  int32_t tmp32,tmp32d,tmptmp32;
  static u8 which40106villager=0,whichlmvillager=0,whichhdgenervillager=0,whichmaximvillager=0,whichhwvillager=1,readoverlay=0;
  extern u8 howmanyhardvill,howmany40106vill,howmanylmvill,howmanyhdgenervill,howmanymaximvill;
  extern u8 hardcompress;
  static u8 whichw=0,whichr=0,delread=0,delwrite=0,delfiltin=0,delfiltout=0;
  extern u8 readspeed,writespeed,filtinspeed,filtoutspeed;
  extern int16_t dirryw,dirryr,dirryf,dirryff;

  static u16 counter=0,counterr=0,counthw=0;
  //  static u16 readbegin=0,readend=32767,readoffset=32768,writebegin=0,writeend=32767,writeoffset=32768, readstartoffset=0,writestartoffset=0;
  extern u8 howmanydatavill, howmanyeffectvill,howmanydatagenwalkervill;;
  //  u8 mainmode;
  extern villagerr village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  extern villagerr village_filtin[MAX_VILLAGERS+1];
  extern villagerr village_filtout[MAX_VILLAGERS+1];
  extern villager_datagenwalker village_datagenwalker[MAX_VILLAGERS+1];
  extern villager_generic village_datagen[MAX_VILLAGERS+1];
  extern villager_effect village_effect[17];
  extern villager_hardware village_hardware[17];
  extern villager_hardwarehaha village_40106[17];
  extern villager_hardwarehaha village_hdgener[17];
  extern villager_hardwarehaha village_lm[17];
  extern villager_hardwarehaha village_maxim[17];

  extern u16 databegin,dataend,counterd;
  extern u8 dataspeed;  
  extern int16_t dirryd;
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
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;

	/// HARDWARE at start

#ifdef TEST_EEG
	// write buf16 into mono
	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos%32768];//-32768;
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
	    *ldst++=*(src++);
	    tmp=*(src); 
	    *rdst++=*(src++);
	  delread++;
	  if (delread>=readspeed) {
	    delread=0;
	      }
	  lasttmp=0,lasttmp16=0;

	  for (x=0;x<howmanyreadvill;x++){
	    tmpp=village_read[x].compress; if (tmpp==0) tmpp=1;

	    if (delread==0) village_read[x].counterr+=dirryr;
	    if (village_read[x].counterr>=(32768/tmpp)) {
	      village_read[x].counterr=0;
	      village_read[x].running=1;
	    }
	    if ((village_read[x].offset/tmpp)<=village_read[x].counterr && village_read[x].running==1){

	    lp=village_read[x].samplepos%32768;

	      // TODO: we (could) have: overlay as:
	      // effect (=,&,+,*)=4=2 bits
	      // overlay(=,|,+,last)=4=2 bits - 16 bits
	      // Fmodded always on effects// also inv mod - where is fmod again? effect and effectinv
	      // constrained or not 1 bit = 5 bits=32+top=64TOTAL constrain=overlay&16
	      // top bit is swop or not - 2 sets of cases as before
	      //	      village_read[x].overlay=2; // TESTY!!!
	      if (village_read[x].overlay&32){ // datagen business readin! - top bit=32
	      tmp16=buf16[lp]-32768;
	      switch(village_read[x].overlay&15){
	      case 0: // straight. no fmod
		buf16[lp]=tmp+32768;
		audio_buffer[lp]=tmp16;
	      break;
	      case 1:
		buf16[lp]|=tmp+32768;
		audio_buffer[lp]|=tmp16;
	      break;
	      case 2:
		tmp32d=buf16[lp]+tmp;
		tmp32=audio_buffer[lp]+tmp16;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 3:
		if (tmp>lasttmp) buf16[lp]=tmp+32768;
		if (tmp16>lasttmp16) audio_buffer[lp]=tmp16;
		lasttmp=tmp; lasttmp16=tmp16;
	      break;
	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32&=tmp32d;
		tmp32d&=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32&=tmp32d;
		tmp32d&=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32+32768;
		audio_buffer[lp]|=tmp32d;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32&=tmp32d;
		tmp32d&=tmptmp32;

		tmp32d=buf16[lp]+tmp32;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
		break;
	      case 7:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32&=tmp32d;
		tmp32d&=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32>lasttmp16) buf16[lp]=tmp32+32768;
		if (tmp32d>lasttmp16) audio_buffer[lp]=tmp32d;
		lasttmp=tmp32; lasttmp16=tmp32d;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32+32768;
		audio_buffer[lp]|=tmp32d;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;

		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		tmp32d=buf16[lp]+tmp32;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 11:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32>lasttmp16) buf16[lp]=tmp32+32768;
		if (tmp32d>lasttmp16) audio_buffer[lp]=tmp32d;
		lasttmp=tmp32; lasttmp16=tmp32d;
	      break;

	      case 12: // effect as * with fmod
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32+32768;
		audio_buffer[lp]|=tmp32d;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;

		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		tmp32d=buf16[lp]*tmp32;
		tmp32=audio_buffer[lp]*tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 15:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32>lasttmp16) buf16[lp]=tmp32+32768;
		if (tmp32d>lasttmp16) audio_buffer[lp]=tmp32d;
		lasttmp=tmp32; lasttmp16=tmp32d;
	      break;
		

	      }// switch
	      }
	      else // straight UP
		{
	      tmp16=buf16[lp]-32768;
	      switch(village_read[x].overlay&15){
	      case 0: // straight. no fmod
		audio_buffer[lp]=tmp;
	      break;
	      case 1:
		audio_buffer[lp]|=tmp;
	      break;
	      case 2:
		tmp32=audio_buffer[lp]+tmp;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) audio_buffer[lp]=tmp;
		lasttmp=tmp;
	      break;

	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as + with fmod
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;
	      /////

	      } ///end last switch 
		} //
	    
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
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


#ifndef LACH
	  // READFILTIN - effects are across LEFT and right

	  ldst=left_buffer;
	  rdst=right_buffer;

	  for (xx=0;xx<sz/2;xx++){

	    tmp=*(ldst++); // left
	    tmp16=*(rdst++); // right

	  delfiltin++;
	  if (delfiltin>=filtinspeed) {
	    delfiltin=0;
	      }
	  lasttmp=0;
	  for (x=0;x<howmanyfiltinvill;x++){

	    tmpp=village_filtin[x].compress; if (tmpp==0) tmpp=1;

	    if (delfiltin==0) village_filtin[x].counterr+=dirryf;
	    if (village_filtin[x].counterr>=(32768/tmpp)) {
	      village_filtin[x].counterr=0;
	      village_filtin[x].running=1;
	    }

	    if ((village_filtin[x].offset/tmpp)<=village_filtin[x].counterr && village_filtin[x].running==1){

	    lp=village_filtin[x].samplepos%32768;

	    // switcher:
	      switch(village_filtin[x].overlay&15){
	      case 0: // straight. no fmod
		audio_buffer[lp]=tmp;
	      break;
	      case 1:
		audio_buffer[lp]|=tmp;
	      break;
	      case 2:
		tmp32=audio_buffer[lp]+tmp;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) audio_buffer[lp]=tmp;
		lasttmp=tmp;
	      break;
	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as + with fmod
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;
	      /////

	    }// end of switcher

	      if (++village_filtin[x].del>=village_filtin[x].step){
	      countf=((village_filtin[x].samplepos-village_filtin[x].start)+village_filtin[x].dirry);
	      if (countf<village_filtin[x].wrap && countf>0)
	      {
		village_filtin[x].samplepos+=village_filtin[x].dirry;//)%32768;
		  }
	      else
		{
		  village_filtin[x].running==0;
		if (village_filtin[x].dir==2) village_filtin[x].dirry=newdirection[wormdir];
		else if (village_filtin[x].dir==3) village_filtin[x].dirry=direction[adc_buffer[DOWN]&1]*village_filtin[x].speed;
		else village_filtin[x].dirry=direction[village_filtin[x].dir]*village_filtin[x].speed;
		if (village_filtin[x].dirry>0) village_filtin[x].samplepos=village_filtin[x].start;
		  else village_filtin[x].samplepos=village_filtin[x].start+village_filtin[x].wrap;
		}
	    village_filtin[x].del=0;
	      }
	    }
	  }
	  }

	  // WRITEFILTOUT - effects are across LEFT in and audio_buffer???
	  // NOW AS: or buf16 and audio???

	  //	  ldst=left_buffer;
	  //	  rdst=right_buffer;

	  for (xx=0;xx<sz/2;xx++){
	    left_buffer[xx]=0;

	  delfiltout++;
	  if (delfiltout>=filtoutspeed) {
	    delfiltout=0;
	      }
	  lasttmp=0;

	  for (x=0;x<howmanyfiltoutvill;x++){

	    tmpp=village_filtout[x].compress; if (tmpp==0) tmpp=1;

	  if (delfiltout==0) village_filtout[x].counterr+=dirryff;
	  if (village_filtout[x].counterr>=(32768/tmpp)) {
	    village_filtout[x].counterr=0;
	    village_filtout[x].running=1;
	  }

	    if ((village_filtout[x].offset/tmpp)<=village_filtout[x].counterr && village_filtout[x].running==1){

	      lp=village_filtout[x].samplepos%32768;
	      tmp16=buf16[lp]-32768;
	      tmp=audio_buffer[lp];

	      switch(village_filtout[x].overlay&15){
	      case 0: // straight. no fmod
		left_buffer[xx]=tmp;
		break;
	      case 1:
		left_buffer[xx]|=tmp;
	      break;
	      case 2:
		tmp32=left_buffer[xx]+tmp;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) left_buffer[xx]=tmp;
		lasttmp=tmp;
	      break;

	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=left_buffer[xx]+tmp32;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) left_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=left_buffer[xx]+tmp32;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) left_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as * with fmod
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=left_buffer[xx]+tmp32;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) left_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;
	      ////////////////////////
	      } // end of overlay switch
	      //////
	      if (++village_filtout[x].del>=village_filtout[x].step){
	      countff=((village_filtout[x].samplepos-village_filtout[x].start)+village_filtout[x].dirry);
	      if (countff<village_filtout[x].wrap && countff>=0)
	      {
		village_filtout[x].samplepos+=village_filtout[x].dirry;//)%32768;
		  }
	      else
		{
		  village_filtout[x].running=0;
		if (village_filtout[x].dir==2) village_filtout[x].dirry=newdirection[wormdir];
		if (village_filtout[x].dir==3) village_filtout[x].dirry=direction[adc_buffer[DOWN]&1]*village_filtout[x].speed;
		else village_filtout[x].dirry=direction[village_filtout[x].dir]*village_filtout[x].speed;

		if (village_filtout[x].dirry>0) village_filtout[x].samplepos=village_filtout[x].start;
		  else village_filtout[x].samplepos=village_filtout[x].start+village_filtout[x].wrap;
		}
	    village_filtout[x].del=0;
	    }
	    }
	  }
	  }
#endif

	// WRITE!

	  for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;

	  delwrite++;
	  if (delwrite>=writespeed) {
	    delwrite=0;
	      }

	  lasttmp=0;
	  for (x=0;x<howmanywritevill;x++){
	  if (delwrite==0) village_write[x].counterr+=dirryw;

	  tmpp=village_write[x].compress; if (tmpp==0) tmpp=1;

	  if (village_write[x].counterr>=(32768/tmpp)) {
	    village_write[x].counterr=0;
	    village_write[x].running=1;
	  }

	    if ((village_write[x].offset/tmpp)<=village_write[x].counterr && village_write[x].running==1){

	      lp=village_write[x].samplepos%32768;
	      tmp16=buf16[lp]-32768;
	      tmp=audio_buffer[lp];
	      switch(village_write[x].overlay&15){
	      case 0: // straight. no fmod
		mono_buffer[xx]=tmp;
		break;
	      case 1:
		mono_buffer[xx]|=tmp;
	      break;
	      case 2:
		tmp32=mono_buffer[xx]+tmp;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) mono_buffer[xx]=tmp;
		lasttmp=tmp;
	      break;

	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=mono_buffer[xx]+tmp32;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) mono_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=mono_buffer[xx]+tmp32;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) mono_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as * with fmod
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=mono_buffer[xx]+tmp32;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) mono_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;
	      ////////////////////////
	      } // end of overlay switch
	      //////
	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>=0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	    }
	    }
	  }
}

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

	  set40106pwm(tmp+(buf16[(village_40106[x].dataoffset+village_40106[x].samplepos)%32768])%(2048-tmp));

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

	  hdgener=(tmp+(buf16[(village_hdgener[x].dataoffset+village_hdgener[x].samplepos)%32768])%(255-tmp));

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
	  setmaximpwm(tmp+(buf16[(village_maxim[x].dataoffset+village_maxim[x].samplepos)%32768])%(8192-tmp));
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

	  setlmpwm(tmp+(buf16[(village_lm[x].dataoffset+village_lm[x].samplepos)%32768])%(4096-tmp));

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
