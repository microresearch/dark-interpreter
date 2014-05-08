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

u8 EFFECTREAD,EFFECTWRITE,EFFECTFILTER;

signed char newdir[2]={-1,1};
signed char direction[2]={-1,1};
signed char villagedirection[2]={-1,1};
signed char villagedirectionf[2]={-1,1};
signed char villagedirectionw[2]={-1,1};
signed char newdirf[2]={-1,1};
signed char directionf[2]={-1,1};
signed char newdirread[2]={-1,1};
signed char directionread[2]={-1,1};

#define VILLAGE_SIZE 192 // was 64 *2=128 now 96*2=192

u8 villagestackpos=0;
u16 villager[VILLAGE_SIZE];
u16 stackery[48]; // 16*3 MAX
u16 stacker[48]; // 16*3 MAX
u8 ww[3]={12,15,0};
u8 freqy[3]={10,10,0};

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


#define randi() (adc_buffer[9]) // 12 bits

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

extern int16_t audio_buffer[AUDIO_BUFSZ];

u8* datagenbuffer = (u8*)0x10000000;
extern u8 digfilterflag;

//u8 testdirection;
//u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array
u16 settingsarray[64];
u16 FOLDD[45]; // MAX size 44!!!



u8 villagepush(u8 villagepos, u16 start, u16 wrap){
  if (villagepos<190) /// size -2
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

  u8 handleft, handright, up=0,down=0,left=0,right=0;
  u8 handupp, handdown;
  u8 result;

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
  u8 result=0;

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
  int16_t tmps;
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
 if (tmpsetting==0) tmpsetting=1;
  }
  return tmpsetting;
}

u8 fingervalright(u8 tmpsetting, u8 wrap){
  u8 handup,handdown;
  int16_t tmps;
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
  u16 x,addr,tmp,tmppp,hdtmp,tmphardware,HARDWARE,knobby,coo=0;
  u16 settings,setted;
  u8 oldsettings,oldsetted,tmper,stackerstart,stackerwrap;
  u8 mirror=0,mirrortoggle,villagemirror=0,mirrordel=0, fingermod,oldfingermod,fingerfing,whichdir=0,whichdiritis=0,speed,whichspeed=0,foldback=0, whichstack=0,step=0,whichstep=0,constrained=0,started=0,stackmuch=10,exespot=0,cpur=0,cpu,startrr=0,wraprr=0;
  u16 startr,wrapr;
  u16 constrain,foldbackset;
  u16 m1flag,m2flag;
  u8 effects;
  u8 machine_count=0,leak_count=0; 
  u8 exeperms[88]={0,1,2,3, 0,1,3,2, 0,2,3,1 ,0,2,1,3, 0,3,1,2, 0,3,2,1, 1,0,2,3, 1,0,3,2, 1,2,3,0, 1,2,0,3, 1,3,2,0, 1,3,0,2, 2,1,0,3, 2,1,3,0, 2,3,1,0, 2,3,0,1, 3,0,1,2, 3,0,2,1, 3,1,0,2, 3,1,2,0, 3,2,0,1, 3,2,1,0}; 

  signed char hwdir[2]={1,-1};

  inittable(3,4,randi(),table);

  const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase;
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

  u8 hwdel=0; u8 effectmod=1;
  u16 hwpos=0,hwposss,wrapper;
  u8 stack_pos=0;
  u8 stack_posy=0;
  u16 start,wrap;

  struct stackey stackyy[STACK_SIZE];
  struct stackey stackyyy[STACK_SIZE];
  u16 *buf16 = (u16*) datagenbuffer;
  u8 *audio_buf = (u8*) audio_buffer;
  u8 leakiness=randi()%255;
  u8 infection=randi()%255;

#ifndef LACH
  dohardwareswitch(2,0);
#endif

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
    settingsarray[x]=32768;//>>15
  }//DIR

  settingsarray[51]=0; //EFFECTS
  settingsarray[52]=0;
  settingsarray[53]=0;
	 
  // CPUintrev2:
  for (x=0; x<100; x++) // was 100
    {
      addr=randi()<<3;
      cpustackpush(m,datagenbuffer,addr,randi()<<3,randi()%31,randi()%24);
    }

  //pureleak

  for (x=0;x<100;x++){
    addr=randi()<<3;
    cpustackpushhh(datagenbuffer,addr,randi()<<3,randi()%31,randi()%24);
  }

  // CA
  for (x=0;x<STACK_SIZE;x++){
    start=randi()<<3;
    wrap=randi()<3;
    stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,randi()%24,start,wrap); 
    villagestackpos=villagepush(villagestackpos,start,wrap);
  }

  //simulationforstack:	
    for (x=0;x<STACK_SIZE;x++){
  //  for (x=0;x<2;x++){ // TESTY!
    start=randi()<<3;
    wrap=randi()<3;
    stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,randi()%24,start,wrap);
    villagestackpos=villagepush(villagestackpos,start,wrap);
  }

  start=0;

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


      /////////////////////////////
      // KKNOBBBSSS

      // TODO: should these be before or after mirroring-TEST!

