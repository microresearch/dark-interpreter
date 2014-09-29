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
#define VILLAGE_SIZE (STACK_SIZE*2) // 128

#ifdef LACH
#define SETSIZE 36
#define INFECTSIZE 740 
#define SETSHIFT 11
#define SHIFTY 7
#define THREADERR 32
#define VILLAGERR 33
#define POSERR 34
#define POSYERR 35
#else
#define SETSIZE 66
#define INFECTSIZE 770 
#define SETSHIFT 10
#define SHIFTY 6
#define THREADERR 62
#define VILLAGERR 63
#define POSERR 64
#define POSYERR 65
#endif

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
#include "settings.h"
uint16_t *adc_buffer;
u8 digfilterflag;
int16_t *villager,*stacker,*stackery,*settingsarray;
int16_t *src, *dst;
u8 *village_effects;// [VILLAGE_SIZE/2];

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
#include "settings.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

u16 villager[VILLAGE_SIZE];
u8 village_effects[STACK_SIZE]; // is half village_size
u16 stackery[STACK_SIZE*4]; // 64*4 MAX now
u16 stacker[STACK_SIZE*4]; // 64*4 MAX
u16 settingsarray[SETSIZE];
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
#define randi() ((rand()*adc_buffer[9])%4096) // 12 bits
//#define randi() (adc_buffer[9]) // 12 bits
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

u16 villagepush(u16 villagepos, u16 start, u16 wrap,u8 effect){
  if (villagepos<(VILLAGE_SIZE-1)) /// size -2
    {
      village_effects[villagepos/2]=effect;
      villager[villagepos++]=start;
      villager[villagepos++]=wrap;
}
  return villagepos;
}

u16 villagepop(u16 villagepos){
  if (villagepos>2)
    {
      villagepos-=2;
    }
  return villagepos;
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
    result=0; 
    //    *speedmod=upspeed;
    *speedmod=adc_buffer[UP]>>5;// was >>6 for 6 bits AUG
  }
  else if (down>4 && down>left && down>right) {
    result=2; 
    //*speedmod=downspeed;
    *speedmod=adc_buffer[DOWN]>>5;
  }
  else if (left>4 && left>right) {
    result=3; 
    //    *speedmod=leftspeed;
    *speedmod=adc_buffer[LEFT]>>5;
  }
  else if (right>4) {
    result=1; 
    //*speedmod=rightspeed;
    *speedmod=adc_buffer[RIGHT]>>5;
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

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 hwalksel,coo,x,addr,tmp=0,tmphw=0,tmphardware; u8 HARDWARE=0;
  u8 del=0,attache=0,machine_count=0,tmpacht=0,villagerdest,spd; 
  u8 exestack[MAX_EXE_STACK];
  u16 tmper,foldy;

u16 villagestackpos=0;

#ifdef PCSIM
u8 *settingsarrayattached; //64
//u8 **settingsarrayinfected; 
//u8 settingsarrayinfected[INFECTSIZE][2];
u8 *stackerattached; ///256
u8 *stackeryattached;//256
u8 *villagerattached;//[128];
u8 *villagereffattached;//[64];
u8 *cpuattached;//[64];
#else
u8 stackerattached[256];
u8 stackeryattached[256];
u8 villagerattached[128];
u8 villagereffattached[64];
u8 cpuattached[64];
u8 settingsarrayinfected[INFECTSIZE][2];
u8 settingsarrayattached[SETSIZE];
#endif

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
  Codec_Init(32000); // TODO: variable on startup
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
  settingsarray=malloc(SETSIZE*sizeof(int16_t));
  villager=malloc(VILLAGE_SIZE*sizeof(int16_t));
  stacker=malloc(4*64*sizeof(int16_t));
  stackery=malloc(4*64*sizeof(int16_t));
  adc_buffer=malloc(10*sizeof(int16_t));
  initaudio();
  srandom(time(0));
  src=malloc(BUFF_LEN*sizeof(int16_t));
  dst=malloc(BUFF_LEN*sizeof(int16_t));
  village_effects=malloc(VILLAGE_SIZE/2);
  settingsarrayattached=malloc(SETSIZE);
  u8 **settingsarrayinfected = malloc(sizeof(char *)*INFECTSIZE);

  for(i=0; i<INFECTSIZE; i++){
    settingsarrayinfected[i] = malloc(2); 
  }

  for (x=0;x<(BUFF_LEN);x++){
    src[x]=rand()%65536;
    dst[x]=rand()%65536;
  }

  for (x=0;x<32768;x++){
    audio_buffer[x]=rand()%65536;
  }

  stackerattached=malloc(256); ///256
  stackeryattached=malloc(256); ///256
villagerattached=malloc(128);//[128];
villagereffattached=malloc(64);//[64];
cpuattached=malloc(64);//[64];
#endif

  u16 hwdel=0;
  u16 hwpos=0;
  signed char stack_pos=0;
  signed char stack_posy=0;
  u16 start=0,wrap=0;
  u8 exenums=0, which=0,other=1;

  struct stackey stackyy[STACK_SIZE];
  struct stackeyyy stackyyy[STACK_SIZE];
  buf16 = (u16*) datagenbuffer;
  u8 leakiness=randi()%255;
  u8 infection=randi()%255;

  for (x=0;x<INFECTSIZE;x++){
    if ((rand()%255) > (adc_buffer[SECOND]>>4))
      settingsarrayinfected[x][0]=1; // infected
    else settingsarrayinfected[x][0]=0;
  }

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
     buf16[x]=randi()<<4;
    delayxx();
  }

  for (x=0;x<SETSIZE;x++){
    settingsarrayattached[x]=0;
  }

  for (x=0;x<256;x++){
    stackerattached[x]=0;
    stackeryattached[x]=0;
  }

  for (x=0;x<128;x++){
    villagerattached[x]=0;
  }

  for (x=0;x<64;x++){
    villagereffattached[x]=0;
  }

  //****** setup code for walkers
