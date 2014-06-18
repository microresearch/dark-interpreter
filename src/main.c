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

#define FOLD_SIZE 28
#define MAX_EXE_STACK 4
#define VILLAGE_SIZE (STACK_SIZE*2) // 128

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
//typedef float float32_t;
u8 digfilterflag;
//int16_t src[BUFF_LEN], dst[BUFF_LEN];

int16_t *villager,*stacker,*stackery,*settingsarray;
int16_t *src, *dst;
u8 *village_effects;// [VILLAGE_SIZE/2];
u8 *settingsarrayattached; //64
u8 *settingsarrayinfected; //64

void  dohardwareswitch(u8 one,u8 two){
  // nothing
}

void set40106pwm(u16 one){
}

void setmaximpwm(u16 one){
}

void setlmpwm(u16 one, u16 two){
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
u16 settingsarray[64];
u16 FOLDD[FOLD_SIZE]; // 28
u8 settingsarrayattached[64];
u8 settingsarrayinfected[64];
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

signed char direction[2]={-1,1};
u8 setwalk[8]={239,240,1,17,16,15,254,238}; 

u16 villagestackpos=0;
u8 www[3]={10,10,0};
u8 freqyy[3]={12,15,0};

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
#define randi() (adc_buffer[9]) // 12 bits
#else //PCSIM
#define randi() (rand()%4096)
u8* datagenbuffer;
extern int16_t* audio_buffer;
u16* FOLDD; // MAX size 28 now*sizeof(int16_t));
#endif
extern u8 digfilterflag;

//u8 testdirection;
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
  if (villagepos<(VILLAGE_SIZE-2)) /// size -2
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

u8 fingerdir(void){
  u8 handleft, handright, up=0,down=0,left=0,right=0;
  u8 handupp, handdown;
  u8 result=5;

  for (u8 x=0;x<16;x++){
  handupp=adc_buffer[UP]>>8; 
  handdown=adc_buffer[DOWN]>>8;
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handupp>2) up++;
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (up>8 && up>down && up>left && up>right) {
    result=0;
  }
  else if (down>8 && down>left && down>right) {
    result=2; 
  }
  else if (left>8 && left>right) {
    result=3;
  }
  else if (right>8) {
    result=1;
  }
  }
  return result;
}

u8 fingerdirr(void){

  u8 handleft, handright, up=0,down=0,left=0,right=0;
  u8 handupp, handdown;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handupp=adc_buffer[UP]>>8; 
  handdown=adc_buffer[DOWN]>>8;
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handupp>2) up++;
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (up>8 && up>down && up>left && up>right) {
    result=-8;
  }
  else if (down>8 && down>left && down>right) {
    result=+9; 
  }
  else if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8) {
    result=1;
  }
  }
  return result;
}

u8 fingerdirleftright(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
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

signed char fingerdirupdownn(void){
  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirleftrightt(void){
  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-1;
  }
  else if (right>8 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirleftrighttx(signed char vall){
  u8 handleft, handright, left=0,right=0;
  static signed char result=0; signed char tmp;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    tmp=-1;
  }
  else if (right>8 && right>left) {
    tmp=1;
  }
  }
  //  return tmp;
  result+=tmp;
  if (result<=(-vall)) {
    result=0;
    tmp=-1;
  }
  else if (result>=vall) {
    result=0;
    tmp=1;
  }
  else tmp=0;
  return tmp;
}

signed char fingerdirupdownx(signed char vall){
  u8 handleft, handright, left=0,right=0;
  static signed char result=0; signed char tmp;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    tmp=-1;
  }
  else if (right>8 && right>left) {
    tmp=1;
  }
  }
  //  return tmp;
  result+=tmp;
  if (result<=(-vall)) {
    result=0;
    tmp=-1;
  }
  else if (result>=vall) {
    result=0;
    tmp=1;
  }
  else tmp=0;
  return tmp;
}



float32_t fingerdirleftrighttf(void){ 

  u8 handleft, handright, left=0,right=0;
  float32_t result=0.0f;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;
  if (left>8 && left>right) {
    result=-0.001f;  // TODO tuning
  }
  else if (right>8 && right>left) {
    result=0.001f;
  }
  }
  return result;
}


