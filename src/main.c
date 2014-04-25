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

// for knobwork
// TENE: 2,0,3,4,1 
// else: 3,0,2,4,1

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

int16_t newdir[4]={-180,1,180,-1};
int16_t direction[4]={-180,1,180,-1};
int16_t villagedirection[4]={-180,1,180,-1};
int16_t villagedirectionf[4]={-180,1,180,-1};
int16_t villagedirectionw[4]={-180,1,180,-1};
int16_t newdirf[4]={-180,1,180,-1};
int16_t directionf[4]={-180,1,180,-1};
int16_t newdirread[4]={-180,1,180,-1};
int16_t directionread[4]={-180,1,180,-1};

u8 villagestackpos=0;
u16 villager[129];

extern u16 stackery[64]; // 16*4 MAX
extern u16 stacker[64]; // 16*4 MAX

#define delay()						 do {	\
    register unsigned int i;					\
    for (i = 0; i < 1000000; ++i)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int i;					\
    for (i = 0; i < 1000; ++i)				\
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

#define VILLAGE_SIZE 128 // 64 *2

u8 villagepush(u8 villagepos, u16 start, u16 wrap){
  if (villagepos<VILLAGE_SIZE)
    {
      villager[villagepos]=start;
      villager[villagepos+1]=wrap;
      villagepos+=2;
}
  return villagepos;
}

u8 villagepop(u8 villagepos){
  if (villagepos>2)
    {
      villagepos-=2;
    }
  return villagepos;
}

u8 fingerdir(void){

  u8 handleft, handright, up=0,down=0,left=0,right=0,handcount=0;
  u8 handupp, handdown;
  static u8 result;

  for (u8 x=0;x<16;x++){
  handupp=adc_buffer[UP]>>8; 
  handdown=adc_buffer[DOWN]>>8;
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
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

u16 fingervalup(u16 tmpsetting, u8 inc){
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

u16 fingervalright(u16 tmpsetting, u8 inc){
  u8 handup,handdown;
  u8 ttss=0,sstt=0;u8 x;

  for (x=0;x<16;x++){
  handup=adc_buffer[RIGHT]>>8;
  handdown=adc_buffer[LEFT]>>8;
  if (handup>8) ttss++;
  else if (handdown>8) sstt++;
  }
  if (ttss>sstt) tmpsetting+=inc;
  else if (ttss<sstt) tmpsetting-=inc;
  return tmpsetting;
}


void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 *buf; u8 *buff;
  u16 x,addr,tmp;
  u8 tmper;
  u16 hdtmp;
  u8 hardware=0, thardware=200; u16 tmphardware, HWSPEEDY, HDGENERCONSY;
  u8 oldhardware;
  u8 effects=200;
  u8 machine_count=0,leak_count=0; 
  u8 tmpsettings, settings=0, settingsops=200;
  u16 setted;
  u8 tmpstack, stack, stackops;
  u8 exeperms[88]={0,1,2,3, 0,1,3,2, 0,2,3,1 ,0,2,1,3, 0,3,1,2, 0,3,2,1, 1,0,2,3, 1,0,3,2, 1,2,3,0, 1,2,0,3, 1,3,2,0, 1,3,0,2, 2,1,0,3, 2,1,3,0, 2,3,1,0, 2,3,0,1, 3,0,1,2, 3,0,2,1, 3,1,0,2, 3,1,2,0, 3,2,0,1, 3,2,1,0}; 

  int16_t hwdir[4]={-180,1,180,-1};
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

  u8 hwdel=0;
  u16 hwpos=0,wrapper;
	 
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

  //   EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0; // TESTY!

  villagestackpos=0;
  // TESTY for villager:
for (x=0;x<1;x++){
  //  villager[x][0]=0;villager[x][1]=32767;
  villagestackpos=villagepush(villagestackpos,0,32767);//pos/start/wrap
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

  for (x=54;x<64;x++){
    settingsarray[x]=16384;//>>14
  }//DIR

  settingsarray[51]=0; //EFFECTS
  settingsarray[52]=0;
  settingsarray[53]=0;
	 
  // CPUintrev2:
  for (x=0; x<1; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,datagenbuffer,addr,addr+randi(),randi()%31,1);//randi()%255);
    }

  //pureleak

  for (x=0;x<1;x++){
    addr=randi()<<3;
        cpustackpushhh(datagenbuffer,addr,addr+randi(),randi()%31,1);
  }

  // CA
  for (x=0;x<STACK_SIZE;x++){
    stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,100,0,randi()<<3);//howmuch,start,wrap 
  }

  //simulationforstack:	
  for (x=0;x<STACK_SIZE;x++){
    stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,10,0,randi()<<3);//howmuch,start,wrap 
  }

  ///////////////////////////

  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

      for (x=0;x<4;x++){
	switch(exeperms[((EXESPOT%22)*4)+x]){
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
	    //	    machine_runnn(datagenbuffer);
	    leak_count=0;
	  }
	}
      }
      /////////////////////////////
      // KKNOBBBSSS TODO!!!

      /// MIRROR!
      mirror=adc_buffer[FIRST]>>4; // 8 bits or less?

