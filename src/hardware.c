/* switching signal paths and hardware setup */

/* TODO:

- for whatever reason now when we use 40106power we get huge noise
  from power supply/usb suddenly (also with battery)

something coming into lineINR (as when 0 this in audio.c then no noise)

seems to overamplify input

///

test leave all hanging= GPIO_Mode_IN_FLOATING

- maybe do all hardware init/setup in one go - as maybe problem with so many
- structures try also with BSRR or with GPIO_WriteBit(GPIO_TypeDef*
- GPIOx, uint16_t GPIO_Pin, BitAction BitVal);

- lmpwm is ringing (1-1000)

- clean up for functions below and test all combinations:

  functions:

- justfilter
- justdistortion
- filterthendistortion
- distortionthenfilter
- distortioninfilter on/off (could be done fast also)
- setfiltfeedbackpath - digital/lm13700
- feedback (leave for now)
- setfloating (enum list)
- setallfloating (or how to set diff ones but not just one)

- setlmpwm
- setmaximpwm
- set40106power

////

- test leave all hanging= GPIO_Mode_IN_FLOATING

*/

#include "hardware.h"
#include "misc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_pwr.h"

extern __IO uint16_t adc_buffer[10];

int duty_cycle;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t CCR4_Val = 333;
uint16_t PrescalerValue = 0;

uint16_t fakep;

void TIM_Config(void);


void TIM4_IRQHandler(void)
{
  static int flag=0;
if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
{
TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//GPIO_ToggleBits(GPIOC, GPIO_Pin_14);
 if (flag>10){
   GPIO_ResetBits(GPIOC, GPIO_Pin_14);
   if (flag>(10+fakep)) flag=0;
 }
 else GPIO_SetBits(GPIOC, GPIO_Pin_14);
 flag++;

}
}


/* what are pins we need to be switching?

- SW0 PB0 - out to 40106
- SW1 PC11 - out to filter
- SW2 PB2 - straightout to jack ***
- SW3 PB3 - filter to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
- SW6 PB6 - 40106 to jack

// 4053:

- SW7 PB7 - input
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

#define JACKOUT (1 << 2) /* port B, pin 2 */
#define FILTIN (1 << 4) /* port B, pin 4 */
#define LINEINN (1 << 7) /* port B, pin 7 */

/*

all clocks set in datagens/elsewhere

*/

void dohardwareswitch(uint16_t modder){
  uint16_t res;
  // mod is 12 bits 0-4096

  // last 2 bits toggle input

  /*
feedback on/off - jackin-> - lm358in->

1-feedon jackin xx
2-feedon xx     lmin
3-feedoff jackin xx
4-feedoff xx     lmin
  */

  //#define JACKOUT (1 << 2) /* port B, pin 2 */
  //#define FILTIN (1 << 4) /* port B, pin 4 */
  //#define LINEINN (1 << 7) /* port B, pin 7 */

  //PORTB - PB0-9 is all switches except PB1(filterpwm)
  //PC8 is feedback switch
  //jitter???
  //    res=(modder>>4)&3;
  //  res=modder/1024;

  //    res = (uint16_t)(((float)modder) / 512.f)%4;

  res= (modder>>5)&3; // 12 bits now lose 5 = 7 bits = 0->255
  res=2;
  switch(res){
 case 0:
   GPIOB->BSRRH = (1<<7);
   GPIOC->BSRRL = (1<<8);
   break;
 case 1:
   GPIOB->BSRRL = (1<<7);
   GPIOC->BSRRL = (1<<8);
   break;
 case 2:
   GPIOB->BSRRH = (1<<7);
   GPIOC->BSRRH = (1<<8);
   break;
 case 3:
   GPIOB->BSRRL = (1<<7);
   GPIOC->BSRRH = (1<<8);
 }


  // leave hang and datagen for now as extra bit HERE 16 options + 1 bit

  res=(modder>>8); // so now we have 4 bits left = 0->4 options
  res=0;
  switch(res){
  case 0:
   //1-straightout
    // clear other options up here:
    GPIOB->BSRRH= (1<<0) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL = (1<<2);
    GPIOC->BSRRL= (1<<10);

   break;
  case 1:
    //2-unhang all [where to re-hang-use a flag]+1 extra option: clocks hang/clocks unhang here
    break;
  case 2:
    //2-unhang all [where to re-hang-use a flag]+1 extra option: clocks hang/clocks unhang here
    break;
  case 3:
    //3-just40106
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL=(1<<0) | (1<<6);
    break;
  case 4:
    //4-filterthen40106-->|filterpath=1lm/2digital-distort on/off in each case
    //filterpath->lm no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOB->BSRRL=(1<<3) | (1<<6);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    break;
  case 5:
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL=(1<<3) | (1<<6);
    GPIOC->BSRRL=(1<<11);
    break;
  case 6:
    //filterpath->digital no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5);
    GPIOB->BSRRL=(1<<3) | (1<<6) |(1<<8) |(1<<9);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    break;
  case 7:    
    //filterpath->digital distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL=(1<<3) | (1<<6) |(1<<8) |(1<<9);
    GPIOC->BSRRL=(1<<11);
    break;
  case 8:
    //5-40106thenfilter-->|
    //filterpath->lm no distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5);
    GPIOC->BSRRL= (1<<10);
    break;
  case 9:
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11) | (1<<10);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5);
    break;
  case 10:
    //filter->digital no distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOC->BSRRL= (1<<10);
    break;
  case 11:
    //filter->digital distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6);
    GPIOC->BSRRH= (1<<11) | (1<<10);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    break;
  case 12:
    //6-justfilter------->|
    //filterpath->lm no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6) | (1<<8) | (1<<9);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<10) | (1<<11);
    break;
  case 13:
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<11);
    break;
  case 14:
    //filterpath->digital no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6);
    GPIOB->BSRRL= (1<<4) | (1<<8) | (1<<9);
    GPIOC->BSRRL= (1<<10) | (1<<11);
    break;
  case 15:
    //filterpath->digital distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6);
    GPIOC->BSRRH= (1<<10) | (1<<8) | (1<<9);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<11);
    break;
  }
}

