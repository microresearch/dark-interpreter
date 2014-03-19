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

#define randi() (adc_buffer[9])
  //#define randi() rand()
  //define randi() (datagenbuffer[adc_buffer[9]<<4])

 /* DMA buffers for I2S */
 __IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

 /* DMA buffer for ADC  & copy */
 __IO uint16_t adc_buffer[10];

 //uint16_t datagenbuffer[32] __attribute__ ((section (".ccmdata")))  __attribute__((aligned(4)));

u8* datagenbuffer = (u8*)0x10000000;

extern u8 digfilterflag;

u8 wormdir; // worm direction
u8 cons; //constraint/grain size used in audio.c
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array
u8 settingsarray[64];
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

 u16 sampel;

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
   u16 x,addr,tmp,oldhardware,hardware; 
   u16 speedwrapper=0;
   u16 genspeed=1, hardspeed,hardcount;
   u8 cpuspeed,cpucount;
   u8 exenums=0,machine_count=0,leak_count=0; 
   u8 exestack[MAX_EXE_STACK];
   u8 settings, oldsettings=0,settings_trap=0,setted,settingsindex=0;
   u8 pushypop,pushpopflag=0;


   //	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); //FPU - but should be in define

   u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768
   u16 direction8bit[8]={65279,65280,1,257,256,255,65534,65278}; // for 8 bits into counter

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
      yi= 2047*sinf(phase); // was 2047
      phase=phase+w;
      sign_samp=2047+yi;     // dc offset translated for a 12 bit DAC
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
	 machine_create(m,(u8 *)(datagenbuffer)); // this just takes care of pointer to machine and malloc for threads

	 u8 hdgenerdel=0,lmerdel=0,f0106erdel=0,maximerdel=0;
	 u16 hdgenerpos=0,lmerpos=0,f0106erpos=0,maximerpos=0;
	 u8 stack_pos=0;
	 u8 stack_posy=0;
	 struct stackey stackyy[STACK_SIZE];
	 struct stackey stackyyy[STACK_SIZE];
	 u16 *buf16 = (u16*) datagenbuffer;

 #ifndef LACH
	 dohardwareswitch(2,0);
 #endif

////////////////////TESTCODE_TODO_REMOVE - but do replace with some
////////////////////minimal setup code to get started

	 // CPUintrev2:
	 for (x=0; x<100; x++)
	   {
	     addr=randi()<<4;
	     cpustackpush(m,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
	   }

	 	 //simulationforstack:	
	 for (x=0;x<STACK_SIZE;x++){
	   stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,1,10);
	   //	   stack_pos=func_pushn(stackyy,2,buf16,stack_pos,1,10);
	 	   }

	 // execution stack
	 for (x=0;x<MAX_EXE_STACK;x++){
	   exenums=exestackpush(exenums,exestack,0); //exetype=0-3;
	 }

	 exenums=exestackpop(exenums,exestack);

