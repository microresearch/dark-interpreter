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

VocoderInstance* vocoder;
villagerr village_write[MAX_VILLAGERS+1];
villagerr village_read[MAX_VILLAGERS+1];
villagerr village_filt[MAX_VILLAGERS+1];
villager_generic village_datagen[MAX_VILLAGERS+1];
u8 howmanydatavill;

u16 testfunction(void){
  u16 tester=adc_buffer[FIRST];
  return tester;
}


u16 counterd=0, databegin=0,dataend=32767;
u8 deldata=0,dataspeed=1;
int16_t dirryd=1;
u8 inp;


void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 x,addr,count;
  u8 exestack[MAX_EXE_STACK];

  vocoder=instantiateVocoder();

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

  for (u8 xx=0;xx<64;xx++){
    village_read[xx].counterr=0;
    village_write[xx].counterr=0;
    village_read[xx].compress=1;
    village_write[xx].compress=1;
	}

  // random init: TESTY!

  /*      	for (u8 xx=0;xx<64;xx++){
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
	  }*/


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

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
    buf16[x]=rand()%65536; // was RANDI OCT
    delayxx();
  }
  
	    // compile test
	    //	    	    	    testvocode();
	    //	    	    x=sqrtf(x);

  // inits for datagens

  seirinit(buf16);// LEAVE IN!
  sicrinit(buf16);// LEAVE IN!
  ifsinit(buf16);// LEAVE IN!


  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

#ifdef TEST_EFFECTS
      //runsine
      //u16 runsine(u8 step, u16 count, u16 start, u16 wrap){
      count=runnoise(1,count,0,32767);
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

  if (++deldata>=dataspeed) {
    counterd+=dirryd;
    deldata=0;
  }

  if ((counterd-databegin)>dataend) {
    counterd=databegin;
    for (u8 x=0;x<howmanydatavill;x++){
      village_datagen[x].running=1;
    }
  }	    

  if (counterd<databegin) {
    counterd=dataend+databegin;
    for (u8 x=0;x<howmanydatavill;x++){
      village_datagen[x].running=1;
	  }	    
	  }

  for (x=0;x<howmanydatavill;x++){
    // speed for each
        if ((village_datagen[x].start)<=counterd && village_datagen[x].running==1){// in town
	  if (++village_datagen[x].del>=village_datagen[x].step){

	    //village_datagen[x].cpu=11; // CRASH TESTY!
    switch(village_datagen[x].cpu){      
    case 0:
      village_datagen[x].position=runnoney(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 1:
      village_datagen[x].position=runkrum(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 2:
      village_datagen[x].position=runhodge(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 3:
      village_datagen[x].position=runhodgenet(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 4:
      village_datagen[x].position=runlife(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 5:
      village_datagen[x].position=runcel(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 6:
      village_datagen[x].position=runcel1d(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 7:
      village_datagen[x].position=runfire(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 8:
      village_datagen[x].position=runwire(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 9:
      village_datagen[x].position=runSIR(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 10:
      village_datagen[x].position=runSIR16(village_datagen[x].speed,village_datagen[x].position);
      break;
      /// add in sims
    case 11:
      village_datagen[x].position=runform(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 12:
      village_datagen[x].position=runconv(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 13:
      village_datagen[x].position=runsine(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 14:
      village_datagen[x].position=runconv(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 15:
      village_datagen[x].position=runchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 16:
      village_datagen[x].position=runderefchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 17:
      village_datagen[x].position=runwalkerchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 18:
      village_datagen[x].position=runswapchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 19:
      village_datagen[x].position=runinc(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 20:
      village_datagen[x].position=rundec(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 21:
      village_datagen[x].position=runleft(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 22:
      village_datagen[x].position=runright(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 23:
      village_datagen[x].position=runswap(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 24:
      village_datagen[x].position=runnextinc(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 25:
      village_datagen[x].position=runnextdec(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 26:
      village_datagen[x].position=runnextmult(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 27:
      village_datagen[x].position=runnextdiv(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 28:
      village_datagen[x].position=runcopy(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 29:
      village_datagen[x].position=runzero(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 30:
      village_datagen[x].position=runfull(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 31:
      village_datagen[x].position=runrand(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 32:
      village_datagen[x].position=runknob(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 33:
      village_datagen[x].position=runswapaudio(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 34:
      village_datagen[x].position=runORaudio(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 35:
      village_datagen[x].position=runsimplesir(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 36:
      village_datagen[x].position=runseir(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 37:
      village_datagen[x].position=runsicr(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 38:
      village_datagen[x].position=runifs(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 39:
      village_datagen[x].position=runrossler(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 40:
      village_datagen[x].position=runsecondrossler(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 41:
      village_datagen[x].position=runbrussel(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 42:
      village_datagen[x].position=runspruce(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 43:
      village_datagen[x].position=runoregon(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 44:
      village_datagen[x].position=runfitz(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 45:
      machine_runnn(village_datagen[x].start,village_datagen[x].wrap); 
      village_datagen[x].position+=village_datagen[x].speed;
      break;
     case 46:
      village_datagen[x].position=xxrunleakystack(&village_datagen[x]);
      break;
     case 47:
      village_datagen[x].position=xxrunbiota(&village_datagen[x]);
      break;
     case 48:
      village_datagen[x].position=xxrun1(&village_datagen[x]);
      break;
     case 49:
      village_datagen[x].position=xxrunworm(&village_datagen[x]);
      break;
     case 50:
      village_datagen[x].position=xxrunstack(&village_datagen[x]);
      break;
     case 51:
      village_datagen[x].position=xxrunbefunge(&village_datagen[x]);
      break;
     case 52:
      village_datagen[x].position=xxrunlang(&village_datagen[x]);
      break;
     case 53:
      village_datagen[x].position=xxrunbf(&village_datagen[x]);
      break;
     case 54:
      village_datagen[x].position=xxrunturm(&village_datagen[x]);
      break;
     case 55:
      village_datagen[x].position=xxrunca(&village_datagen[x]);
      break;
     case 56:
      village_datagen[x].position=xxrunhodge(&village_datagen[x]);
      break;
     case 57:
      village_datagen[x].position=xxrunworm2(&village_datagen[x]);
      break;
     case 58:
      village_datagen[x].position=xxrunleaky(&village_datagen[x]);
      break;
     case 59:
      village_datagen[x].position=xxrunconvy(&village_datagen[x]);
      break;
     case 60:
      village_datagen[x].position=xxrunplague(&village_datagen[x]);
      break;
     case 61:
      village_datagen[x].position=xxrunmicro(&village_datagen[x]);
      break;
     case 62:
      village_datagen[x].position=xxruncw(&village_datagen[x]);
      break;
     case 63:
      village_datagen[x].position=xxrunmasque(&village_datagen[x]);
      break;
    } // end of switch

    if (village_datagen[x].position>(village_datagen[x].start+village_datagen[x].wrap)) {
      village_datagen[x].running=0;
      // reset
      village_datagen[x].position=village_datagen[x].start;
    }
    village_datagen[x].del=0;
      }
    } // if running
  } // end of x/run thru all villagers


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
#endif //test effects
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
