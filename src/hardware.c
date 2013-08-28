/* switching signal paths and hardware setup */

/* TODO:

- do all hardware init/setup in one go - as maybe problem with so many
- structures try also with BSRR or with GPIO_WriteBit(GPIO_TypeDef*
- GPIOx, uint16_t GPIO_Pin, BitAction BitVal);

- testing filter switches and PWM - NEARLY but we need 3 different PWMs?

- testing 40106 powerPWM =????

filterpwm-PB1 (is for lm13700)
filterclock-PC9 (is for maxim)
40106powerpwm-PC14

PC14 - not part of PWM

(PB1 can be TIM3_CH4, PC9 as TIM8_CH4, but PC14 has no TIM assoc. but
could be with interrupt and hard toggle???)

- test all hardware switches: filt+distort on, feedback, distortion in
  filter, filter feedback variations

- test leave all hanging= GPIO_Mode_IN_FLOATING

*/

#include "hardware.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_pwr.h"

extern __IO uint16_t adc_buffer[10];

int duty_cycle;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t CCR1_Val = 333; // was 333
uint16_t CCR2_Val = 333;
uint16_t CCR3_Val = 333;
uint16_t CCR4_Val = 333;
uint16_t PrescalerValue = 0;

void TIM_Config(void);

/* what are pins we need to be switching?

- SW0 PB0 - out to 40106
- SW1 PC11 - out to filter
- SW2 PB2 - straightout to jack ***
- SW3 PB3 - filter to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
- SW6 PB6 - 40106 to jack
- SW7 PB7 - last 3 for 4053-input
- SW8 PB8 - filterin
- SW9 PB9 - filterout
- feedbacksw-PC8
- filterdistort-PC10 1 to stop distortion

4053:

X0/jackin->X-14-LINEINR
X1/lm358

Y0/13700out->Y-15-filtin
Y1/LINEOUTL

Z0/13700in->Z-4-filtout
Z1/LINEINL

A-PB7=switchesX
B-PB8=Y
C-PB9=Z


are any of these swapped???

*/

/* first step switch - SW2 PB2 - out to jack ***/


#define JACKOUT (1 << 2) /* port B, pin 2 */
#define FILTIN (1 << 4) /* port B, pin 4 */
#define LINEINN (1 << 7) /* port B, pin 7 */

void setup_switches(void)
{

  //  GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);    
  //  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);   


    	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  //	GPIOB->MODER |= (1 << (2 * 2)) | (1 << (4 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)) | (1 << (6 * 2)) | (1 << (0 * 2)) ;	// JACK

	// try this way as PB4 has problem with JTRST - but somehow
	// this screws up rest settings sometimes
	// somehow funny one either/or works???

  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ; // this works for pin 4 (should use for all pins?)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  //GPIO_SetBits(GPIOB, GPIO_Pin_4);
  	

	// switch off PC8/feedback???
  
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	//	GPIOC->MODER |= (GPIO_MODER_MODER8_0) | (GPIO_MODER_MODER11_0) | (GPIO_MODER_MODER10_0) ; // another way to do it

	GPIOC->MODER |= (1 << (8 * 2)) | (1 << (11 * 2)) | (1 << (10 * 2));

	GPIOC->ODR = 0;
}

void switchalloff(void)
{
  GPIOC->ODR = 0;
  GPIOB->ODR = 0;
}

void switch_jack(void)
{
  // clear first 3 and JACKOUT is on

  // try also with BSRR

  GPIO_SetBits(GPIOB, GPIO_Pin_2);


  //  GPIOB->ODR &= ~(7);
  //  GPIOB->ODR |= JACKOUT;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


void test_filter(void)
{
  // connect the filter in signal path:

  //- SW1 PC11 - out to filter PC10 also 1

  GPIOC->ODR |= (1<<11) | (1<<10); //test distortion in filter = remove 1<<10

  //- SW4 PB4 - filter to jack

  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));

  GPIOB->ODR |= FILTIN;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across


}

void test_40106(void)
{

  GPIOC->ODR |= (1<<10); //test distortion in filter = remove 1<<10

  //- SW0 PB0 - out to 40106
  //- SW6 PB6 - 40106 to jack

  GPIOB->ODR = 1 | (1<<6);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

  // was |=

}

/* fill in rest of switch functions - also to leave all floating for
   pads as HW option FLOATIT */

void test_filtand40106(void)
{

  /*
- SW3 PB3 - filter to 40106
- SW6 PB6 - 40106 to jack
- SW1 PC11 - out to filter
  */

  // connect the filter in signal path:

  //- SW1 PC11 - out to filter PC10 also 1

  GPIOC->ODR |= (1<<11) | (1<<10);

  //- SW4 PB3 and PB6

  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));

  GPIOB->ODR |= (1<<3) | (1<<6);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
}

void test_40106andfilt(void)
{
  /*
- SW0 PB0 - out to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
  */

  GPIOC->ODR |= (1<<10);


  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));

  GPIOB->ODR |= (1<<0 | (1<<4)) | (1<<5);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


/* 3 timers/pwm */

void retryagainpwm(void){ // THIS ONE WORKS! 

  /*

    - re-try with MAXIM - now it works but how to change it?
    - if period is higher then we go lower in frequency
    - but can't seem to change duty cycle

  */

  TIM_Config();

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // 28 MHz

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 665; // was 665
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR2_Val;

  TIM_OC2Init(TIM3, &TIM_OCInitStructure);

  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel3 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR3_Val;

  TIM_OC3Init(TIM3, &TIM_OCInitStructure);

  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = CCR4_Val;

    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM3, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

void changepwm(uint16_t value){

  TIM3->CCR1 = value;
  TIM3->CCR2 = value;
  TIM3->CCR3 = value;
  TIM3->CCR4 = value; // somehow need set all 4????
}

/**
  * @brief  Configure the TIM3 Ouput Channels.
  * @param  None
  * @retval None
  */
void TIM_Config(void) // can't seem to map to pc14 or pc11 or pc10 but pc8 and 9 work!
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* GPIOC Configuration: TIM3 pc9 we also want pc14 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ; //was UP
  GPIO_Init(GPIOC, &GPIO_InitStructure); 


  //  PWR_BackupAccessCmd(ENABLE); // Enable access to LSE
  //  RCC_LSEConfig(RCC_LSE_OFF); // PC14 PC15 as GPIO

  
  /* GPIOB Configuration:  TIM3 CH3 (PB0) and TIM3 CH4 (PB1) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 |GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  /* Connect TIM3 pins to AF2 */  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3); 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);
}