/*

 Oh, eternity with outstretched wings, that broodest over the secret
 truths in whose roots lie the mysteries of man—his whence, his
 whither—have I searched thee, and struck a right key on thy dreadful
 organ!

 [Thomas de Quincey. The Dark Interpreter] 

*/

/*
 PATH=~/sat/bin:$PATH
 PATH=~/stm32f4/stlink/flash:$PATH
 make stlink_flash
*/

#ifdef PCSIM
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "audio.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
//#include "settings.h"
uint16_t *adc_buffer;
u8 digfilterflag;
int16_t *stacker,*stackery;
int16_t *src, *dst;

void  dohardwareswitch(u8 one,u8 two){
  // nothing
}

void set40106pwm(u16 one){
}

void setmaximpwm(u16 one){
}

void setlmmmpwm(u16 one){
}

#else
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
#include "mdavocoder.h"
#include "simulation.h"
#include "CPUint.h"
#include "CA.h"
//#include "settings.h"
#include "vocode.h"
#include "biquad.h"
#include "effect.h"
#include "PV_vocoder.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

u16 stackery[STACK_SIZE*4]; // 64*4 MAX now
u16 stacker[STACK_SIZE*4]; // 64*4 MAX
#endif

// for knobwork
// TENE: 2,0,3,4,1 // else: 3,0,2,4,1

#ifdef TENE
#define FIRST 2
#define SECOND 0
#define THIRD 3
#define FOURTH 4
#define FIFTH 1
#define UP 6
#define DOWN 8
#define LEFT 5
#define RIGHT 7
#else
#define FIRST 3
#define SECOND 0
#define THIRD 2
#define FOURTH 4
#define FIFTH 1
#define UP 5
#define DOWN 6
#define LEFT 8
#define RIGHT 7
#endif

  const float freq[5][5] __attribute__ ((section (".flash"))) = {
      {600, 1040, 2250, 2450, 2750},
      {400, 1620, 2400, 2800, 3100},
      {250, 1750, 2600, 3050, 3340},
      {400, 750, 2400, 2600, 2900},
      {350, 600, 2400, 2675, 2950}
  };

  const float qqq[5][5] __attribute__ ((section (".flash"))) = {
    {14.424072,21.432398,29.508234,29.453644,30.517193},
    {14.424072,29.213112,34.623474,33.661621,37.268467},
    {6.004305,28.050945,37.508934,36.667324,40.153900},
    {14.424072,13.522194,34.623474,31.257082,34.863983},
{12.620288,10.816360,34.623474,32.158768,35.465038}
  };

  const float mull[5][5] __attribute__ ((section (".flash"))) = {
    {1, 0.44668359215096, 0.35481338923358, 0.35481338923358, 0.1},
    {1, 0.25118864315096, 0.35481338923358, 0.25118864315096, 0.12589254117942},
    {1, 0.031622776601684, 0.15848931924611, 0.079432823472428, 0.03981071705535},
    {1, 0.28183829312645, 0.089125093813375, 0.1, 0.01},
    { 1, 0.1, 0.025118864315096, 0.03981071705535, 0.015848931924611}
  };

