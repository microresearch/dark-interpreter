/* switching signal paths and hardware setup */

#include "hardware.h"
#include "misc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_pwr.h"

u8 digfilterflag;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
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

- ADD PC13 - in latest design/on/off audio in - use with PC8

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

*/

void reset_switches(void);
void reset_clocks(void);

void dohardwareswitch(u8 modder, u8 inp, u8 hdgen){
  u8 res2;
  static uint8_t hangflag=0, clockhangflag=0;//, augflag=0;
  GPIO_InitTypeDef  GPIO_InitStructure;

  //PORTB - PB0-9 is all switches except PB1(filterpwm)
  //PC8 is feedback switch

#ifdef TENE
  if ((modder>15 || modder<11) && hangflag==1){ //
    hangflag=0;
    reset_switches();
  }
#endif

  if (inp!=3 && clockhangflag==1){
    clockhangflag=0;
    reset_clocks();
    }

#ifdef TEST_STRAIGHT
      inp=0;
      modder=0;
#endif

  /*
RES: feedback on/off - jackin-> - lm358in->
0-feedoff jackin xx
1-feedoff xx     lmin
2-feedon 
3-feedon xx     lmin ??? makes no sense - NOW replaced with clock unhang
  */

      //      inp=3;//TESTY!
  switch(inp){
 case 0:
   GPIOB->BSRRH = (1<<7); //JACK
   GPIOC->BSRRH = (1<<8);
   GPIOC->BSRRL = (1<<13);
   break;
 case 1:
   GPIOB->BSRRL = (1<<7); //358 
   GPIOC->BSRRH = (1<<8);
   GPIOC->BSRRH = (1<<13); // was L
   break;
 case 2:
   GPIOB->BSRRH = (1<<7);
   GPIOC->BSRRL = (1<<8); // BSRRL sets BIT! = feedback
   GPIOC->BSRRH = (1<<13); //
   break;
 case 3:
   //   GPIOB->BSRRH = (1<<7);
   //   GPIOC->BSRRH = (1<<8); // was H!
   //   GPIOC->BSRRL = (1<<13);
   // add unhang for clocks- DONE! 
   // AUG - don't unhang 40106= as makes no sense!
   //   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   //   GPIO_InitStructure.GPIO_Mode = 0x04;
   //   GPIO_Init(GPIOA, &GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
   GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
   GPIO_Init(GPIOB, &GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
   GPIO_Init(GPIOC, &GPIO_InitStructure);
   clockhangflag=1;
   break;
 }

  //digfilterflag= 32.16.8.4.2.1=filterfeedin,switch_hardware,maxim,lm,40106,digfilter_process

  //if (augflag) modder=24;  // TESTY!AUG

  //  modder=0; //now test 40106 --- test for digfilterflag//TESTY!
  // now as 32 options with digfilterflag as 32 for filterfeed
  switch(modder){
  case 0:
   //1-straightout
    GPIOB->BSRRH= (1<<0) | (1<<3) | (1<<4) | (1<<5) | (1<<6);// | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL = (1<<2) | (1<<8) | (1<<9);// this gets rid of hum
    GPIOC->BSRRL= (1<<10);
    digfilterflag=0;
   break;
  case 1:
   //1-straightout with filtermix-straight
    // clear other options up here:
    GPIOB->BSRRH= (1<<0) | (1<<3) | (1<<4) | (1<<5) | (1<<6);// | (1<<8);
    GPIOB->BSRRL = (1<<2) | (1<<9) | (1<<8);//
    GPIOC->BSRRL= (1<<10) | (1<<11);
    if (clockhangflag==0) digfilterflag=9;// AUG
    //    else digfilterflag=1;
   break;
  case 2:
   //1-straightout with filtermix-distort
    // clear other options up here:
    GPIOB->BSRRH= (1<<0) | (1<<3) | (1<<4) | (1<<5) | (1<<6);// | (1<<8);
    GPIOB->BSRRL = (1<<2) | (1<<9) | (1<<8) ;//
    GPIOC->BSRRH= (1<<10);
    GPIOC->BSRRL= (1<<11);
    if (clockhangflag==0) digfilterflag=9;
    //    else digfilterflag=1;
   break;
  case 3:
   //1-40106 with filtermix-no distort
    // clear other options up here:
    GPIOB->BSRRH= (1<<3) | (1<<4) | (1<<6);
    GPIOB->BSRRL = (1<<0) |(1<<2) | (1<<5) | (1<<8) | (1<<9);//
    GPIOC->BSRRH= (1<<11);
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=9;
    //    else digfilterflag=1;
   break;
  case 4:
   //1-40106 with filtermix - distort
    // clear other options up here:
    GPIOB->BSRRH= (1<<3) | (1<<4) | (1<<6);
    GPIOB->BSRRL = (1<<0) | (1<<2) | (1<<5) | (1<<8) | (1<<9);//
    GPIOC->BSRRH= (1<<11) | (1<<10);
    if (clockhangflag==0) digfilterflag=9;
    //    else digfilterflag=1;
   break;

  case 5:
    //3-just40106
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<4) | (1<<5);// | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<10) | (1<<11);
    GPIOB->BSRRL=(1<<0) | (1<<6) | (1<<8) | (1<<9);// this gets rid of hum
    digfilterflag=0;
    break;
  case 6:
    //3-just40106 - filtermix no distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<4) | (1<<5);
    GPIOC->BSRRH= (1<<10) | (1<<11);
    GPIOB->BSRRL=(1<<0) | (1<<6) | (1<<5) | (1<<8) | (1<<9);
    if (clockhangflag==0) digfilterflag=9;
    //    else digfilterflag=1;
    break;
  case 7:
    //3-just40106 - filtermix distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<4) | (1<<5);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL=(1<<0) | (1<<6) | (1<<5)  | (1<<8) | (1<<9);
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=9;
    //    else digfilterflag=1;
    break;
    //////////////additions DONE
  case 8:
    // GPIOB->0,3,4,5,6,8,9 set by hdgen and flagged so
        GPIOB->ODR &= ~(1 | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<8) | (1<<9));
        GPIOB->ODR |= ((hdgen&2)>>1 | ((hdgen&4)<<1) | ((hdgen&8)<<1) | ((hdgen&16)<<1) | ((hdgen&32)<<1) | ((hdgen&64)<<2) | ((hdgen&128)<<2)); // checked		    
    
    // CPIOC->10 (8 bits total) - bottom bit
    // set PC11 always... WHY?
	GPIOC->BSRRL=(1<<11); // out to filter
	GPIOC->BSRRH=(1<<10);
    GPIOC->ODR|= ((hdgen&1)<<10); // bit 10 - filter distort bit
    if (clockhangflag==0) digfilterflag=29; // flags to use hdgen - run all
    else digfilterflag=17;
    break;
  case 9:
    // GPIOB->0,3,4,5,6,8,9 set by hdgen and flagged so
        GPIOB->ODR &= ~(1 | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<8) | (1<<9));
        GPIOB->ODR |= ((hdgen&2)>>1 | ((hdgen&4)<<1) | ((hdgen&8)<<1) | ((hdgen&16)<<1) | ((hdgen&32)<<1) | ((hdgen&64)<<2) | ((hdgen&128)<<2));		    
    
    // CPIOC->10 (8 bits total) - bottom bit
    // set PC11 always...
	GPIOC->BSRRL=(1<<10);
	GPIOC->BSRRH=(1<<11);
    GPIOC->ODR|=((hdgen&1)<<11); // bit 11
    if (clockhangflag==0) digfilterflag=31; // flags to use hdgen
    else digfilterflag=17;
    break;

  case 10:
    // HDGEN on filter ops only
    // so filter to jack=4. HDGEN on B 1,3,5,6,8,9, C=10 and 11 
        GPIOB->ODR &= ~(1 | (1<<3) | (1<<5) | (1<<6) | (1<<8) | (1<<9));
	///
        GPIOB->ODR |= ((hdgen&2)>>1 | ((hdgen&4)<<1) | ((hdgen&16)<<1) | ((hdgen&32)<<1) | ((hdgen&64)<<2) | ((hdgen&128)<<2));		    
    
    // CPIOC->10 (8 bits total) - bottom bit
    // set PC11 always...
	GPIOC->BSRRH=(1<<10);
	GPIOC->BSRRH=(1<<11);
	GPIOC->ODR|=((hdgen&1)<<11) | ((hdgen&8)<<7); // bit 11
    if (clockhangflag==0) digfilterflag=31; // flags to use hdgen
    else digfilterflag=17;
    break;

    ////////
    