///////////////////////////

	  while(1)
	  {

 #ifdef TEST_STRAIGHT

	    // nothing???

 #else


	    //0- speed knob=hardware speed, cpu speed(shift>>)=generic speed, grainsize (>>also)
	    genspeed=adc_buffer[0];
	    cons=(genspeed>>5)&15; // granulation
	    cpuspeed=genspeed>>4; // 8 bits
	    hardspeed=genspeed<<2; // 14 bits

	    cpucount++;
	    if (cpucount>=cpuspeed){
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
	    }

	    // 3-deal with settingsarray - should this be in slow/speed loop?

	    // settingsarray, dir settings and stack push/pull to handle

	    /* start with basic code to actually set via knobs - test code TODO
	       
	       - find setting in array/or if after array this is where
	       - X knob [4] stops???  if knob has stayed the same for
	       - x amount of time then we use as setting
	       
	       - then if Y[1] is 0 the use fingers - how to set there? test code TODO
	       - or set with [1] as to where it stops

	     */
	    
	    settings=adc_buffer[4]>>6; // we have 64 settings or so!
	    if (oldsettings==settings) settings_trap++;
	    else settings_trap=0;
	    oldsettings=settings;
	    
	    if (settings_trap>64){ // threshold is 64
	      pushpopflag=0;
	      settings_trap=0;
	      //find our setting
	      settingsindex=settings;
	    }
	    // now we can set it
	    if (settings_trap>0) setted=adc_buffer[1]>>4; // 8 bits
	    if (setted==0){
	      // do finger thing for all settings/push pop etc.
	    }
	    else 
	      {
		if (settingsindex<BEFORESTACK){ 
		  settingsarray[settingsindex]=setted;
		}
		else
		  {
		    // push/pop array
		    // but how to stop repeated pushings/poppings
		    if (pushpopflag==0)
		      {
			//first 4 is push of each// last is pop
			pushypop=settingsindex-BEFORESTACK; 
			switch(pushypop){
			case 0:
			  stack_pos=func_pushn(stackyy,PUSHONE8BIT%NUM_FUNCS,buf16,stack_pos,PUSHTWO8BIT,PUSHTHREE8BIT);
			  break;
			case 1:
			  stack_posy=ca_pushn(stackyyy,PUSHONE8BIT%NUM_CA,datagenbuffer,stack_posy,PUSHTWO8BIT,PUSHTHREE8BIT);
			  break;
			case 2:
			  cpustackpush(m,PUSHONE16BIT,PUSHTWO16BIT,PUSHONE8BIT%31,PUSHTWO8BIT);
			  break;
			case 3:
			  cpustackpushhh(datagenbuffer,PUSHONE16BIT,PUSHTWO16BIT,PUSHONE8BIT%31,PUSHTWO8BIT);
			  break;
			  ///
			case 4:
			  stack_pos=func_pop(stackyy,stack_pos);
			  break;
			case 5:
			  stack_posy=ca_pop(stackyyy,stack_posy);
			  break;
			case 6:
			  cpustackpop(m);
			  break;
			case 7:
			  cpustackpoppp(datagenbuffer);
			  break;
			case 8:
			  exenums=exestackpush(exenums,exestack,EXESTACKPUSH);		  
			  break;
			case 9:
			  exenums=exestackpop(exenums,exestack);
			}
			pushpopflag=1;
		      }
		  }
	      }

	 /////
	 //+++///* some kind of foldback where walkers also set settingsarray
	 // set by MASTER_WALKER (and where is this set???- in complexity?)


#ifndef LACH

	   // 4-hardware operations

	    hardcount++;
	    if (hardcount>=hardspeed){
	      hardcount=0;

	  // do hardware datagen walk into hdgen (8 bit) if flagged
	     if (digfilterflag&16){ // if we use hdgen at all
	    if (++hdgenerdel==HDGENERSPEED){
	      if (HDGENERWORMFLAG&1) tmp=HDGENERSTEP*direction8bit[wormdir];
	      else tmp=HDGENERSTEP*direction8bit[HDGENERDIR];
	    hdgenerpos+=tmp;
	    tmp=hdgenerpos;
	    dohardwareswitch(adc_buffer[2]>>5,datagenbuffer[tmp]);
	    hdgenerdel=0;
	    }
	     }
	  else
	    {
	    hardware=adc_buffer[2]>>5;
	    if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	    oldhardware=hardware;
	    }
		   
	  // 3 datagenclocks->40106/lm/maxim - filterflag as bits as we also need signal which clocks we
	  if (digfilterflag&2){
	    if (++f0106erdel==F0106ERSPEED){
	      if (F0106ERWORMFLAG&1) tmp=F0106ERSTEP*direction[wormdir];
	      else tmp=F0106ERSTEP*direction[F0106ERDIR];
	    f0106erpos+=tmp;
	    tmp=f0106erpos%32768;
	    set40106pwm(buf16[tmp]); 
	    f0106erdel=0;
	    }
	  }
	  
	  if (digfilterflag&4){
	    if (++lmerdel==LMERSPEED){
	    //lmer - set lmpwm
	      if (LMERWORMFLAG&1) tmp=LMERSTEP*direction[wormdir];
	    else tmp=LMERSTEP*direction[LMERDIR];
	    lmerpos+=tmp;
	    x=lmerpos%32768;
	    tmp=(lmerpos+1)%32768;
	    setlmpwm(buf16[x],buf16[tmp]); 
	    lmerdel=0;
	    }

	  }
	  
	  if (digfilterflag&8){
	    //maximer - setmaximpwm - just one
	    if (++maximerdel==MAXIMERSPEED){
	      if (MAXIMERWORMFLAG&1) tmp=MAXIMERSTEP*direction[wormdir];
	    else tmp=MAXIMERSTEP*direction[MAXIMERDIR];
	    maximerpos+=tmp;
	    tmp=maximerpos%32768;
	    setmaximpwm(buf16[tmp]); 
	    maximerdel=0;
	    }
	  }
	    } // hardcount
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