static const u16 loggy[4096] __attribute__ ((section (".flash"))) ={1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 64, 64, 64, 64, 64, 64, 64, 65, 65, 65, 65, 65, 65, 66, 66, 66, 66, 66, 66, 67, 67, 67, 67, 67, 68, 68, 68, 68, 68, 68, 69, 69, 69, 69, 69, 69, 70, 70, 70, 70, 70, 71, 71, 71, 71, 71, 71, 72, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 74, 74, 74, 74, 74, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 79, 79, 79, 79, 79, 80, 80, 80, 80, 80, 81, 81, 81, 81, 81, 82, 82, 82, 82, 82, 83, 83, 83, 83, 83, 84, 84, 84, 84, 85, 85, 85, 85, 85, 86, 86, 86, 86, 87, 87, 87, 87, 87, 88, 88, 88, 88, 89, 89, 89, 89, 89, 90, 90, 90, 90, 91, 91, 91, 91, 92, 92, 92, 92, 92, 93, 93, 93, 93, 94, 94, 94, 94, 95, 95, 95, 95, 96, 96, 96, 96, 97, 97, 97, 97, 98, 98, 98, 98, 99, 99, 99, 99, 100, 100, 100, 100, 101, 101, 101, 101, 102, 102, 102, 102, 103, 103, 103, 103, 104, 104, 104, 105, 105, 105, 105, 106, 106, 106, 106, 107, 107, 107, 107, 108, 108, 108, 109, 109, 109, 109, 110, 110, 110, 111, 111, 111, 111, 112, 112, 112, 113, 113, 113, 113, 114, 114, 114, 115, 115, 115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 121, 121, 121, 121, 122, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 126, 126, 126, 127, 127, 127, 128, 128, 128, 128, 129, 129, 129, 130, 130, 130, 131, 131, 131, 132, 132, 132, 133, 133, 133, 134, 134, 135, 135, 135, 136, 136, 136, 137, 137, 137, 138, 138, 138, 139, 139, 139, 140, 140, 140, 141, 141, 142, 142, 142, 143, 143, 143, 144, 144, 144, 145, 145, 146, 146, 146, 147, 147, 147, 148, 148, 149, 149, 149, 150, 150, 150, 151, 151, 152, 152, 152, 153, 153, 154, 154, 154, 155, 155, 156, 156, 156, 157, 157, 158, 158, 158, 159, 159, 160, 160, 160, 161, 161, 162, 162, 162, 163, 163, 164, 164, 164, 165, 165, 166, 166, 167, 167, 167, 168, 168, 169, 169, 170, 170, 170, 171, 171, 172, 172, 173, 173, 174, 174, 174, 175, 175, 176, 176, 177, 177, 178, 178, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183, 183, 184, 184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 190, 191, 191, 192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199, 199, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 210, 210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220, 220, 221, 222, 222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228, 228, 229, 230, 230, 231, 231, 232, 233, 233, 234, 234, 235, 236, 236, 237, 237, 238, 239, 239, 240, 240, 241, 242, 242, 243, 243, 244, 245, 245, 246, 247, 247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 254, 254, 255, 256, 256, 257, 257, 258, 259, 259, 260, 261, 261, 262, 263, 263, 264, 265, 265, 266, 267, 267, 268, 269, 270, 270, 271, 272, 272, 273, 274, 274, 275, 276, 276, 277, 278, 279, 279, 280, 281, 281, 282, 283, 284, 284, 285, 286, 286, 287, 288, 289, 289, 290, 291, 292, 292, 293, 294, 295, 295, 296, 297, 298, 298, 299, 300, 301, 301, 302, 303, 304, 305, 305, 306, 307, 308, 308, 309, 310, 311, 312, 312, 313, 314, 315, 316, 316, 317, 318, 319, 320, 320, 321, 322, 323, 324, 325, 325, 326, 327, 328, 329, 329, 330, 331, 332, 333, 334, 335, 335, 336, 337, 338, 339, 340, 341, 341, 342, 343, 344, 345, 346, 347, 348, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 433, 434, 435, 436, 437, 438, 439, 440, 441, 443, 444, 445, 446, 447, 448, 449, 450, 452, 453, 454, 455, 456, 457, 459, 460, 461, 462, 463, 464, 466, 467, 468, 469, 470, 472, 473, 474, 475, 476, 478, 479, 480, 481, 482, 484, 485, 486, 487, 489, 490, 491, 492, 494, 495, 496, 497, 499, 500, 501, 502, 504, 505, 506, 508, 509, 510, 512, 513, 514, 515, 517, 518, 519, 521, 522, 523, 525, 526, 527, 529, 530, 531, 533, 534, 535, 537, 538, 540, 541, 542, 544, 545, 546, 548, 549, 551, 552, 553, 555, 556, 558, 559, 561, 562, 563, 565, 566, 568, 569, 571, 572, 573, 575, 576, 578, 579, 581, 582, 584, 585, 587, 588, 590, 591, 593, 594, 596, 597, 599, 600, 602, 603, 605, 606, 608, 610, 611, 613, 614, 616, 617, 619, 620, 622, 624, 625, 627, 628, 630, 632, 633, 635, 636, 638, 640, 641, 643, 645, 646, 648, 650, 651, 653, 654, 656, 658, 659, 661, 663, 665, 666, 668, 670, 671, 673, 675, 676, 678, 680, 682, 683, 685, 687, 689, 690, 692, 694, 696, 697, 699, 701, 703, 705, 706, 708, 710, 712, 714, 715, 717, 719, 721, 723, 724, 726, 728, 730, 732, 734, 736, 737, 739, 741, 743, 745, 747, 749, 751, 753, 755, 756, 758, 760, 762, 764, 766, 768, 770, 772, 774, 776, 778, 780, 782, 784, 786, 788, 790, 792, 794, 796, 798, 800, 802, 804, 806, 808, 810, 812, 814, 816, 818, 821, 823, 825, 827, 829, 831, 833, 835, 837, 840, 842, 844, 846, 848, 850, 852, 855, 857, 859, 861, 863, 866, 868, 870, 872, 874, 877, 879, 881, 883, 886, 888, 890, 892, 895, 897, 899, 901, 904, 906, 908, 911, 913, 915, 918, 920, 922, 925, 927, 929, 932, 934, 936, 939, 941, 944, 946, 948, 951, 953, 956, 958, 961, 963, 965, 968, 970, 973, 975, 978, 980, 983, 985, 988, 990, 993, 995, 998, 1000, 1003, 1005, 1008, 1011, 1013, 1016, 1018, 1021, 1024, 1026, 1029, 1031, 1034, 1037, 1039, 1042, 1045, 1047, 1050, 1053, 1055, 1058, 1061, 1063, 1066, 1069, 1071, 1074, 1077, 1080, 1082, 1085, 1088, 1091, 1093, 1096, 1099, 1102, 1105, 1107, 1110, 1113, 1116, 1119, 1122, 1124, 1127, 1130, 1133, 1136, 1139, 1142, 1145, 1147, 1150, 1153, 1156, 1159, 1162, 1165, 1168, 1171, 1174, 1177, 1180, 1183, 1186, 1189, 1192, 1195, 1198, 1201, 1204, 1207, 1210, 1213, 1216, 1220, 1223, 1226, 1229, 1232, 1235, 1238, 1241, 1245, 1248, 1251, 1254, 1257, 1261, 1264, 1267, 1270, 1273, 1277, 1280, 1283, 1286, 1290, 1293, 1296, 1300, 1303, 1306, 1309, 1313, 1316, 1319, 1323, 1326, 1330, 1333, 1336, 1340, 1343, 1347, 1350, 1353, 1357, 1360, 1364, 1367, 1371, 1374, 1378, 1381, 1385, 1388, 1392, 1395, 1399, 1402, 1406, 1410, 1413, 1417, 1420, 1424, 1428, 1431, 1435, 1438, 1442, 1446, 1449, 1453, 1457, 1461, 1464, 1468, 1472, 1475, 1479, 1483, 1487, 1491, 1494, 1498, 1502, 1506, 1510, 1513, 1517, 1521, 1525, 1529, 1533, 1537, 1541, 1545, 1548, 1552, 1556, 1560, 1564, 1568, 1572, 1576, 1580, 1584, 1588, 1592, 1596, 1600, 1604, 1609, 1613, 1617, 1621, 1625, 1629, 1633, 1637, 1642, 1646, 1650, 1654, 1658, 1663, 1667, 1671, 1675, 1680, 1684, 1688, 1692, 1697, 1701, 1705, 1710, 1714, 1718, 1723, 1727, 1732, 1736, 1740, 1745, 1749, 1754, 1758, 1763, 1767, 1772, 1776, 1781, 1785, 1790, 1794, 1799, 1803, 1808, 1813, 1817, 1822, 1826, 1831, 1836, 1840, 1845, 1850, 1854, 1859, 1864, 1869, 1873, 1878, 1883, 1888, 1892, 1897, 1902, 1907, 1912, 1917, 1922, 1926, 1931, 1936, 1941, 1946, 1951, 1956, 1961, 1966, 1971, 1976, 1981, 1986, 1991, 1996, 2001, 2006, 2011, 2017, 2022, 2027, 2032, 2037, 2042, 2048, 2053, 2058, 2063, 2068, 2074, 2079, 2084, 2090, 2095, 2100, 2106, 2111, 2116, 2122, 2127, 2132, 2138, 2143, 2149, 2154, 2160, 2165, 2171, 2176, 2182, 2187, 2193, 2198, 2204, 2210, 2215, 2221, 2226, 2232, 2238, 2244, 2249, 2255, 2261, 2266, 2272, 2278, 2284, 2290, 2295, 2301, 2307, 2313, 2319, 2325, 2331, 2337, 2342, 2348, 2354, 2360, 2366, 2372, 2378, 2385, 2391, 2397, 2403, 2409, 2415, 2421, 2427, 2433, 2440, 2446, 2452, 2458, 2465, 2471, 2477, 2483, 2490, 2496, 2502, 2509, 2515, 2522, 2528, 2534, 2541, 2547, 2554, 2560, 2567, 2573, 2580, 2586, 2593, 2600, 2606, 2613, 2619, 2626, 2633, 2639, 2646, 2653, 2660, 2666, 2673, 2680, 2687, 2694, 2700, 2707, 2714, 2721, 2728, 2735, 2742, 2749, 2756, 2763, 2770, 2777, 2784, 2791, 2798, 2805, 2812, 2820, 2827, 2834, 2841, 2848, 2856, 2863, 2870, 2877, 2885, 2892, 2899, 2907, 2914, 2922, 2929, 2937, 2944, 2951, 2959, 2967, 2974, 2982, 2989, 2997, 3004, 3012, 3020, 3027, 3035, 3043, 3051, 3058, 3066, 3074, 3082, 3090, 3097, 3105, 3113, 3121, 3129, 3137, 3145, 3153, 3161, 3169, 3177, 3185, 3193, 3201, 3209, 3218, 3226, 3234, 3242, 3250, 3259, 3267, 3275, 3284, 3292, 3300, 3309, 3317, 3326, 3334, 3343, 3351, 3360, 3368, 3377, 3385, 3394, 3403, 3411, 3420, 3429, 3437, 3446, 3455, 3464, 3472, 3481, 3490, 3499, 3508, 3517, 3526, 3535, 3544, 3553, 3562, 3571, 3580, 3589, 3598, 3607, 3616, 3626, 3635, 3644, 3653, 3663, 3672, 3681, 3691, 3700, 3709, 3719, 3728, 3738, 3747, 3757, 3766, 3776, 3785, 3795, 3805, 3814, 3824, 3834, 3844, 3853, 3863, 3873, 3883, 3893, 3903, 3913, 3922, 3932, 3942, 3952, 3963, 3973, 3983, 3993, 4003, 4013, 4023, 4034, 4044, 4054, 4064, 4075, 4085, 4096, 4106, 4116, 4127, 4137, 4148, 4158, 4169, 4180, 4190, 4201, 4212, 4222, 4233, 4244, 4255, 4265, 4276, 4287, 4298, 4309, 4320, 4331, 4342, 4353, 4364, 4375, 4386, 4397, 4408, 4420, 4431, 4442, 4453, 4465, 4476, 4488, 4499, 4510, 4522, 4533, 4545, 4556, 4568, 4580, 4591, 4603, 4615, 4626, 4638, 4650, 4662, 4674, 4685, 4697, 4709, 4721, 4733, 4745, 4757, 4770, 4782, 4794, 4806, 4818, 4830, 4843, 4855, 4867, 4880, 4892, 4905, 4917, 4930, 4942, 4955, 4967, 4980, 4993, 5005, 5018, 5031, 5044, 5056, 5069, 5082, 5095, 5108, 5121, 5134, 5147, 5160, 5173, 5186, 5200, 5213, 5226, 5239, 5253, 5266, 5279, 5293, 5306, 5320, 5333, 5347, 5360, 5374, 5388, 5401, 5415, 5429, 5443, 5457, 5470, 5484, 5498, 5512, 5526, 5540, 5554, 5569, 5583, 5597, 5611, 5625, 5640, 5654, 5668, 5683, 5697, 5712, 5726, 5741, 5755, 5770, 5785, 5799, 5814, 5829, 5844, 5859, 5874, 5889, 5903, 5918, 5934, 5949, 5964, 5979, 5994, 6009, 6025, 6040, 6055, 6071, 6086, 6102, 6117, 6133, 6148, 6164, 6180, 6195, 6211, 6227, 6243, 6259, 6274, 6290, 6306, 6322, 6338, 6355, 6371, 6387, 6403, 6419, 6436, 6452, 6469, 6485, 6501, 6518, 6535, 6551, 6568, 6585, 6601, 6618, 6635, 6652, 6669, 6686, 6703, 6720, 6737, 6754, 6771, 6788, 6806, 6823, 6840, 6858, 6875, 6892, 6910, 6928, 6945, 6963, 6981, 6998, 7016, 7034, 7052, 7070, 7088, 7106, 7124, 7142, 7160, 7178, 7197, 7215, 7233, 7252, 7270, 7288, 7307, 7326, 7344, 7363, 7382, 7400, 7419, 7438, 7457, 7476, 7495, 7514, 7533, 7552, 7571, 7591, 7610, 7629, 7649, 7668, 7688, 7707, 7727, 7746, 7766, 7786, 7806, 7826, 7845, 7865, 7885, 7905, 7926, 7946, 7966, 7986, 8006, 8027, 8047, 8068, 8088, 8109, 8129, 8150, 8171, 8192, 8212, 8233, 8254, 8275, 8296, 8317, 8338, 8360, 8381, 8402, 8424, 8445, 8466, 8488, 8510, 8531, 8553, 8575, 8596, 8618, 8640, 8662, 8684, 8706, 8728, 8751, 8773, 8795, 8817, 8840, 8862, 8885, 8907, 8930, 8953, 8976, 8998, 9021, 9044, 9067, 9090, 9113, 9137, 9160, 9183, 9206, 9230, 9253, 9277, 9300, 9324, 9348, 9371, 9395, 9419, 9443, 9467, 9491, 9515, 9540, 9564, 9588, 9612, 9637, 9661, 9686, 9711, 9735, 9760, 9785, 9810, 9835, 9860, 9885, 9910, 9935, 9960, 9986, 10011, 10036, 10062, 10088, 10113, 10139, 10165, 10191, 10216, 10242, 10268, 10295, 10321, 10347, 10373, 10400, 10426, 10453, 10479, 10506, 10533, 10559, 10586, 10613, 10640, 10667, 10694, 10721, 10749, 10776, 10803, 10831, 10858, 10886, 10914, 10941, 10969, 10997, 11025, 11053, 11081, 11109, 11138, 11166, 11194, 11223, 11251, 11280, 11309, 11337, 11366, 11395, 11424, 11453, 11482, 11511, 11541, 11570, 11599, 11629, 11659, 11688, 11718, 11748, 11778, 11807, 11837, 11868, 11898, 11928, 11958, 11989, 12019, 12050, 12080, 12111, 12142, 12173, 12204, 12235, 12266, 12297, 12328, 12360, 12391, 12423, 12454, 12486, 12518, 12549, 12581, 12613, 12645, 12677, 12710, 12742, 12774, 12807, 12839, 12872, 12905, 12938, 12971, 13003, 13037, 13070, 13103, 13136, 13170, 13203, 13237, 13270, 13304, 13338, 13372, 13406, 13440, 13474, 13508, 13543, 13577, 13612, 13646, 13681, 13716, 13751, 13785, 13821, 13856, 13891, 13926, 13962, 13997, 14033, 14068, 14104, 14140, 14176, 14212, 14248, 14284, 14321, 14357, 14394, 14430, 14467, 14504, 14540, 14577, 14615, 14652, 14689, 14726, 14764, 14801, 14839, 14877, 14914, 14952, 14990, 15028, 15067, 15105, 15143, 15182, 15220, 15259, 15298, 15337, 15376, 15415, 15454, 15493, 15533, 15572, 15612, 15652, 15691, 15731, 15771, 15811, 15852, 15892, 15932, 15973, 16013, 16054, 16095, 16136, 16177, 16218, 16259, 16301, 16342, 16384, 16425, 16467, 16509, 16551, 16593, 16635, 16677, 16720, 16762, 16805, 16848, 16890, 16933, 16976, 17020, 17063, 17106, 17150, 17193, 17237, 17281, 17325, 17369, 17413, 17457, 17502, 17546, 17591, 17635, 17680, 17725, 17770, 17815, 17861, 17906, 17952, 17997, 18043, 18089, 18135, 18181, 18227, 18274, 18320, 18367, 18413, 18460, 18507, 18554, 18601, 18649, 18696, 18743, 18791, 18839, 18887, 18935, 18983, 19031, 19080, 19128, 19177, 19225, 19274, 19323, 19372, 19422, 19471, 19521, 19570, 19620, 19670, 19720, 19770, 19820, 19871, 19921, 19972, 20023, 20073, 20125, 20176, 20227, 20278, 20330, 20382, 20433, 20485, 20537, 20590, 20642, 20695, 20747, 20800, 20853, 20906, 20959, 21012, 21066, 21119, 21173, 21227, 21281, 21335, 21389, 21443, 21498, 21553, 21607, 21662, 21717, 21773, 21828, 21883, 21939, 21995, 22051, 22107, 22163, 22219, 22276, 22332, 22389, 22446, 22503, 22560, 22618, 22675, 22733, 22791, 22849, 22907, 22965, 23023, 23082, 23141, 23199, 23258, 23318, 23377, 23436, 23496, 23556, 23615, 23675, 23736, 23796, 23856, 23917, 23978, 24039, 24100, 24161, 24223, 24284, 24346, 24408, 24470, 24532, 24595, 24657, 24720, 24783, 24846, 24909, 24972, 25036, 25099, 25163, 25227, 25291, 25355, 25420, 25485, 25549, 25614, 25679, 25745, 25810, 25876, 25942, 26007, 26074, 26140, 26206, 26273, 26340, 26407, 26474, 26541, 26609, 26676, 26744, 26812, 26880, 26949, 27017, 27086, 27155, 27224, 27293, 27362, 27432, 27502, 27571, 27642, 27712, 27782, 27853, 27924, 27995, 28066, 28137, 28209, 28281, 28352, 28424, 28497, 28569, 28642, 28715, 28788, 28861, 28934, 29008, 29081, 29155, 29230, 29304, 29378, 29453, 29528, 29603, 29678, 29754, 29829, 29905, 29981, 30057, 30134, 30210, 30287, 30364, 30441, 30519, 30596, 30674, 30752, 30830, 30909, 30987, 31066, 31145, 31224, 31304, 31383, 31463, 31543, 31623, 31704, 31784, 31865, 31946, 32027, 32109, 32190, 32272, 32354, 32436, 32519, 32602, 32684, 32767};

