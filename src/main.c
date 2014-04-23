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

int16_t newdir[4]={-180,1,180,-1};
int16_t direction[4]={-180,1,180,-1};
int16_t newdirf[4]={-180,1,180,-1};
int16_t directionf[4]={-180,1,180,-1};
int16_t newdirread[4]={-180,1,180,-1};
int16_t directionread[4]={-180,1,180,-1};

#define delay()						 do {	\
    register unsigned int i;					\
    for (i = 0; i < 1000000; ++i)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)


#define randi() (datagenbuffer[adc_buffer[9]<<3]) // 15 bits

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

extern int16_t audio_buffer[AUDIO_BUFSZ];

u8* datagenbuffer = (u8*)0x10000000;

int16_t newdir[4];
int16_t direction[4];
int16_t newdirf[4];
int16_t directionf[4];
int16_t newdirread[4];
int16_t directionread[4];

extern u8 digfilterflag;

u8 testdirection;
u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array
u16 settingsarray[64];

struct dgenwalker{
  u8 step,dir,speed,del,wormflag;
  u16 pos;
};


u8 fingerdir(void){

  // TODO: cycle here or run through as statics????
  u8 handleft, handright, up=0,down=0,left=0,right=0,handcount=0;
  u8 handupp, handdown;
  static u8 result;

  for (u8 x=0;x<16;x++){
    // could re-org for lower board TODO!
  handupp=adc_buffer[6]>>8; 
  handdown=adc_buffer[8]>>8;
  handleft=adc_buffer[5]>>8;
  handright=adc_buffer[7]>>8;
  if (handupp>8) up++; //TODO: TWEAKING but seems okay...
  if (handdown>8) down++;
  if (handleft>8) left++;
  if (handright>8) right++;
  //  handcount++;
  if (up>8 && up>down && up>left && up>right) {
    // up=0;down=0;left=0;right=0;
    result=0;
  }
  else if (down>8 && down>left && down>right) {
    //    up=0;down=0;left=0;right=0;
    result=2; 
      //int16_t hddir[4]={-180,1,180,-1};
  }
  else if (left>8 && left>right) {
    //    up=0;down=0;left=0;right=0;
    result=3;
  }
  else if (right>8) {
    //    up=0;down=0;left=0;right=0;
    result=1;
  }
  }
  return result;
  /*  if (handcount>9){
    handcount=0;up=0;down=0;left=0;right=0;
    }*/
}

u16 fingerval(void){
  static u8 oldhandup=0,handup,handdown,oldhanddown=0;
  static u8 ttss=0,sstt=0;
  static u8 tmpsetting=0;
  // could re-org 6/8 as 5/6 for lower board TODO!

  handup=adc_buffer[6]>>8;
  if (handup>oldhandup) sstt++;// else sstt=0;

  if (sstt>64){ // TODO: tune this figure or have as parameter but in principle works
    sstt=0;
    tmpsetting++;
  }

  oldhandup=handup;

  handdown=adc_buffer[8]>>8;
  if (handdown>oldhanddown) ttss++;//  else ttss=0;
  if (ttss>64){
    ttss=0;
    tmpsetting--;
    }
    oldhanddown=handdown;
  return tmpsetting;
}

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 *buf; u8 *buff;
  u16 x,addr,tmp;
  u8 hardware=0; u16 tmphardware;
  u8 oldhardware,tmppushpull,pushpull=0,Ysettings,tmpspeed,speed=0,tmpsettings,micromacro=0,foldback=0,which=0;
  u8 effects=0;
  u16 speedwrapper=0;
  u8 machine_count=0,leak_count=0; 
  u8 settings=0, oldsettings=0,settings_trap=0,setted=0,pushsetted=0,settingsindex=0;
  u8 pushypop,pushpopflag=0;
  u8 index=0, finaldel=0;
  u16 finalpos=0;
  u8 exeperms[88]={0,1,2,3, 0,1,3,2, 0,2,3,1 ,0,2,1,3, 0,3,1,2, 0,3,2,1, 1,0,2,3, 1,0,3,2, 1,2,3,0, 1,2,0,3, 1,3,2,0, 1,3,0,2, 2,1,0,3, 2,1,3,0, 2,3,1,0, 2,3,0,1, 3,0,1,2, 3,0,2,1, 3,1,0,2, 3,1,2,0, 3,2,0,1, 3,2,1,0}; u8 exespot=0;

  u16 pushsetting[4]={0,0,32767,10}; 

  int16_t hddir[4]={-180,1,180,-1};
  int16_t lmdir[4]={-180,1,180,-1};
  int16_t mxdir[4]={-180,1,180,-1};
  int16_t dir40106[4]={-180,1,180,-1};
  u16 tempsetting=0;
  u8 mirror;

  inittable(3,4,randi(),table);

  float pi= 3.141592;
  float w;
  float yi;
  float phase;
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

  machine *m=(machine *)malloc(sizeof(machine));
  m->m_threadcount=0;
  m->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED

  u8 hdgenerdel=0,lmerdel=0,f0106erdel=0,maximerdel=0;
  u16 hdgenerpos=0,lmerpos=0,f0106erpos=0,maximerpos=0,wrapper;
	 
  u8 stack_pos=0;
  u8 stack_posy=0;

  struct stackey stackyy[STACK_SIZE];
  struct stackey stackyyy[STACK_SIZE];
  u16 *buf16 = (u16*) datagenbuffer;
  u8 *audio_buf = (u8*) audio_buffer;