void switch_jack(void)
{
  // clear first 3 and JACKOUT is on

  //  GPIO_SetBits(GPIOB, GPIO_Pin_2); 

  //  GPIO_SetBits(GPIOC, GPIO_Pin_8); // feedback on PC8

  //  GPIOC->ODR = 0; // should clear more than this?
  GPIOB->ODR = JACKOUT;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
  //  GPIOC->ODR |= (1<<10);


}

void setup_switches(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ; // this works for pin 4 (should use for all pins?)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

  // different method to above???

  GPIOC->MODER |= (1 << (8 * 2)) | (1 << (11 * 2)) | (1 << (10 * 2));
  GPIOC->ODR = 0;
  GPIOB->ODR = 0;

}

void switchalloff(void)
{
  GPIOC->ODR = 0;
  GPIOB->ODR = 0;
}

void retryagainpwm(void){ // THIS ONE WORKS! 

  TIM_Config();

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // 28 MHz

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000; // was 665 was 2400 - this can change

  // 717 should give us 39 KHz if we start from 28 MHz

  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  TIM_OCStructInit(&TIM_OCInitStructure);


  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM3, &TIM_OCInitStructure);

  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);

  /* do TIM8 channel and enables */

  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

  // new structure???

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM8, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM8, ENABLE);

  /* TIM8 enable counter */
  TIM_Cmd(TIM8, ENABLE);


}

void setlmpwm(uint16_t value, uint16_t value2){

  TIM3->CCR4 = value2;
  //  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // was by 28 MHz
  PrescalerValue = 2;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = value; // was 665
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* do TIM8 channel and enables */

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  // new structure???

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 100;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM3, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM3, ENABLE);

  /* TIM8 enable counter */
  TIM_Cmd(TIM3, ENABLE);



}

void setmaximpwm(uint16_t value){

  // uint32_t SystemCoreClock = 168000000;

  //  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // 28 MHz
  PrescalerValue = 2;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = value; // was 665
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* do TIM8 channel and enables */
  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
    
  TIM_OCStructInit(&TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 100;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM8, &TIM_OCInitStructure);
      
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
  
  TIM_CtrlPWMOutputs(TIM8, ENABLE);
  TIM_Cmd(TIM8, ENABLE);
  //  TIM8->CCR4 = value;
  

}

void TIM_Config(void) 
{

  /*
filterpwm-PB1 (is for lm13700)
filterclock-PC9 (is for maxim)

-PB1 can be TIM3_CH4, PC9 as TIM8_CH4, 

  */

  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* TIM8 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE); // APB2?

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // AHB1???

  
  /* GPIOB Configuration:  TIM3 CH4 (PB1) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  /* Connect TIM3 pins to PB1 */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);

  /* TIM8 pins to PC9 */  

  /* GPIOC Configuration: TIM8_CH4 (PC9) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ; //was UP
  GPIO_Init(GPIOC, &GPIO_InitStructure); 

  /* Connect TIM8 pins to PC9 */  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM8);

}

void setup40106power(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;


  PWR_BackupAccessCmd(ENABLE); // Enable access to LSE
  RCC_LSEConfig(RCC_LSE_OFF); // PC14 PC15 as GPIO
  //  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

NVIC_InitTypeDef NVIC_InitStructure;
/* Enable the TIM2 gloabal Interrupt */
NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);

TIM_OCStructInit(&TIM_OCInitStructure);
 
/* TIM2 clock enable */
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
/* Time base configuration */
 TIM_TimeBaseStructure.TIM_Period = 1; // 1000 - 1 // 1 MHz down to 1 KHz (1 ms)
 TIM_TimeBaseStructure.TIM_Prescaler =10-1;// 84 - 1; // was 84 - 1// 24 MHz Clock down to 1 MHz (adjust per your clock)

TIM_TimeBaseStructure.TIM_ClockDivision = 0;
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
/* TIM IT enable */
TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
/* TIM2 enable counter */
TIM_Cmd(TIM4, ENABLE);
}

void set40106power(uint16_t value){

  fakep=value;

}
