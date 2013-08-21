/* switching signal paths and hardware setup */

#include "hardware.h"

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

PWM: so 3 channels...

filterpwm-PB1 (is for lm13700)
filterclock-PC9 (is for maxim)
40106powerpwm-PC14

are any of these swapped???

*/

/* first step switch - SW2 PB2 - out to jack ***/


#define JACKOUT (1 << 2) /* port B, pin 2 */
#define LINEINN (1 << 7) /* port B, pin 7 */

void setup_switches(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER |= (1 << (2 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)) ;	// JACK
}

void switch_jack(void)
{
  // clear first 3 and JACKOUT is on
  GPIOB->ODR &= ~(5);
  GPIOB->ODR |= JACKOUT;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


/* fill in rest of switch functions*/

/* 3 timers/pwm */

