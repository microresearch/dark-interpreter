/* switching signal paths and hardware setup */

#ifndef __hardware__
#define __hardware__

//#define ARM_MATH_CM4

#include "stm32f4xx.h"
#include "arm_math.h"

void dohardwareswitch(uint16_t modder, u8 hdgen);
void setup_switches(void);
void reset_switches(void);
void reset_clocks(void);
void switch_jack(void);
void changepwm(uint16_t value);
void initpwm(void);
void setlmpwm(uint16_t value, uint16_t value2);
void set40106pwm(uint16_t value);
void setmaximpwm(uint16_t value);
#endif

/* what are pins we need to be switching?

- SW0 PB0 - out to 40106
- SW1 PC11 - out to filter
- SW2 PB2 - out to jack ***
- SW3 PB3 - filter to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
- SW6 PB6 - 40106 to jack
- SW7 PB7 - last 3 just as pads?
- SW8 PB8
- SW9 PB9
- feedbacksw-PC8
- filterdistort-PC10

PWM:

filterpwm-PB1
filterclock-PC9
40106powerpwm-PC14

are any of these swapped???

*/

/* what are ADC pins

ADC0,1,2,3,4=potis /// on ADC1
5,6,7,8=touch /// 8 is on ADC1_IN10

AD620 is on ADC1_IN11

*/

/* audio hardware

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/
