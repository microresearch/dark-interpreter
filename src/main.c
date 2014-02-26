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

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

//uint16_t datagenbuffer[32] __attribute__ ((section (".ccmdata")))  __attribute__((aligned(4)));

u8* datagenbuffer = (u8*)0x10000000;

extern u8 digfilterflag;

u8 wormdir; // worm direction

struct dgenwalker{
  // xxxx(samp/hard/clocks)->step,position,direction(into array),speed,start,end 
  u8 step,dir,speed,del;
  u16 pos,start,end;
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

/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */


caddr_t _sbrk(int incr) {

    extern char _ebss; // Defined by the linker
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;

   
char * stack = (char*) __get_MSP();
     if (heap_end + incr >  stack)
     {
       //         _write (STDERR_FILENO, "Heap and stack collision\n", 25);
         errno = ENOMEM;
         return  (caddr_t) -1;
         //abort ();
     }

    heap_end += incr;
    return (caddr_t) prev_heap_end;

    }


void main(void)
{

  // order that all inits and audio_init called seems to be important

  //	uint32_t state;
  //	int32_t idx, rcount,wcount;
  //	uint16_t data,x,y,i,highest,lowest;
	u16 tmp,oldhardware,hardware;
	
	//	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); //FPU - but should be in define

	
	struct dgenwalker *lmer=malloc(sizeof(struct dgenwalker));
	struct dgenwalker *maximer=malloc(sizeof(struct dgenwalker));
	struct dgenwalker *f0106er=malloc(sizeof(struct dgenwalker));
	struct dgenwalker *hdgener=malloc(sizeof(struct dgenwalker));

	//  u8 step,dir,speed;  u16 pos,start,end; 
	lmer->step=1; lmer->speed=1; lmer->dir=1;lmer->pos=1;lmer->start=1;lmer->end=32767; lmer->del=0;
	maximer->step=1; maximer->speed=1; maximer->dir=1;maximer->pos=1;maximer->start=1;maximer->end=32767; maximer->del=0;
	f0106er->step=1; f0106er->speed=1; f0106er->dir=1;f0106er->pos=1;f0106er->start=1;f0106er->end=32767; f0106er->del=0;
	hdgener->step=1; hdgener->speed=1; hdgener->dir=1;hdgener->pos=1;hdgener->start=1;hdgener->end=65535; hdgener->del=0;
	
	u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768
	u16 direction8bit[8]={65279,65280,1,257,256,255,65534,65278}; // for 8 bits into counter

	//	ADC1_Initonce();
	ADC1_Init((uint16_t *)adc_buffer);

#ifndef LACH
	setup_switches();
#endif

	// maintain order

	Audio_Init();
	Codec_Init(48000);
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

	// CA and simulation - TESTING
	signed char stack_pos=-1;
	struct stackey stackyy[STACK_SIZE];
	stack_pos=func_pushn(stackyy,1,datagenbuffer,stack_pos);
	dohardwareswitch(0,0);

	while(1)
	{

#ifdef TEST_STRAIGHT

	  // just to test
	    hardware=adc_buffer[2]>>5;
	    if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	    oldhardware=hardware;
	    //	    tmp=runsine(tmp,10,datagenbuffer,10,unit);
	    // test simulations
	    func_runall(stackyy,datagenbuffer,stack_pos);

#else

	  // **TODO: WORM_OVER_RIDE for all directions!!!!
	  // generic speed modifier

	  // top down knobs: 2,0,3,4,1 

	  // 1-run machine/datagen code (how to select?)
	  // machine_run(m); with delay wrapper - also complexity bitwise

	  // and rest as x functions??? no pointers as different structures
	  // or would void pointer work?


	  // 3-deal with knobs (esp. with micro-macro ops) - as many as direct
	  // -micro/macro - adc_buffer[0]
	  // complexity??? 	  // generic speed and samplerate - adc_buffer[4]
	  //	Codec_Init(48000); [1-44.1, 2-16, 3-48, 4-96, 5-8, 6-88.2 KHz]

#ifndef LACH
	  // 4-hardware operations->
	  // do hardware datagen walk into hdgen (8 bit) if flagged
	  if (digfilterflag&16){ // if we use hdgen at all
	    if (++hdgener->del==lmer->speed){

    	    tmp=hdgener->step*direction8bit[hdgener->dir];
	    if ((hdgener->start+hdgener->pos+tmp)>=hdgener->end) hdgener->pos=(hdgener->pos+tmp)%(hdgener->end-hdgener->start);
	    else hdgener->pos+=tmp;
	    dohardwareswitch(adc_buffer[2]>>5,((u8 *)(datagenbuffer))[hdgener->start+hdgener->pos]);
	    hdgener->del=0;
	    }
	  }
	  else
	    {
	    hardware=adc_buffer[2]>>5;
	    if (hardware!=oldhardware) dohardwareswitch(hardware>>5,0);
	    oldhardware=hardware;
	    }

	  // 3 datagenclocks->40106/lm/maxim - filterflag as bits as we also need signal which clocks we
	  if (digfilterflag&2){
	    if (++f0106er->del==f0106er->speed){
    	    tmp=f0106er->step*direction[f0106er->dir];
	    if ((f0106er->start+f0106er->pos+tmp)>=f0106er->end) f0106er->pos=(f0106er->pos+tmp)%(f0106er->end-f0106er->start);
	    else f0106er->pos+=tmp;
	    set40106pwm(datagenbuffer[(f0106er->start+f0106er->pos)]); 
	      f0106er->del=0;
	    }
	  }

	  if (digfilterflag&4){
	    if (++lmer->del==lmer->speed){
	    //lmer - set lmpwm
	    tmp=lmer->step*direction[lmer->dir];
	    if ((lmer->start+lmer->pos+tmp)>=lmer->end) lmer->pos=(lmer->pos+tmp)%(lmer->end-lmer->start);
	    else lmer->pos+=tmp;
	    setlmpwm(datagenbuffer[(lmer->start+lmer->pos)],datagenbuffer[(lmer->start+lmer->pos+1)%32768]); 
	    lmer->del=0;
	    }

	  }

	  if (digfilterflag&8){
	    //maximer - setmaximpwm - just one
	    if (++maximer->del==maximer->speed){
	    tmp=maximer->step*direction[maximer->dir];
	    if ((maximer->start+maximer->pos+tmp)>=maximer->end) maximer->pos=(maximer->pos+tmp)%(maximer->end-maximer->start);
	    else maximer->pos+=tmp;
	    setmaximpwm(datagenbuffer[(maximer->start+maximer->pos)]); 

	      maximer->del=0;
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
