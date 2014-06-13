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
#define MAX_EXE_STACK 8
#define VILLAGE_SIZE (STACK_SIZE*2) // was 64 *2=128 now 96*2=192 STACK_SIZE is 16 // TESTY!
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

u8 village_effects[VILLAGE_SIZE/2]; // but where do we set this?

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
u8 village_effects[VILLAGE_SIZE/2]; // but where do we set this?
u16 stackery[STACK_SIZE*4]; // 16*4 MAX
u16 stacker[STACK_SIZE*4]; // 16*4 MAX
u16 settingsarray[64];
u16 FOLDD[FOLD_SIZE]; // MAX size 14!

#endif

u8 settingsarrayattached[64];
u8 settingsarrayinfected[64];

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
  if (handupp>8) up++;
  if (handdown>8) down++;
  if (handleft>8) left++;
  if (handright>8) right++;
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

u8 fingerdirleftright(void){

  u8 handleft, handright, left=0,right=0;
  u8 result=2; 

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>8) left++;
  if (handright>8) right++;
  if (left>8 && left>right) {
    result=0;
  }
  else if (right>8 && right>left) {
    result=1;
  }
  }
  return result;
}

u8 fingerdirupdown(void){

  u8 handleft, handright, left=0,right=0;
  u8 result=2;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>8) left++;
  if (handright>8) right++;
  if (left>8 && left>right) {
    result=0;
  }
  else if (right>8 && right>left) {
    result=1;
  }
  }
  return result;
}