u8 howmanyhardvill=1,howmany40106vill=1,howmanylmvill=1,howmanyhdgenervill=1,howmanymaximvill=1;
u8 readspeed=1,writespeed=1,filtinspeed=1,filtoutspeed=1;
int16_t dirryw=1,dirryr=1,dirryf=1,dirryff=1;
u8 hardcompress=1;
int16_t newdirection[8]={-256,-255,1,255,256,254,-1,-257};
signed char direction[2]={-1,1};
u8 wormflag[10]={0,0,0,0,0,0,0,0,0,0};
u16 *buf16;

#define delay()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#ifndef PCSIM
extern int16_t audio_buffer[AUDIO_BUFSZ];
u8* datagenbuffer = (u8*)0x10000000;
//#define randi() ((rand()*adc_buffer[9])%4096) // 12 bits
#define randi() (adc_buffer[9]) // 12 bits
//#define randi() (rand()%4096)
#else //PCSIM
#define randi() (rand()%4096)
u8* datagenbuffer;
extern int16_t* audio_buffer;
#endif
extern u8 digfilterflag;

u8 wormdir; // worm direction
u8 table[21]; 
u16 sin_data[256];  // sine LUT Array

u8 fingerdir(u8 *speedmod){
  u8 handleft, handright, up=0,down=0,left=0,right=0;//,upspeed=0,downspeed=0,leftspeed=0,rightspeed=0;
  u8 handupp, handdown;
  u8 result=5;

  for (u8 x=0;x<8;x++){
    handupp=adc_buffer[UP]>>8; // 4 bits=16 
    handdown=adc_buffer[DOWN]>>8;
    handleft=adc_buffer[LEFT]>>8;
    handright=adc_buffer[RIGHT]>>8;
    /*    upspeed+=handupp;
    downspeed+=handdown;
    leftspeed+=handleft;
    rightspeed+=handright;*/

    if (handupp>2) up++; // was 3 - more sensitive now
  if (handdown>2) down++;
  if (handleft>2) left++;
  if (handright>2) right++;
  } // changed from end
  if (up>4 && up>down && up>left && up>right) {
    result=2; 
    //    *speedmod=upspeed;
    *speedmod=adc_buffer[UP]>>4;// was >>6 for 6 bits AUG
  }
  else if (down>4 && down>left && down>right) {
    result=3; 
    //*speedmod=downspeed;
    *speedmod=adc_buffer[DOWN]>>4;
  }
  else if (left>4 && left>right) {
    result=0; 
    //    *speedmod=leftspeed;
    *speedmod=adc_buffer[LEFT]>>4;
  }
  else if (right>4) {
    result=1; 
    //*speedmod=rightspeed;
    *speedmod=adc_buffer[RIGHT]>>4;
  }
  //  }
  return result;
}


u8 fingerdirupdown(void){

  u8 handleft, handright, left=0,right=0;

  for (u8 x=0;x<16;x++){
  handleft=adc_buffer[UP]>>8;
  handright=adc_buffer[DOWN]>>8;
  if (handleft>2) left++;
  if (handright>2) right++;

  if (left>8 && left>right) {
    return 0;
  }
  else if (right>8 && right>left) {
    return 1;
  }
  }
  return 2;
}

////////////////////////////////////////////////////////////////////////////////

villagerr village_write[MAX_VILLAGERS+1];
villagerr village_read[MAX_VILLAGERS+1];
villagerr village_filtin[MAX_VILLAGERS+1];
villagerr village_filtout[MAX_VILLAGERS+1];

villager_effect village_effect[17];

//mirror mvillage_write[MAX_VILLAGERS+1];
//mirror mvillage_read[MAX_VILLAGERS+1];
//mirror mvillage_filtin[MAX_VILLAGERS+1];
//mirror mvillage_filtout[MAX_VILLAGERS+1];


villager_datagenwalker village_datagenwalker[MAX_VILLAGERS+1];
villager_generic village_datagen[MAX_VILLAGERS+1];
villager_hardware village_hardware[17];
villager_hardwarehaha village_40106[17];
villager_hardwarehaha village_hdgener[17];
villager_hardwarehaha village_lm[17];
villager_hardwarehaha village_maxim[17];

u8 howmanydatagenwalkervill=1, howmanydatavill=1,howmanyeffectvill=1,howmanywritevill=1,howmanyfiltinvill=1, howmanyfiltoutvill=1,howmanyreadvill=1;

u16 counterd=0, databegin=0,dataend=32767;
u8 deldata=0,dataspeed=1;
int16_t dirryd=1;
u8 inp=0;

// effects tests

