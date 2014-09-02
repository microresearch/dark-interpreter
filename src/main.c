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
      
      /////////////////////////////////////
	
#ifdef LACH
      settingsarray[6]=adc_buffer[FIFTH]<<4; // 16 bits SAMPLEWRAP!!!
      settingsarrayattached[6]=0;
#endif

      ////settingssss
      
      for (x=0;x<SETSIZE;x++){
	switch(settingsarrayattached[x]){
	case 0:
	  break;
	case 1:
	  settingsarray[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  printf("x %d where %d setted %d\n",x,((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768,settingsarray[x]);
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  settingsarray[x]=adc_buffer[9]<<4;
#else
	  settingsarray[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  settingsarray[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  settingsarray[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

     
      for (x=0;x<(STACKPOS*4);x++){  // AUG. fixed before was not *4
	switch(stackerattached[x]){
	case 0:
	  break;
	case 1:
	  stacker[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  coo++;
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG! 
	  break;
	case 2:
#ifdef TENE
	  stacker[x]=adc_buffer[9]<<4;
#else
	  stacker[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stacker[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stacker[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<(STACKPOSY*4);x++){ // AUG. fixed before was not *4
	switch(stackeryattached[x]){
	case 0:
	  break;
	case 1:
	  stackery[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  coo++;
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  stackery[x]=adc_buffer[9]<<4;
#else
	  stackery[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stackery[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stackery[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<VILLAGESTACKPOS;x++){
	switch(villagerattached[x]){
	case 0:
	  break;
	case 1:
	  villager[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  coo++; 
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  villager[x]=adc_buffer[9]<<4;
#else
	  villager[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  villager[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  villager[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<VILLAGESTACKPOS/2;x++){
	switch(villagereffattached[x]){
	case 0:
	  break;
	case 1:
	  village_effects[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768]>>12;//4 bits
	  //	  coo++; 
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  village_effects[x]=adc_buffer[9]>>8;
#else
	  village_effects[x]=adc_buffer[DOWN]>>8;
#endif
	  break;
	case 3:
	  village_effects[x]=adc_buffer[THIRD]>>8;
	  break;
	case 4:
	  village_effects[x]=adc_buffer[SECOND]>>8; // where?
	  break;
	}
      }

      for (x=0;x<THREADCOUNT;x++){
	switch(cpuattached[x]){
	case 0:
	  break;
	case 1:
	  m->m_threads[x].m_CPU=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768]>>11;
	  //	  coo++;
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  m->m_threads[x].m_CPU=adc_buffer[9]>>7;//cpu - 5 bits
#else
	  m->m_threads[x].m_CPU=adc_buffer[DOWN]>>7;//cpu - 5 bits
#endif
	  break;
	case 3:
	  m->m_threads[x].m_CPU=adc_buffer[THIRD]>>7;
	  break;
	case 4: // never used...
	  m->m_threads[x].m_CPU=adc_buffer[SECOND]>>7; // where?
	  break;
	}
      }
      
      //MODECODE      /////////////////////////////////////

      mainmode=adc_buffer[FIRST]>>8; // 4 bits=16
      //      mainmode=0; // TESTY!
      //////
      switch(mainmode){
#ifdef LACH 
      case 0:
	xx=fingerdir(&spd);
	if (xx!=5) {
	EFFECTWRITE=adc_buffer[FOURTH]>>6;
	EFFECTREAD=adc_buffer[SECOND]>>6;
	// what spd could be? mod? (max 64=6bits<<10) 3 mods:
	settingsarray[20+xx]=spd<<10;
	settingsarrayattached[20+xx]=0;
	}
	break;
#else
      case 0: // up/down/left/right as INPUT
	xx=fingerdir(&spd);
	//	xx=5;// TESTY!
	if (xx!=5) {
	  inp=xx;
	  EFFECTWRITE=adc_buffer[FOURTH]>>6;
	  EFFECTREAD=adc_buffer[SECOND]>>6;
	  EFFECTFILTER=adc_buffer[THIRD]>>6;
	  
	// what spd could be? mod? (max 64=6bits<<10) 3 mods:
	settingsarray[46+xx]=spd<<10;
	settingsarrayattached[46+xx]=0;
	}
	break;
#endif	
	  ///////////////////////////////////
      case 1: // directions - and speed and step!
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5) {
	dirpos=adc_buffer[SECOND]>>9; // 3 bits
	dirpos=dirpos%6;
	settingsarray[12+dirpos]=adc_buffer[FOURTH]<<4;//steps
	settingsarrayattached[12+dirpos]=0;

	if (xx==0) {
	  wormflag[dirpos]=1; // up
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<8;
	    settingsarrayattached[18]=0;
	  }
	}
	else if (xx==1) { 
	  settingsarray[25+dirpos]=(1<<15); wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  } 
	}//right
	else if (xx==3) {//left 
	  settingsarray[25+dirpos]=0; wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  }
	} // left=0
	else { //down
	  settingsarray[25+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	    if (dirpos==2) {
	      settingsarray[19]=spd<<9;
	      settingsarrayattached[19]=0;
	    }
	    else if (dirpos==3) {
	      settingsarray[18]=spd<<9;
	      settingsarrayattached[18]=0;
	    }
	}
	}
#else
	if (xx!=5) {
	dirpos=adc_buffer[SECOND]>>8; // 4 bits=16
	dirpos=dirpos%10;
	//	dirpos=0;//TESTY!
	settingsarray[32+dirpos]=adc_buffer[FOURTH]<<4;
	settingsarrayattached[32+dirpos]=0;
	if (xx==0) {
	  wormflag[dirpos]=1; // up
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	}
	else if (xx==1) { //right
	  settingsarray[52+dirpos]=(1<<15); wormflag[dirpos]=0;
	  settingsarrayattached[52+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	} //right
	else if (xx==3) { //left
	  settingsarray[52+dirpos]=0; wormflag[dirpos]=0;
	  settingsarrayattached[52+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	} // left=0
	else { //down
	  settingsarray[52+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;
	  settingsarrayattached[52+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	}
	}
#endif
	break; 

      case 2:
	// set start and wrap for r/w/village_r/w according to fingers
	// speed is step
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5){
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){ // UP=read= start/wrap/step
	  settingsarray[1]=tmp;
	  settingsarray[7]=tmper;
	  settingsarray[14]=spd<<8;
	  settingsarrayattached[1]=0;
	  settingsarrayattached[7]=0;
	  settingsarrayattached[14]=0;
	}
	else if (xx==2){ // DOWN=write= start/speed/step
	  settingsarray[0]=tmp;
	  settingsarray[18]=tmper;  // changed as is already set // now is speed
	  settingsarray[15]=spd<<8; // step!!!
	  settingsarrayattached[0]=0;
	  settingsarrayattached[18]=0;
	  settingsarrayattached[15]=0;
	}
	else if (xx==3){ // LEFT=village_r= start/wrap/step - AUG - anyread
	  settingsarray[5]=tmp;
	  settingsarray[11]=tmper;
	  settingsarray[16]=spd<<10;

	  settingsarray[3]=tmp;
	  settingsarray[8]=tmper;
	  settingsarray[12]=spd<<10;

	  settingsarrayattached[3]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[12]=0;

	  settingsarrayattached[5]=0;
	  settingsarrayattached[11]=0;
	  settingsarrayattached[16]=0;
	}
	else { // RIGHT=village_w= start/wrap/step - AUG - anywrite
	  settingsarray[4]=tmp;
	  settingsarray[10]=tmper;
	  settingsarray[17]=spd<<10;

	  settingsarray[2]=tmp;
	  settingsarray[9]=tmper;
	  settingsarray[13]=spd<<10;

	  settingsarrayattached[2]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[13]=0;

	  settingsarrayattached[4]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[17]=0;
	}
	}
#else
	if (xx!=5){
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){ // UP=read= start/wrap/step
	  settingsarray[2]=tmp;
	  settingsarray[16]=tmper;
	  settingsarray[36]=spd<<8;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[16]=0;
	  settingsarrayattached[36]=0;
	}
	else if (xx==2){ // DOWN=write= start/wrap/step/
	  settingsarray[1]=tmp;
	  settingsarray[15]=tmper;
	  settingsarray[37]=spd<<8;
	  settingsarrayattached[1]=0;
	  settingsarrayattached[15]=0;
	  settingsarrayattached[37]=0;
	}

	else if (xx==3){ // LEFT=village_r= start/wrap/step// AUG also anystepread
	  settingsarray[8]=tmp;
	  settingsarray[22]=tmper;
	  settingsarray[39]=spd<<10;

	  settingsarray[5]=tmp;
	  settingsarray[18]=tmper;
	  settingsarray[33]=spd<<10;

	  settingsarrayattached[5]=0;
	  settingsarrayattached[18]=0;
	  settingsarrayattached[33]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[22]=0;
	  settingsarrayattached[39]=0;
	}
	else { // RIGHT=village_w= start/wrap/step // AUG also anystepwrite
	  settingsarray[7]=tmp; //7,21,40
	  settingsarray[21]=tmper;
	  settingsarray[40]=spd<<10; //16 bits

	  settingsarray[4]=tmp; //7,21,40
	  settingsarray[19]=tmper;
	  settingsarray[34]=spd<<10; //16 bits

	  settingsarrayattached[4]=0;
	  settingsarrayattached[19]=0;
	  settingsarrayattached[34]=0;
	  settingsarrayattached[7]=0;
	  settingsarrayattached[21]=0;
	  settingsarrayattached[40]=0;
	}
	}
#endif	
	break;
      case 3:
	// HW=start,wrap,offsets
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5){
	// 4 sets of settings:
	// ANYSTART/ANYWRAP/ANYSTEP/VILLAGESTEP *2
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){
	  settingsarray[2]=tmp; // anystart and anystartread - AUG villages starts
	  settingsarray[3]=tmper;
	  settingsarray[4]=tmp; // anystart and anystartread - AUG villages starts
	  settingsarray[5]=tmper;

	  settingsarrayattached[2]=0;
	  settingsarrayattached[3]=0;
	  settingsarrayattached[4]=0;
	  settingsarrayattached[5]=0;
	}
	else if (xx==1){
	  settingsarray[8]=tmp; // anywrap and anywrapread - AUG villages wraps
	  settingsarray[9]=tmper;
	  settingsarray[10]=tmp; // anywrap and anywrapread - AUG villages wraps
	  settingsarray[11]=tmper;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[11]=0;
	}
	else if (xx==2){ //AUG villagesteps
	  settingsarray[12]=tmp;
	  settingsarray[13]=tmper;
	  settingsarray[16]=tmp;
	  settingsarray[17]=tmper;

	  settingsarrayattached[12]=0;// anystep and anystepread
	  settingsarrayattached[13]=0;
	  settingsarrayattached[16]=0;// anystep and anystepread
	  settingsarrayattached[17]=0;

	}
	else {
	  settingsarray[23]=tmp; // villager but this should be another set of settings *2-FOLD=AUG
	  settingsarray[24]=tmper;

	  settingsarrayattached[23]=0;
	  settingsarrayattached[24]=0;
	}
	}
#else
	//HW start and wrap
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx!=5){
	if (xx==0){ // UP=generic HW start/wrap/hdgenercons/speed
	  settingsarray[0]=tmp;
	  settingsarray[14]=tmper;
	  settingsarray[24]=adc_buffer[THIRD]<<4;
	  settingsarray[42]=spd<<10;
	  settingsarrayattached[0]=0;
	  settingsarrayattached[14]=0;
	  settingsarrayattached[24]=0;
	  settingsarrayattached[42]=0;
	}
	else 	if (xx==2){ //DOWN---offset/cons/base 
	  settingsarray[28]=tmp;
	  settingsarray[29]=adc_buffer[THIRD]<<4;
	  settingsarray[25]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[28]=0;
	  settingsarrayattached[29]=0;
	  settingsarrayattached[25]=0;
	  //	  settingsarrayattached[10]=0;
	}
	else 	if (xx==3){ //LEFT=1016er-offset/cons/base
	  settingsarray[30]=tmp;
	  settingsarray[12]=adc_buffer[THIRD]<<4;
	  settingsarray[26]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[30]=0;
	  settingsarrayattached[26]=0;
	  settingsarrayattached[12]=0;
	}
	else { //RIGHT=maximer
	  settingsarray[31]=tmp; //offset
	  settingsarray[27]=adc_buffer[THIRD]<<4; //cons
	  settingsarray[13]=tmper; //base
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[31]=0;
	  settingsarrayattached[27]=0;
	  settingsarrayattached[13]=0;
	}
	}
#endif
	break;

      case 4:
	//	  select stackmax (left/right) (knob and set)
	xx=fingerdir(&spd);
	if (xx!=5) {
	  if (xx==0) {//UP
	    if (THREADCOUNT>0){

	    m->m_threads[(adc_buffer[THIRD]>>6)%THREADCOUNT].m_CPU=spd%31; // AUG - re-arranged so set first
	    }
	    else m->m_threads[0].m_CPU=spd%31;

	  settingsarray[THREADERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[THREADERR]=0;
	  settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[VILLAGERR]=0;
	}
	  else if (xx==1) //RIGHT=CA
	  {
	    if (STACKPOSY>0){
	    stackery[(((adc_buffer[THIRD]>>6)%STACKPOSY)*4)+3]=(spd%11)<<12; //type AUG <<12 must be there
	    }
	    else stackery[0]=(spd%11)<<12; //type AUG <<12 must be there
	    settingsarray[POSYERR]=adc_buffer[SECOND]<<4;
	    settingsarrayattached[POSYERR]=0;
	    settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	    settingsarrayattached[VILLAGERR]=0;
	  }
	  else if (xx==2) {//DOWN
	    if (STACKPOS>0){
	    stacker[(((adc_buffer[THIRD]>>6)%STACKPOS)*4)+3]=(spd%34)<<10;  //type AUG <<10 must be there
	    }
	    else stacker[0]=(spd%11)<<12; //type AUG <<12 must be there
	  settingsarray[POSERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[POSERR]=0;
	  settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[VILLAGERR]=0;
	}
	  else//LEFT
	  {
	    exestack[adc_buffer[SECOND]>>10]=spd%4; // 2 bits 0,1,2,3=spd%4 or spd&3
	    settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	    settingsarrayattached[VILLAGERR]=0;
	    }
	}
	break;

      case 5:
	// anydata stuff
	//for LACH is simply anystart*2,anywrap*2,anystep*2
	//for TENE and SUSP is *3 (inc filt)
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5){
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){ // UP
	  settingsarray[3]=tmp;//anystartread
	  settingsarray[8]=tmper;//wrap
	  settingsarray[12]=spd<<10;//step

	  settingsarray[5]=tmp;//anystartread AUG DOUBLE VILLAGERS
	  settingsarray[11]=tmper;//wrap
	  settingsarray[16]=spd<<10;//step

	  settingsarrayattached[5]=0;
	  settingsarrayattached[11]=0;
	  settingsarrayattached[16]=0;

	  settingsarrayattached[3]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[12]=0;
	}
	else 	if (xx==2){ //DOWN
	  settingsarray[2]=tmp;//write
	  settingsarray[9]=tmper;
	  settingsarray[13]=spd<<10;

	  settingsarray[4]=tmp;//write AUG DOUBLE VILLAGERS
	  settingsarray[10]=tmper;
	  settingsarray[17]=spd<<10;

	  settingsarrayattached[4]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[17]=0;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[13]=0;
	}
	//#define FOLDTOP (settingsarray[23]) 
	//#define FOLDOFFSET (settingsarray[24]) 
	else { 	  //left and right
	  settingsarray[23]=tmp; //AUG
	  settingsarray[24]=tmper;
	  settingsarray[31]=spd<<10;
	  settingsarrayattached[23]=0;
	  settingsarrayattached[24]=0;
	  settingsarrayattached[31]=0;
	}
	}
#else
	//HW start and wrap
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx!=5){
	if (xx==0){ // UP=generic HW start/wrap/hdgenercons/speed
	  settingsarray[0]=tmp;
	  settingsarray[14]=tmper;
	  settingsarray[24]=adc_buffer[THIRD]<<4;
	  settingsarray[42]=spd<<10;
	  settingsarrayattached[0]=0;
	  settingsarrayattached[14]=0;
	  settingsarrayattached[24]=0;
	  settingsarrayattached[42]=0;
	}
	else 	if (xx==2){ //DOWN---offset/cons/base 
	  settingsarray[28]=tmp;
	  settingsarray[29]=adc_buffer[THIRD]<<4;
	  settingsarray[25]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[28]=0;
	  settingsarrayattached[29]=0;
	  settingsarrayattached[25]=0;
	  //	  settingsarrayattached[10]=0;
	}
	else 	if (xx==3){ //LEFT=1016er-offset/cons/base
	  settingsarray[30]=tmp;
	  settingsarray[12]=adc_buffer[THIRD]<<4;
	  settingsarray[26]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[30]=0;
	  settingsarrayattached[26]=0;
	  settingsarrayattached[12]=0;
	}
	else { //RIGHT=maximer
	  settingsarray[31]=tmp; //offset
	  settingsarray[27]=adc_buffer[THIRD]<<4; //cons
	  settingsarray[13]=tmper; //base
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[31]=0;
	  settingsarrayattached[27]=0;
	  settingsarrayattached[13]=0;
	}
	}
#endif
	break;
	/////////////////////////////////////////////////////////////////////////////////////////

      case 6: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>SHIFTY; // 6bits=64 or7 for LACH
	groupstart=adc_buffer[FOURTH]>>SHIFTY;
	for (x=0;x<groupwrap;x++){
	  settingsarrayattached[(groupstart+x)%SETSIZE]=groupsel;
	}
	}
	break;

      case 7: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>5; // 7bits=128
	groupstart=adc_buffer[FOURTH]>>5;
	for (x=0;x<groupwrap;x++){
	  villagerattached[(groupstart+x)%VILLAGE_SIZE]=groupsel;
	  villagereffattached[((groupstart+x)/2)%64]=groupsel;
	}
	}
	break;

      case 8:
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>4; // 8bits
	groupstart=adc_buffer[FOURTH]>>4;
	for (x=0;x<groupwrap;x++){
	  stackerattached[(groupstart+x)%256]=groupsel;
	}
	}
	break;

      case 9: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>4; // 8bits
	groupstart=adc_buffer[FOURTH]>>4;
	for (x=0;x<groupwrap;x++){
	  stackeryattached[(groupstart+x)%256]=groupsel;
	}
	}
	break;

      case 10: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	groupstart=adc_buffer[FOURTH]>>6;
	for (x=0;x<groupwrap;x++){
	  cpuattached[(groupstart+x)%64]=groupsel;
	}
	}
	break;

	///////////////////////////////
	//algo-attach into all:
      case 11:
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	foldy=adc_buffer[SECOND]>>7; // how many from knob2 =max 32
	for (x=0;x<foldy;x++){
	  settingsposl=adc_buffer[FOURTH]<<3; // 15 bits
	  tmper=buf16[(settingsposl+x)%32768];
#ifdef LACH
	tmper=(tmper>>6)%804;
	if (tmper<36) settingsarrayattached[tmper]=groupsel;
	else 	if (tmper<292) stackerattached[tmper-36]=groupsel; // now 256
	else 	if (tmper<548) stackeryattached[tmper-292]=groupsel; // now 256
	else 	if (tmper<676) villagerattached[tmper-548]=groupsel; // 128
	else 	if (tmper<740) villagereffattached[tmper-676]=groupsel; // 64
	else 	cpuattached[tmper-740]=groupsel; //64
#else
	tmper=(tmper>>6)%834;
	if (tmper<66) settingsarrayattached[tmper]=groupsel;
	else 	if (tmper<322) stackerattached[tmper-66]=groupsel;
	else 	if (tmper<578) stackeryattached[tmper-322]=groupsel;
	else 	if (tmper<706) villagerattached[tmper-578]=groupsel;
	else 	if (tmper<770) villagereffattached[tmper-706]=groupsel;
	else 	cpuattached[tmper-770]=groupsel;
#endif
	}
	}
	break;

      case 12:
	groupsel=fingerdir(&spd);
	if (groupsel==0){//UP
	  foldy=adc_buffer[SECOND]>>SHIFTY; // howmuch-64
	foldposy=adc_buffer[THIRD]>>SHIFTY; // offset
	foldpos=adc_buffer[FOURTH]>>SHIFTY;
	for (x=0;x<(foldy);x++){
	  settingsarray[(foldpos+x)%SETSIZE]=settingsarray[(foldpos+foldposy+x)%SETSIZE];
	  }
	}
	else if (groupsel==1){// RIGHT
	foldy=adc_buffer[SECOND]>>5; // howmuch=128
	foldposy=adc_buffer[THIRD]>>5; // offset
	foldpos=adc_buffer[FOURTH]>>5;//start

	for (x=0;x<foldy;x++){
	  villager[(foldpos+x)%VILLAGE_SIZE]=villager[(foldpos+foldposy+x)%VILLAGE_SIZE];
	}
	}

	else if (groupsel==2){ //DOWN// various stack and villager exchanges - 
	// starts and ends only of stacks (not CPU) -> villagers
	  foldy=adc_buffer[SECOND]>>6; // howmuch-64
	  foldposy=adc_buffer[THIRD]>>6; // offset-64
	  foldpos=adc_buffer[FOURTH]>>5;//128
	///////
	for (x=0;x<foldy;x++){ // 64 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point 
	  villagerdest=((foldpos>>1)+x+foldposy)%(STACK_SIZE); // village entry
	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    villager[villagerdest*2]=stacker[tmper*4];  
	    villager[(villagerdest*2)+1]=stacker[(tmper*4)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    villager[villagerdest*2]=stackery[tmper*4];  
	    villager[(villagerdest*2)+1]=stackery[(tmper*4)+1];  
	  }
	}
	}
	else if (groupsel==3) {//LEFT // various stack and villager exchanges -  // bugfix AUG
	// other way round
	
	foldy=adc_buffer[SECOND]>>6; // howmuch-64
	foldposy=adc_buffer[THIRD]>>6; // offset
	foldpos=adc_buffer[FOURTH]>>5;

	for (x=0;x<(foldy);x++){ 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=((foldpos>>1)+x+foldposy)%(STACK_SIZE); // village entry

	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    stacker[tmper*4]=villager[villagerdest*2];  
	    stacker[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    stackery[tmper*4]=villager[villagerdest*2];  
	    stackery[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	}
	}
	break;

      case 13: // dump (all) to datagen//back
	xx=fingerdirupdown();
	if (xx==1) { //down
	foldy=adc_buffer[SECOND]>>2; // howmuch=10 bits=1024
	foldpos=adc_buffer[FOURTH]<<4; // 16 bits

	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%740; // full house//10 bits=1024
	  if (tmper<36) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<292) buf16[(foldpos+x)%32768]=stacker[tmper-36];
	  else if (tmper<548) buf16[(foldpos+x)%32768]=stackery[tmper-292];
	  else if (tmper<612) buf16[(foldpos+x)%32768]=m->m_threads[tmper-548].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-612];
#else
	  tmper=((foldpos>>6)+x)%770; // full house//10 bits=1024
	  if (tmper<66) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<322) buf16[(foldpos+x)%32768]=stacker[tmper-66];
	  else if (tmper<578) buf16[(foldpos+x)%32768]=stackery[tmper-322];
	  else if (tmper<642) buf16[(foldpos+x)%32768]=m->m_threads[tmper-578].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-642];
#endif
	}
	}
	else if (xx==0) {
	foldy=adc_buffer[SECOND]>>2; // howmuch=10 bits=1024
	foldpos=adc_buffer[FOURTH]<<4; // 16 bits
	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<292) stacker[tmper-32]=buf16[(foldpos+x)%32768];
	  else if (tmper<548) stackery[tmper-288]=buf16[(foldpos+x)%32768];
	  else if (tmper<612) m->m_threads[tmper-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-612]=buf16[(foldpos+x)%32768];
#else
	  tmper=((foldpos>>6)+x)%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<322) stacker[tmper-66]=buf16[(foldpos+x)%32768];
	  else if (tmper<578) stackery[tmper-322]=buf16[(foldpos+x)%32768];
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-642]=buf16[(foldpos+x)%32768];
#endif
	}
	}
	break;
	////////////////
      case 14: // infection
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability

      xx=fingerdir(&spd);

      if (xx!=5) {
	tmp=adc_buffer[SECOND]>>2; // 10 bits//offset
	tmper=adc_buffer[FOURTH]>>2;//amount-10 bits

	if ((adc_buffer[FOURTH]>>5)==0){
	  for (x=0;x<(tmper%INFECTSIZE);x++){
	    if ((rand()%255) > (spd)) settingsarrayinfected[(tmp+x)%INFECTSIZE][which]=1; // infected 3
	  else settingsarrayinfected[(tmp+x)%INFECTSIZE][which]=0;
	  } // reset!
	}
	  // run infection at speed third knob
	  else {

	    if (++del>=(adc_buffer[THIRD]>>7)){ // speed
	      del=0;
	  for (i=0;i<(tmper%INFECTSIZE);i++){
      	    // infection - how many infected (not dead) round each one?
	    x=(i+tmp)%INFECTSIZE;
	      tmpacht=(x-1)%INFECTSIZE;

	      if (settingsarrayinfected[x][which]==0 && settingsarrayinfected[tmpacht][which]>=1 && settingsarrayinfected[tmpacht][which]<128 && settingsarrayinfected[(x+1)%INFECTSIZE][which]>=1 && settingsarrayinfected[(x+1)%INFECTSIZE][which]<128 && ((rand()%255) > (spd))) {
		settingsarrayinfected[x][other]=1;
		//			  printf("infecte\n");
	      }
	    // inc
	      else if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128) {
		  settingsarrayinfected[x][other]++;
		  //		  	  printf("not\n");
	      }
#ifdef LACH
	    if (x<36) {
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	settingsarray[x]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) settingsarray[x]+=settingsarrayinfected[x][other];
	    }
	    else if (x<292) {
	      //	      stacker[x-32]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stacker[x-36]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stacker[x-36]+=settingsarrayinfected[x][other];;

	    }
	    else if (x<548) {
	      //	      stackery[x-288]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stackery[x-292]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stackery[x-292]+=settingsarrayinfected[x][other];;
	    }
	    else if (x<612) {
	      //	      m->m_threads[x-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	m->m_threads[x-548].m_CPU-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) m->m_threads[x-548].m_CPU+=settingsarrayinfected[x][other];;
	    }
	    else {
	      //	      villager[x-608]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)  villager[x-612]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) villager[x-612]+=settingsarrayinfected[x][other];;
	    }
#else
	    if (x<66) {
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	settingsarray[x]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) settingsarray[x]+=settingsarrayinfected[x][other];;
	    }
	    else if (x<322) {
	      //	      stacker[x-32]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stacker[x-66]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stacker[x-66]+=settingsarrayinfected[x][other];;

	    }
	    else if (x<578) {
	      //	      stackery[x-288]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stackery[x-322]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stackery[x-322]+=settingsarrayinfected[x][other];;
	    }
	    else if (x<642) {
	      //	      m->m_threads[x-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	m->m_threads[x-578].m_CPU-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) m->m_threads[x-578].m_CPU+=settingsarrayinfected[x][other];;
	    }
	    else {
	      //	      villager[x-608]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)  villager[x-642]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) villager[x-642]+=settingsarrayinfected[x][other];
	    }
#endif
	  }
	    }//del!
	  which^=1;
	  other^=1;
	  //	  printf("wich %d uther %d\n",which,other);
	  }
      }
	break;
    case 15: // fingers in the code... navigate and insert code - no knobs(?)
      // left-right move in datagen
      // down-up into all code values
      xx=fingerdir(&spd);
	    // xx=2;
      if (xx==1){ //right
	fingerposl+=spd;
	buf16[fingerposl%32768]=adc_buffer[RIGHT]<<4;
      }
      else if (xx==3){ //left
	fingerposl-=spd;
	buf16[fingerposl%32768]=adc_buffer[LEFT]<<4;
      }
      else if (xx==2){ //down
	fingerposl+=(spd>>1);
#ifdef LACH
	tmper=fingerposl%740; // full house//AUG was >>6
	  if (tmper<36) settingsarray[tmper]=adc_buffer[DOWN]<<4;
	  else if (tmper<292) stacker[tmper-36]=adc_buffer[DOWN]<<4;
	  else if (tmper<548) stackery[tmper-292]=adc_buffer[DOWN]<<4;
	  else if (tmper<612) m->m_threads[tmper-548].m_CPU=adc_buffer[DOWN]>>7;
	  else villager[tmper-612]=adc_buffer[DOWN]<<4;
#else
	  tmper=fingerposl%770; // full house//AUG was >>6
	  if (tmper<66) settingsarray[tmper]=adc_buffer[DOWN]<<4;
	  else if (tmper<322) stacker[tmper-66]=adc_buffer[DOWN]<<4;
	  else if (tmper<578) stackery[tmper-322]=adc_buffer[DOWN]<<4;
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=adc_buffer[DOWN]>>7;
	    else villager[tmper-642]=adc_buffer[DOWN]<<4;
#endif
      }
      else {//UP!
	fingerposl-=(spd>>1);
#ifdef LACH
		  tmper=fingerposl%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=adc_buffer[UP]<<4;
	  else if (tmper<292) stacker[tmper-36]=adc_buffer[UP]<<4;
	  else if (tmper<548) stackery[tmper-292]=adc_buffer[UP]<<4;
	  else if (tmper<612) m->m_threads[tmper-548].m_CPU=adc_buffer[UP]>>7;
	  else villager[tmper-612]=adc_buffer[UP]<<4;
#else
	  tmper=fingerposl%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=adc_buffer[UP]<<4;
	  else if (tmper<322) stacker[tmper-66]=adc_buffer[UP]<<4;
	  else if (tmper<578) stackery[tmper-322]=adc_buffer[UP]<<4;
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=adc_buffer[UP]>>7;
	    else villager[tmper-642]=adc_buffer[UP]<<4;
#endif
      }
      break;
      }

      //END MODECODE      /////////////////////////////////////
      /// DEAL with settingsattach and other attachs....

      /////////////////////////////////////
      // 4-hardware operations

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