#ifdef LACH
	  //SAMPLEWRAP (out-play) TO TEST!!

	  for (x=0;x<14;x++){
	    settingsarray[11+x]=adc_buffer[FIRST]<<4; // 16 bit value
	  }

      effects=adc_buffer[SECOND]>>5;  // 7 bits

      /*      if (effectmod&1 || effectmod&4) settingsarray[51]=effects<<9;
	      if (effectmod&2) settingsarray[52]=effects<<9; // WRITE=PLAY*/

      // TESTY

      if (effectmod&1 || effectmod&4) EFFECTREAD=effects<<9;
      if (effectmod&2) EFFECTWRITE=effects<<9; // WRITE=PLAY*/

#else
      tmphardware=0;
      for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIRST]>>5; // 7 bits
      }
            HARDWARE=tmphardware>>8; //was >>8 to divide average
      //      hardware=rand()%127; // TESTY!
            effects=adc_buffer[SECOND]>>5;  // 7 bits

	    /*	    if (digfilterflag&1){
      if (effectmod&1) settingsarray[51]=effects<<9;
      if (effectmod&2) settingsarray[52]=effects<<9; // WRITE=PLAY
      if (effectmod&4) settingsarray[53]=effects<<9; 
	    }
	    else
	      {
      if (effectmod&1 || effectmod&4) settingsarray[51]=effects<<9;
      if (effectmod&2) settingsarray[52]=effects<<9; // WRITE=PLAY
      }*/

	    if (digfilterflag&1){
      if (effectmod&1) EFFECTREAD=effects;
      if (effectmod&2) EFFECTWRITE=effects; // WRITE=PLAY
      if (effectmod&4) EFFECTFILTER=effects; 
	    }
	    else
	      {
      if (effectmod&1 || effectmod&4) EFFECTREAD=effects;
      if (effectmod&2) EFFECTWRITE=effects; // WRITE=PLAY
      }


