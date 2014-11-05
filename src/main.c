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
#include "mdavocoder.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
//#include "settings.h"
#include "vocode.h"
#include "biquad.h"
#include "effect.h"
#include "PV_vocoder.h"

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

villagerr village_write[MAX_VILLAGERS+1];
villagerr village_read[MAX_VILLAGERS+1];
villagerr village_filtin[MAX_VILLAGERS+1];
villagerr village_filtout[MAX_VILLAGERS+1];

villager_effect village_effect[17];

//mirror mvillage_write[MAX_VILLAGERS+1];
//mirror mvillage_read[MAX_VILLAGERS+1];
//mirror mvillage_filtin[MAX_VILLAGERS+1];
//mirror mvillage_filtout[MAX_VILLAGERS+1];


villager_datagenwalker village_datagenwalker[MAX_VILLAGERS+1];
villager_generic village_datagen[MAX_VILLAGERS+1];
villager_hardware village_hardware[17];
villager_hardwarehaha village_40106[17];
villager_hardwarehaha village_hdgener[17];
villager_hardwarehaha village_lm[17];
villager_hardwarehaha village_maxim[17];

u8 howmanydatagenwalkervill=1, howmanydatavill=1,howmanyeffectvill=1,howmanywritevill=1,howmanyfiltinvill=1, howmanyfiltoutvill=1,howmanyreadvill=1;

u16 counterd=0, databegin=0,dataend=32767;
u8 deldata=0,dataspeed=1;
int16_t dirryd=1;
u8 inp=0;

// effects tests

biquad *biquaddd;
VocoderInstance* vocoder;
BBandPass *unit;
Formlet *unitt;
mdavocoder *unittt;
mdavocal *unitttt;
PV *pv;
BPFSC* bpfunit;



//arm_biquad_casd_df1_inst_f32* df1;
//float* state;
//float coeffs[5];

u16 nextdatagen(void){
  u16 tmp,tmpp;
  u8 x;
  static u8 whichdatagenwalkervillager=0;
  static u16 countdatagenwalker=0;

  x=whichdatagenwalkervillager%howmanydatagenwalkervill;
  countdatagenwalker+=village_datagenwalker[x].step;
  tmp=village_datagenwalker[x].knoboffset; // as is =32768 for datagenwalker

  tmpp=tmp+(buf16[(village_datagenwalker[x].dataoffset+village_datagenwalker[x].samplepos)%32768])%(32768-tmp);
  //  tmp=buf16[(village_datagenwalker[x].dataoffset+village_datagenwalker[x].samplepos)%32768];

  village_datagenwalker[x].samplepos+=village_datagenwalker[x].dirry;
  if (village_datagenwalker[x].samplepos>=village_datagenwalker[x].length) village_datagenwalker[x].samplepos=0;
  else if (village_datagenwalker[x].samplepos<0) village_datagenwalker[x].samplepos=village_datagenwalker[x].length;

  if (countdatagenwalker>=village_datagenwalker[x].length){
    countdatagenwalker=0;
    whichdatagenwalkervillager++; //u8
  }
  return tmpp;
}

