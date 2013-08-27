/* switching signal paths and hardware setup */

/* TODO:

- try also with BSRR

- testing filter switches and PWM
- test all hardware switches
- test leave all hanging

*/

#include "hardware.h"
#include "stm32f4xx_tim.h"

extern __IO uint16_t adc_buffer[10];

int duty_cycle;

/* what are pins we need to be switching?

- SW0 PB0 - out to 40106
- SW1 PC11 - out to filter
- SW2 PB2 - out to jack ***
- SW3 PB3 - filter to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
- SW6 PB6 - 40106 to jack
- SW7 PB7 - last 3 for 4053-input
- SW8 PB8 - filterin
- SW9 PB9 - filterout
- feedbacksw-PC8
- filterdistort-PC10

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
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER |= (1 << (2 * 2)) | (1 << (4 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)) ;	// JACK

	// switch off PC8/feedback???

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	GPIOC->MODER |= (GPIO_MODER_MODER8_0) | (GPIO_MODER_MODER11_0); // another way to do it
	GPIOC->ODR = 0;
}

void switch_jack(void)
{
  // clear first 3 and JACKOUT is on

  // try also with BSRR

  GPIOB->ODR &= ~(7);
  GPIOB->ODR |= JACKOUT;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


/* fill in rest of switch functions - also to leave all floating for
   pads as HW option FLOATIT */

/* 3 timers/pwm */

void Init_TimeBase(TIM_TypeDef* TIMx, uint16_t Prescaler, uint32_t TIM_Period)
{
 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 
 TIM_TimeBaseStructure.TIM_Period = TIM_Period; //final frequency = clock frequency/(prescalervalue * tim_period)
               //TIM_Period is written to the ARR (Auto reload register)
               //when the timer counter register reaches the ARR value, it is reset
               //and a interrupt generated
 TIM_TimeBaseStructure.TIM_Prescaler = Prescaler; //Prescaler acts as a clock frequency divider
 TIM_TimeBaseStructure.TIM_ClockDivision = 0;
 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //count from 0 to the value in the ARR register
 
 TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
}

uint16_t Calc_Prescaler(uint16_t TIMx, uint32_t Tim_Freq)
{
 uint16_t Prescaler = 0;
 switch(TIMx)
 {
 case 1:
 case 8: 
 case 9:
 case 10:
 case 11:
  /*TIM1 and TIM8 to TIM11 are clocked by the APB2 clock (PCLK2)
   * TIMx clock will be set to 2 * APB2 clock
   * TIMxCLK = 2 * PCLK2
   * PCLK2 = HCLK/(APB2 Prescaler)
   * => TIMxCLK = HCLK = SystemCoreClock
   * the prescaler is calculated as follows to achieve the specified clock frequency
   * Prescaler = (SystemCoreClock)/Tim_Freq) - 1
   */
  Prescaler = (uint16_t) ((SystemCoreClock) / Tim_Freq) - 1;
  break;
 case 2:
 case 3:
 case 4:
 case 5:
 case 6:
 case 7:
 case 12:
 case 13:
 case 14:
  /*TIM2 to TIM7 and TIM12 to Tim14 are clocked by the APB1 clock (PCLK1)
   * TIMx clock will be set to 2 * APB1 clock
   * TIMxCLK = 2 * PCLK1
   * PCLK1 = HCLK / (APB1 Prescaler)
   * => TIMxCLK = HCLK / 2 = SystemCoreClock/2
   * the prescaler is calculated as follows to achieve the specified clock frequency
   * Prescaler = (SystemCoreClock/2)/Tim_Freq) - 1
   */
  Prescaler = (uint16_t) ((SystemCoreClock/2)/Tim_Freq) - 1;
  break;
 }
 return (Prescaler);
}

void PWM_OC(uint32_t Pulse, TIM_TypeDef* TIMx, uint32_t channel)
{
 TIM_OCInitTypeDef  TIM_OCInitStructure;
 
 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //In PWM mode 1,
               //output is high when timer_counter <= capture_compare_register
                  //output is low when timer_counter >= capture_compare_register
 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
 TIM_OCInitStructure.TIM_Pulse = Pulse; //affects the duty cycle of the PWM signal
             //duty cycle= TIM_Pulse/TIM_Period*100
             //initial value to be loaded into the capture compare register
 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
 
 switch(channel)
 {
 case 1:
  TIM_OC1Init(TIMx, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable); //mandatory for PWM mode
  break;
 case 2:
  TIM_OC2Init(TIMx, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable); //mandatory for PWM mode;
  break;
 case 3:
   //  TIM_OC3Init(TIMx, TIM_OCInitStructure);
   //  TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable); //mandatory for PWM mode
  break;
 case 4:
  TIM_OC4Init(TIMx, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable); //mandatory for PWM mode
  break;
 }
}

void InitGPIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode)
{
 GPIOPuPd_TypeDef PuPdMode = 0;
 GPIO_InitTypeDef  GPIO_InitStructure;
 
 switch(GPIO_Mode)
 {
 case GPIO_Mode_OUT:
  PuPdMode = GPIO_PuPd_NOPULL; //digital output. Not using open drain mode as I do not know how that operates
  break;
 case GPIO_Mode_IN:
  PuPdMode = GPIO_PuPd_NOPULL; //digital read have Pin as input floating
  break;
 case GPIO_Mode_AN:
  PuPdMode = GPIO_PuPd_NOPULL; //for analogue read have Pin as input floating
  break;
 case GPIO_Mode_AF: //need to do a remapping if using alternate functions
  PuPdMode = GPIO_PuPd_UP; //for PWM have not looked at accounting for the various other alternate functions
  break;
 }
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode;
 GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //used for digital output and PWM output
               //this setting does not matter for ADC and digital read
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
 GPIO_InitStructure.GPIO_PuPd = PuPdMode;
 GPIO_Init(GPIOx, &GPIO_InitStructure);
}

void setup_pwm(void){

/* start with just PC9 to test filter (also need to route filter eg. function filteron)

PWM: 3 channels...

filterpwm-PB1 (is for lm13700)
filterclock-PC9 (is for maxim)
40106powerpwm-PC14

*/
  duty_cycle=50;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //enable clock for GPIOC
  InitGPIO(GPIOC, GPIO_Pin_9, GPIO_Mode_AF); //configure GPIO in alternate function mode
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM3); //connect pin to TIM3 for channel 1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //enable clock for timer3
  uint16_t Prescaler = Calc_Prescaler(3, 3000000); //calculate prescaler
  Init_TimeBase(TIM3, Prescaler, 60000);
  PWM_OC(duty_cycle, TIM3, 1);
  //  Attach_Tim_Interrupt(TIM3_IRQn, 0); //Attach an interrupt to TIM3 - can't find this anywhere

  TIM_Cmd(TIM3, ENABLE); //enable timer 3
  TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE); //enable generation of interrupts on timer 3

}

void TIM3_IRQHandler(void) //Tim3 ISR
{
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) //check if timer counter matches the timer compare value
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1); //clear the interrupt
    TIM3->CCR1 = duty_cycle; //update the duty cycle
  }
}

void test_filter(void)
{

  // connect the filter in signal path:

  //- SW1 PC11 - out to filter

  GPIOC->ODR |= (1<<11);

  //- SW4 PB4 - filter to jack

  GPIOB->ODR &= ~(7);
  GPIOB->ODR |= FILTIN;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in


  // poti controls duty cycle - test also for fingers

  duty_cycle=adc_buffer[0];

}