signed char fingerdirupdownn(void){

  u8 handleft, handright, left=0,right=0;
  signed char result=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>8) left++;
  if (handright>8) right++;
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
  if (handup>8) ttss++;
  else if (handdown>8) sstt++;
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
  if (handup>8) ttss++;
  else if (handdown>8) sstt++;
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
  if (handup>8) ttss++;
  else if (handdown>8) sstt++;
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
  for (x=0;x<32;x++){ // TODO: tweak for speed?
    handup=adc_buffer[RIGHT]>>8; // 4bits=16
    handdown=adc_buffer[LEFT]>>8;
    if (handup>8) ttss++;
    else if (handdown>8) sstt++;
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
  if (handup>8) ttss++;
  else if (handdown>8) sstt++;
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
  u16 coo,x,addr,tmp=0,tmppp=0,hdtmp=0,tmphardware=0,HARDWARE=0;
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
    //    buf16[x]=rand()%32768;
  }

  for (x=0;x<64;x++){
    settingsarrayattached[x]=0;
  }

  // setup code for walkers
  for (x=0;x<11;x++){
    settingsarray[x]=0;
  }//start

  for (x=11;x<25;x++){
    settingsarray[x]=65535;
  }//wrap

  for (x=25;x<35;x++){
    settingsarray[x]=511; //>>8
  }//step

  for (x=35;x<41;x++){
    settingsarray[x]=511;
  }//speed

  for (x=43;x<46;x++){
    settingsarray[x]=0;
  }//start



  for (x=46;x<49;x++){
    settingsarray[x]=65535;
  }//wrap

  for (x=54;x<64;x++){
    settingsarray[x]=32768;//>>15
  }//DIR

  settingsarray[49]=0; // STACKFUNC
  settingsarray[50]=8192; // STACKMUCH=8 <<10
  settingsarray[51]=0; //was EFFECTS // is now EXPANSION TODO!
  settingsarray[52]=0;
  settingsarray[53]=0;
	 
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
    //    cpustackpushhh(datagenbuffer,addr,randi()<<3,randi()%31,randi()%24);
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
	//	        printf("TESTY:%d\n",wrap);
  }


    // execution stack - TESTER!
    for (x=0;x<MAX_EXE_STACK;x++){
      exenums=exestackpush(exenums,exestack,randi()%4); //exetype=0-3 TESTY!
    }

    //exenums=exestackpop(exenums,exestack);


  start=0;

  ///////////////////////////

  u8 mastermode;
  u8 eff[4];
  u8 groupstart,groupwrap;
  u8 cpur=0,cpu=0;

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

      
	      for (x=0;x<exenums;x++){
		switch(exestack[x]%4){
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
      //// MODE/KNOB CODE START
      /////////////////////////////////////
      
      u8 xx;
      mastermode=adc_buffer[FIRST]>>7; // 32=5 bits

      eff[0]=adc_buffer[SECOND]>>5; // was 7 bits=128//jitter???
      eff[1]=adc_buffer[THIRD]>>5;
      eff[2]=adc_buffer[FOURTH]>>5;

      //***mode quadrant as 1-groups/2-finger/3-mirror(inc eeg)/4-attach=finger/process/detach/knob
      //***8 per quadrant=32 total      
      
      mastermode=30; // TESTY!!

      switch(mastermode){
      case 0: // GROUPS0=EFFMODE

	xx=fingerdir(); // 0-3 -change konb assign depends on fingers
	if (xx!=5){
#ifdef LACH
	EFFECTREAD=eff[xx%2];
	EFFECTWRITE=eff[(xx+1)%2];
	settingsarray[12]=adc_buffer[FOURTH]<<4;
#else
	EFFECTREAD=eff[xx%3];
	EFFECTWRITE=eff[(xx+1)%3];
	EFFECTFILTER=eff[(xx+2)%3];
#endif
	}
	  break;
      case 1:	//GROUPS1// preformed= X-ALL STEP Y-WRAP Z-SPEED
	if (fingerdirupdown()==0){
	  // STEP NOW!
	    for (x=25;x<35;x++){
	      settingsarray[x]=adc_buffer[SECOND]<<4; // 16 bit value
	    }

	    for (x=11;x<25;x++){
	      	      settingsarray[x]=adc_buffer[THIRD]<<4; // 16 bit value
	    }
	    for (x=35;x<41;x++){
	      	      settingsarray[x]=adc_buffer[FOURTH]<<4; // 16 bit value
	    }	   
	}
	    break;
      case 2: //GROUPS2// second group=R,W,F parameters
	if (fingerdirupdown()==0){
	  // READ - 2,5,13,19,27,30,37,44 - expand and contract these
	  settingsarray[2]=adc_buffer[SECOND]<<4;
	  settingsarray[5]=adc_buffer[SECOND]<<4;
	  settingsarray[13]=adc_buffer[SECOND]<<4;
	  settingsarray[19]=adc_buffer[SECOND]<<4;
	  settingsarray[27]=adc_buffer[SECOND]<<4;
	  settingsarray[30]=adc_buffer[SECOND]<<4;
	  settingsarray[37]=adc_buffer[SECOND]<<4;
	  settingsarray[44]=adc_buffer[SECOND]<<4;
	  
	  // WRITE - 1,4,12,16,18,26,29,36,43
	  settingsarray[1]=adc_buffer[THIRD]<<4;
	  settingsarray[4]=adc_buffer[THIRD]<<4;
	  settingsarray[12]=adc_buffer[THIRD]<<4;
	  settingsarray[16]=adc_buffer[THIRD]<<4;
	  settingsarray[18]=adc_buffer[THIRD]<<4;
	  settingsarray[26]=adc_buffer[THIRD]<<4;
	  settingsarray[29]=adc_buffer[THIRD]<<4;
	  settingsarray[36]=adc_buffer[THIRD]<<4;
	  settingsarray[43]=adc_buffer[THIRD]<<4;

	  // FILTER - 3,6,14,17,20,28,31,38,45
#ifdef LACH
	  settingsarray[12]=adc_buffer[FOURTH]<<4;
#else
	  settingsarray[3]=adc_buffer[FOURTH]<<4;
	  settingsarray[6]=adc_buffer[FOURTH]<<4;
	  settingsarray[14]=adc_buffer[FOURTH]<<4;
	  settingsarray[17]=adc_buffer[FOURTH]<<4;
	  settingsarray[20]=adc_buffer[FOURTH]<<4;
	  settingsarray[28]=adc_buffer[FOURTH]<<4;
	  settingsarray[31]=adc_buffer[FOURTH]<<4;
	  settingsarray[38]=adc_buffer[FOURTH]<<4;
	  settingsarray[45]=adc_buffer[FOURTH]<<4;
#endif
	}
	break;
      case 3: //GROUPS3: expand in this case (or contract?-fingers) R/W/F
	// R 52
	if (fingerdirupdown()==0){
	  settingsarray[52]=adc_buffer[SECOND]<<4;

	// W 51
	  settingsarray[51]=adc_buffer[THIRD]<<4;	  

	// F 53
	  settingsarray[53]=adc_buffer[FOURTH]<<4;
	}

	else if (fingerdirupdown()==1){
	  settingsarray[13]=adc_buffer[SECOND]<<4;

	// W 51
	  settingsarray[12]=adc_buffer[THIRD]<<4;	  

	// F 53
	  settingsarray[15]=adc_buffer[FOURTH]<<4;
	}



	break;
      case 4: //GROUPS4- HW settings (TODO: IFDEF)
	// START: 0 // lmerbase-8 // f0106erbase-9 // maximerbase-10 // hdgener=41
	// WRAP: 11 //lmercons-22 // 23            // 24             // 42
	// STEP: 25	// SPEED: 35
	if (fingerdirupdown()==0){
	// start/base
	  settingsarray[0]=adc_buffer[SECOND]<<4;
	  settingsarray[8]=adc_buffer[SECOND]<<4;
	  settingsarray[9]=adc_buffer[SECOND]<<4;
	  settingsarray[10]=adc_buffer[SECOND]<<4;
	  settingsarray[41]=adc_buffer[SECOND]<<4;

	// wraps/cons
	  settingsarray[11]=adc_buffer[THIRD]<<4;
	  settingsarray[22]=adc_buffer[THIRD]<<4;
	  settingsarray[23]=adc_buffer[THIRD]<<4;
	  settingsarray[24]=adc_buffer[THIRD]<<4;
	  settingsarray[42]=adc_buffer[THIRD]<<4;

	// step and speed
	  settingsarray[25]=adc_buffer[FOURTH]<<4;
	  settingsarray[35]=adc_buffer[FOURTH]<<4;
	}
	////
      case 5:
	/// access stacks and CPUs- UP and DOWN! just CPUs now!
	  /*
	    SIM: STACKER[STACK_SIZE*4]:16*4=48 start,wrap,howmuch,CPU
	    CA: STACKERY[STACK_SIZE*4]:16*4=48 start,wrap,howmuch,CPU
	    CPUINT:
	    cpur to max m->m_threadcount (max is 120=7 bits)
	    m->m_threads[cpur].m_CPU=cpu%31;
	  */
	if (fingerdirupdown()==0){
	//SIM/CA/CPU:
	  stacker[(((eff[0]>>1)%STACK_SIZE)*4)+3]++;
	  stackery[(((eff[1]>>1)%STACK_SIZE)*4)+3]++;
	  cpur=(eff[2])%(m->m_threadcount);
	  cpu++;
	  m->m_threads[cpur].m_CPU=cpu%31;
	}
      	else if (fingerdirupdown()==1){
	  //SIM/CA/CPU:
	  //if ((rand()%2)==1){
	  stacker[(((eff[0]>>1)%STACK_SIZE)*4)+3]--;
	  stackery[(((eff[1]>>1)%STACK_SIZE)*4)+3]--;
	  cpur=(eff[2])%(m->m_threadcount);
	  cpu--;
	  m->m_threads[cpur].m_CPU=cpu%31;
	}

	break;
	////
      case 6: //GROUPS5	// X and Y as chunk start/wrap	// Z as knob
	// total 64
	if (fingerdirupdown()==0){
	  groupstart=eff[0]>>1; // 6bits
	groupwrap=eff[1]>>1;
	for (x=0;x<groupwrap;x++){
	  settingsarray[(groupstart+x)%64]=adc_buffer[FOURTH]<<4;
	}
	}
	break;
      case 7: //GROUPS6 // algo group1: X,Y,Z
	if (fingerdirupdown()==0){
	  groupstart=datagenbuffer[adc_buffer[SECOND]<<4]>>2;// 16 bits//6 bits
	  groupwrap=datagenbuffer[adc_buffer[THIRD]<<4]>>2;// 16 bits//6 bits
	for (x=0;x<groupwrap;x++){
	  settingsarray[(groupstart+x)%64]=adc_buffer[FOURTH]<<4;
	}
	}
	break;
	///////////////////////////////////////////////////////////////////////////

	// FINGERS
	// 8-15
	// and what do 3 knobs set? TODO!!!

      case 8: //FINGERS0 // set directions 54-63 left/right UP/DOWN to set
	directionxx=fingervalupwrap(directionxx,10);
	xx=fingerdirleftright();
	if (xx!=2) settingsarray[54+directionxx]=xx<<15;
	break;

      case 9: //FINGERS1 // in code: left/right=navigate stacker,stackery and CPUs up/down inc/dec
	//	48+48+120=216
	codepos=fingervalright(codepos,216); // TODO is stacker fixed as so=216!!! TESTY!
	if (codepos<48){
	  stacker[codepos]+=fingerdirupdownn(); 
	  stacker[codepos]%=32768;
	}
	else if (codepos<96){
	  stackery[codepos-48]+=fingerdirupdownn();
	  stackery[codepos-48]%=32768;
	}
	  else {
	  cpur=(codepos-96)%(m->m_threadcount);
	  cpu+=fingerdirupdownn();
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	break;

      case 10: //FINGERS2 // in villagers
	villagepos=fingervalright(villagepos,VILLAGE_SIZE);
	villager[villagepos]+=(fingerdirupdownn()*2);
	villager[villagepos]%=32768;
	break;

      case 11: //FINGERS3 // into settings
	settingspos=fingervalright(settingspos,64);
	settingsarray[settingspos]+=(fingerdirupdownn()*2);
	break;

      case 12: //FINGERS4 // into stacks=push and pop = EXE,CA,SIM,CPU,PURE,VILLAGER
	// HERE if we wanted to knobs could set START,WRAP and FUNC! TO TESTY!!!

	// START=7/WRAP=21/FUNC=49
	settingsarray[7]=adc_buffer[SECOND]<<4;
	settingsarray[21]=adc_buffer[THIRD]<<4;
	settingsarray[49]=eff[2]<<9;

		whichpushpop=fingervalright(whichpushpop,8);
	//	whichpushpop=7;
	if (fingerdirupdown()==0){//THIS IS UP!!!!!
	  // push which
	  switch(whichpushpop){
	  case 0:
	    cpustackpush(m,datagenbuffer,STACKSTART,STACKWRAP,STACKFUNC%31,STACKMUCH);
	    break;
	  case 1:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,audio_buf,stack_posy,STACKMUCH,STACKSTART,STACKWRAP);	
	    break;
	  case 2:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,datagenbuffer,stack_posy,STACKMUCH,STACKSTART,STACKWRAP);	    
	    break;
	  case 3:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,buf16,stack_pos,STACKMUCH,STACKSTART,STACKWRAP);
	    break;
	  case 4:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,audio_buffer,stack_pos,STACKMUCH,STACKSTART,STACKWRAP);
	    break;
	  case 5:
	    villagestackpos=villagepush(villagestackpos,STACKSTART,STACKWRAP,STACKFUNC%16);//pos/start/wrap
	    break;
	  case 6:
	    cpustackpushhh(datagenbuffer,STACKSTART,STACKWRAP,STACKFUNC%31,STACKMUCH);
	    break;
	  case 7:
	    exenums=exestackpush(exenums,exestack,STACKFUNC%4); //exetype=0-3;
	  }
      }
      else if (fingerdirupdown()==1){
	// pop which
	  switch(whichpushpop){
	  case 0:
	    //pop CPU
	    cpustackpop(m);
	    break;
	  case 1:
	  case 2:
	    //pop CA
	    stack_posy=ca_pop(stack_posy);
	    break;
	  case 3:
	  case 4:
	    //pop SIM
	    stack_pos=func_pop(stack_pos);
	    break;
	  case 5:
	    //pop village
	    villagestackpos=villagepop(villagestackpos);
	    break;
	  case 6:
	    // pop pureleak
	    cpustackpoppp(datagenbuffer);
	      break;
	  case 7:
	    exenums=exestackpop(exenums,exestack);
	  }
      }
	break;

	//13/14/15=bare into CODE, VILLAGERS, SETTINGS (navigate and CHANGE SETTING)

      case 13:
	codepos=fingervalright(codepos,216); // TODO is stacker fixed as so=216!!! TESTY!
	if (codepos<48){
	  stacker[codepos]=(adc_buffer[DOWN]<<3);
	}
	else if (codepos<96){
	  stackery[codepos-48]=(adc_buffer[DOWN]<<3);
	}
	  else {
	  cpur=(codepos-96)%(m->m_threadcount);
	  cpu=(adc_buffer[DOWN]>>7)%31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	break;

      case 14:
	villagepos=fingervalright(villagepos,VILLAGE_SIZE);
	villager[villagepos]=(adc_buffer[DOWN]<<3);
	// TEST on wrap which is 12
	//	settingsarray[12]=fingervalright(settingsarray[12],254);
	break;

      case 15:
	settingspos=fingervalright(settingspos,64);
	settingsarray[settingspos]=(adc_buffer[DOWN]<<4);
	break;


	///////////////////////////////////////////////////////////////////////////
	// MIRROR
	// 16-23
	// 8 mirror modes
	// 3 knobs=//region//mirror walkers??? CHECK discard
	// mirror process set/unset by fingers
	// modes: mirror into 1/settingsarray 2/villager 3/stacks/CPU 4/foldback
	// mirror operations: buf16 walker(set it), EEG, where are swaps(or swaps above)
	// knob1/2/3: mirror walkers/settings
	///////////////////////////////////////////////////////////////////////////

      case 16: 
	// set FOLD 0,1,2
	FOLDD[0]=eff[0]; // all 7 bits TODO
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){ //was >>9
	  settingsarray[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>10)+1)))%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>1)+1)))%32768];
	  coo++;
	}
	// toggle flag with the finger
	if (fingerdirupdown()==0) m1flag|=1; //sets
	else if (fingerdirupdown()==1) m1flag&=~1;
	break;

      case 17: 
	// set FOLD 0,1,2
	FOLDD[0]=eff[0];
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){ //was >>9
	  settingsarray[(((FOLDD[1])>>1)+(x%((FOLDD[2]>>1)+1)))%64]=(randi()<<4);
	  coo++;
	}
	// toggle flag with the finger
	if (fingerdirupdown()==0) m1flag|=2; //sets
	else if (fingerdirupdown()==1) m1flag&=~2; 
	break;

      case 18:
	// set FOLD 0,1,2
	FOLDD[0]=eff[0];
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  villager[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>1)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>1)+1)))%32768]>>1;
	  coo++;
	}
	if (fingerdirupdown()==0) m1flag|=4; //sets
	else if (fingerdirupdown()==1) m1flag&=~4; 
	break;

      case 19:
	// set FOLD 0,1,2
	FOLDD[0]=eff[0];
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  villager[(((FOLDD[1])>>1)+(x%((FOLDD[2]>>1)+1)))% VILLAGE_SIZE]=(randi()<<3);
	    coo++;
	}

	if (fingerdirupdown()==0) m1flag|=8; //sets
	else if (fingerdirupdown()==1) m1flag&=~8; 
	break;

      case 20: // STACKS and CPU
	// set FOLD 0,1,2
	FOLDD[0]=adc_buffer[SECOND]<<4;
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=adc_buffer[FOURTH]<<4;
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[1])>>8)+(x%((FOLDD[2]>>8)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stacker[tmper]%=32768;
	}
	else if (tmper<96){
	  stackery[tmper-48]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stackery[tmper-48]%=32768;
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
	if (fingerdirupdown()==0) m1flag|=16; //sets
	else if (fingerdirupdown()==1) m1flag&=~16; 
	break;

      case 21: // STACKS and CPU
	// set FOLD 0,1,2
	FOLDD[0]=adc_buffer[SECOND]<<4;
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=adc_buffer[FOURTH]<<4;
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[1])>>8)+(x%((FOLDD[2]>>10)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=(randi()<<3);
	  stacker[tmper]%=32768;
	}
	else if (tmper<96){
	  stackery[tmper-48]=(randi()<<3);
	  stackery[tmper-48]%=32768;
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=randi()>>7;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
	if (fingerdirupdown()==0) m1flag|=32; //sets
	else if (fingerdirupdown()==1) m1flag&=~32; 
	break;

      case 22:
	// set FOLD 0,1,2 ---> FOLDBACK!
	FOLDD[0]=eff[0];
	FOLDD[1]=adc_buffer[THIRD];
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  FOLDD[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>1)+1)))% FOLD_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>1)+1)))%32768]>>1;
	  	  coo++;
	}
	if (fingerdirupdown()==0) m1flag|=64; //sets
	else if (fingerdirupdown()==1) m1flag&=~64; 
	break;

      case 23:
	// set FOLD 0,1,2 ---> FOLDBACK!
	FOLDD[0]=eff[0];
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  FOLDD[(((FOLDD[1])>>1)+(x%((FOLDD[2]>>1)+1)))% FOLD_SIZE]=randi()<<4;
	  	  coo++;
	}
	if (fingerdirupdown()==0) m1flag|=128; //sets
	else if (fingerdirupdown()==1) m1flag&=~128; 
	break;

	///////////////////////////////////////////////////////////////////////////
	// MIRROR_MIRROR//swop=mirror region to region of...???
	// 
	// 1/settingsarray 2/villager 3/stacksandCPU 4/foldback
	//24-1-1//25-2-2//26-3-3//27-4-4/28-2-3(start<->end only)//29-3->2
	//toggle swop or not?
	// knobs=start/wrap/offset to copy+
      case 24:
	// settingsarray to itself
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[1]>>1)+1);x++){
	  settingsarray[(((FOLDD[0])>>1)+x)%64]=settingsarray[((FOLDD[2]>>1)+((FOLDD[1])>>1)+x)%64];
	}
	break;

      case 25:
	// villagerarray to itself
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[1]>>1)+1);x++){ //was >>9
	  //	  settingsarray[(((FOLDD[0])>>10)+x)%64]=settingsarray[FOLDD[2]+(((FOLDD[1])>>10)+x)%64];
	  villager[(((FOLDD[0])>>1)+x)%VILLAGE_SIZE]=villager[((FOLDD[2]>>1)+((FOLDD[0])>>1)+x)%VILLAGE_SIZE];
	}
	break;

      case 26:
	// stacks and CPUs - how to deal with crossover (also missmatch of values?)??? TODO!
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=adc_buffer[THIRD];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[1]>>4)+1);x++){
	}
	break;

      case 27:
	// foldback
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[1]>>1)+1);x++){ 
	  FOLDD[(((FOLDD[0])>>1)+x)% FOLD_SIZE]=FOLDD[((FOLDD[2]>>1)+((FOLDD[0])>>1)+x)% FOLD_SIZE];
	}
	break;

      case 28:
	// starts and ends only of stacks (not CPU) -> villagers
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	//	settingsarray[12]=128; // TESTY!!!!
	for (x=0;x<((FOLDD[1]>>1)+1);x++){ 
	  tmper=((FOLDD[0])+x)%(STACK_SIZE*2); // so both stacks entry point
	  villagerdest=(((FOLDD[0])>>1)+x+(FOLDD[2]>>1))%(VILLAGE_SIZE/2); // village entry
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

      case 29:
	// starts and ends only of villagers -> stacks
	// starts and ends only of stacks (not CPU) -> villagers
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	//	settingsarray[12]=128; // TESTY!!!!
	for (x=0;x<((FOLDD[1]>>1)+1);x++){ 
	  tmper=((FOLDD[0])+x)%(STACK_SIZE*2); // so both stacks entry point
	  villagerdest=(((FOLDD[0])>>1)+x+(FOLDD[2]>>1))%(VILLAGE_SIZE/2); // village entry
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

	//30->into settingsarray
	// UPDOWNLEFTRIGHT=1-process-buf16int/2-fingerbare/3-knob/4-detach
	// knob1,2=group settings/extent
	// knob3=setting
	// TODO: to toggle

      case 30:
	groupstart=eff[0]>>1; // 6bits
	groupwrap=eff[1]>>1;

	for (x=0;x<groupwrap;x++){
	switch(fingerdir()){
	case 0:
	  settingsarray[(groupstart+x)%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  settingsarrayattached[(groupstart+x)%64]=1;
	  coo++;
	  break;
	case 2:
	  settingsarray[(groupstart+x)%64]=adc_buffer[DOWN]<<4;
	  settingsarrayattached[(groupstart+x)%64]=2;
	  break;
	case 3:
	  settingsarray[(groupstart+x)%64]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[(groupstart+x)%64]=3;
	  break;
	case 4:
	  // DEtach
	  settingsarrayattached[(groupstart+x)%64]=0;
	  break;
	}
	}
	break;

      case 31:
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability
	if (eff[0]==0){
	  for (x=0;x<64;x++){
	    if ((rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1; // infected
	  else settingsarrayinfected[x]=0;
	  }
	  // run infection at speed eff[2] 

	  for (x=0;x<64;x++){
	    // infection - how many infected (not dead) round each one?
	    if (++del==eff[0]){
	      if (settingsarrayinfected[x]==0 && ((settingsarrayinfected[(x-1)%64]>=1 && settingsarrayinfected[x-1]<128) || (settingsarrayinfected[(x+1)%64]>=1 && settingsarrayinfected[x+1]<128)) && (rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1;
	    // inc
	    if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128) settingsarrayinfected[x]++;
	    }

	  // overmap onto buffer eff[0]: 0=stay same/infect=reduce by days/dead=128=zero
	    //0/settingsarray 1/villager 2/3/4//stacksandCPU 5/foldback	    
	    switch(eff[2]>>4) // 8 cases
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
		  cpur=x%(m->m_threadcount);
		  m->m_threads[cpur].m_CPU=0;//cpu
		}
		break;
	      case 5:
		if (x<14){
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	FOLDD[x]-=settingsarrayinfected[x]; // foldd max
		else if (settingsarrayinfected[x]>127) FOLDD[x]=0; // foldd max
		}
		break;
	      }
	    /////
	  }
	}
	break;
      }

      //// DEAL WITH toggled MIRRORING

      if (m1flag&1){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  settingsarray[(((FOLDD[3])>>10)+(x%((FOLDD[4]>>10)+1)))%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  coo++;
	}
      }

      if (m1flag&2){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  settingsarray[(((FOLDD[4])>>10)+(x%((FOLDD[5]>>10)+1)))%64]=(randi()<<4);
	  coo++;
	}
      }

      if (m1flag&4){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  villager[(((FOLDD[6])>>10)+(x%((FOLDD[7]>>10)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768]>>1;
	  coo++;
	}
      }

      if (m1flag&8){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  villager[(((FOLDD[8])>>10)+(x%((FOLDD[9]>>10)+1)))% VILLAGE_SIZE]=(randi()<<3);
	  coo++;
	}
      }

      if (m1flag&16){ 
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[10])>>8)+(x%((FOLDD[11]>>8)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stacker[tmper]%=32768;
	}
	else if (tmper<96){
	  stackery[tmper-48]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stackery[tmper-48]%=32768;
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
      }

      if (m1flag&32){ 
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[10])>>8)+(x%((FOLDD[11]>>8)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=(randi()<<3);
	}
	else if (tmper<96){
	  stackery[tmper-48]=(randi()<<3);
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=randi()>>7;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
      }

      if (m1flag&64){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  FOLDD[(((FOLDD[11])>>10)+(x%((FOLDD[12]>>10)+1)))% FOLD_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768]>>1;
	  	  coo++;
	}
      }

      if (m1flag&128){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  FOLDD[(((FOLDD[12])>>10)+(x%((FOLDD[13]>>10)+1)))% FOLD_SIZE]=randi()<<4;
	  	  coo++;
	}
      }

      ///////////////////////////////
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
	  settingsarray[x]=adc_buffer[FOURTH]<<4;
	  break;
	}
      }

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
	  tmp=(HWSTART+(hwpos%HWWRAP))%32768; //to cover all directions
	  hwdel=0;
	  hwposss+=(HWSTEP*direction[HWDIR]);
	  tmppp=(HWSTART+(hwposss%HWWRAP))%32768; //to cover all directions
      }

      if (digfilterflag&16){
       	hdtmp=(HWSTART+(hwpos%HWWRAP)); 
	if (HDGENERCONS==0) settingsarray[42]=256; //(settingsarray[42]>>8)
	dohardwareswitch(HARDWARE,HDGENERBASE+(datagenbuffer[hdtmp]%HDGENERCONS));
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
		     		     
      //      if (digfilterflag&2){
            set40106pwm(F0106ERBASE+(buf16[tmp]%F0106ERCONS)); // constrain all to base+constraint
      //      set40106pwm(32768);
	  //      }
	  

      if (digfilterflag&4){
	setlmpwm(LMERBASE+(buf16[tmppp]%LMERCONS),LMERBASE+(buf16[tmp]%LMERCONS)); 
      }
	  
      if (digfilterflag&8){
		  setmaximpwm(MAXIMERBASE+(buf16[tmp]%MAXIMERCONS));
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