u16 fingervalup16bits(u16 tmpsetting, u8 inc){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) tmpsetting+=inc;
  else if (ttss<sstt) tmpsetting-=inc;
  return tmpsetting;
}

u8 fingervalup(u8 tmpsetting){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) tmpsetting+=1;
  else if (ttss<sstt) tmpsetting-=1;
  return tmpsetting;
}

u8 fingervaleff(u8 tmpsetting){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){
  handup=adc_buffer[RIGHT]>>8;
  handdown=adc_buffer[LEFT]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    if (tmpsetting>=8) tmpsetting=1;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==0) tmpsetting=7; // wraps
  }
  return tmpsetting;
}

u8 fingervalright(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){ 
    handup=adc_buffer[RIGHT]>>8; // 4bits=16
    handdown=adc_buffer[LEFT]>>8;
    if (handup>2) ttss++;// was 8 - TODO: tweak
    else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    tmpsetting=tmpsetting%wrap;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==255) tmpsetting=wrap-1;
  }
  return tmpsetting;
}

u8 fingervalupwrap(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;
  for (x=0;x<16;x++){
  handup=adc_buffer[UP]>>8;
  handdown=adc_buffer[DOWN]>>8;
  if (handup>2) ttss++;
  else if (handdown>2) sstt++;
  }
  if (ttss>sstt) {
    tmpsetting+=1;
    tmpsetting=tmpsetting%wrap;
  }
  else if (ttss<sstt) {
tmpsetting-=1;
 if (tmpsetting==255) tmpsetting=wrap-1;
  }
  return tmpsetting;
}

////////////////////////////////////////////////////////////////////////////////

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 coo,x,addr,tmp=0,tmphardware=0;u8 HARDWARE=0;
  u8 del=0,machine_count=0,leak_count=0,villagepos=0,settingspos=0,m1flag=0,villagerdest,cpur; 
  u8 exestack[MAX_EXE_STACK];
  u16 tmper;

  inittable(3,4,randi());
  wormdir=0;
  const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase=0;
  int sign_samp,i;
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
  Codec_Init(48000); // was 48000
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
  audio_buffer=(u8*)malloc(65536);
  settingsarray=malloc(64*sizeof(int16_t));
  villager=malloc(VILLAGE_SIZE*sizeof(int16_t));
  stacker=malloc(4*64*sizeof(int16_t));
  stackery=malloc(4*64*sizeof(int16_t));
  FOLDD=malloc(28*sizeof(int16_t));
  adc_buffer=malloc(10*sizeof(int16_t));
  initaudio();
  srandom(time(0));
  src=malloc(BUFF_LEN*sizeof(int16_t));
  dst=malloc(BUFF_LEN*sizeof(int16_t));
  village_effects=malloc(VILLAGE_SIZE/2);
  settingsarrayattached=malloc(64);
  settingsarrayinfected=malloc(64);

  for (x=0;x<(BUFF_LEN);x++){
    src[x]=rand()%65536;
    dst[x]=rand()%65536;
  }

  for (x=0;x<32768;x++){
    audio_buffer[x]=rand()%65536;
  }