#endif

      // KNOB FIVE = knobby //TODO: underused
      knobby=adc_buffer[FIFTH];
      // KNOB THREE = modifiers and fingers
      fingermod=adc_buffer[THIRD]>>7;// 5 bits=32 
      fingerfing=fingermod&1; //finger open0 or as dir
      fingermod=fingermod>>1; //4 bits=16
      //      fingermod=13; // TESTY!!!
      switch(fingermod){
      case 0:
	//effectmod
	effectmod=fingervaleff(effectmod); // return 1-7
	break;
      case 1:
	// stack
	whichstack=fingervalright(whichstack,5);
	//	whichstack=3; // TESTY!!
	//	whichstack=rand()%5; //TESTY
	if (fingerdirupdown()==1) {
	  switch (whichstack){ // which stack to push=0-4
	  case 0:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,audio_buf,stack_posy,stackmuch,STACKSTART,STACKWRAP);	    
	    break;
	  case 1:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,datagenbuffer,stack_posy,stackmuch,STACKSTART,STACKWRAP);	    
	    break;
	  case 2:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,audio_buffer,stack_pos,stackmuch,STACKSTART,STACKWRAP);
	    break;
	  case 3:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,buf16,stack_pos,stackmuch,STACKSTART,STACKWRAP);
	    break;
	  case 4:
	    villagestackpos=villagepush(villagestackpos,STACKSTART,STACKWRAP);//pos/start/wrap
			  }
	}
	else if (fingerdirupdown()==0){
	  ///	  whichstack=1; // TESTY!
	  switch (whichstack%3){ // which stack to pop=0-3
	  case 0:
	    stack_posy=ca_pop(stackyyy,stack_posy);
	    break;
	  case 1:
	    	    stack_pos=func_pop(stackyy,stack_pos);
	    break;
	  case 2:
	    villagestackpos=villagepop(villagestackpos);
	  }
	}
	break;
      case 2:
	//stackmuch 
	if (fingerfing&1) stackmuch=fingervalright(stackmuch,100);
	else stackmuch=adc_buffer[UP]>>5;
      case 3:
	// direction
	whichdir=fingervalright(whichdir,10);//
	whichdiritis=(fingerdirupdown()); // was UP
	settingsarray[54+whichdir]=whichdiritis<<15;  //<<1 bit to 16
	break;
      case 4:
	// speed
	whichspeed=fingervalright(whichspeed,6);
	if (fingerfing&1) speed=fingervalup(speed);
	else speed=adc_buffer[UP]>>4;
	settingsarray[35+whichspeed]=speed<<8; // 8 bits to 16
	break;
      case 5:
	// step
	whichstep=fingervalright(whichstep,10);
	if (fingerfing&1) step=fingervalup(step);
	else step=adc_buffer[UP]>>8;
	settingsarray[25+whichstep]=step<<8; // 8 bits to 16
	break;
      case 6:
	// start 16 bits
	started=fingervalright(started,11);
	if (fingerfing&1) start=fingervalup16bits(start,32);
	else start=adc_buffer[UP]<<4;
	settingsarray[started]=start; // 16 bit value
	break;
      case 7:
	// start all
	for (x=0;x<14;x++){
	if (fingerfing&1) start=fingervalup16bits(settingsarray[x],32);
	else start=adc_buffer[UP]<<4;
	  settingsarray[x]=start; // 16 bit value
      }
	break;
      case 8:
	// wrap 16 bits
	constrained=fingervalright(constrained,15);
	if (fingerfing&1) constrain=fingervalup16bits(constrain,32);
	else constrain=adc_buffer[UP]<<4;
	settingsarray[11+constrained]=constrain; // 16 bit value
	break;
      case 9:
	// wrap all
	for (x=11;x<25;x++){
	constrain=fingervalup16bits(settingsarray[x],32);
	  settingsarray[x]=constrain; // 16 bit value
      }
	break;
      case 10:
	// exespot
	exespot=fingervalright(exespot,32);
	settingsarray[50]=exespot<<11; 
	break;
      case 11:
	cpur=fingervalright(exespot,m->m_threadcount);
	if (fingerfing&1) cpu=fingervalup(cpu);
	else cpu=adc_buffer[UP]>>4;
	m->m_threads[cpur].m_CPU=cpu%31;
	break;
      case 12:
	startrr=fingervalright(startrr,m->m_threadcount);
	if (fingerfing&1) startr=fingervalup16bits(startr,32);
	else startr=adc_buffer[UP]<<4;
	m->m_threads[startrr].m_start=startr;
	break;
      case 13:
	wraprr=fingervalright(wraprr,m->m_threadcount);
	if (fingerfing&1) wrapr=fingervalup16bits(wrapr,32);
	else wrapr=adc_buffer[UP]<<4;
	m->m_threads[wraprr].m_wrap=m->m_threads[wraprr].m_start+wrapr;
	break;
      case 14:
	// folder?
	foldback=fingervalright(foldback,44);
	if (fingerfing&1) foldbackset=fingervalup16bits(foldbackset,32);
	else foldbackset=adc_buffer[UP]<<4;
	FOLDD[foldback]=foldbackset;
	break;
      case 15:
	// fold all?
	for (x=0;x<44;x++){
	if (fingerfing&1) foldbackset=fingervalup16bits(FOLDD[x],32);
	else foldbackset=adc_buffer[UP]<<4;
	FOLDD[x]=foldbackset;
	}
      } ///end of fingermod

      // KNOB FOUR = mirrors ---> TODO: last or first??
      mirror=adc_buffer[FOURTH]>>6; // 6 bits
      // toggle repeats is top bit but how we set this 
      mirrortoggle=mirror>>5; // top bit
      mirror=mirror&31; // so now 32... fixed as kept mirrortoggle bit...
      FOLDD[44]=knobby<<4; // knob five as length of mirror
      FOLDD[1]=knobby<<4; // knob five as length of mirror???TODO


      //////////////////////////////////////////////// INTO THE MIRROR

      switch(mirror){
      case 0: // do nothing
	break;
     case 1:
       for (x=0;x<((FOLDD[1]>>10)+1);x++){ //was >>9
	 settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768];
	  coo++;
	}

       if (mirrortoggle&1) m1flag^=1;
	break;
      case 2:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=(randi()<<4);
	}
	if (mirrortoggle&1) m1flag^=2;
	break;
      case 3:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=adc_buffer[UP]<<4;
	}
	if (mirrortoggle&1) m1flag^=4;
	break;
      case 4:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=knobby<<4;
	}
	if (mirrortoggle&1) m1flag^=8;
	break;
      case 5:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]+=8;
	}
	if (mirrortoggle&1) m1flag^=16;
	break;
      case 6:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]-=8; //TODO TUNING this +-1!!!
	}
	if (mirrortoggle&1) m1flag^=32;
	break;
      case 7:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=settingsarray[(((FOLDD[2])>>10)+8+(x%((FOLDD[3]>>10)+1)))%64];
	}
	if (mirrortoggle&1) m1flag^=64;
	break;
	////////////////////////////////VILLAGER MIRROR
      case 8: // do nothing
	break;
      case 9:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  	  coo++;
	}
	if (mirrortoggle&1) m1flag^=128;
	break;
      case 10:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(randi()<<3);
	}
	if (mirrortoggle&1) m1flag^=256;
	break;
      case 11:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=adc_buffer[UP]<<3;
	}
	if (mirrortoggle&1) m1flag^=512;
	break;
      case 12:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=knobby<<3;
	}
	if (mirrortoggle&1) m1flag^=1024;
	break;
      case 13:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]+1)%32768; // TUNING +-1 TODO
	}
	if (mirrortoggle&1) m1flag^=2048;
	break;
      case 14:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]-1)%32768;
	}
	if (mirrortoggle&1) m1flag^=4096;
	break;
      case 15:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  // take start and ends from stacks TODO!	  
	  tmper=((((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))*3)%96; // div by 3 96/3=32
	  //extract stackerstart and stackerwrap
	  // start is 0 wrap is +2;
	  if (tmper<48) {
	    stackerstart=stackery[tmper];
	    stackerwrap=stackery[(tmper+2)%48];
	  }
	  else {
	    tmper=tmper-48;
	    stackerstart=stacker[tmper];
	    stackerwrap=stacker[(tmper+2)%48];
	  }
	  villager[((((FOLDD[2])>>10)*2)+(x%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerstart;
	  villager[((((FOLDD[2])>>10)*2)+((x+1)%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerwrap;
	}
	if (mirrortoggle&1) m1flag^=8192;
	break;
	////////////////////////////////STACKER MIRROR
      case 16:// do nothing
	break;
      case 17:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
		}
	  coo++;
	}
	if (mirrortoggle&1) m1flag^=16384;
      break;
      case 18:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(randi()<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(randi()<<3);
		}
	}
	if (mirrortoggle&1) m1flag^=32768;
      break;
      case 19:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=adc_buffer[UP]<<3;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=adc_buffer[UP]<<3;
		}
	}
	if (mirrortoggle&1) m2flag^=1;
      break;

      case 20:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(knobby<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(knobby<<3);
		}
	}
	if (mirrortoggle&1) m2flag^=2;
      break;
      case 21:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]+1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]+1)%32768; // TODO TUNING +-1
		}
	}
	if (mirrortoggle&1) m2flag^=4;
      break;
      case 22:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]-1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]-1)%32768; // TODO TUNING +-1
		}
	}
	if (mirrortoggle&1) m2flag^=8;
      break;
      case 23:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=stackery[(tmper+3)%48];
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=stacker[(tmper+3)%48];
		}
	}
	if (mirrortoggle&1) m2flag^=16;
      break;

      ////////////////////////////////MIRROR | MIRROR
      // but can't short circuit itself
      case 24:
	break; // do nothing
      case 25:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[44]>>10)+1)))%32768];
	  coo++;
		}
	//	if (mirrortoggle&1) mirrorflag^=2097152;
	break;
      case 26:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=(randi()<<4);
		}
	//	if (mirrortoggle&1) mirrorflag^=4194304;
	break;
      case 27:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=adc_buffer[UP]<<4;
		}
	//	if (mirrortoggle&1) mirrorflag^=8388608;
	break;
      case 28:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=knobby<<4;
		}
	//	if (mirrortoggle&1) mirrorflag^=16777216;
	break;
      case 29:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]+=1; // TODO tuning! and below
		}
	//	if (mirrortoggle&1) mirrorflag^=33554432;
	break;
      case 30:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]-=1;
		}
	//	if (mirrortoggle&1) mirrorflag^=67108864;
	break;
      case 31:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=FOLDD[((tmper+1)%44)];
		}
      } // end of mirror switch
      //	if (mirrortoggle&1) mirrorflag^=134217700;

      ////////////////////////////////////////////////MIRROR ACTIONZ
	//// leave as 2 and 3 and inc here!

      //      /* for mirror
            
      if (m1flag&1){ //skip 0
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[4])>>10)+(x%((FOLDD[5]>>10)+1)))%64]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768];
	  coo++;
	}
      }

      if (m1flag&2){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[6])>>10)+(x%((FOLDD[7]>>10)+1)))%64]=(randi()<<4);
	}
    }

      if (m1flag&4){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[8])>>10)+(x%((FOLDD[9]>>10)+1)))%64]=adc_buffer[UP]<<4;
	}
}

      if (m1flag&8){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[10])>>10)+(x%((FOLDD[11]>>10)+1)))%64]=knobby<<4;
	}
}

      if (m1flag&16){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[12])>>10)+(x%((FOLDD[13]>>10)+1)))%64]+=8;
	}
}

      if (m1flag&32){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[14])>>10)+(x%((FOLDD[15]>>10)+1)))%64]-=8; //TODO TUNING this +-1!!!
	}
}

      if (m1flag&64){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[16])>>10)+(x%((FOLDD[17]>>10)+1)))%64]=settingsarray[(((FOLDD[2])>>10)+8+(x%((FOLDD[3]>>10)+1)))%64];
	}
}

      if (m1flag&128){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[18])>>10)+(x%((FOLDD[19]>>10)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  	  coo++;
	}
}

      if (m1flag&256){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[20])>>10)+(x%((FOLDD[21]>>10)+1)))% VILLAGE_SIZE]=(randi()<<3);
	}
}

      if (m1flag&512){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[22])>>10)+(x%((FOLDD[23]>>10)+1)))% VILLAGE_SIZE]=adc_buffer[UP]<<3;
	}
}

      if (m1flag&1024){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[24])>>10)+(x%((FOLDD[25]>>10)+1)))% VILLAGE_SIZE]=knobby<<3;
	}
}

      if (m1flag&2048){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[26])>>10)+(x%((FOLDD[27]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]-1)%32768;
	}
}

      if (m1flag&4096){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[26])>>10)+(x%((FOLDD[27]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)-1)))% VILLAGE_SIZE]-1)%32768;
	}
}

      if (m1flag&8192){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  // take start and ends from stacks TODO!	  
	  tmper=((((FOLDD[28])>>10)+(x%((FOLDD[29]>>10)+1)))*3)%96; // div by 3 96/3=32
	  //extract stackerstart and stackerwrap
	  // start is 0 wrap is +2;
	  if (tmper<48) {
	    stackerstart=stackery[tmper];
	    stackerwrap=stackery[(tmper+2)%48];
	  }
	  else {
	    tmper=tmper-48;
	    stackerstart=stacker[tmper];
	    stackerwrap=stacker[(tmper+2)%48];
	  }
	  villager[((((FOLDD[2])>>10)*2)+(x%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerstart;
	  villager[((((FOLDD[2])>>10)*2)+((x+1)%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerwrap;
	}
}

      //skip 0
      if (m1flag&16384){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[30])>>10)+(x%((FOLDD[31]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
		}
	  	  coo++;
	}
}

      if (m1flag&32768){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[32])>>10)+(x%((FOLDD[33]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(randi()<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(randi()<<3);
		}
	}
}

      if (m2flag&1){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[34])>>10)+(x%((FOLDD[35]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=adc_buffer[UP]<<3;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=adc_buffer[UP]<<3;
		}
	}
}

      if (m2flag&2){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[36])>>10)+(x%((FOLDD[37]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(knobby<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(knobby<<3);
		}
	}
}

      if (m2flag&4){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[38])>>10)+(x%((FOLDD[39]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]+1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]+1)%32768; // TODO TUNING +-1
		}
	}
}

      if (m2flag&8){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[40])>>10)+(x%((FOLDD[41]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]-1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]-1)%32768; // TODO TUNING +-1
		}
	}
}

      if (m2flag&16){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[42])>>10)+(x%((FOLDD[43]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=stackery[(tmper+3)%48];
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=stacker[(tmper+3)%48];
		}
	}
}
	/// mirror miror or not??? no as can short circuit?
            	
      ////////////////////////////////////////////////END OF MIRROR



#ifndef LACH
      /////////////////////////////////////
      // 4-hardware operations

      /// general HW walk in/as tmp
      if (++hwdel>=HWSPEED){
	  hwpos+=(HWSTEP*hwdir[HWDIR]);
	  tmp=(HWSTART+(hwpos%HWWRAP))%32768; //to cover all directions
	  hwdel=0;
	  hwposss+=(HWSTEP*hwdir[HWDIR]);
	  tmppp=(HWSTART+(hwposss%HWWRAP))%32768; //to cover all directions
	  }

      if (digfilterflag&16){
	hdtmp=(HWSTART+(hwpos%HWWRAP)); 
	dohardwareswitch(HARDWARE,HDGENERBASE+(datagenbuffer[hdtmp]%HDGENERCONS));
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
