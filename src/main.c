/*

 Oh, eternity with outstretched wings, that broodest over the secret
 truths in whose roots lie the mysteries of man—his whence, his
 whither—have I searched thee, and struck a right key on thy dreadful
 organ!

 [Thomas de Quincey. The Dark Interpreter] 

 */

 /* first steps for the dark interpreter based on work by: */
 /* E. Brombaugh 07-31-2012                                        */
 /* M. Howse 01.2014 */

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

#define MAX_EXE_STACK 8

//#define randi() (adc_buffer[9])
//#define randi() rand()
#define randi() (datagenbuffer[adc_buffer[9]<<4])

 /* DMA buffers for I2S */
 __IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

 /* DMA buffer for ADC  & copy */
 __IO uint16_t adc_buffer[10];

 //uint16_t datagenbuffer[32] __attribute__ ((section (".ccmdata")))  __attribute__((aligned(4)));

extern int16_t audio_buffer[AUDIO_BUFSZ];

u8* datagenbuffer = (u8*)0x10000000;

extern u8 digfilterflag;

u8 wormdir; // worm direction
//u8 cons; //constraint/grain size used in audio.c
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array
u16 settingsarray[64];
//u8 steppy; // TODO_ testing

 struct dgenwalker{
   // xxxx(samp/hard/clocks)->step,position,direction(into array),speed,start,end 
   u8 step,dir,speed,del,wormflag;
   u16 pos;
 };


 #define delay()						\
 do {							\
   register unsigned int i;				\
   for (i = 0; i < 1000000; ++i)				\
     __asm__ __volatile__ ("nop\n\t":::"memory");	\
 } while (0)

 #define delay2()						\
 do {							\
   register unsigned int i;				\
   for (i = 0; i < 10000; ++i)				\
     __asm__ __volatile__ ("nop\n\t":::"memory");	\
 } while (0)


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

 void main(void)
 {

   // order that all inits and audio_init called seems to be important
   u16 x,addr,tmp;
   u8 oldhardware,hardware,tmphardware;
   u8 effects;
   u16 speedwrapper=0;
   u16 genspeed, hardspeed,hardcount;
   u16 cpuspeed,cpucount;
   u8 exenums=0,machine_count=0,leak_count=0; 
   u8 exestack[MAX_EXE_STACK];
   u8 settings, oldsettings=0,settings_trap=0,setted,settingsindex=0;
   u8 pushypop,pushpopflag=0;
   u8 index=0, finaldel=0;
   u16 finalpos=0;

   //	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); //FPU - but should be in define

   //   u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768
   //   u16 direction8bit[8]={65279,65280,1,257,256,255,65534,65278}; // for 8 bits into counter

	// set dirs
   int16_t hddir[4]={-180,1,180,-1};
   int16_t lmdir[4]={-180,1,180,-1};
   int16_t mxdir[4]={-180,1,180,-1};
   int16_t dir40106[4]={-180,1,180,-1};

   u8 sstt=0,ttss=0,tempsetting=0,handup, oldhandup, handdown, oldhanddown;
   u8 mirror;
   u8 handleft, handright, oldhandleft,oldhandright,up=0,down=0,left=0,right=0,handcount=0;

   inittable(3,4,randi()<<4,table);

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
	 //	x=rcount=i=wcount=highest=lowest=0;

	 // init machine/datagens
	 machine *m=(machine *)malloc(sizeof(machine));
	 //	 machine_create(m,(u8 *)(datagenbuffer)); // this just takes care of pointer to machine and malloc for threads

   m->m_threadcount=0;
   m->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED



	 u8 hdgenerdel=0,lmerdel=0,f0106erdel=0,maximerdel=0;
	 u16 hdgenerpos=0,lmerpos=0,f0106erpos=0,maximerpos=0,wrapper;
	 
	 u8 stack_pos=0;
	 u8 stack_posy=0;

	 struct stackey stackyy[STACK_SIZE];
	 struct stackey stackyyy[STACK_SIZE];
	 u16 *buf16 = (u16*) datagenbuffer;
	 u8 *audiobuf = (u8*) audio_buffer;

 #ifndef LACH
	 dohardwareswitch(2,0);
 #endif

////////////////////TESTCODE_TODO_REMOVE - but do replace with some
////////////////////minimal setup code to get started
	 //TESTER!

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


	 // CPUintrev2:
	 for (x=0; x<100; x++)
	   {
	     addr=randi()<<4;
	     cpustackpush(m,datagenbuffer,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
		     // cpustackpush(m,audiobuf,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
	   }

	 //pureleak

  for (x=0;x<MAX_FRED;x++){
    addr=randi()%65536;
    cpustackpushhh(datagenbuffer,addr,addr+randi()%65536,randi()%31,1);
  }

	 // CA
	 for (x=0;x<STACK_SIZE;x++){
	   stack_posy=ca_pushn(stackyyy,rand()%NUM_CA,datagenbuffer,stack_posy,100,0,32767);//howmuch,start,wrap 
	   }


	 	 //simulationforstack:	
	 for (x=0;x<STACK_SIZE;x++){
	   stack_pos=func_pushn(stackyy,rand()%NUM_FUNCS,buf16,stack_pos,10,0,rand()%32768);//howmuch,start,wrap 
	   //	   stack_pos=func_pushn(stackyy,28,buf16,stack_pos,10,0,32767);//howmuch,start,wrap
	 }


	 // execution stack
	 for (x=0;x<MAX_EXE_STACK;x++){
	   exenums=exestackpush(exenums,exestack,2); //exetype=0-3;
	 }

	 exenums=exestackpop(exenums,exestack);

///////////////////////////

	  while(1)
	  {

#ifdef TEST_STRAIGHT

	    // nothing???

#else


	    //0- speed knob=hardware speed, cpu speed(shift>>)=generic speed, grainsize (>>also)
	    //	    genspeed=adc_buffer[0]>>5;
 	    cpuspeed=genspeed; //TODO! tune speeds
	    hardspeed=genspeed;//<<2; // 14 bits
	    //	    cpuspeed=1; hardspeed=1;
	    if (MACHINESPEED==0) MACHINESPEED=1;
	    if (LEAKSPEED==0) LEAKSPEED=1;
	    cpucount++;

//	    	    if (cpucount>=cpuspeed){ 	  //TESTER!
	      cpucount=0;

	      for (x=0;x<exenums;x++){
		switch(exestack[x]%4){
		case 0:
		  func_runall(stackyy,buf16,stack_pos); // simulations
		  break;
		case 1:
		  ca_runall(stackyyy,datagenbuffer,stack_posy); // CA
		  break;
		case 2:
		  machine_count++;
		  if (machine_count>=MACHINESPEED){
		  machine_run(m); //cpu - WRAP own speedTODO
		  m->m_leakiness=LEAKINESS;
		  m->m_infectprob=INFECTION;
		  m->m_mutateprob=MUTATION;
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
	      //	    }
	 
	    
	      // TODO:3-deal with settingsarray - should this be in slow/speed loop?

	      // KNOBS->>>>
	      // LACH: NO HARDWARE so need re-assign 
	      // SUSP (and LACH): 3,0,2,4,1
	      // TENE: 2,0,3,4,1 

	      // 0-4 top down

	      // 0=mirror left///right selector and ops across all knobbed settings/feedbacks
	      // 1=hardware///1=filterops/effects
	      // 2=push///2=pull-datagen ops and actions _or_ 2=settings Y
	      // 3=speed///3=micro-macro
	      // 4=settingsX///fingers on 0/dir//4=ops on settings array/foldbacks/feedbacks


	      /*
#ifdef TENE
	      //0-mirror settings

	      mirror=adc_buffer[2]>>4; // 8 bits or less?
	      // how we divide up mirror?

	      //1-hardware
	      // set hardware for below...
	      tmphardware=adc_buffer[0]>>5; //7 bits but what of jitter?
	      if (mirror<128 && tmphardware!=effects){
		hardware=tmphardware;
	      }
	      else
	      //1-filterops/effects
	      if (mirror>=128 && tmphardware!=hardware){
		effects=tmphardware;
	      }


	      //2-push/pop
	      //knob as 0 to push, 255 to pop with settings divided inbetween
	      //type(which,func,exetype)//howmuch//start//wrap(check>???) 	   	   
	      //also which buffer to attack as bitwise option

	      //cpustackpush(m,buffer,addr,addr+(randi()<<4),randi()%31,1);
	      //cpustackpushhh(datagenbuffer,addr,addr+randi()%65536,randi()%31,1);
	      //stack_posy=ca_pushn(stackyyy,rand()%NUM_CA,datagenbuffer,stack_posy,100,0,32767);
	      //stack_pos=func_pushn(stackyy,rand()%NUM_FUNCS,buf16,stack_pos,10,0,32767);
	      //exenums=exestackpush(exenums,exestack,exetype); //exetype=0-3;

	      //3-speed -global action on all speed settings

	      //3-micro-macro - constraints and expansion

	      //4-settings-all finger at 0

	      //4-foldback

#endif

#ifdef SUSP
	      // as above but different knob alignments


#endif

#ifdef LACH
	     // diff knobs and no hardware

	      //1- 1-
	      // depending on quarter mirror as 2x start.edge read.write


#endif
		    
	      */
#ifndef LACH

	   // 4-hardware operations
	      hardware=adc_buffer[2]>>5; // TESTER!!
	      //	    hardcount++;
	      //	    if (hardcount>=hardspeed){
	      	      hardcount=0;

	  // do hardware datagen walk into hdgen (8 bit) if flagged
	    	     if (digfilterflag&16){ // if we use hdgen at all
		       if (HDGENERSPEED==0) HDGENERSPEED=1;
		       if (HDGENERSTEP==0) HDGENERSTEP=1;
		       if (++hdgenerdel==HDGENERSPEED){
	      // 8 bitdir so leave as is
	      	      hdgenerpos+=(HDGENERSTEP*hddir[HDGENERDIR]);
		      wrapper=HDGENERWRAP; // can go 65536
		      if (wrapper==0) wrapper=1;
		      //	      	      tmp=HDGENERSTART+(hdgenerpos%wrapper);
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
	      //	      if ((F0106ERSTART+wrapper)>AUDIO_BUFSZ) wrapper=AUDIO_BUFSZ-F0106ERSTART;
	      if (wrapper==0) wrapper=1;
	      // tmp=F0106ERSTART+(f0106erpos%wrapper);
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
		      //	      if ((LMERSTART+wrapper)>AUDIO_BUFSZ) wrapper=AUDIO_BUFSZ-LMERSTART;
	      if (wrapper==0) wrapper=1;
	      //	      	      x=LMERSTART+(lmerpos%wrapper);
	      x=(LMERSTART+(lmerpos%wrapper))%32768; //to cover all directions

	      	      lmerpos+=(LMERSTEP*lmdir[LMERDIR]);
	      	      wrapper=LMERWRAP;
		      //	      if ((LMERSTART+wrapper)>AUDIO_BUFSZ) wrapper=AUDIO_BUFSZ-LMERSTART;
	      if (wrapper==0) wrapper=1;
	      //	      tmp=LMERSTART+(lmerpos%wrapper);
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
		      //	      	      if ((MAXIMERSTART+wrapper)>AUDIO_BUFSZ) wrapper=AUDIO_BUFSZ-MAXIMERSTART;
	      if (wrapper==0) wrapper=1;
	      //	      	      x=(MAXIMERSTART+(maximerpos%wrapper))%32768;
	      x=(MAXIMERSTART+(maximerpos%wrapper))%32768; //to cover all directions
	      //	      u8 constraint=8;
	      //	      setmaximpwm(200); 
	      maximerdel=0;
	      setmaximpwm(MAXIMERBASE+(buf16[tmp]%MAXIMERCONS)); //TODO add constraints here!!! ><  //TESTER!
	    }

	  }
	  //	  } // hardcount
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