#endif

  u8 hwdel=0;
  u16 hwpos=0;
  signed char stack_pos=0;
  signed char stack_posy=0;
  u16 start,wrap;
  u8 exenums=0;

  struct stackey stackyy[STACK_SIZE];
  struct stackeyyy stackyyy[STACK_SIZE];
  u16 *buf16 = (u16*) datagenbuffer;
  u8 *audio_buf = (u8*) audio_buffer;
  u8 leakiness=randi()%255;
  u8 infection=randi()%255;

  for (x=0;x<64;x++){
    if ((rand()%255) > (adc_buffer[SECOND]>>4)) settingsarrayinfected[x]=1; // infected
    else settingsarrayinfected[x]=0;
  }

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
     buf16[x]=randi()<<4;
    delayxx();
  }

  for (x=0;x<64;x++){
    settingsarrayattached[x]=0;
  }

  //****** setup code for walkers
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

  settingsarray[46]=0; // EFFROFFSET
  settingsarray[47]=0; // EFFFOFFSET

  EFFECTWRITE=0;

  for (x=48;x<51;x++){
    settingsarray[x]=65535;
  }//fmods

  for (x=51;x<54;x++){
    settingsarray[x]=0; 
  }//expand

  for (x=54;x<64;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR
	 
  // CPUintrev3:
  for (x=0; x<100; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,datagenbuffer,addr,randi()<<3,randi()%CPU_TOTAL,randi()%24);
    }

  for (x=0;x<FOLD_SIZE;x++){
        FOLDD[x]=randi()<<4; 
  }

  //pureleak

  for (x=0;x<100;x++){
    addr=randi()<<3;
        cpustackpushhh(datagenbuffer,addr,randi()<<3,randi()%CPU_TOTAL,randi()%24);
  }

  // CA
  for (x=0;x<(STACK_SIZE);x++){
    //      start=0; wrap=32768; // TESTY!
          start=randi()<<4;
          wrap=randi()<4;
	  stack_posy=ca_pushn(stackyyy,randi()<<4,datagenbuffer,stack_posy,randi()<<4,start,wrap); 
  }

  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
  //  for (x=0;x<2;x++){ // TESTY!
                start=randi()<<4;
      //      start=0; wrap=32767; // TESTY!
            wrap=randi()<<4;
      stack_pos=func_pushn(stackyy,randi()<<4,buf16,stack_pos,randi()<<4,start,wrap);
            villagestackpos=villagepush(villagestackpos,start,wrap,randi()%16);
      //      villagestackpos=villagepush(villagestackpos,start,wrap,);
	//	        printf("TESTY:%d\n",wrap);
  }

    // execution stack - TESTER!
        for (x=0;x<MAX_EXE_STACK;x++){
	        exenums=exestackpush(exenums,exestack,randi()%4); //exetype=0-3 TESTY!
	  //      exenums=exestackpush(exenums,exestack,); //exetype=0-3 TESTY!
      }

	u8 mainmode,groupstart,groupwrap;
	u8 xx,stackerpos,stackerypos,cpupos,villageepos,dirpos,groupsel,foldposss,attachpos,groupstartt,wormstart,wormpos;
	u16 foldpos,settingsposl,datagenpos;

	float32_t settingsposf;

  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

#ifdef TEST_EEG
      //write ADC9 into buf16
  for (x=0;x<32768;x++){
    buf16[x]=randi()<<4; // 16 bits
  }
#else

#ifdef PCSIM
      // randomise adc_buffer
      for (x=0;x<10;x++){
	adc_buffer[x]=randi();
      }

      I2S_RX_CallBack(src, dst, BUFF_LEN/2); 
