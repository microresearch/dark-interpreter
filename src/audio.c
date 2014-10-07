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
//#include "settings.h"
#include "hardware.h"
#include "simulation.h"
extern __IO uint16_t adc_buffer[10];
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define float float32_t
#endif

int16_t newdirection[8]={-256,-255,1,255,256,254,-1,-257};

extern signed char direction[2];
extern u8 EFFECTWRITE, EFFECTREAD,EFFECTFILTER;
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

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  u8 xx,spd;
  u8 mainmode, whichvillager,step;
  u16 count;
  static u8 howmanywritevill=1,howmanyreadvill=1;
  static u8 speedw=1,speed=1,whichw=0,whichr=0,delread=0,delwrite=0,dirryw=1,dirry=1;
  static u16 sampleposwrite=0,sampleposread=0,startread=0,startwrite=0,wrapread=32767,wrapwrite=32767;

  extern villagerr village_write[MAX_VILLAGERS];
  extern villagerr village_read[MAX_VILLAGERS];
  extern villagerr village_filt[MAX_VILLAGERS];


#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;

#ifdef TEST_EEG
	// write buf16 into mono
	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[sampleposread%32768];//-32768;
	  sampleposread++;
	}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	// TODO all audio!
	// 1- set villagers

	// if fingerdown we set appropriate villager based on mode
	
	xx=fingerdir(&spd);

	if (xx!=5){
	  // which mode are we in?
	  mainmode=adc_buffer[FIFTH]>>8; // 4 bits=16

	  switch(mainmode){
	  case 0:// WRITE
	    whichvillager=adc_buffer[FIRST]>>5; // 7 bits=128
	    howmanywritevill=whichvillager+1;
	    village_write[whichvillager].start=adc_buffer[SECOND]<<3; // to do as logarithmic
	    village_write[whichvillager].wrap=adc_buffer[THIRD]<<3; // to do as logarithmic
	    village_write[whichvillager].effect=adc_buffer[FOURTH]>>4; // 8 bits so far 
	    village_write[whichvillager].dir=xx;
	    village_write[whichvillager].speed_step=spd; // check how many bits is spd? 8 as changed in main.c 
	    break;
	  case 1:// READ
	    whichvillager=adc_buffer[FIRST]>>5; // 7 bits=128
	    howmanyreadvill=whichvillager+1;
	    village_read[whichvillager].start=adc_buffer[SECOND]<<3; // to do as logarithmic
	    village_read[whichvillager].wrap=adc_buffer[THIRD]<<3; // to do as logarithmic
	    village_read[whichvillager].effect=adc_buffer[FOURTH]>>4; // 8 bits so far 
	    village_read[whichvillager].dir=xx;
	    village_read[whichvillager].speed_step=spd; // check how many bits is spd? 8 as changed in main.c 

	    break;
	}
	}
	//////	
	// process villagers - first attempt sans effects

	// READ!
      	for (xx=0;xx<sz/2;xx++){
	  src++;
	  audio_buffer[sampleposread%32768]=*(src++);

	  if (++delread>=speed){
	    count=((sampleposread-startread)+dirry);
	    if (count<wrapread && count>0)
	      {
		sampleposread+=dirry;//)%32768;
		  }
		else {
		  // on to next villager
		  whichr++;
		  whichr=whichr%howmanyreadvill;
		  startread=village_read[whichvillager].start;
		  wrapread=village_read[whichvillager].wrap;
		  if (wrapread==0) wrapread=1;

		  step=(village_read[whichr].speed_step%16)+1;
		  speed=(village_read[whichr].speed_step/16)+1;

		  if (village_read[whichr].dir==2) dirry=newdirection[wormdir];
		  else if (village_read[whichr].dir==3) dirry=direction[adc_buffer[DOWN]&1]*step;
		  else dirry=direction[village_read[whichr].dir]*step;

		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		}
	  delread=0;
	  } // close delread
	}

	// WRITE!

	ldst=left_buffer;

      	for (xx=0;xx<sz/2;xx++){
	  ldst++;
	  mono_buffer[xx]=audio_buffer[sampleposwrite%32768];

	  if (++delwrite>=speedw){
	    count=((sampleposwrite-startwrite)+dirryw);
	    if (count<wrapread && count>0)
	      {
		sampleposwrite+=dirryw;//)%32768;
		  }
		else {
		  // on to next villager
		  whichw++;
		  whichw=whichw%howmanywritevill;
		  startwrite=village_write[whichvillager].start;
		  wrapwrite=village_write[whichvillager].wrap;
		  if (wrapwrite==0) wrapwrite=1;

		  step=(village_write[whichw].speed_step%16)+1;
		  speedw=(village_write[whichw].speed_step/16)+1;

		  if (village_write[whichw].dir==2) dirryw=newdirection[wormdir];
		  else if (village_write[whichw].dir==3) dirryw=direction[adc_buffer[DOWN]&1]*step;
		  else dirryw=direction[village_write[whichw].dir]*step;

		  if (dirryw>0) sampleposwrite=startwrite;
		  else sampleposwrite=startwrite+wrapwrite;
		  
		}
	  delwrite=0;
	  } // close delwrite
	}



	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

#ifdef PCSIM
//    for (x=0;x<sz/2;x++){
//         printf("%c",mono_buffer[x]);
// 	   }
#endif

      /////////////////////////////////////

  // 4-hardware operations shifted here
  // question of granularity

#endif // for test eeg
#endif // for straight

}