#ifdef TENE
  case 11:
        //2-unhang all except input 
    // input is pb7
       hangflag=1;
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9; //pin 9 floats crashes but test - seems okay
       GPIO_InitStructure.GPIO_Mode = 0x04;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
         GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11;
         GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
           GPIO_Init(GPIOC, &GPIO_InitStructure);
    if (clockhangflag==0) digfilterflag=15;
    //    else digfilterflag=1;
    break;
  case 12:
    //2-unhang all [where to re-hang-use a flag]+1 extra option: clocks hang/clocks unhang here
    //question is if really makes sense to unhang _all_
    hangflag=1;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ;
      GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
      GPIO_Init(GPIOB, &GPIO_InitStructure);

  // and what to hang on c=8,10,11,13
    //  GPIO_Init(GPIOC, &GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11;// | GPIO_Pin_13; //nohang
      GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
      GPIO_Init(GPIOC, &GPIO_InitStructure);
    if (clockhangflag==0) digfilterflag=15;
    //    else digfilterflag=1;
    break;

    //////
  case 13:
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<11);
    // straight to filter and unhang filter options
    hangflag=1;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; 
       GPIO_InitStructure.GPIO_Mode = 0x04;
       GPIO_Init(GPIOB, &GPIO_InitStructure);
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
       GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
       GPIO_Init(GPIOC, &GPIO_InitStructure);
    if (clockhangflag==0) digfilterflag=15;
    //    else digfilterflag=1;
    break;

  case 14:
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5);
    // straight to 40106->filter and unhang filter options
       hangflag=1;
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
       GPIO_InitStructure.GPIO_Mode = 0x04;
       GPIO_Init(GPIOB, &GPIO_InitStructure);
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
       GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
       GPIO_Init(GPIOC, &GPIO_InitStructure);
       if (clockhangflag==0) digfilterflag=15;
       //       else digfilterflag=1;
       break;

  case 15:
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5);
    GPIOB->BSRRL=(1<<3) | (1<<6);
    GPIOC->BSRRL=(1<<11);
    // straight to filter->40106 and unhang filter options
       hangflag=1;
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
       GPIO_InitStructure.GPIO_Mode = 0x04;
       GPIO_Init(GPIOB, &GPIO_InitStructure);
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
       GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
       GPIO_Init(GPIOC, &GPIO_InitStructure);
       if (clockhangflag==0) digfilterflag=15;
       //       else digfilterflag=1;
       break;

