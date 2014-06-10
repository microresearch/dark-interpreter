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
u16 stackery[STACK_SIZE*4]; // 16*4 MAX
u16 stacker[STACK_SIZE*4]; // 16*4 MAX
u16 settingsarray[64];
//u16 FOLDD[45]; // MAX size 44!!!

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
    register unsigned int i;					\
    for (i = 0; i < 1000000; ++i)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int i;					\
    for (i = 0; i < 1000; ++i)				\
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
u16* FOLDD; // MAX size 44!!!
#endif
extern u8 digfilterflag;

//u8 testdirection;
//u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array


u16 villagepush(u16 villagepos, u16 start, u16 wrap){
  if (villagepos<(VILLAGE_SIZE-2)) /// size -2
    {
      villager[villagepos++]=start;
      villager[villagepos++]=wrap;
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

  // TESTY!
  //  return (rand()%2);

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
  for (x=0;x<16;x++){
  handup=adc_buffer[RIGHT]>>8;
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

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 x,addr,tmp=0,tmppp=0,hdtmp=0,tmphardware=0,HARDWARE=0;
  u8 machine_count=0,leak_count=0; 
  //  u8 exeperms[88]={0,1,2,3, 0,1,3,2, 0,2,3,1 ,0,2,1,3, 0,3,1,2, 0,3,2,1, 1,0,2,3, 1,0,3,2, 1,2,3,0, 1,2,0,3, 1,3,2,0, 1,3,0,2, 2,1,0,3, 2,1,3,0, 2,3,1,0, 2,3,0,1, 3,0,1,2, 3,0,2,1, 3,1,0,2, 3,1,2,0, 3,2,0,1, 3,2,1,0}; 

  inittable(3,4,randi());

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
  stacker=malloc(48*sizeof(int16_t));
  stackery=malloc(48*sizeof(int16_t));
  FOLDD=malloc(45*sizeof(int16_t));
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

  struct stackey stackyy[STACK_SIZE];
  struct stackeyyy stackyyy[STACK_SIZE];
  u16 *buf16 = (u16*) datagenbuffer;
  u8 *audio_buf = (u8*) audio_buffer;
  u8 leakiness=randi()%255;
  u8 infection=randi()%255;

  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;

  // fill datagenbuffer???

  for (x=0;x<65535;x++){
    datagenbuffer[x]=randi()%255;
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
    settingsarray[x]=8192;
  }//speed


  for (x=46;x<49;x++){
    settingsarray[x]=65535;
  }//wrap

  for (x=54;x<64;x++){
    settingsarray[x]=32768;//>>15
  }//DIR

  settingsarray[51]=0; //was EFFECTS // is now EXPANSION TODO!
  settingsarray[52]=0;
  settingsarray[53]=0;
	 
  // CPUintrev2:
  for (x=0; x<100; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,datagenbuffer,addr,randi()<<3,randi()%31,randi()%24);
    }

  for (x=0;x<45;x++){
    //    FOLDD[x]=randi()<<4; // TESTY!
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
	  //	  villagestackpos=villagepush(villagestackpos,start,wrap);
  }

  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
  //  for (x=0;x<2;x++){ // TESTY!
          start=randi()<<3;
	  //start=0; wrap=32768; // TESTY!
      wrap=randi()<3;
      stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,randi()%24,start,wrap);
    //    stack_pos=func_pushn(stackyy,0,buf16,stack_pos,randi()%24,start,wrap);
    //    printf("stackpos %d\n",stack_pos);
    
        villagestackpos=villagepush(villagestackpos,start,wrap);
  }

  start=0;
  u16 count=0;

  ///////////////////////////

  u8 mastermode,mainmode,oldmainmode=0,changeflagone=0,changeflagtwo=0,changeflagthree=0,fingermode;
  u8 eff[4];

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

      func_runall(stackyy,stack_pos); // simulations
      //            machine_run(m);
      //      machine_runnn(datagenbuffer);
      //  ca_runall(stackyyy,stack_posy); // CA

	/*
      machine_count++;
      if (machine_count>=MACHINESPEED){

      for (x=0;x<4;x++){
	switch(exeperms[((EXESPOT%22)*4)+x]){
	case 0:
	  func_runall(stackyy,stack_pos); // simulations
	  break;
	case 1:
	  ca_runall(stackyyy,stack_posy); // CA
	  break;
	case 2:
	  machine_run(m);
	    m->m_leakiness=leakiness;
	    m->m_infectprob=infection;
	    machine_count=0;
	  break;
	case 3:
	  leak_count++;
	  if (leak_count>=LEAKSPEED){
	    machine_runnn(datagenbuffer);
	    leak_count=0;
	  }
	}
	}
      } // end of machine count
	*/

      //// MODE/KNOB CODE START
      /////////////////////////////////////
      
      u8 xx;
      mastermode=adc_buffer[FIRST]>>11; // 32=5 bits

      eff[0]=adc_buffer[SECOND]>>5; // was 7 bits=128//jitter???
      eff[1]=adc_buffer[THIRD]>>5;
      eff[2]=adc_buffer[FOURTH]>>5;
      
      switch(mastermode){
      case 0: // EFFMODE
	xx=fingerdir(); // 0-3
	if (xx!=5){
	EFFECTREAD=eff[xx%3];
	EFFECTWRITE=eff[(xx+1)%3];
	EFFECTFILTER=eff[(xx+2)%3];
	}
	  break;
      case 1:

	// knobs change groups X,Y,Z- preformed:
	// X-START Y-WRAP Z-SPEED
	// if enter this case then must be a change (from new mode knobs) to register...
	
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
      case 2:
	// knobs change groups X,Y,Z
	// X and Y as chunk start/wrap
	// Z as knob
	break;
      case 3:
	// knobs change groups X,Y,Z
	// algo walker???? TODO???
	break;
      case 4:
	// knobs change mirror settings - how to toggle?
	break;
      case 5:
	// knobs change mirror settings
	break;
      case 6:
	// knobs change mirror settings
	break;
      case 7:
	// knobs change mirror settings
	break;
	    }

      //      */
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
