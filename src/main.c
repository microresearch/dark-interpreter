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

int16_t newdir[4]={-1,1};
int16_t direction[4]={-1,1};
int16_t villagedirection[4]={-1,1};
int16_t villagedirectionf[4]={-1,1};
int16_t villagedirectionw[4]={-1,1};
int16_t newdirf[4]={-1,1};
int16_t directionf[4]={-1,1};
int16_t newdirread[4]={-1,1};
int16_t directionread[4]={-1,1};

u8 villagestackpos=0;
u16 villager[129];
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

u8 testdirection;
u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array
u16 settingsarray[71];

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
  u8 result=2;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[LEFT]>>8;
  handright=adc_buffer[RIGHT]>>8;
  if (handleft>8) left++;
  if (handright>8) right++;
  //  handcount++;
  if (left>8 && left>right) {
    //    up=0;down=0;left=0;right=0;
    result=0;
  }
  else if (right>8 && right>left) {
    //    up=0;down=0;left=0;right=0;
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
  //  handcount++;
  if (left>8 && left>right) {
    //    up=0;down=0;left=0;right=0;
    result=0;
  }
  else if (right>8 && right>left) {
    //    up=0;down=0;left=0;right=0;
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
  u16 *buf; u8 *buff;
  u16 x,addr,tmp,tmppp,hdtmp,tmphardware;
  u16 settings,setted;
  u8 oldsettings,oldsetted,tmper;
  u8 mirror=0,villagemirror=0,mirrordel=0, fingermod,oldfingermod,whichdir,whichdiritis,speed,whichspeed,foldback; 
  u16 constrain,foldbackset;
  u8 effects,effectmod=0;
  u8 machine_count=0,leak_count=0; 
  u8 exeperms[88]={0,1,2,3, 0,1,3,2, 0,2,3,1 ,0,2,1,3, 0,3,1,2, 0,3,2,1, 1,0,2,3, 1,0,3,2, 1,2,3,0, 1,2,0,3, 1,3,2,0, 1,3,0,2, 2,1,0,3, 2,1,3,0, 2,3,1,0, 2,3,0,1, 3,0,1,2, 3,0,2,1, 3,1,0,2, 3,1,2,0, 3,2,0,1, 3,2,1,0}; 

  int16_t hwdir[2]={1,-1};

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
  Codec_Init(16000); // was 48000
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
    u16 hwpos=0,hwposss,wrapper;
	 
  u8 stack_pos=0;
  u8 stack_posy=0;
  u8 whichstack=0;
  u16 start,wrap;

  struct stackey stackyy[STACK_SIZE];
  struct stackey stackyyy[STACK_SIZE];
  u16 *buf16 = (u16*) datagenbuffer;
  u8 *audio_buf = (u8*) audio_buffer;

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
      cpustackpush(m,datagenbuffer,addr,addr+randi(),randi()%31,randi()%100);
    }

  //pureleak

  for (x=0;x<100;x++){
    addr=randi()<<3;
    cpustackpushhh(datagenbuffer,addr,addr+randi(),randi()%31,randi()%100);
  }

  // CA
  for (x=0;x<STACK_SIZE;x++){
    start=randi()<<3;
    wrap=randi()<3;
    stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,randi()%100,start,wrap); 
    villagestackpos=villagepush(villagestackpos,start,wrap);
  }

  //simulationforstack:	
  for (x=0;x<STACK_SIZE;x++){
    start=randi()<<3;
    wrap=randi()<3;
    stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,randi()%100,start,wrap);
    villagestackpos=villagepush(villagestackpos,start,wrap);
  }

  ///////////////////////////

  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
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
	    m->m_leakiness=LEAKINESS;
	    m->m_infectprob=INFECTION;
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

#ifdef LACH
	  //SAMPLEWRAP (out-play) TO TEST!!

	  for (x=0;x<14;x++){
	    settingsarray[11+x]=adc_buffer[FIRST]<<4; // 16 bit value
	  }

      effects=adc_buffer[SECOND]>>5;  // 7 bits
      //      effectmod=1; // TESTY!
      if (effectmod&1) settingsarray[51]=effects<<9; // READ IN 
      if (effectmod&2 || effectmod&4) settingsarray[52]=effects<<9; // WRITE=PLAY
#else
      tmphardware=0;
      for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIRST]>>5; // 7 bits
      }
            settingsarray[64]=tmphardware<<1; //was >>8 // TESTY!
      //      hardware=rand()%127; // TESTY!
            effects=adc_buffer[SECOND]>>5;  // 7 bits
	    //      effects=rand()%128;
      //      effectmod=rand()%8; // TESTY!
      if (effectmod&1) settingsarray[51]=effects<<9; // READ IN 
      if (effectmod&2) settingsarray[52]=effects<<9; // WRITE=PLAY
      if (effectmod&4) settingsarray[53]=effects<<9; // FILTER