#ifdef TENE
      /// HARDWARE SMOOTHING!
            tmphardware=0; //TESTY!uncomment!
      for (x=0;x<256;x++){
	tmphardware+=adc_buffer[SECOND]>>5; // 7 bits
      }
      tmphardware=tmphardware>>8; //average

      if (mirror<128 && tmphardware!=effects && tmphardware!=effects-1 && tmphardware!=effects+1){
	hardware=tmphardware; // handled all below 
	thardware=tmphardware;
	effects=200; // never near but set to avoid gap...
	    }
	      else if (mirror >128 && tmphardware!=thardware && tmphardware!=thardware-1 && tmphardware!=thardware+1 && tmphardware!=effects){
		effects=tmphardware; //7 bits
		thardware=200;
		if (mirror<160){
		  settingsarray[51]=effects<<9; // EFFECTREAD
		}
		else if (mirror<192){
		  settingsarray[52]=effects<<9; // EFFECTWRITE
		}
		else if (mirror<224){
		  settingsarray[53]=effects<<9; // EFFECTFILT
		}
		else if (mirror<240){
		  settingsarray[51]=settingsarray[52];settingsarray[53]=settingsarray[52];
		}
		else  {
		  settingsarray[51]=0;
		  settingsarray[52]=0;
		  settingsarray[53]=0;
		}
		}
#else
      // TODO: REDO THIS as above when have tested!
      // TESTY as might need to SMOOTH!
      //      tmphardware=tmphardware<<3; // 15 bits
      /*      if (mirror<128 && tmphardware!=effects && tmphardware!=effects-1 && tmphardware!=effects+1){
	SAMPLEWRAP=tmphardware<<3; // handled all below 
	hardware=tmphardware;
	thardware=tmphardware;
	effects=200; // never near but set to avoid gap...
	    }
	      else if (mirror >128 && tmphardware!=thardware && tmphardware!=thardware-1 && tmphardware!=hardware+1 && tmphardware!=effects){
		effects=tmphardware;
		thardware=200;
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
		  //		  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;
		  EFFECTREAD=effects;EFFECTREAD=EFFECTWRITE;EFFECTFILTER=EFFECTREAD;
		}
		}*/
