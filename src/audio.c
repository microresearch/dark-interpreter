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
#include "hardware.h"
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
  u16 tmp=0,tmpw; u16 tmper;
  int16_t tmp16,count;
  int32_t tmp32;
  u8 x,tmpp;
  static u16 start=0,startfilt=0,wrapfilt=1,wrap=1,samplepos=0,sampleposfilt=0,anyposfilt=0,anypos=0;
u16 hwalksel,tmphw=0,tmphardware; u8 HARDWARE=0;
 static u16 hwdel=0;
 static u16 hwpos=0;

  static u8 vilr=0,vilf=0,vilw=0; //AUG
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


	///	///	///	///

	// readin villager processing of left into left and right into audio_buffer
	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;
	float32_t morph_inv,fsum;

	morph_inv = 1.0f - (float32_t)FMOD;

#ifdef TEST_EEG
	// write buf16 into mono
	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos%32768];//-32768;
	  samplepos++;
	}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	// TODO all audio!

audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

#ifdef PCSIM
//    for (x=0;x<sz/2;x++){
//         printf("%c",mono_buffer[x]);
// 	   }
#endif

      /////////////////////////////////////

      // 4-hardware operations shifted here

#ifndef LACH
      
      /* tmphardware=0;
           for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIFTH]>>7; // 5 bits now=32
      }
      HARDWARE=tmphardware>>8; //was >>8 to divide average
      */
      tmphardware=adc_buffer[FIFTH];
      hwalksel=(tmphardware>>6)&1;
      HARDWARE=tmphardware>>7; // 5 bits now!
      //      settingsarray[42]=adc_buffer[FIFTH]<<4; // TESTY HWSPEED!

            
      /// HW as attached/settings or as walkers - AUG


      if (hwalksel){
	set40106pwm(F0106ERCONS);
	//set40106pwm(0); // TESTY!

      if (digfilterflag&4){
	setlmmmpwm(LMERCONS);
      }

      if (digfilterflag&8){
	setmaximpwm(MAXIMERCONS);
      }
      }//hwalksel

      else{

      if (++hwdel>=HWSPEED){
	hwdel=0;

	if (wormflag[0]) hwpos+=direction[wormdir];
	else hwpos+=(HWSTEP*direction[HWDIR]);
	tmphw=HWSTART+(hwpos%HWWRAP); //to cover all directions
	tmper=((adc_buffer[THIRD]>>4)%16)<<8; // 8 bits
      //      set40106pwm(F0106ERBASE+(buf16[(tmp+F0106EROFFSET)%32768]%F0106ERCONS)); // constrain all to base+constraint
	tmp=F0106ERCONS-F0106ERBASE-tmper;
	if (tmp==0) tmp=1;
	set40106pwm(F0106ERBASE+tmper+(buf16[(tmphw+F0106EROFFSET)%32768]%tmp)); // constrain all to base+constraint - what is range? now want 0->2048 // 15 bits to 11 bits - now in cons AUG AUG
	//	set40106pwm(0); // TESTY!

      tmp=LMERCONS-LMERBASE;
	if (tmp==0) tmp=1;
      if (digfilterflag&4){
	//	setlmpwm(LMERBASE+(buf16[(tmphw+LMEROFFSET)%32768]%tmp),LMERBASE+(buf16[(tmphw+LMEROFFSETTWO)%32768]%tmp)); 
	//		setlmpwm(adc_buffer[FOURTH],adc_buffer[THIRD]<<3);//TESTY!=14 bits

	setlmmmpwm(LMERBASE+(buf16[(tmphw+LMEROFFSET)%32768]%tmp)); // AUGUST!
      }
	  
      if (digfilterflag&8){
	tmp=MAXIMERCONS-MAXIMERBASE;
	if (tmp==0) tmp=1;
	//	setmaximpwm(MAXIMERBASE+(buf16[(tmphw+MAXIMEROFFSET)%32768]%tmp)); // constrain CONS rather AUG!!!
	setmaximpwm(adc_buffer[FIFTH]<<2);//TESTY!=14 bits
	//	setmaximpwm(255);//TESTY!=14 bits

      }
      }
      }/////
      
      if (digfilterflag&16){
	dohardwareswitch(HARDWARE,datagenbuffer[tmphw]%HDGENERCONS);
      }
      else
	{
	  dohardwareswitch(HARDWARE,0);
	}


#endif //notLACH


#endif // for test eeg
#endif // for straight

}
