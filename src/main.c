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
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array

 struct dgenwalker{
   // xxxx(samp/hard/clocks)->step,position,direction(into array),speed,start,end 
   u8 step,dir,speed,del;
   u16 pos,start,wrap;
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

 void main(void)
 {

   // order that all inits and audio_init called seems to be important

   //	uint32_t state;
   //	int32_t idx, rcount,wcount;
   //	uint16_t data,x,y,i,highest,lowest;
   u16 x,addr,tmp,oldhardware,hardware; 
   u16 speedwrapper=0;

	 //	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); //FPU - but should be in define


	 struct dgenwalker *lmer=malloc(sizeof(struct dgenwalker));
	 struct dgenwalker *maximer=malloc(sizeof(struct dgenwalker));
	 struct dgenwalker *f0106er=malloc(sizeof(struct dgenwalker));
	 struct dgenwalker *hdgener=malloc(sizeof(struct dgenwalker));

	 //  u8 step,dir,speed;  u16 pos,start,end; 
	 //
	 lmer->step=1; lmer->speed=1; lmer->dir=2;lmer->pos=0;lmer->start=1;lmer->wrap=32767; lmer->del=0;
	 maximer->step=1; maximer->speed=1; maximer->dir=2;maximer->pos=0;maximer->start=1;maximer->wrap=32767; maximer->del=0;
	 f0106er->step=1; f0106er->speed=1; f0106er->dir=2;f0106er->pos=0;f0106er->start=1;f0106er->wrap=32767; f0106er->del=0;
	 hdgener->step=1; hdgener->speed=1; hdgener->dir=2;hdgener->pos=0;hdgener->start=1;hdgener->wrap=65535; hdgener->del=0;

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

   m->m_leakiness=randi()%255;
   m->m_infectprob=randi()%255;
   m->m_mutateprob=randi()%255;

   /*  for (x=0;x<65535;x++){
     datagenbuffer[x]=x;
     }*/

	 // CA and simulation - TESTING
	 u8 stack_pos=0;
	 u8 stack_posy=0;
	 struct stackey stackyy[STACK_SIZE];
	 struct stackey stackyyy[STACK_SIZE];
	 u16 *buf16 = (u16*) datagenbuffer;

	 //simulationforstack:	
	 for (x=0;x<STACK_SIZE;x++){
	   	   stack_pos=func_pushn(stackyy,3,buf16,stack_pos,1,10);
	   //	   stack_pos=func_pushn(stackyy,2,buf16,stack_pos,1,10);
	 	   }

 #ifndef LACH
	 dohardwareswitch(2,0);
 #endif

	 // CPUintrev2:
	 /*	 for (x=0; x<100; x++)
		  {
		    addr=randi()<<4;
		    cpustackpush(m,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
		    }*/

	 // pureleak:
	 /*
		 for (x=0;x<MAX_FRED;x++){//define MAX_FRED 60
		   //			  addr=randi()<<4;
		   // addr=x*1000;
		   addr=randi()<<4;
		   //	  	  cpustackpushhh(datagenbuffer,addr,addr+100,x%31,1);
		   cpustackpushhh(datagenbuffer,addr,addr+(randi()<<4),randi()%31,1);
		   }*/
	 // CA:
		 


	 for (x=0;x<STACK_SIZE;x++){
	   //	   stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,1,10); // delay,howmany);
	   stack_posy=ca_pushn(stackyyy,4,datagenbuffer,stack_posy,1,10); // delay,howmany);
	   }


	  while(1)
	  {

 #ifdef TEST_STRAIGHT
	    //(top down= 2,0,3,4,1):
	    // just to test
	    //	   hardware=adc_buffer[2]>>5;
	    //	   if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	    //	   oldhardware=hardware;
	    //	   ca_runall(stackyyy,datagenbuffer,stack_posy); // some crash
	    //	   testing ADC9/ad620
	    //	   	   int16_t *buf16 = (int16_t*) datagenbuffer;
	    //	   x++;
	    //	   buf16[x%32768]=(adc_buffer[9]<<4)-32768;
	    //	   speedwrapper++;
		    tmp=(adc_buffer[0]<<2);
		    if (speedwrapper>=tmp){
		      speedwrapper=0;
	      //	     func_runall(stackyy,buf16,stack_pos); // simulations
	      //	     ca_runall(stackyyy,datagenbuffer,stack_posy); // CA
			    machine_run(m); //cpu - WRAP own speedTODO
		      //	     	     machine_runnn(datagenbuffer); // pureleak WRAP own speedTODO

			    }

		     dohardwareswitch(adc_buffer[3]>>5,0);

 #else

	    // 0- GENERIK SPEED WRAPPER

	    speedwrapper++;
	    tmp=(adc_buffer[0]); //adjust if 2 slow
	    if (speedwrapper>=tmp){
	      speedwrapper=0;

	   // 1-run machine/datagen code - based on complexity?

	      	      func_runall(stackyy,buf16,stack_pos); // simulations
	      	      //	      machine_run(m); //cpu - WRAP own speedTODO
	      	      ca_runall(stackyyy,datagenbuffer,stack_posy); // CA
	      //	      machine_runnn(datagenbuffer); // pureleak WRAP own speedTODO


	      // memory TEST push/pop
	      // sims/ca/machine

	      if (rand()%2==1) {
		stack_pos=func_pop(stackyy,stack_pos);
		stack_posy=ca_pop(stackyyy,stack_posy);
			//cpustackpop(m);
			//			cpustackpoppp(datagenbuffer);
						
	      }
	      else {
		//			addr=randi()<<4;
		//		    cpustackpush(m,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
		//			addr=randi()<<4;
		//			cpustackpushhh(datagenbuffer,addr,addr+(randi()<<4),1,1);
		//				stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,1,10);
		stack_pos=func_pushn(stackyy,1,buf16,stack_pos,1,10);
		//		stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,1,10); // delay,howmany);
		//stack_posy=ca_pushn(stackyyy,4,datagenbuffer,stack_posy,1,10); // delay,howmany);
		//CRASH on 4-cel1d table
	     }


	     // 3-deal with setting knob + up/down/dir 
	     // walk into settings array

#ifndef LACH

	   // 4-hardware operations

	     // **TODO: WORM_OVER_RIDE for all directions!!!!

	  // do hardware datagen walk into hdgen (8 bit) if flagged
	     if (digfilterflag&16){ // if we use hdgen at all
	    if (++hdgener->del==hdgener->speed){

    	    tmp=hdgener->step*direction8bit[hdgener->dir];
	    if ((hdgener->pos+tmp)>=hdgener->wrap) hdgener->pos=(hdgener->pos+tmp)%(hdgener->wrap+1);
	    else hdgener->pos+=tmp;
	    tmp=hdgener->start+hdgener->pos;
	    dohardwareswitch(adc_buffer[2]>>5,datagenbuffer[tmp]);
	    hdgener->del=0;
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
	    if (++f0106er->del==f0106er->speed){
    	    tmp=f0106er->step*direction[f0106er->dir];
	    if ((f0106er->pos+tmp)>=f0106er->wrap) f0106er->pos=(f0106er->pos+tmp)%(f0106er->wrap+1);
	    else f0106er->pos+=tmp;
	    tmp=(f0106er->start+f0106er->pos)%32768;
	    set40106pwm(buf16[tmp]); 
	      f0106er->del=0;
	    }
	  }
	  
	  if (digfilterflag&4){
	    if (++lmer->del==lmer->speed){
	    //lmer - set lmpwm
	    tmp=lmer->step*direction[lmer->dir];
	    if ((lmer->pos+tmp)>=lmer->wrap) lmer->pos=(lmer->pos+tmp)%(lmer->wrap+1);
	    else lmer->pos+=tmp;
	    x=(lmer->start+lmer->pos)%32768;
	    tmp=(lmer->start+lmer->pos+1)%32768;
	    	    setlmpwm(buf16[x],buf16[tmp]); 
	    lmer->del=0;
	    }

	  }
	  
	  if (digfilterflag&8){
	    //maximer - setmaximpwm - just one
	    if (++maximer->del==maximer->speed){
	    tmp=maximer->step*direction[maximer->dir];
	    if ((maximer->pos+tmp)>=maximer->wrap) maximer->pos=(maximer->pos+tmp)%(maximer->wrap+1);
	    else maximer->pos+=tmp;
	    tmp=(maximer->start+maximer->pos)%32768;
	    setmaximpwm(buf16[tmp]); 
	    maximer->del=0;
	    }
	  }
#endif
	  	   } // speedwrapper
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
