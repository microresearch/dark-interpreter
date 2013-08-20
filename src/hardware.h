/* switching signal paths and hardware setup */

/* what are pins we need to be switching?

- SW0 PB0
- SW1 PC11
- SW2 PB2
- SW3 PB3
- SW4 PB4
- SW5 PB5
- SW6 PB6
- SW7 PB7
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