mdavocoder *mdavocod;
arm_biquad_casd_df1_inst_f32 df[5][5];
float coeffs[5][5][5];//{a0 a1 a2 -b1 -b2} b1 and b2 negate

u16 nextdatagen(void){
  u16 tmp,tmpp;
  u8 x;
  static u8 whichdatagenwalkervillager=0;
  static u16 countdatagenwalker=0;

  x=whichdatagenwalkervillager%howmanydatagenwalkervill;
  countdatagenwalker+=village_datagenwalker[x].step;
  tmp=village_datagenwalker[x].knoboffset; // as is =32768 for datagenwalker
  if (tmp==32768) tmp=32767;
  tmpp=tmp+(buf16[(village_datagenwalker[x].dataoffset+village_datagenwalker[x].samplepos)%32768])%(32768-tmp);
  //  tmp=buf16[(village_datagenwalker[x].dataoffset+village_datagenwalker[x].samplepos)%32768];

  village_datagenwalker[x].samplepos+=village_datagenwalker[x].dirry;
  if (village_datagenwalker[x].samplepos>=village_datagenwalker[x].length) village_datagenwalker[x].samplepos=0;
  else if (village_datagenwalker[x].samplepos<0) village_datagenwalker[x].samplepos=village_datagenwalker[x].length;

  if (countdatagenwalker>=village_datagenwalker[x].length){
    countdatagenwalker=0;
    whichdatagenwalkervillager++; //u8
  }
  return tmpp;
}

void main(void)
{
  // formerly in audio.c
  u8 whichvillager;
  u16 tmpw,tmps,tmpp;

  // order that all inits and audio_init called seems to be important
  u16 x,addr,count;
  //  u8 exestack[MAX_EXE_STACK];
  u8 xx; // TESTY!

  // effects init

  mdavocod=(mdavocoder *)malloc(sizeof(mdavocoder));
  mdaVocoder_init(mdavocod);

  float Fc,Fs=48000.0f,Q,peakGain;
  float a0,a1,a2,b1,b2,norm,V,K;
  float *state[5][5];

  for (xx=0;xx<5;xx++){// five formants

  for (x=0;x<5;x++){
    Fc=freq[xx][x];
    Q=qqq[xx][x];
    K = tanf(M_PI * Fc / Fs);
    norm = 1 / (1 + K / Q + K * K);
    a0 = K / Q * norm;
    a1 = 0;
    a2 = -a0;
    b1 = 2 * (K * K - 1) * norm;
    b2 = (1 - K / Q + K * K) * norm;

    coeffs[xx][x][0]=a0*mull[xx][x]; coeffs[xx][x][1]=a1*mull[xx][x]; coeffs[xx][x][2]=a2*mull[xx][x]; coeffs[xx][x][3]=-b1*mull[xx][x]; coeffs[xx][x][4]=-b2*mull[xx][x];
      /// can also just mult coeffs????
    state[xx][x] = (float*)malloc(4*sizeof(float));
    arm_biquad_cascade_df1_init_f32(&df[xx][x],1,coeffs[xx][x],state[xx][x]);
  }
  }

  //////////////

  // init all basic villagers

  for (xx=0;xx<64;xx++){

    village_read[xx].counterr=0;
    village_read[xx].del=0;
    village_read[xx].compress=1;
    village_read[xx].mirrormod=0;
    village_read[xx].mirrordel=0;
    village_read[xx].infected=0;
    village_read[xx].samplepos=0;
    village_read[xx].speed=1;
    village_read[xx].step=1;
    village_read[xx].start=0;
    village_read[xx].wrap=100; // TODO test
    village_read[xx].dir=1;
    village_read[xx].dirry=1;
    village_read[xx].samplepos=0;
    village_read[xx].offset=0;
    village_read[xx].effect=1.0f;
    village_read[xx].effectinv=0.0f;
    village_read[xx].overlay=0;
    village_read[xx].running=0;

    village_write[xx].counterr=0;
    village_write[xx].del=0;
    village_write[xx].compress=1;
    village_write[xx].mirrormod=0;
    village_write[xx].mirrordel=0;
    village_write[xx].infected=0;
    village_write[xx].samplepos=0;
    village_write[xx].speed=1;
    village_write[xx].step=1;
    village_write[xx].start=0;
    village_write[xx].wrap=100; // TODO test
    village_write[xx].dir=1;
    village_write[xx].dirry=1;
    village_write[xx].samplepos=0;
    village_write[xx].offset=0;
    village_write[xx].effect=1.0f;
    village_write[xx].effectinv=0.0f;
    village_write[xx].overlay=0;
    village_write[xx].running=0;

#ifndef LACH
    village_filtin[xx].counterr=0;
    village_filtin[xx].del=0;
    village_filtin[xx].compress=1;
    village_filtin[xx].mirrormod=0;
    village_filtin[xx].mirrordel=0;
    village_filtin[xx].infected=0;
    village_filtin[xx].samplepos=0;
    village_filtin[xx].speed=1;
    village_filtin[xx].step=1;
    village_filtin[xx].start=0;
    village_filtin[xx].wrap=100; // TODO test
    village_filtin[xx].dir=1;
    village_filtin[xx].dirry=1;
    village_filtin[xx].samplepos=0;
    village_filtin[xx].offset=0;
    village_filtin[xx].effect=1.0f;
    village_filtin[xx].effectinv=0.0f;
    village_filtin[xx].overlay=0;
    village_filtin[xx].running=0;

    village_filtout[xx].counterr=0;
    village_filtout[xx].del=0;
    village_filtout[xx].compress=1;
    village_filtout[xx].mirrormod=0;
    village_filtout[xx].mirrordel=0;
    village_filtout[xx].infected=0;
    village_filtout[xx].samplepos=0;
    village_filtout[xx].speed=1;
    village_filtout[xx].step=1;
    village_filtout[xx].start=0;
    village_filtout[xx].wrap=100; // TODO test
    village_filtout[xx].dir=1;
    village_filtout[xx].dirry=1;
    village_filtout[xx].samplepos=0;
    village_filtout[xx].offset=0;
    village_filtout[xx].effect=1.0f;
    village_filtout[xx].effectinv=0.0f;
    village_filtout[xx].overlay=0;
#endif

    // datagenwalker
    village_datagenwalker[xx].length = 100;
    village_datagenwalker[xx].dataoffset = 0;
    village_datagenwalker[xx].knoboffset = 0;
    village_datagenwalker[xx].samplepos = 0;
    village_datagenwalker[xx].dirry =1;
    village_datagenwalker[xx].speed = 1;
    village_datagenwalker[xx].step = 1;
    village_datagenwalker[xx].dir = 1;
    
    // datagen
    village_datagen[xx].start = 0;
    village_datagen[xx].CPU = 0;
    village_datagen[xx].wrap=100;
    village_datagen[xx].position=0;
    village_datagen[xx].del=0;
    village_datagen[xx].step=1;
    village_datagen[xx].speed=1;
    village_datagen[xx].dir=1;
    village_datagen[xx].running=0;
    village_datagen[xx].m_stack_pos=0;
    village_datagen[xx].m_reg16bit1=0;
    village_datagen[xx].m_reg8bit1=0;
    village_datagen[xx].m_reg8bit2=0;
	}


#ifndef LACH
  for (xx=0;xx<17;xx++){
    village_hardware[xx].length=16;
    village_hardware[xx].inp=0;
    village_hardware[xx].setting=0;

    village_40106[xx].length=16;
    village_40106[xx].dataoffset=0;
    village_40106[xx].knoboffset=0;
    village_40106[xx].samplepos=0;
    village_40106[xx].dirry=1;
    village_40106[xx].dir=1;
    village_40106[xx].speed=1;
    village_40106[xx].step=1;

    village_hdgener[xx].length=16;
    village_hdgener[xx].dataoffset=0;
    village_hdgener[xx].knoboffset=0;
    village_hdgener[xx].samplepos=0;
    village_hdgener[xx].dirry=1;
    village_hdgener[xx].dir=1;
    village_hdgener[xx].speed=1;
    village_hdgener[xx].step=1;

    village_lm[xx].length=16;
    village_lm[xx].dataoffset=0;
    village_lm[xx].knoboffset=0;
    village_lm[xx].samplepos=0;
    village_lm[xx].dirry=1;
    village_lm[xx].dir=1;
    village_lm[xx].speed=1;
    village_lm[xx].step=1;

    village_maxim[xx].length=16;
    village_maxim[xx].dataoffset=0;
    village_maxim[xx].knoboffset=0;
    village_maxim[xx].samplepos=0;
    village_maxim[xx].dirry=1;
    village_maxim[xx].dir=1;
    village_maxim[xx].speed=1;
    village_maxim[xx].step=1;
  }
#endif

  // any other inits/variables????

  // think is all set

  //////////////// older inits
  inittable(3,4,randi());
  static const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase=0;
  int sign_samp,i;
  w= 2*pi;
  w= w/256;

  for (i = 0; i <= 256; i++)
    {
      yi= 32767*sinf(phase); // was 2047???
      phase=phase+w;
      sign_samp=32767+yi;     // dc offset
      sin_data[i]=sign_samp; // write value into array
    }

#ifndef PCSIM
  //	ADC1_Initonce();
  ADC1_Init((uint16_t *)adc_buffer);

#ifndef LACH
  setup_switches();
#endif

  // maintain order
  Audio_Init();
  Codec_Init(48000); // TODO: variable on startup
  delay();

#ifndef LACH
  initpwm(); 	
#endif		

    I2S_Block_Init();
    I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);

