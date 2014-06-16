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

#define FOLD_SIZE 14
#define MAX_EXE_STACK 4
#define VILLAGE_SIZE (STACK_SIZE*2) // was 64 *2=128 now 96*2=192 STACK_SIZE is 64 =128 // TESTY!
//#define VILLAGE_SIZE 32 // was 64 *2=128 now 96*2=192 STACK_SIZE is 16

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
u8 village_effects[STACK_SIZE]; // but where do we set this?
u16 stackery[STACK_SIZE*4]; // 16*4 MAX
u16 stacker[STACK_SIZE*4]; // 16*4 MAX
u16 settingsarray[64];
u16 FOLDD[FOLD_SIZE]; // MAX size 14!
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

u16 EFFECTREAD,EFFECTWRITE,EFFECTFILTER;

signed char direction[2]={-1,1};

u16 villagestackpos=0;
u8 www[3]={12,15,0};
u8 freqyy[3]={10,10,0};

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
#else
#define randi() (rand()%4096)
u8* datagenbuffer;
extern int16_t* audio_buffer;
u16* FOLDD; // MAX size 14 now*sizeof(int16_t));
#endif
extern u8 digfilterflag;

//u8 testdirection;
u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array



u8 exestackpush(u8 exenum, u8* exestack, u8 exetype){
  u8 tmp;
  if (exenum<MAX_EXE_STACK){
    exestack[exenum]=exetype;
    exenum++;
  }
  return exenum;
}

u8 exestackpop(u8 exenum, u8* exestack){
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
      //        printf("pos:%d start:%d wrap:%d\n",villagepos,start,wrap);

      //      villagepos+=2;
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
  //  u8 result=2;

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
  //  u8 result=2;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;

  if (left>8 && left>right) {
    //    result=0; // UP
    return 0;
  }
  else if (right>8 && right>left) {
    //    result=1; //DOWN
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
  for (x=0;x<16;x++){ // TODO: tweak for speed?
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


void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 coo,x,addr,tmp=0,tmphardware=0,HARDWARE=0;
  u8 del=0,tmper,machine_count=0,leak_count=0,directionxx=0,codepos=0,villagepos=0,settingspos=0,whichpushpop=0,m1flag=0,villagerdest; 
  u8 exestack[MAX_EXE_STACK];

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
      yi= 16383*sinf(phase); // was 2047
      phase=phase+w;
      sign_samp=16383+yi;     // dc offset translated for a 12 bit DAC - but is 16 bit?
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
  stacker=malloc(64*sizeof(int16_t));
  stackery=malloc(64*sizeof(int16_t));
  FOLDD=malloc(14*sizeof(int16_t));
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
  u16 hwpos=0,hwposss;
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

  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;

  // fill datagenbuffer???

  /*  for (x=0;x<65535;x++){
        datagenbuffer[x]=randi()%255;
	}*/

  /*  for (x=0;x<100;x++){
  delay();
  }*/

  for (x=0;x<64;x++){
    if ((rand()%255) > (adc_buffer[SECOND]>>4)) settingsarrayinfected[x]=1; // infected
    else settingsarrayinfected[x]=0;
  }


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

  for (x=42;x<48;x++){
    settingsarray[x]=511;
  }//speed

  for (x=48;x<51;x++){
    settingsarray[x]=65535;
  }//fmods

  for (x=51;x<54;x++){
    settingsarray[x]=0; 
  }//expand

  for (x=54;x<64;x++){
    settingsarray[x]=32768;//>>15 = 1
  }//DIR
	 
  // CPUintrev2:
  for (x=0; x<100; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,datagenbuffer,addr,randi()<<3,randi()%31,randi()%24);
    }

  for (x=0;x<FOLD_SIZE;x++){
        FOLDD[x]=randi()<<4; // TESTY!
  }

  //pureleak

  for (x=0;x<100;x++){
    addr=randi()<<3;
        cpustackpushhh(datagenbuffer,addr,randi()<<3,randi()%31,randi()%24);
  }

  // CA
  for (x=0;x<(STACK_SIZE);x++){
    //      start=0; wrap=32768; // TESTY!
          start=randi()<<3;
          wrap=randi()<3;
	  stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,randi()%24,start,wrap); 
  }

  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
  //  for (x=0;x<2;x++){ // TESTY!
                start=randi()<<3;
      //      start=0; wrap=32767; // TESTY!
            wrap=randi()<<3;
      stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,randi()%24,start,wrap);
    //    stack_pos=func_pushn(stackyy,0,buf16,stack_pos,randi()%24,start,wrap);
    //    printf("stackpos %d\n",stack_pos);
            villagestackpos=villagepush(villagestackpos,start,wrap,randi()%16);
      //      villagestackpos=villagepush(villagestackpos,start,wrap,);
	//	        printf("TESTY:%d\n",wrap);
  }


    // execution stack - TESTER!
        for (x=0;x<MAX_EXE_STACK;x++){
	        exenums=exestackpush(exenums,exestack,randi()%4); //exetype=0-3 TESTY!
	  //      exenums=exestackpush(exenums,exestack,); //exetype=0-3 TESTY!
      }

    //exenums=exestackpop(exenums,exestack);

  ///////////////////////////

  u8 fingermode;

  ///////////////////////////

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

      u8 ii;

      
	      for (x=0;x<exenums;x++){
		switch(exestack[x]){
		case 0:
		  func_runall(stackyy,stack_pos); // simulations
		  break;
		case 1:
		  ca_runall(stackyyy,stack_posy); // CA
		  break;
		case 2:
		  machine_count++;
		  if (machine_count>=MACHINESPEED){
		    machine_run(m); //cpu - WRAP own speedTODO
		  m->m_leakiness=leakiness;
		  m->m_infectprob=infection;
		  machine_count=0;
		  }
		  break;
		case 3:
		  leak_count++;
		  if (leak_count>=LEAKSPEED){
		    machine_runnn(datagenbuffer); // pureleak WRAP own speedTODO-SLOW
		    leak_count=0;
		  }
		    break;
		    }
	      }

	      /////////////////////////////////////
	      
      //TODO MODECODE      /////////////////////////////////////

      /// NOTES:
      // 1- effects and udlr(updownleftright) as wormdir/left/right/set ???

      // if 1st knob is 4 modes with gradations need way transition
      // without changing gradation (eg.exp/contract)

      // so full array is:

      // effects 
      // 1-settings=64
      // 2-stacker=64x4(howmuch.start.end.cpu)=256
      // 3-stackery=64x4=256
      // 4-cpu=64
      // 5-exestack=	 4
      // [5-foldback=xx?? = walker for datagen into xxxxx]
      // 6-villager=128
	      //7	      village_effects=64 (64 total villagers)
      // 8datagen
 
      // <-> datagen (as ongoing, or not with its own walker which is
      // the foldback setting), finger, knob(s), EEG

      // + processes across arrays...
      // + swop across arrays and within arrays
      // + dump into datagen

      // navigate each array(sel-knob0) - set with knob(1) + toggle attachment/process (knob2)
      // navigation of overlaid grids - walkers
      // how to: toggle wormcode navigation
	      
      // toggle as down. bare finger as up...

	      u8 xx,stackerpos,stackerypos,cpupos,exepos,villageepos;

	      fingermode=6; // TESTY!
	      switch(fingermode){
	      case 0:
	      settingspos+=fingerdirleftrightt();
	      settingspos=settingspos%64;
	      xx=fingerdirupdown();
	      if (xx==1){
	      settingsarray[settingspos]=adc_buffer[FIRST]<<4;
	      }
	      if (xx==0){
		settingsarray[settingspos]=adc_buffer[UP]<<4;
	      }
	      break;
	      case 1:
	      stackerpos+=fingerdirleftrightt();
	      xx=fingerdirupdown();
	      if (xx==1){
	      stacker[stackerpos]=adc_buffer[FIRST]<<4;
	      }
	      if (xx==0){
		stacker[stackerpos]=adc_buffer[UP]<<4;
	      }
	      break;
	      case 2:
	      stackerypos+=fingerdirleftrightt();
	      xx=fingerdirupdown();
	      if (xx==1){
	      stackery[stackerypos]=adc_buffer[FIRST]<<4;
	      }
	      if (xx==0){
		stackery[stackerypos]=adc_buffer[UP]<<4;
	      }
	      break;
	      case 3:
	      cpupos+=fingerdirleftrightt();
	      cpupos=cpupos%64;
	      xx=fingerdirupdown();
	      if (xx==1){
		m->m_threads[cpupos].m_CPU=adc_buffer[FIRST]>>7; // 5 bits
	      }
	      if (xx==0){
		m->m_threads[cpupos].m_CPU=adc_buffer[UP]>>7;
	      }
	      case 4:
	      exepos+=fingerdirleftrightt();
	      exepos=exepos%4;
	      xx=fingerdirupdown();
	      if (xx==1){
		exestack[exepos]=adc_buffer[FIRST]>>10; // 2 bits
	      }
	      if (xx==0){
		exestack[exepos]=adc_buffer[UP]>>10;
	      }
	      break;
	      case 5:
	      villagepos+=fingerdirleftrightt();
	      villagepos=villagepos%128;
	      xx=fingerdirupdown();
	      if (xx==1){
		villager[villagepos]=adc_buffer[FIRST]<<4;
	      }
	      if (xx==0){
		villager[villagepos]=adc_buffer[UP]<<4;
	      }
	      break;
	      case 6:
	      villageepos+=fingerdirleftrightt();
	      villageepos=villageepos%64;
	      xx=fingerdirupdown();
	      if (xx==1){
		village_effects[villageepos]=adc_buffer[FIRST]>>8; // 4 bits
	      }
	      if (xx==0){
		village_effects[villageepos]=adc_buffer[UP]>>8;
	      }
	      break;

	      }
      //END MODECODE      /////////////////////////////////////


      //// DEAL last with hardware:
#ifdef LACH
      // deal as step???TODO?TEST!!!!

	  for (x=0;x<10;x++){
	    settingsarray[25+x]=adc_buffer[FIFTH]<<4; // 16 bit value
	  }
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
      // 4-hardware operations TODO: fix HW walkers...
      
      /// general HW walk in/as tmp
      if (++hwdel>=HWSPEED){
	  hwpos+=(HWSTEP*direction[HWDIR]);
	  tmp=HWSTART+(hwpos%HWWRAP); //to cover all directions
	  hwdel=0;
      }

      if (digfilterflag&16){
	if (HDGENERCONS==0) settingsarray[42]=256; //(settingsarray[42]>>8)
	dohardwareswitch(HARDWARE,HDGENERBASE+(datagenbuffer[tmp]%HDGENERCONS));
	//dohardwareswitch(HARDWARE,0);
      }
      else
	{
	  dohardwareswitch(HARDWARE,0);
	  //	  if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	  //	  oldhardware=hardware;
	}
	     		   
      // 3 datagenclocks->40106/lm/maxim - filterflag as bits as we also need signal which clocks we

      // just leave this running
		     		     
// LMEROFFSET, LMEROFFSETTWO, F0106EROFFSET, MAXIMEROFFSET
      //      if (digfilterflag&2){
      set40106pwm(F0106ERBASE+(buf16[(tmp+F0106EROFFSET)%32768]%F0106ERCONS)); // constrain all to base+constraint
      //      set40106pwm(32768);
	  //      }
	  

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