#else

       // new cases 11,12,13,14,15
       // =4 +1 (filter->40106->filter)//40106->filter=pb5
  case 11:
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<8) | (1<<9);
    GPIOB->BSRRL=(1<<3) | (1<<6) | (1<<5);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 12:
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<8) | (1<<9);
    GPIOB->BSRRL=(1<<3) | (1<<5) | (1<<6);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 13:
    //filterpath->digital no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4);
    GPIOB->BSRRL=(1<<3) | (1<<5) | (1<<6) |(1<<8) |(1<<9);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 14: //
    //filterpath->digital distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL=(1<<3) | (1<<5) | (1<<6) |(1<<8) |(1<<9);
    GPIOC->BSRRL=(1<<11);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 15:///????to filter/digital with distort as HDGEN bit
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6);
    GPIOC->BSRRH= (1<<11) | (1<<8) | (1<<9);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= ((1<<10)&((hdgen&1)<<10));
    if (clockhangflag==0) digfilterflag=25;
    //    else digfilterflag=1; // 40106 always running anyways+8
    break;
  
#endif

    ////////////

  case 16:
    //4-filterthen40106-->|filterpath=1lm/2digital-distort on/off in each case
    //filterpath->lm no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOB->BSRRL=(1<<3) | (1<<6);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 17: // 
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL=(1<<3) | (1<<6);
    GPIOC->BSRRL=(1<<11);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 18:
    //filterpath->digital no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5);
    GPIOB->BSRRL=(1<<3) | (1<<6) |(1<<8) |(1<<9);
    GPIOC->BSRRL=(1<<11) | (1<<10);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 19: //
    //filterpath->digital distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<4) | (1<<5);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL=(1<<3) | (1<<6) |(1<<8) |(1<<9);
    GPIOC->BSRRL=(1<<11);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 20:
    //5-40106thenfilter-->|
    //filterpath->lm no distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5);
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 21:
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11) | (1<<10);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 22:
    //filter->digital no distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 23:
    //filter->digital distort
    GPIOB->BSRRH= (1<<2) | (1<<3) | (1<<6);
    GPIOC->BSRRH= (1<<11) | (1<<10);
    GPIOB->BSRRL= (1<<0) | (1<<4) | (1<<5) | (1<<8) | (1<<9);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 24:
    //6-justfilter------->|
    //filterpath->lm no distort
    //    augflag=1; // TESTY! AUG
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6) | (1<<8) | (1<<9);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<10) | (1<<11);
    if (clockhangflag==0) digfilterflag=12;
    else digfilterflag=0;
    break;
  case 25:
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<10);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<11);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0; // 40106 always running anyways
    break;
  case 26:
    //filterpath->digital no distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6);
    GPIOB->BSRRL= (1<<4) | (1<<8) | (1<<9);
    GPIOC->BSRRL= (1<<10) | (1<<11);
    if (clockhangflag==0) digfilterflag=9;
    //    else digfilterflag=1;
    break;
  case 27:
    //filterpath->digital distort
    GPIOB->BSRRH= (1<<0) | (1<<2) | (1<<3) | (1<<5) | (1<<6);
    GPIOC->BSRRH= (1<<11) | (1<<8) | (1<<9);
    GPIOB->BSRRL= (1<<4);
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1; // 40106 always running anyways
    break;

  case 28: // then final 40106
    //5-40106thenfilter-->|
    //filterpath->lm no distort
    GPIOB->BSRRH= (1<<2) | (1<<4) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<5) | (1<<3) | (1<<6) ;
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 29:
    //filterpath->lm distort
    GPIOB->BSRRH= (1<<2) | (1<<4) | (1<<8) | (1<<9);
    GPIOC->BSRRH= (1<<11) | (1<<10);
    GPIOB->BSRRL= (1<<0) | (1<<5) | (1<<3) | (1<<6) ;
    if (clockhangflag==0) digfilterflag=14;
    else digfilterflag=0;
    break;
  case 30:
    //filter->digital no distort
    GPIOB->BSRRH= (1<<2) | (1<<4);
    GPIOC->BSRRH= (1<<11);
    GPIOB->BSRRL= (1<<0) | (1<<5) | (1<<8) | (1<<9) | (1<<3) | (1<<6);
    GPIOC->BSRRL= (1<<10);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  case 31:
    //filter->digital distort
    GPIOB->BSRRH= (1<<2) | (1<<4);
    GPIOC->BSRRH= (1<<11) | (1<<10);
    GPIOB->BSRRL= (1<<0) | (1<<5) | (1<<6) | (1<<8) | (1<<9) | (1<<3);
    if (clockhangflag==0) digfilterflag=11;
    //    else digfilterflag=1;
    break;
  }
}