#endif
      
      ////// KNOB THIRD - mod for fingers // foldback
      fingermod=adc_buffer[THIRD]>>6; // 64=6 bits//TESTY!
      // fingermod=rand()%64; // TESTY FORCRASH!
      // 1-mod effectmod 0-7
      if (fingermod<8){
	effectmod=fingervalright(effectmod,8);
      }
      // 2-push/pop with template settings and type fronm left/right
                  else if (fingermod<16){ // TODO: move stack to first position above
		    whichstack=fingervalright(whichstack,5);
		    //		    whichstack=rand()%5; TESTY
		    if (fingerdirupdown()==1) {
		      //if ((rand()%2)==1){
		  	  switch (whichstack){ // which stack to push=0-4
	  case 0:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,audio_buf,stack_posy,STACKMUCH,STACKSTART,STACKWRAP);	    
	    break;
	  case 1:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,datagenbuffer,stack_posy,STACKMUCH,STACKSTART,STACKWRAP);	    
	    break;
	  case 2:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,audio_buffer,stack_pos,STACKMUCH,STACKSTART,STACKWRAP);
	    break;
	  case 3:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,buf16,stack_pos,STACKMUCH,STACKSTART,STACKWRAP);
	    break;
	  case 4:
	    villagestackpos=villagepush(villagestackpos,STACKSTART,STACKWRAP);//pos/start/wrap
	    break;
			  }
		    }
		  }
		  else if (fingerdirupdown()==0){
			  //else {
	  switch (whichstack%3){ // which stack to pop=0-
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


      // 3-directions???
      // up/down selects and other sets dir or other way round here?
      else if (fingermod<24){
	//54])>>15 directions are 54 to 65==12
	whichdir=fingervalright(whichdir,12);//
	whichdiritis=(fingerdirupdown()); // was UP
	settingsarray[54+whichdir]=whichdiritis<<15;  //<<1 bit to 16
      }

      // 4-speed
      // select and expand or contract
      else if (fingermod<28){
	// speeds are: 35->40==6
	whichspeed=fingervalright(whichspeed,6);
	speed=fingervalup(speed);
	settingsarray[35+whichspeed]=speed<<8; // 8 bits to 16
      }

      else if (fingermod<32){
	// second half is group expand and contract
	speed=fingervalup(speed);
	for (x=0;x<6;x++){
	settingsarray[35+x]=speed<<8;
	}
      }

      // 5-micro-macro - expand or contract
      else if (fingermod<40){
	// change wraps [11-24]
	constrain=fingervalup16bits(constrain,32);
	for (x=0;x<14;x++){
	  settingsarray[11+x]=constrain; // 16 bit value
      }
      }
      // 6-is foldback with finger settings - MIRROR!!!!
            else {
	// action is from knob 40-64
	// finger-> foldback settings 66-70

	      foldback=fingervalright(foldback,5);
	foldbackset=fingervalup16bits(foldbackset, 32);	
	settingsarray[66+foldback]=foldbackset;
	//      mirror: - all as action and one off toggle on off
	if (oldfingermod!=fingermod){
	// 1-datagen to region of settings:
	//copy region of datagen to settings
	if (fingermod<42){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768];
			}
	mirror^=1;
	}	
	// 2-datagen to region of stack:
	else if (fingermod<44){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 64- so just wrap on 96)
	  if (tmper<48) stackery[tmper]=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768];
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768]>>1;
		}
	}
	mirror^=2;
	}

	else if (fingermod<46){ // TODO TEST/jitter??? maybe +-1 also?
	
	  for (x=0;x<FOLDSWRAP;x++){ // villager array
	    villager[(FOLDDSTART+(x%FOLDDWRAP))%128]=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768]>>1;
			}
	villagemirror^=1;
	}	
	
	// 3-adc_buffer[9] to region of settings:// no ifdef!
	else if (fingermod<48){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]=randi()<<4;
			}
	mirror^=4;
	}	
	// 4-adc_buffer[9] to region of stack:

	else if (fingermod<50){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 64- so just wrap on 96)
	  	  if (tmper<48) stackery[tmper]=randi()<<4;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(randi()<<3)%32768; // 15 bits
	    }
		}
	mirror^=8;
	}	

	//      5-inc a region of settings:
	else if (fingermod<52){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]+=8;
			}
	mirror^=16;
	}	

	//      6-inc a region of stack:
	else if (fingermod<54){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 64- so just wrap on 96)
	  	  if (tmper<48) stackery[tmper]=stackery[tmper]+8;
	  else {
	    tmper=tmper-48;
	    	    stacker[tmper]=(stacker[tmper]+8)%32768;
		    }
	}
	mirror^=32;
	}	

	else if (fingermod<56){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  	  villager[(FOLDDSTART+(x%FOLDDWRAP))%128]=(villager[(FOLDDSTART+(x%FOLDDWRAP))%128]+8)%32768;
			}
	villagemirror^=16;
	}	

	//      7-reduce a region of settings:
	else if (fingermod<58){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	    settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]-=8;
			}
	mirror^=64;
	}	

	//      8-reduce a region of stack:

	else if (fingermod<60){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 64- so just wrap on 96)
	  	  if (tmper<48) stackery[tmper]=stackery[tmper]-8;
	  else 
	    {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]-8)%32768;
	    }
		}
	mirror^=128;
	}	


	else if (fingermod<62){ // TODO TEST/jitter??? maybe +-1 also?
	
	for (x=0;x<FOLDSWRAP;x++){
	  villager[(FOLDDSTART+(x%FOLDDWRAP))%128]=(villager[(FOLDDSTART+(x%FOLDDWRAP))%128]-8)%32768;
			}
	villagemirror^=128;
	}	


	//	fingermod=64; mirror=1;// TESTY!!!
	else { // TODO TEST/jitter??? maybe +-1 also?
		for (x=0;x<7;x++){
	  tmper=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768];
	  settingsarray[x+64]=tmper; // TESTER-was big crash!
		}
	}	
      }
      }
      oldfingermod=fingermod;
     

      ////////////////////////////////////////////////////////////////////////////////////
      
            if (++mirrordel>=FOLDSPEED){

      if (mirror&1){
	for (x=0;x<FOLDSWRAP;x++){
	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768];
			}
      }

      if (mirror&2){
	for (x=0;x<FOLDSWRAP;x++){
 	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 48- so just wrap on 96)
	  if (tmper<48) stackery[tmper]=buf16[(FOLDSSTART+(x%FOLDSWRAP)%32768)]>>1;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=buf16[(FOLDSSTART+(x%FOLDSWRAP)%32768)]>>1;
	  }
	}
      }

      if (mirror&4){
	for (x=0;x<FOLDSWRAP;x++){
	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]=randi()<<3;
			}
	}	

      if (mirror&8){	
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 48- so just wrap on 96)
	  if (tmper<48) stackery[tmper]=randi()<<4;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(randi()<<3)%32768;
	  }
		}
	}	

      if (mirror&16){	
	for (x=0;x<FOLDSWRAP;x++){
	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]+=8;
			}
      }

      if (mirror&32){	
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 48- so just wrap on 96)
	  if (tmper<48) stackery[tmper]=(stackery[tmper]+8)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]+8)%32768;
	  }
		}
	}	

      if (mirror&64){
	for (x=0;x<FOLDSWRAP;x++){
	  settingsarray[(FOLDDSTART+(x%FOLDDWRAP))%64]-=8;
			}
	}	

      if (mirror&128){
	for (x=0;x<FOLDSWRAP;x++){
	  tmper=(FOLDDSTART+(x%FOLDDWRAP))%96;
	  // into stackery stacker (both 48- so just wrap on 96)
	  if (tmper<48) stackery[tmper]=stackery[tmper]-8;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]-8)%32768;
		}
	}
      }

      if (villagemirror&1){
	  for (x=0;x<FOLDSWRAP;x++){ // villager array
	  villager[(FOLDDSTART+(x%FOLDDWRAP))%128]=buf16[(FOLDSSTART+(x%FOLDSWRAP))%32768]>>1;
			}
      }

      if (villagemirror&16){
for (x=0;x<FOLDSWRAP;x++){
	  villager[(FOLDDSTART+(x%FOLDDWRAP))%128]=(villager[(FOLDDSTART+(x%FOLDDWRAP))%128]+8)%32768;
			}
      }

      if (villagemirror&128){
	for (x=0;x<FOLDSWRAP;x++){
	  villager[(FOLDDSTART+(x%FOLDDWRAP))%128]=(villager[(FOLDDSTART+(x%FOLDDWRAP))%128]-8)%32768;
			}
      }
      mirrordel=0;
            }
      ////////////////////////////////////////////////////////////////////////////////////

      //////// KNOB FOURTH - settings X and KNOB FIFTH - settings Y // foldback settings?
      // TODO; not q right... maybe averaging or???
	    setted=0;settings=0;
	    for(x=0;x<16;x++){
      setted+=adc_buffer[FOURTH]>>6; // 64
      //      setted=12; // TESTY!!! samplewrap=12,hwspeed=35,machinespeed=40
      settings+=adc_buffer[FIFTH]>>6;// 4 bits here
	    }	
	    setted=setted>>4;
	    settings=settings>>4;

      //      settingsarray[setted]=settings<<8; // TESTY!!!

      if (setted==oldsetted && settings!=oldsettings){// && settings!=oldsettings+1 && settings!=oldsettings-1){
	settingsarray[setted]=adc_buffer[FIFTH]<<4;
      }
      oldsettings=settings;oldsetted=setted;

      ////////////////////////////////////////////////
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
	hdtmp=(HWSTART+(hwpos%wrapper)); 
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
