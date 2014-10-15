/*

 Oh, eternity with outstretched wings, that broodest over the secret
 truths in whose roots lie the mysteries of man—his whence, his
 whither—have I searched thee, and struck a right key on thy dreadful
 organ!

 [Thomas de Quincey. The Dark Interpreter] 

*/

/*
 PATH=~/sat/bin:$PATH
 PATH=~/stm32f4/stlink/flash:$PATH
 make stlink_flash
*/

#define MAX_EXE_STACK 4

#ifdef PCSIM
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "audio.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
//#include "settings.h"
uint16_t *adc_buffer;
u8 digfilterflag;
int16_t *stacker,*stackery;
int16_t *src, *dst;

void  dohardwareswitch(u8 one,u8 two){
  // nothing
}

void set40106pwm(u16 one){
}

void setmaximpwm(u16 one){
}

void setlmmmpwm(u16 one){
}

#else
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

#include "stm32f4xx.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "audio.h"
#include "hardware.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
//#include "settings.h"
#include "vocode.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

u16 stackery[STACK_SIZE*4]; // 64*4 MAX now
u16 stacker[STACK_SIZE*4]; // 64*4 MAX
#endif

// for knobwork
// TENE: 2,0,3,4,1 // else: 3,0,2,4,1

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

u8 EFFECTWRITE;
u8 EFFECTREAD;
u8 EFFECTFILTER;

signed char direction[2]={-1,1};
u8 wormflag[10]={0,0,0,0,0,0,0,0,0,0};
u8 inp;
u16 *buf16;

#define delay()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#ifndef PCSIM
extern int16_t audio_buffer[AUDIO_BUFSZ];
u8* datagenbuffer = (u8*)0x10000000;
//#define randi() ((rand()*adc_buffer[9])%4096) // 12 bits
#define randi() (adc_buffer[9]) // 12 bits
//#define randi() (rand()%4096)
#else //PCSIM
#define randi() (rand()%4096)
u8* datagenbuffer;
extern int16_t* audio_buffer;
#endif
extern u8 digfilterflag;

u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array

u8 exestackpush(u8 exenum, u8* exestack, u8 exetype){
  if (exenum<MAX_EXE_STACK){
    exestack[exenum]=exetype;
    exenum++;
  }
  return exenum;
}

u8 exestackpop(u8 exenum){
  if (exenum>0){
    exenum--;
  }
  return exenum;
  }

u8 fingerdir(u8 *speedmod){
  u8 handleft, handright, up=0,down=0,left=0,right=0;//,upspeed=0,downspeed=0,leftspeed=0,rightspeed=0;
  u8 handupp, handdown;
  u8 result=5;

  for (u8 x=0;x<8;x++){
    handupp=adc_buffer[UP]>>8; // 4 bits=16 
    handdown=adc_buffer[DOWN]>>8;
    handleft=adc_buffer[LEFT]>>8;
    handright=adc_buffer[RIGHT]>>8;
    /*    upspeed+=handupp;
    downspeed+=handdown;
    leftspeed+=handleft;
    rightspeed+=handright;*/

    if (handupp>2) up++; // was 3 - more sensitive now
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  } // changed from end
  if (up>4 && up>down && up>left && up>right) {
    result=2; 
    //    *speedmod=upspeed;
    *speedmod=adc_buffer[UP]>>4;// was >>6 for 6 bits AUG
  }
  else if (down>4 && down>left && down>right) {
    result=3; 
    //*speedmod=downspeed;
    *speedmod=adc_buffer[DOWN]>>4;
  }
  else if (left>4 && left>right) {
    result=0; 
    //    *speedmod=leftspeed;
    *speedmod=adc_buffer[LEFT]>>4;
  }
  else if (right>4) {
    result=1; 
    //*speedmod=rightspeed;
    *speedmod=adc_buffer[RIGHT]>>4;
  }
  //  }
  return result;
}


u8 fingerdirupdown(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;

  if (left>8 && left>right) {
    return 0;
  }
  else if (right>8 && right>left) {
    return 1;
  }
  }
  return 2;
}

////////////////////////////////////////////////////////////////////////////////


  villagerr village_write[MAX_VILLAGERS+1];
  villagerr village_read[MAX_VILLAGERS+1];
  villagerr village_filt[MAX_VILLAGERS+1];
villager_generic village_datagen[MAX_VILLAGERS+1];
u8 howmanydatavill;

u16 testfunction(void){
  u16 tester=adc_buffer[FIRST];
  return tester;
}