#endif
      
      for (x=0;x<exenums;x++){
	switch(exestack[x]){
	case 0:
	  func_runall(stackyy,stack_pos); // simulations
	  break;
	case 1:
	  ca_runall(stackyyy,stack_posy); // CA
	  break;
	case 2:
	  //	  machine_count++;
	  //	  if (machine_count>=MACHINESPEED){
	    machine_run(m); //cpu
	    m->m_leakiness=leakiness;
	    m->m_infectprob=infection;
	    //	    machine_count=0;
	    //	  }
	  break;
	case 3:
	  //	  leak_count++;
	  //	  if (leak_count>=LEAKSPEED){
	    machine_runnn(datagenbuffer); // pureleak
	    //	    leak_count=0;
	    //	  }
	  break;
	}
      }

      /////////////////////////////////////
	      
      //MODECODE      /////////////////////////////////////


      EFFECTWRITE=adc_buffer[FOURTH]>>5; // 7 bits = 128 rest of effects as offsets///
      // do we need to average or reduce to 64?
      // top bit as wormcode
      mainmode=adc_buffer[FIRST]>>7; // 5 bits = 32 // TESTY! TODO!
      //            mainmode=29; 
      switch(mainmode){
      case 0:
	settingspos+=fingerdirleftrighttx(10);
	settingspos=settingspos%64;
	xx=fingerdirupdown();
	if (xx==1){
	  settingsarray[settingspos]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  settingsarray[settingspos]=adc_buffer[UP]<<4;
	}
	break;
	/////////////////////////////
      case 1:
	stackerpos+=fingerdirleftrighttx(4);
	xx=fingerdirupdown();
	if (xx==1){
	  stacker[stackerpos]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  stacker[stackerpos]=adc_buffer[UP]<<4;
	}
	break;
      case 2:
	stackerypos+=fingerdirleftrighttx(4);
	xx=fingerdirupdown();
	if (xx==1){
	  stackery[stackerypos]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  stackery[stackerypos]=adc_buffer[UP]<<4;
	}
	break;
      case 3:
	cpupos+=fingerdirleftrighttx(4);
	cpupos=cpupos%68; //64,65,66,67
	xx=fingerdirupdown();
	if (xx==1){
	  if (cpupos<63) m->m_threads[cpupos].m_CPU=adc_buffer[SECOND]>>7; // 5 bits
	  else exestack[cpupos-64]=adc_buffer[SECOND]>>10; // 2 bits 
	}
	if (xx==0){
	  if (cpupos<63) m->m_threads[cpupos].m_CPU=adc_buffer[UP]>>7; // 5 bits
	  else exestack[cpupos-64]=adc_buffer[UP]>>10; // 2 bits 
	}
	break;
      case 4:
	villagepos+=fingerdirleftrighttx(4);
	villagepos=villagepos%128;
	xx=fingerdirupdown();
	if (xx==1){
	  villager[villagepos]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  villager[villagepos]=adc_buffer[UP]<<4;
	}
	break;
      case 5:
	villageepos+=fingerdirleftrighttx(4);
	villageepos=villageepos%64;
	xx=fingerdirupdown();
	if (xx==1){
	  village_effects[villageepos]=adc_buffer[SECOND]>>8; // 4 bits
	}
	if (xx==0){
	  village_effects[villageepos]=adc_buffer[UP]>>8;
	}
	break;
      case 6:
	datagenpos+=fingerdirleftrightt();
	datagenpos=datagenpos%32768;
	xx=fingerdirupdown();
	if (xx==1){
	  buf16[datagenpos]=adc_buffer[SECOND]<<4; 
	}
	if (xx==0){
	  buf16[datagenpos]=adc_buffer[UP]<<4; 
	}
	break;
      case 7: // directions
	dirpos+=fingerdirleftrighttx(24);
	dirpos=dirpos%10;
	xx=fingerdirupdown();
	if (xx==1){
	  settingsarray[54+dirpos]=adc_buffer[SECOND]<<4; 
	}
	if (xx==0){
	  settingsarray[54+dirpos]=adc_buffer[UP]<<4; 
	}
	break; 
      case 8: // foldback
	foldposss+=fingerdirleftrighttx(16);
	foldposss=foldposss%FOLD_SIZE;
	xx=fingerdirupdown();
	if (xx==1){
	  FOLDD[foldposss]=adc_buffer[SECOND]<<4; 
	}
	if (xx==0){
	  FOLDD[foldposss]=adc_buffer[UP]<<4; 
	}
	break; 
      case 9: // settingsarrayattached
	attachpos+=fingerdirleftrighttx(16);
	attachpos=attachpos%64;
	xx=fingerdirupdown();
	if (xx==1){
	  settingsarrayattached[attachpos]=adc_buffer[SECOND]>>10; 
	}
	if (xx==0){
	  settingsarrayattached[attachpos]=adc_buffer[UP]>>10; 
	}
	break; 
	//// algo groups
      case 10:
	settingsposl+=fingerdirleftrightt();
	groupstartt=datagenbuffer[settingsposl]>>2;// 16 bits//6 bits
	xx=fingerdirupdown();
	if (xx==1){
	  settingsarray[groupstartt]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  settingsarray[groupstartt]=adc_buffer[UP]<<4;
	}
	break;
      case 11: // villager
	settingsposl+=fingerdirleftrightt();
	groupstartt=datagenbuffer[settingsposl]>>1;// 16 bits//7 bits
	xx=fingerdirupdown();
	if (xx==1){
	  villager[groupstartt]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  villager[groupstartt]=adc_buffer[UP]<<4;
	}
	break;
      case 12: // foldback
	settingsposl+=fingerdirleftrightt();
	groupstartt=datagenbuffer[settingsposl]>>3;// 16 bits//5 bits
	xx=fingerdirupdown();
	if (xx==1){
	  FOLDD[groupstartt%FOLD_SIZE]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  FOLDD[groupstartt%FOLD_SIZE]=adc_buffer[UP]<<4;
	}
	break;
      case 13: // wormdir walker across settingsarray//villagers14//foldback15
	wormstart+=fingerdirleftrighttx(16);
	wormpos+=setwalk[wormdir];
	xx=fingerdirupdown();
	if (xx==1){
	  settingsarray[(wormstart+wormpos)%64]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  settingsarray[(wormstart+wormpos)%64]=adc_buffer[UP]<<4;
	}
	break;
      case 14: // wormdir walker across settingsarray//villagers14//foldback15
	wormstart+=fingerdirleftrightt();
	wormpos+=setwalk[wormdir];
	xx=fingerdirupdown();
	if (xx==1){
	  villager[(wormstart+wormpos)%VILLAGE_SIZE]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  villager[(wormstart+wormpos)%VILLAGE_SIZE]=adc_buffer[UP]<<4;
	}
	break;
      case 15: // wormdir walker across settingsarray//villagers14//foldback15
	wormstart+=fingerdirleftrightt();
	wormpos+=setwalk[wormdir];
	xx=fingerdirupdown();
	if (xx==1){
	  FOLDD[(wormstart+wormpos)%FOLD_SIZE]=adc_buffer[SECOND]<<4;
	}
	if (xx==0){
	  FOLDD[(wormstart+wormpos)%FOLD_SIZE]=adc_buffer[UP]<<4;
	}
	break;

	////////////////////////////////////////////////////////////SETTED AND MIRROR

      case 16: //SETTED!
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	// 6 bits groupstart by way of finger LEFT/RIGHT
	groupstart+=fingerdirleftrighttx(16);
	groupstart=groupstart%64;
	groupsel+=fingerdirupdownx(32); // TODO-redo as leftrighttx
	groupsel=groupsel%4;

	for (x=0;x<groupwrap;x++){
	switch(groupsel){
	case 0:
	  settingsarray[(groupstart+x)%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  settingsarrayattached[(groupstart+x)%64]=1;
	  coo++;
	  break;
	case 1:
	  settingsarray[(groupstart+x)%64]=adc_buffer[DOWN]<<4;
	  settingsarrayattached[(groupstart+x)%64]=2;
	  break;
	case 2:
	  settingsarray[(groupstart+x)%64]=adc_buffer[THIRD]<<4;
	  settingsarrayattached[(groupstart+x)%64]=3;
	  break;
	case 3: 
	  // DEtach
	  settingsarrayattached[(groupstart+x)%64]=0;
	  break;
	}
	}
	break;
	///////////////////////////MIRRORS
	case 17:      // mirror and swaps inc. new datagen dump:
	FOLDD[0]=adc_buffer[SECOND]>>6; // how much=6 bits
	foldpos+=fingerdirleftrightt(); // 16 bits

	for (x=0;x<(FOLDD[0]);x++){ //was >>9
	  settingsarray[(((foldpos)>>10)+x)%64]=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768];
	  coo++;
	}
	// toggle flag with the finger
	xx=fingerdirupdown();
	if (xx==1) m1flag|=1; //sets
	else if (xx==0) m1flag&=~1;
	break;

	case 18:      // mirror and swaps inc. new datagen dump:
	FOLDD[0]=adc_buffer[SECOND]>>6; // how much
	foldpos+=fingerdirleftrighttx(16);

	for (x=0;x<(FOLDD[0]);x++){ //was >>9
	  settingsarray[(foldpos+x)%64]=randi()<<4;
	}
	// toggle flag with the finger
	xx=fingerdirupdown();
	if (xx==1) m1flag|=2; //sets
	else if (xx==0) m1flag&=~2;
	break;

      case 19:
	FOLDD[0]=adc_buffer[SECOND]>>5; // how much = 7 bits as 128
	foldpos+=fingerdirleftrightt(); // 16 bits

	for (x=0;x<(FOLDD[0]);x++){
	  villager[(((foldpos)>>9)+x)% VILLAGE_SIZE]=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768];
	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=4; //sets
	else if (xx==0) m1flag&=~4; 
	break;

      case 20:
	FOLDD[0]=adc_buffer[SECOND]>>5; // how much
	foldpos+=fingerdirleftrighttx(16);

	for (x=0;x<(FOLDD[0]);x++){
	  villager[((foldpos>>9)+x)% VILLAGE_SIZE]=randi()<<4;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=8; //sets
	else if (xx==0) m1flag&=~8; 
	break;

	case 21:
	  FOLDD[0]=adc_buffer[SECOND]>>4; // howmuch-8 bits
	  foldpos+=fingerdirleftrightt(); // 16 bits

	for (x=0;x<(FOLDD[0]);x++){ 
	  stacker[(((foldpos)>>8)+x)%256]=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768];
	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=16; //sets
	else if (xx==0) m1flag&=~16; 
	break;

	case 22:
	  FOLDD[0]=adc_buffer[SECOND]>>4; // howmuch-8 bits
	  foldpos+=fingerdirleftrightt(); // 16 bits

	for (x=0;x<(FOLDD[0]);x++){ 
	  stackery[(((foldpos)>>8)+x)%256]=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768];
	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=32; //sets
	else if (xx==0) m1flag&=~32; 
	break;

      case 23: // cpu
	  FOLDD[0]=adc_buffer[SECOND]>>6; // howmuch-64
	  foldpos+=fingerdirleftrightt(); // 16 bits

	for (x=0;x<(FOLDD[0]);x++){ 
	  m->m_threads[(((foldpos)>>10)+x)%64].m_CPU=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768]>>11; // 5 bits
	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=64; //sets
	else if (xx==0) m1flag&=~64; 
	break;

      case 24: // fold onto fold
	FOLDD[0]=adc_buffer[SECOND]>>6; // howmuch-64
	foldpos+=fingerdirleftrightt(); // 16 bits
	for (x=0;x<((FOLDD[0]>>1));x++){
	  FOLDD[(((foldpos)>>10)+x)% FOLD_SIZE]=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768];
	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=128; //sets
	else if (xx==0) m1flag&=~128; 
	break;
	///////////////////////////////////////////////////////
      case 25: // into attachment
	FOLDD[0]=adc_buffer[SECOND]>>6; // howmuch-64
	foldpos+=fingerdirleftrightt(); // 16 bits

	for (x=0;x<(FOLDD[0]);x++){ //was >>9
	  settingsarrayattached[(((foldpos)>>10)+x)%64]=buf16[((foldpos>>1)+(coo%(FOLDD[0]+1)))%32768]>>14; // last 2 bits
	}

	///////////////////////////////////////////
	// swops now 
	// settingsarray<->settingsarray
	// villager<->villager
	// stacker/y<->villager and vice versa

      case 26:
	FOLDD[0]=adc_buffer[SECOND]>>6; // howmuch-64
	foldpos+=fingerdirleftrighttx(16);
	foldpos=foldpos%64;
	for (x=0;x<(FOLDD[0]);x++){
	  settingsarray[(foldpos+x)%64]=settingsarray[(foldpos+(FOLDD[1]>>10)+x)%64];
	}
	break;

      case 27:
	FOLDD[0]=adc_buffer[SECOND]>>5; // howmuch-128
	foldpos+=fingerdirleftrighttx(16);
	foldpos=foldpos%64;
	for (x=0;x<(FOLDD[0]);x++){
	  villager[(foldpos+x)%VILLAGE_SIZE]=villager[(foldpos+(FOLDD[1]>>9)+x)%VILLAGE_SIZE];
	}
	break;

	/// HERE!

      case 28: // various stack and villager exchanges - 
	// starts and ends only of stacks (not CPU) -> villagers
	
	FOLDD[0]=adc_buffer[SECOND]>>6; // howmuch-64
	foldpos+=fingerdirleftrighttx(16); // 16 bits
	foldpos=foldpos%128;
	for (x=0;x<(FOLDD[0]);x++){ // 64 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=(((foldpos)>>1)+x+(FOLDD[1]>>10))%(VILLAGE_SIZE/2); // village entry
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
	break;

      case 29: // various stack and villager exchanges - 
	// other way round
	
	FOLDD[0]=adc_buffer[SECOND]>>5; // howmuch-128
	foldpos+=fingerdirleftrighttx(16); // 16 bits
	foldpos=foldpos%128;

	for (x=0;x<(FOLDD[0]);x++){ 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=(((foldpos)>>1)+x+(FOLDD[1]>>10))%(VILLAGE_SIZE/2); // village entry

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
	break;

      case 30: // dump (all?) to datagen?
	FOLDD[0]=adc_buffer[SECOND]>>2; // howmuch=10 bits
	foldpos+=fingerdirleftrightt(); // 16 bits
	for (x=0;x<FOLDD[0];x++){ // 10 bits
	  tmper=((foldpos>>6)+x)%768; // full house//10 bits=1024
	  if (tmper<64) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<320) buf16[(foldpos+x)%32768]=stacker[tmper-64];
	  else if (tmper<576) buf16[(foldpos+x)%32768]=stackery[tmper-320];
	  else if (tmper<640) buf16[(foldpos+x)%32768]=m->m_threads[tmper-576].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-640];
	}
	break;
	////////////////
      case 31: // infection
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability
	if ((adc_buffer[SECOND]>>5)==0){
	  for (x=0;x<64;x++){
	    if ((rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1; // infected
	  else settingsarrayinfected[x]=0;
	  }
	  // run infection at speed eff[0] 

	  for (x=0;x<64;x++){
	    // infection - how many infected (not dead) round each one?
	    if (++del==(adc_buffer[SECOND]>>5)){
	      if (settingsarrayinfected[x]==0 && ((settingsarrayinfected[(x-1)%64]>=1 && settingsarrayinfected[x-1]<128) || (settingsarrayinfected[(x+1)%64]>=1 && settingsarrayinfected[x+1]<128)) && (rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1;
	    // inc
	    if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128) settingsarrayinfected[x]++;
	    }

	  // overmap onto buffer eff[0]: 0=stay same/infect=reduce by days/dead=128=zero
	    //0/settingsarray 1/villager 2/3/4//stacksandCPU 5/foldback	    
	    switch(adc_buffer[SECOND]>>9) // 8 cases
	      {
	      case 0:
	      default:
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	settingsarray[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) settingsarray[x]=0;
		break;
	      case 1:
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	stacker[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) stacker[x]=0;
		break;
	      case 2:
 		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	stackery[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) stackery[x]=0;
		break;
	      case 4:
 		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	{
		  cpur=x%(m->m_threadcount);
		  m->m_threads[cpur].m_CPU-=settingsarrayinfected[x];//cpu
		  m->m_threads[cpur].m_CPU%=31;
		}
		else if (settingsarrayinfected[x]>127) {
		  m->m_threads[x%(m->m_threadcount)].m_CPU=0;//cpu
		}
		break;
	      case 5:
		if (x<14){
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	FOLDD[x]-=settingsarrayinfected[x]; // foldd max
		else if (settingsarrayinfected[x]>127) FOLDD[x%FOLD_SIZE]=0; // foldd max
		}
		break;
	      }
	    /////
	  }
	}
	break;

      }

      //END MODECODE      /////////////////////////////////////

      //// DEAL WITH toggled MIRRORING

      if (m1flag&1){ 
	for (x=0;x<((FOLDD[0]>>10));x++){
	  settingsarray[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>10)+1)))%64]=buf16[((FOLDD[3]>>1)+(coo%((FOLDD[4]>>10)+1)))%32768];
	  coo++;
	}
      }

      if (m1flag&2){ 
	for (x=0;x<((FOLDD[0]>>10));x++){
	  settingsarray[(((FOLDD[5])>>10)+(x%((FOLDD[6]>>10)+1)))%64]=(randi()<<4);
	  coo++;
	}
      }

      if (m1flag&4){ 
	for (x=0;x<((FOLDD[0]>>10));x++){
	  villager[(((FOLDD[7])>>9)+(x%((FOLDD[8]>>9)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[9]>>1)+(coo%((FOLDD[10]>>10)+1)))%32768];
	  coo++;
	}
      }

      if (m1flag&8){ 
	for (x=0;x<((FOLDD[0]>>10));x++){
	  villager[(((FOLDD[11])>>9)+(x%((FOLDD[12]>>9)+1)))% VILLAGE_SIZE]=(randi()<<3);
	  coo++;
	}
      }

      if (m1flag&16){ 
	for (x=0;x<((FOLDD[0]>>8));x++){ // goes high enough -8 bits
	  stacker[(((FOLDD[13])>>8)+(x%((FOLDD[14]>>8)+1)))% 256]=buf16[((FOLDD[15]>>1)+(coo%((FOLDD[16]>>8)+1)))%32768];
	    coo++;
	}
      }

      if (m1flag&32){ 
	for (x=0;x<((FOLDD[0]>>8));x++){ // goes high enough -8 bits
	  stackery[(((FOLDD[17])>>8)+(x%((FOLDD[18]>>8)+1)))% 256]=buf16[((FOLDD[19]>>1)+(coo%((FOLDD[20]>>8)+1)))%32768];
	    coo++;
	}
      }

      if (m1flag&64){ // cpu 
	for (x=0;x<((FOLDD[0]>>10));x++){ // 6 bits
	  //	  stackery[tmper]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768];
	  m->m_threads[(((FOLDD[21])>>10)+(x%((FOLDD[22]>>10)+1)))% 64].m_CPU=buf16[((foldpos>>1)+(coo%(FOLDD[23]+1)))%32768]>>11;
	    coo++;
	}
      }

      if (m1flag&128){ 
	for (x=0;x<((FOLDD[0]>>10));x++){
	  FOLDD[(((FOLDD[24])>>10)+(x%((FOLDD[25]>>10)+1)))% FOLD_SIZE]=buf16[((FOLDD[26]>>1)+(coo%((FOLDD[27]>>10)+1)))%32768];
	  	  coo++;
	}
      }

      /// DEAL with settingsattach:

      for (x=0;x<64;x++){
	switch(settingsarrayattached[x]){
	case 1:
	  settingsarray[x]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  break;
	case 2:
	  settingsarray[x]=adc_buffer[DOWN]<<4;
	  break;
	case 3:
	  settingsarray[x]=adc_buffer[THIRD]<<4;
	  break;
	}
      }

      //// DEAL last with hardware:
#ifdef LACH
      // TODO! as EFFECTREAD!
#else
      tmphardware=0;
      for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIFTH]>>5; // 7 bits
      }
      HARDWARE=tmphardware>>8; //was >>8 to divide average