#ifdef LACH
  for (x=0;x<6;x++){
    settingsarray[x]=0;
  }//start

  for (x=6;x<12;x++){
    settingsarray[x]=65535;
  }//wrap

  for (x=12;x<20;x++){
    settingsarray[x]=511; //>>8 now >>12 but we add 1 so???
  }//step

  EFFECTWRITE=0;
  EFFECTREAD=0;

  for (x=20;x<23;x++){
    settingsarray[x]=32768;
  }//fmods

  for (x=23;x<25;x++){
    settingsarray[x]=0; 
  }//expand

  for (x=25;x<31;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR and speed

    settingsarray[31]=0;//attachspeed

  // initialise foldoffset and foldtop
  settingsarray[23]=32768;
  settingsarray[24]=0;

  // initialise exestacks

  settingsarray[32]=65535;//to 63
  settingsarray[33]=65535;
  settingsarray[34]=65535;
  settingsarray[35]=65535;

#else
  for (x=0;x<14;x++){
    settingsarray[x]=0;
  }//start

  for (x=14;x<28;x++){
    settingsarray[x]=65535;
  }//wrap

  // new hardware offsets
  for (x=28;x<32;x++){
    settingsarray[x]=2;
  }

  for (x=32;x<42;x++){
    settingsarray[x]=511; //>>8
  }//step

  for (x=42;x<46;x++){
    settingsarray[x]=511;
  }//speed

  EFFECTWRITE=0;
  EFFECTREAD=0;
  EFFECTFILTER=0;

  for (x=46;x<49;x++){
    settingsarray[x]=32768;
  }//fmods

  // initialise foldoffset and foldtop and machinespeed
  settingsarray[49]=65535;//foldtop
  settingsarray[50]=0; //foldoffset
  settingsarray[51]=0;

  for (x=52;x<62;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR

  // initialise exestack positions

  settingsarray[62]=65535;//to 63
  settingsarray[63]=65535; // 128 for villagestackpos
  settingsarray[64]=65535;
  settingsarray[65]=65535;

#endif //nolACH

  // CPUintrev3:
  for (x=0; x<64; x++) // was 100
    {
      addr=rand()%65536; // AUG!
      //      addr=randi()<<3;
      cpustackpush(m,addr,rand()%65536,rand()%CPU_TOTAL,rand()%24); // was <<3 RAND AUG
    }

  //pureleak

  for (x=0;x<100;x++){
      addr=rand()%65536; // AUG!
      //    addr=randi()<<3;
    cpustackpushhh(datagenbuffer,addr,rand()%65536,rand()%CPU_TOTAL,rand()%24);// was <<3 RAND AUG
  }

  // CA
  for (x=0;x<(STACK_SIZE);x++){
    //          start=randi()<<4;
    //          wrap=randi()<<4;
      start=rand()%65536; // AUG!
      wrap=rand()%65536;
	  stack_posy=ca_pushn(stackyyy,rand()%65536,datagenbuffer,stack_posy,randi()<<4,start,wrap); 
  }


  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
      //    start=randi()<<4;
      //      wrap=randi()<<4;
      start=rand()%65536; // AUG!
      wrap=rand()%65536;
      //      stack_pos=func_pushn(stackyy,randi()<<4,buf16,stack_pos,randi()<<4,start,wrap);
      stack_pos=func_pushn(stackyy,rand()%65536,buf16,stack_pos,rand()%65536,start,wrap);//AUG!
      villagestackpos=villagepush(villagestackpos,start,wrap,randi()%16);
  }

    // execution stack
        for (x=0;x<MAX_EXE_STACK;x++){
	  exenums=exestackpush(exenums,exestack,rand()%4); //exetype=0-3 
			  //exenums=exestackpush(exenums,exestack,1); //exetype=0-3 TESTY!=CPU
      }

	u8 mainmode,groupstart,groupwrap;
	u8 xx,dirpos,groupsel,foldposy,foldpos;
	u16 settingsposl,fingerposl;

	    m->m_leakiness=leakiness;
	    m->m_infectprob=infection;
	    m->m_memory=datagenbuffer;

	    // compile test
	    testvocode();

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

#ifdef PCSIM
      // randomise adc_buffer
      for (x=0;x<10;x++){
	adc_buffer[x]=(randi()%4096);
	//	adc_buffer[x]=adc_buffer[x]%4096;
      }

      I2S_RX_CallBack(src, dst, BUFF_LEN/2); 
      //printf("STACKPOS %d\n",STACKPOSY);
#endif
      
      //      func_runall(stackyy,STACKPOS); // simulations
      
                        for (x=0;x<exenums;x++){
	switch(exestack[x]){
	case 0:
	  func_runall(stackyy,STACKPOS); // simulations
	  break;
	case 1:
	  	  ca_runall(stackyyy,STACKPOSY); // CA
	  break;
	case 2:
	  	  machine_run(m); //cpu
	  break;
	case 3:
	  //	  machine_count++;
	  //	  if (machine_count>=MACHINESPEED){
	    machine_runnn(datagenbuffer); // pureleak
	    //	    machine_count=0;
	    //	  }
	  break;
	case 4: // never used!
	  break;
	}
	}
      
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