void main(void)
{
  // order that all inits and audio_init called seems to be important
  u16 x,addr,count;
  u8 exestack[MAX_EXE_STACK];
  u8 tmp,oldtmp,xx; // TESTY!
  // effects tests

  /*
  vocoder=instantiateVocoder();
  biquaddd=BiQuad_new(BPF,1.0f,50.0f,48000.0f,0.5f);
  //  unit=(BBandPass *)malloc(sizeof(BBandPass));
  unitt=(Formlet *)malloc(sizeof(Formlet));
  bpfunit=(BPFSC*)malloc(sizeof(BPFSC));
  BPFSC_init(bpfunit);
  Formlet_init(unitt);
  unittt=(mdavocoder *)malloc(sizeof(mdavocoder));
  unitttt=(mdavocal *)malloc(sizeof(mdavocal));
  pv=(PV *)malloc(sizeof(PV));
  mdaVocoder_init(unittt);
  mdavocal_init(unitttt);
  */

  // malloc and instantiate all filters, formlet, mdaVocoder, PV? and mdaVocal (anything else?)

  //////////////

  // init all basic villagers

  for (xx=0;xx<64;xx++){

    village_read[xx].counterr=0;
    village_read[xx].del=0;
    village_read[xx].compress=1;
    village_read[xx].mirrormod=0;
    village_read[xx].mirrordel=0;
    village_read[xx].infected=0;
    village_read[xx].samplepos=0;
    village_read[xx].speed=1;
    village_read[xx].step=1;
    village_read[xx].start=0;
    village_read[xx].wrap=100; // TODO test
    village_read[xx].dir=1;
    village_read[xx].dirry=1;
    village_read[xx].samplepos=0;
    village_read[xx].offset=0;
    village_read[xx].effect=1.0f;
    village_read[xx].effectinv=0.0f;
    village_read[xx].overlay=0;
    village_read[xx].running=0;

    village_write[xx].counterr=0;
    village_write[xx].del=0;
    village_write[xx].compress=1;
    village_write[xx].mirrormod=0;
    village_write[xx].mirrordel=0;
    village_write[xx].infected=0;
    village_write[xx].samplepos=0;
    village_write[xx].speed=1;
    village_write[xx].step=1;
    village_write[xx].start=0;
    village_write[xx].wrap=100; // TODO test
    village_write[xx].dir=1;
    village_write[xx].dirry=1;
    village_write[xx].samplepos=0;
    village_write[xx].offset=0;
    village_write[xx].effect=1.0f;
    village_write[xx].effectinv=0.0f;
    village_write[xx].overlay=0;
    village_write[xx].running=0;

#ifndef LACH
    village_filtin[xx].counterr=0;
    village_filtin[xx].del=0;
    village_filtin[xx].compress=1;
    village_filtin[xx].mirrormod=0;
    village_filtin[xx].mirrordel=0;
    village_filtin[xx].infected=0;
    village_filtin[xx].samplepos=0;
    village_filtin[xx].speed=1;
    village_filtin[xx].step=1;
    village_filtin[xx].start=0;
    village_filtin[xx].wrap=100; // TODO test
    village_filtin[xx].dir=1;
    village_filtin[xx].dirry=1;
    village_filtin[xx].samplepos=0;
    village_filtin[xx].offset=0;
    village_filtin[xx].effect=1.0f;
    village_filtin[xx].effectinv=0.0f;
    village_filtin[xx].overlay=0;
    village_filtin[xx].running=0;

    village_filtout[xx].counterr=0;
    village_filtout[xx].del=0;
    village_filtout[xx].compress=1;
    village_filtout[xx].mirrormod=0;
    village_filtout[xx].mirrordel=0;
    village_filtout[xx].infected=0;
    village_filtout[xx].samplepos=0;
    village_filtout[xx].speed=1;
    village_filtout[xx].step=1;
    village_filtout[xx].start=0;
    village_filtout[xx].wrap=100; // TODO test
    village_filtout[xx].dir=1;
    village_filtout[xx].dirry=1;
    village_filtout[xx].samplepos=0;
    village_filtout[xx].offset=0;
    village_filtout[xx].effect=1.0f;
    village_filtout[xx].effectinv=0.0f;
    village_filtout[xx].overlay=0;
#endif

    // datagenwalker
    village_datagenwalker[xx].length = 100;
    village_datagenwalker[xx].dataoffset = 0;
    village_datagenwalker[xx].knoboffset = 0;
    village_datagenwalker[xx].samplepos = 0;
    village_datagenwalker[xx].dirry =1;
    village_datagenwalker[xx].speed = 1;
    village_datagenwalker[xx].step = 1;
    village_datagenwalker[xx].dir = 1;
    
    // datagen
    village_datagen[xx].start = 0;
    village_datagen[xx].CPU = 0;
    village_datagen[xx].wrap=100;
    village_datagen[xx].position=0;
    village_datagen[xx].del=0;
    village_datagen[xx].step=1;
    village_datagen[xx].speed=1;
    village_datagen[xx].dir=1;
    village_datagen[xx].running=0;
    village_datagen[xx].m_stack_pos=0;
    village_datagen[xx].m_reg16bit1=0;
    village_datagen[xx].m_reg8bit1=0;
    village_datagen[xx].m_reg8bit2=0;
	}


#ifndef LACH
  for (xx=0;xx<17;xx++){
    village_hardware[xx].length=16;
    village_hardware[xx].inp=0;
    village_hardware[xx].setting=0;

    village_40106[xx].length=16;
    village_40106[xx].dataoffset=0;
    village_40106[xx].knoboffset=0;
    village_40106[xx].samplepos=0;
    village_40106[xx].dirry=1;
    village_40106[xx].dir=1;
    village_40106[xx].speed=1;
    village_40106[xx].step=1;

    village_hdgener[xx].length=16;
    village_hdgener[xx].dataoffset=0;
    village_hdgener[xx].knoboffset=0;
    village_hdgener[xx].samplepos=0;
    village_hdgener[xx].dirry=1;
    village_hdgener[xx].dir=1;
    village_hdgener[xx].speed=1;
    village_hdgener[xx].step=1;

    village_lm[xx].length=16;
    village_lm[xx].dataoffset=0;
    village_lm[xx].knoboffset=0;
    village_lm[xx].samplepos=0;
    village_lm[xx].dirry=1;
    village_lm[xx].dir=1;
    village_lm[xx].speed=1;
    village_lm[xx].step=1;

    village_maxim[xx].length=16;
    village_maxim[xx].dataoffset=0;
    village_maxim[xx].knoboffset=0;
    village_maxim[xx].samplepos=0;
    village_maxim[xx].dirry=1;
    village_maxim[xx].dir=1;
    village_maxim[xx].speed=1;
    village_maxim[xx].step=1;
  }
#endif

  // any other inits/variables????

  // think is all set

  //////////////// older inits
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
    dohardwareswitch(0,0,0);
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

  // run through datagen villagers

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
    switch(village_datagen[x].CPU){      
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

  u8 xx, mirrormode,ranger,whichx,whichy,spd;

  xx=fingerdir(&spd);

  /* do mirror and infections between:

  extern villagerr village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  extern villagerr village_filtin[MAX_VILLAGERS+1];
  extern villagerr village_filtout[MAX_VILLAGERS+1];
  extern villager_generic village_datagen[MAX_VILLAGERS+1];

  extern villager_hardware village_hardware[17];
  extern villager_hardwarehaha village_40106[17];
  extern villager_hardwarehaha village_hdgener[17];
  extern villager_hardwarehaha village_lm[17];
  extern villager_hardwarehaha village_maxim[17];

   */

	if (xx!=5){
	  mirrormode=adc_buffer[FIFTH]>>7; // 5 bits=32

	  switch(mirrormode){
	  case 18: // swop and copy
	    whichx=adc_buffer[FIRST]>>6; // 6 bits=64 //TODO? restricted as to how many we have below?
	    whichy=adc_buffer[SECOND]>>6; // 6 bits=64 //TODO? restricted as to how many we have below?
	    ranger=adc_buffer[THIRD]>>6; // 64
	    // and FOURTH knob??? as mirror modulator
	    //	    action=xx;
	    switch(xx){ 
	    case 0:
	      switch(ranger){
	      case 0:
		village_write[whichx]=village_write[whichy]; // restrict here or???
		break;
	      case 1:
		village_write[whichx]=village_read[whichy];
		break;
	      case 2:
		village_write[whichx]=village_filtin[whichy];
		break;
	      case 3:
		village_write[whichx]=village_filtout[whichy];
		break;
	      case 4:
		// copy appropriate datagen settings
		village_write[whichx].start=village_datagen[whichy].start;
		village_write[whichx].wrap=village_datagen[whichy].wrap;
		village_write[whichx].samplepos=village_datagen[whichy].position;
		village_write[whichx].dirry=village_datagen[whichy].dirry;
		village_write[whichx].dir=village_datagen[whichy].dir;
		village_write[whichx].step=village_datagen[whichy].step;
		village_write[whichx].speed=village_datagen[whichy].speed;
		break;
	      case 5:
		// datagen walker 
		village_write[whichx].start=village_datagenwalker[whichy].dataoffset;
		village_write[whichx].wrap=village_datagenwalker[whichy].length;
		village_write[whichx].samplepos=village_datagenwalker[whichy].samplepos;
		village_write[whichx].dirry=village_datagenwalker[whichy].dirry;
		village_write[whichx].dir=village_datagenwalker[whichy].dir;
		village_write[whichx].step=village_datagenwalker[whichy].step;
		village_write[whichx].speed=village_datagenwalker[whichy].speed;
		break;
	      case 6:
		// with its mirror
		village_write[whichx].start=village_write[whichy].mstart;
		village_write[whichx].wrap=village_write[whichy].mwrap;
		village_write[whichx].compress=village_write[whichy].mcompress;
		break;
	      case 7:
		/// copy outmod for effects! outstart, outwrap
		village_write[whichx].start=village_effect[whichx].outstart;
		village_write[whichx].wrap=village_effect[whichx].outwrap;
		village_write[whichx].samplepos=village_effect[whichx].outpos;
		break;
		/// TODO: mult all above by village_read,village_datagen,village_datagenwalker,village mirrors,outmod=8*8=64

	      } // end of ranger
	      break;
	    case 1: // next actionsssTODO!
	      break;
		/* example actions
		
		// swop
		tmpvillage=village_write[whichx];
		village_write[whichx]=village_read[whichy];
		village_read[whichy]=tmpvillage;
		break;

	    case 3:// unknown???  constrain %
		village_read[whichx]%=(village_write[whichy]+1);
		*/
	    } // end of xx = action
	    break; // case 18
	  case 19: // mirror
	    // 1-set which villager is mirrored - but depends on how many
	    whichx=adc_buffer[FIRST]>>6; // 6 bits=64 //TODO? restricted as to how many we have below?
	    // 2-which group also TODO!
	    // finger-set what to mirror-> datagen/eeg/finger/knob_as_4 = fingered...
	    village_write[whichx].fingered=xx; // TODO... other groups
	    // 3-set how mirror effects = mirrormod if 0 then nothing happens
	    village_write[whichx].mirrormod=adc_buffer[THIRD]>>6;
	    // finger-speed and step for this
	    village_write[whichx].mirrorspeed=(spd&15)+1; 
	    break;
	    /// TODO:
	    //20- fingers in the code
	    //21- infection across (how?)
	  } // end of mirrormodes
	}// xx!=5

	// MIRROR! TODO- here is just village_write!
	u16 tmpp;

	for (whichx=0;whichx<howmanywritevill;whichx++){
	  if (village_write[whichx].mirrormod){
	    // speed wrapper
	    if (++village_write[whichx].mirrordel>=village_write[whichx].speed){
	      village_write[whichx].mirrordel=0;
	    // then do mirrormod and fingered on start,wrap and compress
	      // but all are same fingered.TODO????
	      switch(village_write[whichx].fingered){
	//fingered - UP.2=datagen DOWN.3=eeg/finger(SUSP) LEFT.0.finger RIGHT.1knob
	      case 0: // LEFT=finger
		village_write[whichx].mstart=adc_buffer[LEFT]<<3;
		village_write[whichx].mwrap=adc_buffer[LEFT]<<3;
		village_write[whichx].mcompress=adc_buffer[LEFT]<<3;
		break;
	      case 1: //RIGHT= knob 
		village_write[whichx].mstart=adc_buffer[FOURTH]<<3;
		village_write[whichx].mwrap=adc_buffer[FOURTH]<<3;
		village_write[whichx].mcompress=adc_buffer[FOURTH]<<3;
		break;
	      case 2: // UP=datagen
		village_write[whichx].mstart=nextdatagen();
		village_write[whichx].mwrap=nextdatagen();
		village_write[whichx].mcompress=nextdatagen();
		break;
	      case 3: // DOWN=EEG ifdef
#ifdef TENE
		village_write[whichx].mstart=adc_buffer[9]<<3;
		village_write[whichx].mwrap=adc_buffer[9]<<3;
		village_write[whichx].mcompress=adc_buffer[9]<<3;
#else
		village_write[whichx].mstart=adc_buffer[LEFT]<<3;
		village_write[whichx].mwrap=adc_buffer[LEFT]<<3;
		village_write[whichx].mcompress=adc_buffer[LEFT]<<3;
#endif
		break;
	      }

	      switch(village_write[whichx].mirrormod){
	      case 1: // straight datagen
		village_write[whichx].start=village_write[whichx].mstart;
		village_write[whichx].wrap=village_write[whichx].mwrap;
		village_write[whichx].compress=village_write[whichx].mcompress;
		break;
	      case 2: // addition with wrap
		village_write[whichx].start=(village_write[whichx].kstart+village_write[whichx].mstart)%32768;
		village_write[whichx].wrap=(village_write[whichx].kwrap+village_write[whichx].mwrap)%32768;
		village_write[whichx].compress=(village_write[whichx].kcompress+village_write[whichx].mcompress)%32768;
		break;
	      case 3: // addition up to 32768
		tmpp=32768-village_write[whichx].kstart;
		village_write[whichx].start=village_write[whichx].kstart+(village_write[whichx].mstart%tmpp);
		tmpp=32768-village_write[whichx].kwrap;
		village_write[whichx].wrap=village_write[whichx].kwrap+(village_write[whichx].mwrap%tmpp);
		tmpp=32768-village_write[whichx].kcompress;
		village_write[whichx].compress=village_write[whichx].kcompress+(village_write[whichx].mcompress%tmpp);
		break;
		// TODO: other cases: subtraction, and, or, modulus
	      }
	    }
	  }	
	}

	////// all effects

	  for (x=0;x<howmanyeffectvill;x++){
	    do_effect(&village_effect[x]);
	  }

	  /// end effects/ALLLL!!!!

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