#ifndef LACH
  dohardwareswitch(2,0);
#endif

  ////////////////////TESTCODE_TODO_REMOVE - but do replace with some
  ////////////////////minimal setup code to get started
  //TESTER!

  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0; // TESTY!

  // setup code for walkers
  for (x=0;x<10;x++){
    settingsarray[x]=0;
  }//start

  for (x=10;x<20;x++){
    settingsarray[x]=32767;
  }//wrap

  for (x=20;x<30;x++){
    settingsarray[x]=1;
  }//step

  for (x=30;x<37;x++){
    settingsarray[x]=1;
  }//speed

  for (x=37;x<48;x++){
    settingsarray[x]=1;
  }//DIR

  HDGENERBASE=0;
  HDGENERCONS=255;
  LMERBASE=0;
  LMERCONS=32768; // should be TODO 65536????
  F0106ERBASE=0; 
  F0106ERCONS=32768;
  MAXIMERBASE=0;
  MAXIMERCONS=32768;
  LEAKSPEED=1;MACHINESPEED=1;
	 
  // CPUintrev2:
  for (x=0; x<10; x++) // was 100
    {
      addr=randi();
      cpustackpush(m,datagenbuffer,addr,addr+randi(),randi()%31,1);//randi()%255);
    }

  //pureleak

  for (x=0;x<MAX_FRED;x++){
    addr=randi()%65536;
    cpustackpushhh(datagenbuffer,addr,addr+randi(),randi()%31,1);
  }

  // CA
  for (x=0;x<STACK_SIZE;x++){
    stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,100,0,randi()%32768);//howmuch,start,wrap 
  }

  //simulationforstack:	
  for (x=0;x<STACK_SIZE;x++){
    stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,10,0,randi()%32768);//howmuch,start,wrap 
    //	   stack_pos=func_pushn(stackyy,28,buf16,stack_pos,10,0,32767);//howmuch,start,wrap
  }

  ///////////////////////////

  //  SAMPLEWRAP=32767;SAMPLEWRAPREAD=32767;

  while(1)
    {

      //      SAMPLEDIRW=fingerdir();  //TESTY!
      //      SAMPLEDIRW=2;
      //testdirection=0;
#ifdef TEST_STRAIGHT
      // nothing???
#else

      if (MACHINESPEED==0) MACHINESPEED=1;
      if (LEAKSPEED==0) LEAKSPEED=1;

      for (x=0;x<4;x++){
	switch(exeperms[((exespot%22)*4)+x]){
	case 0:
	  func_runall(stackyy,stack_pos); // simulations
	  break;
	case 1:
	  ca_runall(stackyyy,stack_posy); // CA
	  break;
	case 2:
	  machine_count++;
	  if (machine_count>=MACHINESPEED){
	    machine_run(m);
	    m->m_leakiness=LEAKINESS;
	    m->m_infectprob=INFECTION;
	    m->m_mutateprob=MUTATION;
	    machine_count=0;
	  }
	  break;
	case 3:
	  leak_count++;
	  if (leak_count>=LEAKSPEED){
	    machine_runnn(datagenbuffer); // TODO: swap here - in 2 below
	    leak_count=0;
	  }
	}
      }
      /////////////////////////////
      // KKNOBBBSSS TODO!!!

      // TENE: 2,0,3,4,1 

      // revise as settings//select ops on related settings (only when changes?)
      // also maintain oldsetting on left side  	     
	    
      //0-mirror -- mirror of mirror  of mirror!!
      //1-hardware //+// walk stacks into hardware params//data///swop datagen/audio 
      //2-stacks/push/pull //inc swap data/audio for cpu ops//exespot
      //3-speed/micro-macro combined //+// walk stacks into sample params//data
      //4-all settings up/down

      // finger tests
      //      hardware=fingerval();//>>9; // 16 bits ideally to 7
      hardware=adc_buffer[2]>>5;
      /// use this smoothing for hardware(only?)
      /*      tmphardware=0;
      for (x=0;x<256;x++){
	tmphardware+=adc_buffer[0]>>5; // 7 bits
      }
      tmphardware=tmphardware>>8;*/
      //      u8 hh; u8 hard[64];u16 thardware;
      // moving average- write into circular buffer say 64
      /*hard[hh%64]=adc_buffer[0]>>5;
	hh++;
	for (x=0;x<64;x++){thardware+=hard[x];}
	tmphardware=thardware>>8; // divide by 64
	thardware=0;*/

       // this mirroring kind of works!
      /*            mirror=adc_buffer[2]>>4; // 8 bits or less?
      if (mirror<128 && tmphardware!=effects && tmphardware!=effects-1 && tmphardware!=effects+1){
	hardware=tmphardware; // handled all below 
	    }
	      else if (mirror >128 && tmphardware!=hardware && tmphardware!=hardware-1 && tmphardware!=hardware+1 && tmphardware!=effects){
		effects=tmphardware;
		if (mirror<160){
		  EFFECTREAD=effects;
		}
		else if (mirror<192){
		  EFFECTWRITE=effects;
		}
		else if (mirror<224){
		  EFFECTFILTER=effects;
		}
		else if (mirror<240){
		  EFFECTREAD=EFFECTWRITE;EFFECTFILTER=EFFECTWRITE;
		}
		else  {
		  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;
		}
		}*/
      	
    
      ////////////////////////////////////////////////
#ifndef LACH
      /////////////////////////////////////
      // 4-hardware operations
      // do hardware datagen walk into hdgen (8 bit) if flagged
      if (digfilterflag&16){ // if we use hdgen at all
	if (HDGENERSPEED==0) HDGENERSPEED=1;
	if (HDGENERSTEP==0) HDGENERSTEP=1;
	if (++hdgenerdel==HDGENERSPEED){
	  hdgenerpos+=(HDGENERSTEP*hddir[HDGENERDIR]);
	  wrapper=HDGENERWRAP; // can go 65536
	  if (wrapper==0) wrapper=1;
	  tmp=(HDGENERSTART+(hdgenerpos%wrapper))%32768; //to cover all directions
	  dohardwareswitch(hardware,HDGENERBASE+(datagenbuffer[tmp]%HDGENERCONS));
	  hdgenerdel=0;
	}

      }
      else
	{
	  if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	  oldhardware=hardware;
	}
	     		   
      // 3 datagenclocks->40106/lm/maxim - filterflag as bits as we also need signal which clocks we		     		     
      if (digfilterflag&2){
	if (F0106ERSPEED==0) F0106ERSPEED=1;
	if (F0106ERSTEP==0) F0106ERSTEP=1;
	if (++f0106erdel==F0106ERSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  f0106erpos+=(F0106ERSTEP*dir40106[F0106ERDIR]);
	  wrapper=F0106ERWRAP;
	  if (wrapper==0) wrapper=1;
	  tmp=(F0106ERSTART+(f0106erpos%wrapper))%32768; //to cover all directions
	  set40106pwm(F0106ERBASE+(buf16[tmp]%F0106ERCONS)); // constrain all to base+constraint

	  f0106erdel=0;
	}
      }
	  
      if (digfilterflag&4){
	if (LMERSPEED==0) LMERSPEED=1;
	if (LMERSTEP==0) LMERSTEP=1;
	if (++lmerdel==LMERSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  lmerpos+=(LMERSTEP*lmdir[LMERDIR]);
	  wrapper=LMERWRAP;
	  if (wrapper==0) wrapper=1;
	  x=(LMERSTART+(lmerpos%wrapper))%32768; //to cover all directions

	  lmerpos+=(LMERSTEP*lmdir[LMERDIR]);
	  wrapper=LMERWRAP;
	  if (wrapper==0) wrapper=1;
	  tmp=(LMERSTART+(lmerpos%wrapper))%32768; //to cover all directions
	  setlmpwm(LMERBASE+(buf16[x]%LMERCONS),LMERBASE+(buf16[tmp]%LMERCONS)); 
	  lmerdel=0;
	}

      }
	  
      if (digfilterflag&8){
	if (MAXIMERSPEED==0) MAXIMERSPEED=1;
	if (MAXIMERSTEP==0) MAXIMERSTEP=1;
	if (++maximerdel==MAXIMERSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  maximerpos+=(MAXIMERSTEP*mxdir[MAXIMERDIR]);
	  wrapper=MAXIMERWRAP;
	  if (wrapper==0) wrapper=1;
	  x=(MAXIMERSTART+(maximerpos%wrapper))%32768; //to cover all directions
	  maximerdel=0;
	  setmaximpwm(MAXIMERBASE+(buf16[x]%MAXIMERCONS));
	}
      }
#endif
#endif
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