#ifndef LACH
    dohardwareswitch(0,0,0);
#endif
#endif // for ifndef PCSIM

#ifdef PCSIM
  datagenbuffer=(u8*)malloc(65536);
  audio_buffer=(int16_t*)malloc(32768*sizeof(int16_t));
  villager=malloc(VILLAGE_SIZE*sizeof(int16_t));
  stacker=malloc(4*64*sizeof(int16_t));
  stackery=malloc(4*64*sizeof(int16_t));
  adc_buffer=malloc(10*sizeof(int16_t));
  initaudio();
  srandom(time(0));
  src=malloc(BUFF_LEN*sizeof(int16_t));
  dst=malloc(BUFF_LEN*sizeof(int16_t));

  for (x=0;x<(BUFF_LEN);x++){
    src[x]=rand()%65536;
    dst[x]=rand()%65536;
  }

  for (x=0;x<32768;x++){
    audio_buffer[x]=rand()%65536;
  }

#endif

  buf16 = (u16*) datagenbuffer;

  // fill datagenbuffer???

  for (x=0;x<32768;x++){
    buf16[x]=rand()%65536; // was RANDI OCT
    delayxx();
  }
  
	    // compile test
	    //	    	    	    testvocode();
	    //	    	    x=sqrtf(x);

  // inits for datagens

  seirinit(buf16);// LEAVE IN!
  sicrinit(buf16);// LEAVE IN!
  ifsinit(buf16);// LEAVE IN!


  while(1)
    {

#ifdef TEST_STRAIGHT
      // do nothing
#else

#ifdef TEST_EFFECTS
      //runsine
      //u16 runsine(u8 step, u16 count, u16 start, u16 wrap){
         count=runnoise(1,count,0,32767);

#else

#ifdef TEST_EEG
      //write ADC9 into buf16
  for (x=0;x<32768;x++){
        buf16[x]=adc_buffer[9]<<4; // 16 bits
    //    buf16[x]=x*16; // 16 bits
  }
#else

  /// HERE!

  // run through datagen villagers

  if (++deldata>=dataspeed) {
    counterd+=dirryd;
    deldata=0;
  }

  if ((counterd-databegin)>dataend) {
    counterd=databegin;
    for (u8 x=0;x<howmanydatavill;x++){
      village_datagen[x].running=1;
    }
  }	    

  if (counterd<databegin) {
    counterd=dataend+databegin;
    for (u8 x=0;x<howmanydatavill;x++){
      village_datagen[x].running=1;
	  }	    
	  }

  for (x=0;x<howmanydatavill;x++){
    // speed for each
        if ((village_datagen[x].start)<=counterd && village_datagen[x].running==1){// in town
	  if (++village_datagen[x].del>=village_datagen[x].step){

	    //village_datagen[x].cpu=11; // CRASH TESTY!
    switch(village_datagen[x].CPU){      
    case 0:
      village_datagen[x].position=runnoney(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 1:
      village_datagen[x].position=runkrum(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 2:
      village_datagen[x].position=runhodge(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 3:
      village_datagen[x].position=runhodgenet(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 4:
      village_datagen[x].position=runlife(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 5:
      village_datagen[x].position=runcel(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 6:
      village_datagen[x].position=runcel1d(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 7:
      village_datagen[x].position=runfire(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 8:
      village_datagen[x].position=runwire(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 9:
      village_datagen[x].position=runSIR(village_datagen[x].speed,village_datagen[x].position);
      break;
    case 10:
      village_datagen[x].position=runSIR16(village_datagen[x].speed,village_datagen[x].position);
      break;
      /// add in sims
    case 11:
      village_datagen[x].position=runform(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 12:
      village_datagen[x].position=runconv(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 13:
      village_datagen[x].position=runsine(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 14:
      village_datagen[x].position=runconv(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 15:
      village_datagen[x].position=runchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 16:
      village_datagen[x].position=runderefchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 17:
      village_datagen[x].position=runwalkerchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 18:
      village_datagen[x].position=runswapchunk(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 19:
      village_datagen[x].position=runinc(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 20:
      village_datagen[x].position=rundec(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 21:
      village_datagen[x].position=runleft(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 22:
      village_datagen[x].position=runright(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 23:
      village_datagen[x].position=runswap(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 24:
      village_datagen[x].position=runnextinc(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 25:
      village_datagen[x].position=runnextdec(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 26:
      village_datagen[x].position=runnextmult(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 27:
      village_datagen[x].position=runnextdiv(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 28:
      village_datagen[x].position=runcopy(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 29:
      village_datagen[x].position=runzero(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 30:
      village_datagen[x].position=runfull(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 31:
      village_datagen[x].position=runrand(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 32:
      village_datagen[x].position=runknob(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 33:
      village_datagen[x].position=runswapaudio(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 34:
      village_datagen[x].position=runORaudio(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 35:
      village_datagen[x].position=runsimplesir(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 36:
      village_datagen[x].position=runseir(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 37:
      village_datagen[x].position=runsicr(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 38:
      village_datagen[x].position=runifs(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 39:
      village_datagen[x].position=runrossler(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 40:
      village_datagen[x].position=runsecondrossler(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 41:
      village_datagen[x].position=runbrussel(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 42:
      village_datagen[x].position=runspruce(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 43:
      village_datagen[x].position=runoregon(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 44:
      village_datagen[x].position=runfitz(village_datagen[x].speed,village_datagen[x].position,village_datagen[x].start,village_datagen[x].wrap);
      break;
    case 45:
      machine_runnn(village_datagen[x].start,village_datagen[x].wrap); 
      village_datagen[x].position+=village_datagen[x].speed;
      break;
     case 46:
      village_datagen[x].position=xxrunleakystack(&village_datagen[x]);
      break;
     case 47:
      village_datagen[x].position=xxrunbiota(&village_datagen[x]);
      break;
     case 48:
      village_datagen[x].position=xxrun1(&village_datagen[x]);
      break;
     case 49:
      village_datagen[x].position=xxrunworm(&village_datagen[x]);
      break;
     case 50:
      village_datagen[x].position=xxrunstack(&village_datagen[x]);
      break;
     case 51:
      village_datagen[x].position=xxrunbefunge(&village_datagen[x]);
      break;
     case 52:
      village_datagen[x].position=xxrunlang(&village_datagen[x]);
      break;
     case 53:
      village_datagen[x].position=xxrunbf(&village_datagen[x]);
      break;
     case 54:
      village_datagen[x].position=xxrunturm(&village_datagen[x]);
      break;
     case 55:
      village_datagen[x].position=xxrunca(&village_datagen[x]);
      break;
     case 56:
      village_datagen[x].position=xxrunhodge(&village_datagen[x]);
      break;
     case 57:
      village_datagen[x].position=xxrunworm2(&village_datagen[x]);
      break;
     case 58:
      village_datagen[x].position=xxrunleaky(&village_datagen[x]);
      break;
     case 59:
      village_datagen[x].position=xxrunconvy(&village_datagen[x]);
      break;
     case 60:
      village_datagen[x].position=xxrunplague(&village_datagen[x]);
      break;
     case 61:
      village_datagen[x].position=xxrunmicro(&village_datagen[x]);
      break;
     case 62:
      village_datagen[x].position=xxruncw(&village_datagen[x]);
      break;
     case 63:
      village_datagen[x].position=xxrunmasque(&village_datagen[x]);
      break;
    } // end of switch

    if (village_datagen[x].position>(village_datagen[x].start+village_datagen[x].wrap)) {
      village_datagen[x].running=0;
      // reset
      village_datagen[x].position=village_datagen[x].start;
    }
    village_datagen[x].del=0;
	  }
    } // if running
  } // end of x/run thru all villagers

  ///// end of DATAGEN villagers!

  u8 xx, mainmode,ranger,whichx,whichy,spd;

  xx=fingerdir(&spd);

	xx=fingerdir(&spd);

	if (xx!=5){
	  // which mode are we in?
	  mainmode=adc_buffer[FIFTH]>>7; // 5 bits=32
	  //	  if ((adc_buffer[FIFTH]>>8)<8)	  mainmode=0; //TESTY!
	  //	  else mainmode=1; //TESTY!
	  //	  mainmode=11; //TESTY!

	  // TODO _ ordering of modes at end!eg. hardware as first...
	  // group as main walkers, followed by compression series...
	  switch(mainmode){
	  case 0:// WRITE
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    howmanywritevill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	      //	    village_write[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	      tmps=loggy[adc_buffer[SECOND]];
	    }
	    if (adc_buffer[THIRD]>10){
	      //	      village_write[whichvillager].wrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	      tmpw=loggy[adc_buffer[THIRD]]; 
	    }
	    if (adc_buffer[FOURTH]>10){
	      village_write[whichvillager].offset=loggy[adc_buffer[FOURTH]]; // as logarithmic
	    }
	    village_write[whichvillager].dir=xx;
	    village_write[whichvillager].speed=(spd&15)+1; 
	    village_write[whichvillager].step=(spd&240)>>4;
	    if (village_write[whichvillager].dir==2) village_write[whichvillager].dirry=newdirection[wormdir];
	    else if (village_write[whichvillager].dir==3) village_write[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_write[whichvillager].speed;
	    else village_write[whichvillager].dirry=direction[village_write[whichvillager].dir]*village_write[whichvillager].speed;

	    // deal with mirror here:
		village_write[whichvillager].kstart=tmps;
		village_write[whichvillager].kwrap=tmpw;
	    
	    if (village_write[whichvillager].mirrormod){
	      switch(village_write[whichvillager].mirrormod){
	      case 1: // straight datagen
		village_write[whichvillager].start=village_write[whichvillager].mstart;
		village_write[whichvillager].wrap=village_write[whichvillager].mwrap;
		break;
	      case 2: // addition with wrap
		village_write[whichvillager].start=(tmps+village_write[whichvillager].mstart)%32768;
		village_write[whichvillager].wrap=(tmpw+village_write[whichvillager].mwrap)%32768;
		break;
	      case 3: // addition up to 32768
		tmpp=32768-tmps;
		village_write[whichvillager].start=tmps+(village_write[whichvillager].mstart%tmpp);
		tmpp=32768-tmpw;
		village_write[whichvillager].wrap=tmpw+(village_write[whichvillager].mwrap%tmpp);
		break;
		// TODO: other cases: subtraction, and, or, modulus
	      }
	    }
	      else {
		village_write[whichvillager].start=tmps;
		village_write[whichvillager].wrap=tmpw;
	      }
	    if (village_write[whichvillager].running==0){
	    if (village_write[whichvillager].dirry>0) village_write[whichvillager].samplepos=village_write[whichvillager].start;
	    else village_write[whichvillager].samplepos=village_write[whichvillager].start+village_write[whichvillager].wrap;
	    }
	    break;

	  case 1: // WRITE overlays and compression
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    // overlap, effect
	    village_write[whichvillager].overlay=adc_buffer[SECOND]>>7;// 5 bits=32
	    village_write[whichvillager].effect=(float)(adc_buffer[THIRD])/4096.0f;// 
	    village_write[whichvillager].effectinv=1.0f-village_write[whichvillager].effect;
	    tmpp=(32768-(adc_buffer[FOURTH]<<3))+1;//
	    writespeed=spd&15; // check how many bits is spd? 8 as changed in main
	    dirryw=(spd&240)>>4;
	    // finger as mirrormod finger is---> UP.2 DOWN.3 LEFT.0. RIGHT.1b
	    if (xx==2 || xx==3){
	    village_write[whichvillager].mirrormod=1;
	    village_write[whichvillager].fingered=xx;
	    }

	    // deal with mirror here
	    village_write[whichvillager].kcompress=tmpp;

	    if (village_write[whichvillager].mirrormod){
	      switch(village_write[whichvillager].mirrormod){
	      case 1: // straight datagen
		village_write[whichvillager].compress=village_write[whichvillager].mcompress;
		break;
		// as above to fill out! TODO!
	      }
	    }
	    else {
	      village_write[whichvillager].compress=tmpp;
	    }
	    break;

	    ///TODO: above as models for rest of walkers

	  case 2:// READ
	    whichvillager=adc_buffer[FIRST]>>6; // 6 bits=64!!!
	    howmanyreadvill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_read[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	    village_read[whichvillager].wrap=loggy[adc_buffer[THIRD]]; // as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	    village_read[whichvillager].offset=loggy[adc_buffer[FOURTH]]; // as logarithmic
	    }
	    village_read[whichvillager].dir=xx;
	    village_read[whichvillager].speed=(spd&15)+1;
	    village_read[whichvillager].step=(spd&240)>>4;

	    if (village_read[whichvillager].dir==2) village_read[whichvillager].dirry=newdirection[wormdir];
	    else if (village_read[whichvillager].dir==3) village_read[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_read[whichvillager].speed;
	    else village_read[whichvillager].dirry=direction[village_read[whichvillager].dir]*village_read[whichvillager].speed;
	    if (village_read[whichvillager].running==0){
	      if (village_read[whichvillager].dirry>0) village_read[whichvillager].samplepos=village_read[whichvillager].start;
	    else village_read[whichvillager].samplepos=village_read[whichvillager].start+village_read[whichvillager].wrap;
	    }
	    break;

	  case 3:
	    // READ again - select villager
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    // overlap, effect as param
	    village_read[whichvillager].overlay=adc_buffer[SECOND]>>6; // 6 bits =64 top bit is datagen
	    village_read[whichvillager].effect=(float)adc_buffer[THIRD]/4096.0f;//
	    village_read[whichvillager].effectinv=1.0f-village_read[whichvillager].effect;
	    village_read[whichvillager].compress=(32768-(adc_buffer[FOURTH]<<3))+1;
	    readspeed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    dirryr=(spd&240)>>4;
	    // no finger/dir?
	    break;

	  case 4: // DATAGEN villagers
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    howmanydatavill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_datagen[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	    village_datagen[whichvillager].wrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	      village_datagen[whichvillager].CPU=adc_buffer[FOURTH]>>6;// now 64 options
	    }
	    //	    village_datagen[whichvillager].dir=xx;
	    village_datagen[whichvillager].speed=(spd&15)+1; // check how many bits is spd? 8 as changed in main.c 
	    village_datagen[whichvillager].step=(spd&240)>>4;

	    // and if is running already?
	    if (village_datagen[whichvillager].running==0){
	      village_datagen[whichvillager].position=village_datagen[whichvillager].start;
	    }
	    break;

	  case 5: // DATAGEN compression??? - not to redo? knob question?TODO
	    // if redo would just be ONE setting!
	    //	    writeoverlay=adc_buffer[FIRST]>>9; // 8 possibles 
	    databegin=loggy[adc_buffer[SECOND]]; //as logarithmic
	    dataend=loggy[adc_buffer[THIRD]]; //as logarithmic
	    //	    writeoffset=loggy[adc_buffer[FOURTH]];
	    dataspeed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    if (xx==0) dirryd=-(((spd&240)>>4)+1);
	    else if (xx==1) dirryd=((spd&240)>>4)+1;
	    else if (xx==2) dirryd=newdirection[wormdir];
	    else dirryd=direction[adc_buffer[DOWN]&1]*(((spd&240)>>4)+1);
	    if (dirryd>0) counterd=databegin;
	      else counterd=dataend+databegin;

	    //TODO: modes before mirrors=READFILTINx2//WRITEFILTIN*2=4 6/7/8/9 + HW=10 (HW as last mode 15!)

	  case 6:// FILTIN
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    howmanyfiltinvill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_filtin[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	      village_filtin[whichvillager].wrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	    village_filtin[whichvillager].offset=loggy[adc_buffer[FOURTH]]; // as logarithmic
	    }
	    village_filtin[whichvillager].dir=xx;
	    village_filtin[whichvillager].speed=(spd&15)+1; 
	    village_filtin[whichvillager].step=(spd&240)>>4;
	    if (village_filtin[whichvillager].dir==2) village_filtin[whichvillager].dirry=newdirection[wormdir];
	    else if (village_filtin[whichvillager].dir==3) village_filtin[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_filtin[whichvillager].speed;
	    else village_filtin[whichvillager].dirry=direction[village_filtin[whichvillager].dir]*village_filtin[whichvillager].speed;

	    if (village_filtin[whichvillager].running==0){
	    if (village_filtin[whichvillager].dirry>0) village_filtin[whichvillager].samplepos=village_filtin[whichvillager].start;
	    else village_filtin[whichvillager].samplepos=village_filtin[whichvillager].start+village_filtin[whichvillager].wrap;
	    }
	    break;

	  case 7: // FILTIN
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    // overlap, effect
	    village_filtin[whichvillager].overlay=adc_buffer[SECOND]>>7;// 5 bits=32
	    village_filtin[whichvillager].effect=(float)(adc_buffer[THIRD])/4096.0f;// 
	    village_filtin[whichvillager].effectinv=1.0f-village_filtin[whichvillager].effect;
	    village_filtin[whichvillager].compress=(32768-(adc_buffer[FOURTH]<<3))+1;//
	    filtinspeed=spd&15; // check how many bits is spd? 8 as changed in main
	    // no finger/dir?
	    dirryf=(spd&240)>>4;
	    break;

	  case 8:// FILTOUT
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    howmanyfiltoutvill=whichvillager+1;
	    if (adc_buffer[SECOND]>10){
	    village_filtout[whichvillager].start=loggy[adc_buffer[SECOND]]; //as logarithmic
	    }
	    if (adc_buffer[THIRD]>10){
	      village_filtout[whichvillager].wrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	    }
	    if (adc_buffer[FOURTH]>10){
	    village_filtout[whichvillager].offset=loggy[adc_buffer[FOURTH]]; // as logarithmic
	    }
	    village_filtout[whichvillager].dir=xx;
	    village_filtout[whichvillager].speed=(spd&15)+1; 
	    village_filtout[whichvillager].step=(spd&240)>>4;
	    if (village_filtout[whichvillager].dir==2) village_filtout[whichvillager].dirry=newdirection[wormdir];
	    else if (village_filtout[whichvillager].dir==3) village_filtout[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_filtout[whichvillager].speed;
	    else village_filtout[whichvillager].dirry=direction[village_filtout[whichvillager].dir]*village_filtout[whichvillager].speed;

	    if (village_filtout[whichvillager].running==0){
	    if (village_filtout[whichvillager].dirry>0) village_filtout[whichvillager].samplepos=village_filtout[whichvillager].start;
	    else village_filtout[whichvillager].samplepos=village_filtout[whichvillager].start+village_filtout[whichvillager].wrap;
	    }
	    break;

	  case 9: // FILTOUT
	    whichvillager=adc_buffer[FIRST]>>6; // 6bits=64
	    // overlap, effect
	    village_filtout[whichvillager].overlay=adc_buffer[SECOND]>>7;// 5 bits=32
	    village_filtout[whichvillager].effect=(float)(adc_buffer[THIRD])/4096.0f;// 
	    village_filtout[whichvillager].effectinv=1.0f-village_filtout[whichvillager].effect;
	    village_filtout[whichvillager].compress=(32768-(adc_buffer[FOURTH]<<3))+1;//
	    filtoutspeed=spd&15; // check how many bits is spd? 8 as changed in main
	    // no finger/dir?
	    dirryff=(spd&240)>>4;
	    break;

	  case 10: // HW walker - what we need for this walker? TEST case now NOV 3!
	    // max is say 16 walkers???
	    // sequential = 1-which,2-speed=same as length, 3-HW set(32 options), 4=compress all=overall speed, finger-input/step?
	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16
	    howmanyhardvill=whichvillager+1;
	    village_hardware[whichvillager].length=adc_buffer[SECOND];
	    village_hardware[whichvillager].setting=adc_buffer[THIRD]>>7; // 5 bits=32
	    hardcompress=(adc_buffer[FOURTH]>>5)+1; // 7 bits
	    village_hardware[whichvillager].inp=xx;
	    // or overlap/bitwise? but then how handle floating?
	    break;

	  case 11: // 40106 walker as sequential but offset as free period???
	    // 1-which,2-length, 3-offset=do nothing, 4-knob offset, finger=dir/speed/step thru datagen as others
	    // but start and wrap for datagen
	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16
	    howmany40106vill=whichvillager+1;
	    village_40106[whichvillager].length=adc_buffer[SECOND]; 
	    village_40106[whichvillager].dataoffset=loggy[adc_buffer[THIRD]]; //as logarithmic
	    village_40106[whichvillager].knoboffset=loggy[adc_buffer[FOURTH]]; //as logarithmic - varies each one!

	    village_40106[whichvillager].dir=xx;
	    village_40106[whichvillager].speed=(spd&15)+1; 
	    village_40106[whichvillager].step=((spd&240)>>4)+1; // change from other walkers here!

	    if (village_40106[whichvillager].dir==2) village_40106[whichvillager].dirry=newdirection[wormdir];
	    else if (village_40106[whichvillager].dir==3) village_40106[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_40106[whichvillager].speed;
	    else village_40106[whichvillager].dirry=direction[village_40106[whichvillager].dir]*village_40106[whichvillager].speed;

	    if (village_40106[whichvillager].dirry>0) village_40106[whichvillager].samplepos=0;
	    else village_40106[whichvillager].samplepos=village_40106[whichvillager].length;
	    break;

	  case 12: // lm walker as sequential but offset as free period???
	    // 1-which,2-length, 3-offset=do nothing, 4-knob offset, finger=dir/speed/step thru datagen as others
	    // but start and wrap for datagen

	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16
	    howmanylmvill=whichvillager+1;
	    village_lm[whichvillager].length=adc_buffer[SECOND]; 
	    village_lm[whichvillager].dataoffset=loggy[adc_buffer[THIRD]]; //as logarithmic
	    village_lm[whichvillager].knoboffset=loggy[adc_buffer[FOURTH]]; //as logarithmic - varies each one!

	    village_lm[whichvillager].dir=xx;
	    village_lm[whichvillager].speed=(spd&15)+1; 
	    village_lm[whichvillager].step=((spd&240)>>4)+1; // change from other walkers here!

	    if (village_lm[whichvillager].dir==2) village_lm[whichvillager].dirry=newdirection[wormdir];
	    else if (village_lm[whichvillager].dir==3) village_lm[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_lm[whichvillager].speed;
	    else village_lm[whichvillager].dirry=direction[village_lm[whichvillager].dir]*village_lm[whichvillager].speed;

	    if (village_lm[whichvillager].dirry>0) village_lm[whichvillager].samplepos=0;
	    else village_lm[whichvillager].samplepos=village_lm[whichvillager].length;
	    break;
	  case 13: // maxim walker as sequential but offset as free period???
	    // 1-which,2-length, 3-offset=do nothing, 4-knob offset, finger=dir/speed/step thru datagen as others
	    // but start and wrap for datagen

	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16
	    howmanymaximvill=whichvillager+1;
	    village_maxim[whichvillager].length=adc_buffer[SECOND]; 
	    village_maxim[whichvillager].dataoffset=loggy[adc_buffer[THIRD]]; //as logarithmic
	    village_maxim[whichvillager].knoboffset=loggy[adc_buffer[FOURTH]]; //as logarithmic - varies each one!

	    village_maxim[whichvillager].dir=xx;
	    village_maxim[whichvillager].speed=(spd&15)+1; 
	    village_maxim[whichvillager].step=((spd&240)>>4)+1; // change from other walkers here!

	    if (village_maxim[whichvillager].dir==2) village_maxim[whichvillager].dirry=newdirection[wormdir];
	    else if (village_maxim[whichvillager].dir==3) village_maxim[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_maxim[whichvillager].speed;
	    else village_maxim[whichvillager].dirry=direction[village_maxim[whichvillager].dir]*village_maxim[whichvillager].speed;

	    if (village_maxim[whichvillager].dirry>0) village_maxim[whichvillager].samplepos=0;
	    else village_maxim[whichvillager].samplepos=village_maxim[whichvillager].length;
	    break;
	  case 14: // hdgener walker as sequential but offset as free period???
	    // 1-which,2-length, 3-offset=do nothing, 4-knob offset, finger=dir/speed/step thru datagen as others
	    // but start and wrap for datagen

	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16
	    howmanyhdgenervill=whichvillager+1;
	    village_hdgener[whichvillager].length=adc_buffer[SECOND]; 
	    village_hdgener[whichvillager].dataoffset=loggy[adc_buffer[THIRD]]; //as logarithmic
	    village_hdgener[whichvillager].knoboffset=loggy[adc_buffer[FOURTH]]; //as logarithmic - varies each one!

	    village_hdgener[whichvillager].dir=xx;
	    village_hdgener[whichvillager].speed=(spd&15)+1; 
	    village_hdgener[whichvillager].step=((spd&240)>>4)+1; // change from other walkers here!

	    if (village_hdgener[whichvillager].dir==2) village_hdgener[whichvillager].dirry=newdirection[wormdir];
	    else if (village_hdgener[whichvillager].dir==3) village_hdgener[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_hdgener[whichvillager].speed;
	    else village_hdgener[whichvillager].dirry=direction[village_hdgener[whichvillager].dir]*village_hdgener[whichvillager].speed;

	    if (village_hdgener[whichvillager].dirry>0) village_hdgener[whichvillager].samplepos=0;
	    else village_hdgener[whichvillager].samplepos=village_hdgener[whichvillager].length;
	    break;

	  case 15: // effects across also case 16:
	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16total
	    howmanyeffectvill=whichvillager+1;
	    // but if effect is running? or will just reset itself????
	    village_effect[whichvillager].instart=village_read[adc_buffer[SECOND]>>6].start;// do we need % howmany or not. NOT so far???
	    village_effect[whichvillager].inwrap=village_read[adc_buffer[SECOND]>>6].wrap;
	    village_effect[whichvillager].modstart=village_write[adc_buffer[THIRD]>>6].start;
	    village_effect[whichvillager].modwrap=village_write[adc_buffer[THIRD]>>6].wrap;
	    village_effect[whichvillager].whicheffect=adc_buffer[FOURTH]>>8; // bits is 4=16
	    village_effect[whichvillager].speed=spd;
	    break;
	  case 16: // effects outstart,outwrap
	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16total
	    village_effect[whichvillager].outstart=loggy[adc_buffer[SECOND]]; //as logarithmic
	    village_effect[whichvillager].outwrap=loggy[adc_buffer[THIRD]]; //as logarithmic
	    village_effect[whichvillager].modifier=loggy[adc_buffer[FOURTH]];
	    village_effect[whichvillager].step=spd;
	    break;
	    // datagen walker????
	  case 17:
	    whichvillager=adc_buffer[FIRST]>>8; // 4bits=16
	    howmanydatagenwalkervill=whichvillager+1;
	    village_datagenwalker[whichvillager].length=adc_buffer[SECOND]; 
	    village_datagenwalker[whichvillager].dataoffset=loggy[adc_buffer[THIRD]]; //as logarithmic
	    village_datagenwalker[whichvillager].knoboffset=loggy[adc_buffer[FOURTH]]; //as logarithmic - varies each one!

	    village_datagenwalker[whichvillager].dir=xx;
	    village_datagenwalker[whichvillager].speed=(spd&15)+1; 
	    village_datagenwalker[whichvillager].step=((spd&240)>>4)+1; // change from other walkers here!

	    if (village_datagenwalker[whichvillager].dir==2) village_datagenwalker[whichvillager].dirry=newdirection[wormdir];
	    else if (village_datagenwalker[whichvillager].dir==3) village_datagenwalker[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_datagenwalker[whichvillager].speed;
	    else village_datagenwalker[whichvillager].dirry=direction[village_datagenwalker[whichvillager].dir]*village_datagenwalker[whichvillager].speed;

	    if (village_datagenwalker[whichvillager].dirry>0) village_datagenwalker[whichvillager].samplepos=0;
	    else village_datagenwalker[whichvillager].samplepos=village_datagenwalker[whichvillager].length;
	    break;

  /* do mirror and infections between:

  extern villagerr village_write[MAX_VILLAGERS+1];
  extern villagerr village_read[MAX_VILLAGERS+1];
  extern villagerr village_filtin[MAX_VILLAGERS+1];
  extern villagerr village_filtout[MAX_VILLAGERS+1];
  extern villager_generic village_datagen[MAX_VILLAGERS+1];

  extern villager_hardware village_hardware[17];
  extern villager_hardwarehaha village_40106[17];
  extern villager_hardwarehaha village_hdgener[17];
  extern villager_hardwarehaha village_lm[17];
  extern villager_hardwarehaha village_maxim[17];

   */

	  case 18: // swop and copy
	    whichx=adc_buffer[FIRST]>>6; // 6 bits=64 //TODO? restricted as to how many we have below?
	    whichy=adc_buffer[SECOND]>>6; // 6 bits=64 //TODO? restricted as to how many we have below?
	    ranger=adc_buffer[THIRD]>>6; // 64
	    // and FOURTH knob??? as mirror modulator
	    //	    action=xx;
	    switch(xx){ 
	    case 0:
	      switch(ranger){
	      case 0:
		village_write[whichx]=village_write[whichy]; // restrict here or???
		break;
	      case 1:
		village_write[whichx]=village_read[whichy];
		break;
	      case 2:
		village_write[whichx]=village_filtin[whichy];
		break;
	      case 3:
		village_write[whichx]=village_filtout[whichy];
		break;
	      case 4:
		// copy appropriate datagen settings
		village_write[whichx].start=village_datagen[whichy].start;
		village_write[whichx].wrap=village_datagen[whichy].wrap;
		village_write[whichx].samplepos=village_datagen[whichy].position;
		village_write[whichx].dirry=village_datagen[whichy].dirry;
		village_write[whichx].dir=village_datagen[whichy].dir;
		village_write[whichx].step=village_datagen[whichy].step;
		village_write[whichx].speed=village_datagen[whichy].speed;
		break;
	      case 5:
		// datagen walker 
		village_write[whichx].start=village_datagenwalker[whichy].dataoffset;
		village_write[whichx].wrap=village_datagenwalker[whichy].length;
		village_write[whichx].samplepos=village_datagenwalker[whichy].samplepos;
		village_write[whichx].dirry=village_datagenwalker[whichy].dirry;
		village_write[whichx].dir=village_datagenwalker[whichy].dir;
		village_write[whichx].step=village_datagenwalker[whichy].step;
		village_write[whichx].speed=village_datagenwalker[whichy].speed;
		break;
	      case 6:
		// with its mirror
		village_write[whichx].start=village_write[whichy].mstart;
		village_write[whichx].wrap=village_write[whichy].mwrap;
		village_write[whichx].compress=village_write[whichy].mcompress;
		break;
	      case 7:
		/// copy outmod for effects! outstart, outwrap
		village_write[whichx].start=village_effect[whichx].outstart;
		village_write[whichx].wrap=village_effect[whichx].outwrap;
		village_write[whichx].samplepos=village_effect[whichx].outpos;
		break;
		/// TODO: mult all above by village_read,village_datagen,village_datagenwalker,village mirrors,outmod=8*8=64

	      } // end of ranger
	      break;
	    case 1: // next actionsssTODO!
	      break;
		/* example actions
		
		// swop
		tmpvillage=village_write[whichx];
		village_write[whichx]=village_read[whichy];
		village_read[whichy]=tmpvillage;
		break;

	    case 2:// constrain %
		village_read[whichx]%=(village_write[whichy]+1);

		case 3:unknown????

		*/
	    } // end of xx = action
	    break; // case 18
	  case 19: // mirror
	    // 1-set which villager is mirrored - but depends on how many
	    whichx=adc_buffer[FIRST]>>6; // 6 bits=64 //TODO? restricted as to how many we have below?
	    // 2-which group also TODO! here is just WRITE!
	    // finger-set what to mirror-> datagen/eeg/finger/knob_as_4 = fingered...
	    village_write[whichx].fingered=xx; // TODO... other groups
	    // 3-set how mirror effects = mirrormod if 0 then nothing happens
	    village_write[whichx].mirrormod=adc_buffer[THIRD]>>6;
	    // finger-speed and step for this
	    village_write[whichx].mirrorspeed=(spd&15)+1; 
	    break;
	    /// TODO:
	    //20- fingers in the code
	    //21- infection across (how?)
	  } // end of mainmodes
	}// Xx!=5

	// all MIRRORS! TODO- here is just village_write!
	// mirrormod for effects we need vill_eff->modifier??
	u16 tmpp;

	for (whichx=0;whichx<howmanywritevill;whichx++){
	  if (village_write[whichx].mirrormod){
	    // speed wrapper
	    if (++village_write[whichx].mirrordel>=village_write[whichx].speed){
	      village_write[whichx].mirrordel=0;
	    // then do mirrormod and fingered on start,wrap and compress
	      // but all are same fingered.TODO????
	      switch(village_write[whichx].fingered){
	//fingered - UP.2=datagen DOWN.3=eeg/finger(SUSP) LEFT.0.finger RIGHT.1knob
	      case 0: // LEFT=finger
		village_write[whichx].mstart=adc_buffer[LEFT]<<3;
		village_write[whichx].mwrap=adc_buffer[LEFT]<<3;
		village_write[whichx].mcompress=adc_buffer[LEFT]<<3;
		break;
	      case 1: //RIGHT= knob 
		village_write[whichx].mstart=adc_buffer[FOURTH]<<3;
		village_write[whichx].mwrap=adc_buffer[FOURTH]<<3;
		village_write[whichx].mcompress=adc_buffer[FOURTH]<<3;
		break;
	      case 2: // UP=datagen
		village_write[whichx].mstart=nextdatagen();
		village_write[whichx].mwrap=nextdatagen();
		village_write[whichx].mcompress=nextdatagen();
		break;
	      case 3: // DOWN=EEG ifdef
#ifdef TENE
		village_write[whichx].mstart=adc_buffer[9]<<3;
		village_write[whichx].mwrap=adc_buffer[9]<<3;
		village_write[whichx].mcompress=adc_buffer[9]<<3;
#else
		village_write[whichx].mstart=adc_buffer[LEFT]<<3;
		village_write[whichx].mwrap=adc_buffer[LEFT]<<3;
		village_write[whichx].mcompress=adc_buffer[LEFT]<<3;
#endif
		break;
	      }

	      switch(village_write[whichx].mirrormod){
	      case 1: // straight datagen
		village_write[whichx].start=village_write[whichx].mstart;
		village_write[whichx].wrap=village_write[whichx].mwrap;
		village_write[whichx].compress=village_write[whichx].mcompress;
		break;
	      case 2: // addition with wrap
		village_write[whichx].start=(village_write[whichx].kstart+village_write[whichx].mstart)%32768;
		village_write[whichx].wrap=(village_write[whichx].kwrap+village_write[whichx].mwrap)%32768;
		village_write[whichx].compress=(village_write[whichx].kcompress+village_write[whichx].mcompress)%32768;
		break;
	      case 3: // addition up to 32768
		tmpp=32768-village_write[whichx].kstart;
		if (tmpp==0) tmpp=1;
		village_write[whichx].start=village_write[whichx].kstart+(village_write[whichx].mstart%tmpp);
		tmpp=32768-village_write[whichx].kwrap;
		if (tmpp==0) tmpp=1;
		village_write[whichx].wrap=village_write[whichx].kwrap+(village_write[whichx].mwrap%tmpp);
		tmpp=32768-village_write[whichx].kcompress;
		if (tmpp==0) tmpp=1;
		village_write[whichx].compress=village_write[whichx].kcompress+(village_write[whichx].mcompress%tmpp);
		break;
		// TODO: other cases: subtraction, and, or, modulus
	      }
	    }
	  }	
	}

	////// all effects

	  for (x=0;x<howmanyeffectvill;x++){
	    do_effect(&village_effect[x]);
	  }

	  /// end effects/ALLLL!!!!

#ifdef PCSIM
      // randomise adc_buffer
      for (x=0;x<10;x++){
	adc_buffer[x]=(randi()%4096);
	//	adc_buffer[x]=adc_buffer[x]%4096;
      }

      I2S_RX_CallBack(src, dst, BUFF_LEN/2); 
      //printf("STACKPOS %d\n",STACKPOSY);
#endif
      
#endif //eeg
#endif //straight
#endif //test effects
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