#endif      	
      // HERE!
      // SETTINGSARRAY
      hardware=adc_buffer[FIRST]>>5; //TESTY!!
      tmpsettings=adc_buffer[THIRD]>>6; // 0-64 ???
      tmper=64;settings=36; // TESTY!!
      setted=fingervalup(settingsarray[settings],tmper);
      settingsarray[settings]=setted; 
      //tmper=64;settings=12; // TESTY!!
      //      setted=fingervalup(settingsarray[settings],tmper);
      //      settingsarray[settings]=setted; 

      //      EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;//TESTER!
      if (mirror<128 && tmpsettings!=settingsops && tmpsettings!=settingsops-1 && tmpsettings!=settingsops+1){
	settings=tmpsettings; 
	settingsops=200;
	// TODO: if is 0 we use fingers left/right!
	// but then we also need to set 0 settings - [settings-1]
	//	if (settings<25) tmper=8; else tmper=2;//?????

	if (settings<46){ // TESTY was 54
	  //	  	  tmper=64;settings=36; // TESTY!!
	  //TODO: wrap here means that we get 16 bits coming in
	  //	  setted=fingervalup(settingsarray[settings],tmper);
	  //	  settingsarray[settings]=setted; 
	  //	  settingsarray[settings]=adc_buffer[FIFTH];
	}
	else
	  {
	// TODO: what of directions???
	  }
      }
      else if (mirror>128 && tmpsettings!=settings && tmpsettings!=settings-1 && tmpsettings!=settings+1){
	settingsops=tmpsettings; // 0-64???
	settings=200;
      // operations which are set and act continuously elsewhere
      // operations which just take place here: contract, expand, shift a region, the region
    }

      // BLACK STACKS AND EXTRA KNOB

      tmpstack=adc_buffer[FOURTH]>>6; // 0-64???

      if (mirror<128 && tmpstack!=stackops && tmpstack!=stackops-1 && tmpstack!=stackops+1){
	stack=tmpstack;
	stackops=200;

	///KEY!!!TODO!!!
	// finger up/down to choose stack and value
	// tmpstack changes value - or vice versa

	// STACKSTART, STACKWRAP, STACKMUCH
	// ((stack_pos+stack_posy)*3) = maximum 32x3=96
	// +villager (max 64)

	// push/pop (push<x pop>x) and set which CPU/grain is pushed or
	// popped [and with what buffer and settings?]
	// previous code was based on mirror to choose which (setting we can inherit from settings.h)
	//- simulation: stack[max=stack_pos]: start, wrap, howmuch
	//- CAforstack: stack[max=stack_posy]: start, wrap, howmuch
	//- villager: start, wrap [][]

	    }
      else if (mirror>128 && tmpstack!=stack && tmpstack!=stack-1 && tmpstack!=stack+1){
	stackops=tmpstack; //0-64???
	stack=200;
      // operations which are set and act continuously elsewhere
      // operations which just take place here: contract, expand, shift a region, the region
    }

      // FIFTH KNOB is spare - use instead of finger up/down = override if changes
      // on mirror we use as mirror/foldback settings with finger up/down - see below
      // with mirror we also have free finger left/right to use 


      /////////// TODO: maintain those operations flagged above
      // and where we have these settings from = finger and fifth knob as above

      //       // ops on region of settingsarray:    
      // none
      // mirror from stack(region), from datagen to region
      // infect, randi across (if TENE)

      // srcstart,srcwrap, deststart,destwrap, speed, buffer
      // none
      //      // ops on region of stacks:    
      // mirror from settings(region), from datagen to region
      // infect,  randi across (if TENE)

      // srcstart,srcwrap, deststart,destwrap, speed, buffer


      ////////////////////////////////////////////////
#ifndef LACH
      /////////////////////////////////////
      // 4-hardware operations

      /// general HW walk in/as tmp

      //	if (HWSPEED==0) HWSPEED=1;
      if (HWSPEEDY==0) HWSPEEDY=1;
	if (++hwdel>=HWSPEEDY){
	  // when wrapper changes we need to redo direction array!!!
	  hwpos+=(HWSTEP*hwdir[HWDIR]);
	  wrapper=HWWRAP;
	  if (wrapper==0) wrapper=1;
	  tmp=(HWSTART+(hwpos%wrapper))%32768; //to cover all directions
	  hwdel=0;
	}

      if (digfilterflag&16){
	if (HDGENERCONS==0) HDGENERCONSY=1;
	  hdtmp=(HWSTART+(hwpos%wrapper)); 
	  dohardwareswitch(hardware,HDGENERBASE+(datagenbuffer[hdtmp]%HDGENERCONSY));
      }
      else
	{
	  if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	  oldhardware=hardware;
	}
	     		   
      // 3 datagenclocks->40106/lm/maxim - filterflag as bits as we also need signal which clocks we

      // just leave this running
		     		     
      //      if (digfilterflag&2){
	  set40106pwm(F0106ERBASE+(buf16[tmp]%F0106ERCONS)); // constrain all to base+constraint
	  //      }
	  

      if (digfilterflag&4){
	if (++hwdel>=HWSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  hwpos+=(HWSTEP*hwdir[HWDIR]);
	  wrapper=HWWRAP;
	  if (wrapper==0) wrapper=1;
	  x=(HWSTART+(hwpos%wrapper))%32768; //to cover all directions
	  hwdel=0;
	}
	  setlmpwm(LMERBASE+(buf16[x]%LMERCONS),LMERBASE+(buf16[tmp]%LMERCONS)); 
      }
	  
      if (digfilterflag&8){
	  setmaximpwm(MAXIMERBASE+(buf16[x]%MAXIMERCONS));
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