void reset_clocks(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* GPIOB Configuration:  TIM1 CH2 (PA9) */
  /*  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 

  // Connect TIM1 pins to PA9 
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);*/

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

void reset_switches(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ; // this works for pin 4 (should use for all pins?)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void setup_switches(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIO_InitTypeDef  GPIO_InitStructure;
  //  GPIO_Init(GPIOB, &GPIO_InitStructure); //????
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ; // this works for pin 4 (should use for all pins?)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  //  GPIO_Init(GPIOC, &GPIO_InitStructure); //????
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
}

void switchalloff(void)
{
  GPIOC->ODR = 0;
  GPIOB->ODR = 0;
}

void initpwm(void){ // THIS ONE WORKS! 

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
  TIM_OCInitStructure.TIM_Pulse = 333;
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
  TIM_OCInitStructure.TIM_Pulse = 333;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM8, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM8, ENABLE);

  /* TIM8 enable counter */
  TIM_Cmd(TIM8, ENABLE);

  /* do TIM1 channel and enables */

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  // new structure???

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* PWM1 Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 333;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM1, ENABLE);

  /* TIM1 enable counter */
  TIM_Cmd(TIM1, ENABLE);
}

void setlmpwm(uint16_t value, uint16_t value2){
 
  ///  TIM3->CCR4 = value2;
  //  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // was by 28 MHz
  PrescalerValue = 2;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = value; // was 665
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* do TIM3 channel and enables */

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  // new structure???

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 100;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC4Init(TIM3, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable); // fixed here AUGUST as was TIM8-no diff really!

  TIM_CtrlPWMOutputs(TIM3, ENABLE);

  /* TIM8 enable counter */
  TIM_Cmd(TIM3, ENABLE);

}

void setmaximpwm(uint16_t value){

  // uint32_t SystemCoreClock = 168000000;

  //  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1; // 28 MHz
  PrescalerValue = 2; //was 2
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
  TIM_OCInitStructure.TIM_Pulse = 100; // was 100
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

  /* TIM1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); // APB2?

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* TIM8 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE); // APB2?

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // AHB1???

  /* GPIOB Configuration:  TIM1 CH2 (PA9) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 

  /* Connect TIM1 pins to PA9 */  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);

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

void setlmmmpwm(uint16_t value){ // AUGUST

  TIM3->CCR4 = value;
  }


void set40106pwm(uint16_t value){

  TIM1->CCR2 = value;
  }