#endif
      /////////////////////////////////////
      //// MODE/KNOB CODE END

#ifndef LACH
      /////////////////////////////////////
      // 4-hardware operations
      
      /// general HW walk in/as tmp
      if (++hwdel>=HWSPEED){
	hwpos+=(HWSTEP*direction[HWDIR]);
	tmp=HWSTART+(hwpos%HWWRAP); //to cover all directions
	hwdel=0;
      }

      if (digfilterflag&16){
	if (HDGENERCONS==0) settingsarray[24]=256; //SET HDGENERCONS=1
	dohardwareswitch(HARDWARE,HDGENERBASE+(datagenbuffer[tmp]%HDGENERCONS));
      }
      else
	{
	  dohardwareswitch(HARDWARE,0);
	}
	     		   
      // just leave this running
		     		     
      set40106pwm(F0106ERBASE+(buf16[(tmp+F0106EROFFSET)%32768]%F0106ERCONS)); // constrain all to base+constraint

      if (digfilterflag&4){
	setlmpwm(LMERBASE+(buf16[(tmp+LMEROFFSET)%32768]%LMERCONS),LMERBASE+(buf16[(tmp+LMEROFFSETTWO)%32768]%LMERCONS)); 
      }
	  
      if (digfilterflag&8){
	setmaximpwm(MAXIMERBASE+(buf16[(tmp+MAXIMEROFFSET)%32768]%MAXIMERCONS));
      }
     
#endif
#endif
#endif
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