void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 x,addr;
  u8 exestack[MAX_EXE_STACK];

  // we just need init first of all villagers NON?

  village_write[0].start=0;
  village_read[0].start=0;
  village_write[0].wrap=32767;
  village_read[0].wrap=32767;
  village_write[0].dir=1;
  village_read[0].dir=1;
  village_write[0].del=0;
  village_read[0].del=0;
  village_write[0].speed=1;
  village_read[0].speed=1;
  village_write[0].step=1;
  village_read[0].step=1;
  village_read[0].dirry=direction[village_read[0].dir]*village_read[0].step;
  village_read[0].samplepos=village_read[0].start;
  village_write[0].dirry=direction[village_write[0].dir]*village_write[0].step;
  village_write[0].samplepos=village_write[0].start;
  village_read[0].offset=0;
  village_write[0].offset=0;

  // TEST!
  /*  for (u8 xx=0;xx<64;xx++){
  village_read[xx].start=rand()%32768;
	  village_read[xx].wrap=rand()%32768;
	  village_read[xx].offset=rand()%32768;
	  village_read[xx].dir=1;
	  village_read[xx].del=0;
	  village_read[xx].samplepos=0;
	  village_read[xx].speed=rand()%16;
	  village_read[xx].step=rand()%16;
	  village_read[xx].dirry=direction[village_read[xx].dir]*village_read[xx].step;
	  village_read[xx].samplepos=village_read[xx].start;
	}

      	for (u8 xx=0;xx<64;xx++){
  village_write[xx].start=rand()%32768;
	  village_write[xx].wrap=rand()%32768;
	  village_write[xx].offset=rand()%32768;
	  village_write[xx].dir=1;
	  village_write[xx].del=0;
	  village_write[xx].samplepos=0;
	  village_write[xx].speed=rand()%16;
	  village_write[xx].step=rand()%16;
	  village_write[xx].dirry=direction[village_write[xx].dir]*village_write[xx].step;
	  village_write[xx].samplepos=village_write[xx].start;
	  }
  */
  inittable(3,4,randi());
  const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase=0;
  int sign_samp,i;
  inp=0; // STRAIGHT IN
  w= 2*pi;
  w= w/256;
  for (i = 0; i <= 256; i++)
    {
      yi= 32767*sinf(phase); // was 2047???
      phase=phase+w;
      sign_samp=32767+yi;     // dc offset
      sin_data[i]=sign_samp; // write value into array
    }

#ifndef PCSIM
  //	ADC1_Initonce();
  ADC1_Init((uint16_t *)adc_buffer);

#ifndef LACH
  setup_switches();
#endif

  // maintain order
  Audio_Init();
  Codec_Init(48000); // TODO: variable on startup
  delay();

#ifndef LACH
  initpwm(); 	
#endif		

    I2S_Block_Init();
    I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);

#ifndef LACH
  dohardwareswitch(0,0);
#endif
#endif // for ifndef PCSIM

  machine *m=(machine *)malloc(sizeof(machine));
  m->m_threadcount=0;
  m->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED

#ifdef PCSIM
  datagenbuffer=(u8*)malloc(65536);
  audio_buffer=(int16_t*)malloc(32768*sizeof(int16_t));
  villager=malloc(VILLAGE_SIZE*sizeof(int16_t));
  stacker=malloc(4*64*sizeof(int16_t));
  stackery=malloc(4*64*sizeof(int16_t));
  adc_buffer=malloc(10*sizeof(int16_t));
  initaudio();
  srandom(time(0));
  src=malloc(BUFF_LEN*sizeof(int16_t));
  dst=malloc(BUFF_LEN*sizeof(int16_t));

  for (x=0;x<(BUFF_LEN);x++){
    src[x]=rand()%65536;
    dst[x]=rand()%65536;
  }

  for (x=0;x<32768;x++){
    audio_buffer[x]=rand()%65536;
  }

#endif

  buf16 = (u16*) datagenbuffer;
  m->m_leakiness=randi()%255;
  m->m_infectprob=randi()%255;
  m->m_memory=datagenbuffer;

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
     buf16[x]=randi()<<4;
    delayxx();
  }
  
	    // compile test
	    //	    	    	    testvocode();
	    //	    	    x=sqrtf(x);

  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

#ifdef TEST_EEG
      //write ADC9 into buf16
  for (x=0;x<32768;x++){
        buf16[x]=adc_buffer[9]<<4; // 16 bits
    //    buf16[x]=x*16; // 16 bits
  }
#else

  /// HERE!

  // experimentally run through datagen villagers - now just with CA!

  u16 counterd, databegin=0,dataend=32768;

  if ((counterd-databegin)>=dataend) {
    counterd=databegin;
    for (u8 x=0;x<howmanydatavill;x++){
      village_datagen[x].running=1;
    }
  }	    
  if (counterd<=databegin) {
    counterd=dataend+databegin;
    for (u8 x=0;x<howmanydatavill;x++){
      village_datagen[x].running=1;
	  }	    
	  }

  for (x=0;x<howmanydatavill;x++){
    if ((village_datagen[x].start)<=counterd && village_datagen[x].running==1){// in town


    switch(village_datagen[x].cpu){      
    }
  }
  }
  ///// end of DATAGEN villagers!

#ifdef PCSIM
      // randomise adc_buffer
      for (x=0;x<10;x++){
	adc_buffer[x]=(randi()%4096);
	//	adc_buffer[x]=adc_buffer[x]%4096;
      }

      I2S_RX_CallBack(src, dst, BUFF_LEN/2); 
      //printf("STACKPOS %d\n",STACKPOSY);
#endif
      
#endif //eeg
#endif //straight
      //#endif 
    }
      }

#ifdef  USE_FULL_ASSERT

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
    {
    }
}
#endif

#if 1
/* exception handlers - so we know what's failing */
void NMI_Handler(void)
{ 
  while(1){};
}

void HardFault_Handler(void)
{ 
  while(1){};
}

void MemManage_Handler(void)
{ 
  while(1){};
}

void BusFault_Handler(void)
{ 
  while(1){};
}

void UsageFault_Handler(void)
{ 
  while(1){};
}

void SVC_Handler(void)
{ 
  while(1){};
}

void DebugMon_Handler(void)
{ 
  while(1){};
}

void PendSV_Handler(void)
{ 
  while(1){};
}
#endif
