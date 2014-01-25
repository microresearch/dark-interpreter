/*

Oh, eternity with outstretched wings, that broodest over the secret
truths in whose roots lie the mysteries of man—his whence, his
whither—have I searched thee, and struck a right key on thy dreadful
organ!

[Thomas de Quincey. The Dark Interpreter] 

*/

/* first steps for the dark interpreter based on work by: */
/* E. Brombaugh 07-31-2012                                        */
/* M. Howse 08.2013 */

 /*
PATH=~/sat/bin:$PATH
PATH=~/stm32f4/stlink/flash:$PATH

make stlink_flash
 */

#include "stm32f4xx.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "audio.h"
#include "hardware.h"
#include "CPUint.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

int16_t datagenbuffer[DATA_BUFSZ] __attribute__ ((section (".ccmdata")));;

u8 wormdir; // worm direction

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


void main(void)
{

  // order that all inits and audio_init called seems to be important

	uint32_t state;
	int32_t idx, rcount,wcount;
	uint16_t data,x,y,i,highest,lowest;
	u8 hdgen;
	
	//	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); //FPU - but should be in define
	
#if 1
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
#else
	for(state=0;state<8;state++)
	{
		for(idx=0;idx<BUFF_LEN/2;idx++)
		{
			tx_buffer[2*idx+0] = state*BUFF_LEN/2 + idx;
			tx_buffer[2*idx+1] = state*BUFF_LEN/2 + idx;
		}
		I2S_RX_CallBack(tx_buffer, rx_buffer, BUFF_LEN);
	}
#endif
	x=rcount=i=wcount=highest=lowest=0;

	// init machine/datagens
	
	while(1)
	{
	  // top down knobs: 2,0,3,4,1 
	  //	  y=adc_buffer[2];

	  // 1-run machine/datagen code (how to select?)

	  // 3-deal with knobs (esp. with micro-macro ops) - as many as direct

	  // 4-any hardware operations

#ifndef LACH

	  // do hardware datagen walk into hdgen( 8 bit)
	  // xxxx(samp/hard/clocks)->step,position,direction(into array),speed,start,end 

	  dohardwareswitch(adc_buffer[2],hdgen);
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
