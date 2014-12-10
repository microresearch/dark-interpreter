// gcc simulation.c -osim -lm
// was datagentest.c previously

/* All simulation data generators: IFS, rossler, secondrossler, fitz,
   oregon, spruce, brussel, simpleSIR, seir, conv, sine */

//+ inc,dec,left,right and so on

/* 

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301 USA

Based in part on SLUGens by Nicholas Collins.

*/

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "simulation.h"
#include <malloc.h>
#define randi() (rand()&4095)
#define float32_t float


u16 sin_data[256];
u16 *stacker;//[256]; // 16*3 MAX
uint16_t adc_buffer[10];
int16_t audio_buffer[32768];
uint16_t buf16[32768];
/* 
u16 sin_data[256];
u16 stacker[256]; 
uint16_t adc_buffer[10];
int16_t* audio_buffer;
*/
#else
#include "audio.h"
#include "simulation.h"
#include <malloc.h>
#include <math.h>
#include <audio.h>
#include "stm32f4xx.h"
#include "arm_math.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern int16_t audio_buffer[32768] __attribute__ ((section (".data")));
extern u16 sin_data[256];
extern u16 *buf16;
#endif


//////////////////////////////////////////////////////////

// formantz

#ifdef LACH
const u16 SAMPLE_FREQUENCY = 48000;
#else
const u16 SAMPLE_FREQUENCY = 32000;
#endif

const float32_t Pi = 3.1415926535f;
const float32_t PI_2 = 6.28318531f;

u8 freqqq[3];
u8 w[3];

void runform(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  freqqq[0]=(u8)buf16[0];
  freqqq[1]=(u8)buf16[1];
  freqqq[2]=(u8)buf16[2];
  w[0]=(u8)buf16[3];
  w[1]=(u8)buf16[4];
  w[2]=(u8)buf16[5];

  //  u8 *buf16=(u8*)buffer;
  float32_t accum; float32_t x; 

   for (u8 xx=0;xx<vill->howmany;xx++){

  for (u8 f = 0; f < 3; f++ ) {
  u8 ff = freqqq[f]; // the three freqqs

  float32_t freqq = (float32_t)ff*(50.0f/SAMPLE_FREQUENCY);

  float32_t buf1Res = 0, buf2Res = 0;
  float32_t q = 1.0f - w[f] * (Pi * 10.0f / SAMPLE_FREQUENCY);
  float32_t xp = 0;
  
  //  for (u8 s = 0; s < 8; s++ ) {
    x=(float32_t)(buf16[count&32767])/65536.0f;
    x = x + 2.0f * cosf ( PI_2 * freqq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;
    if (f==0) accum=x; 
    else
      accum+=x; // as float32_t
    if (f==2){
      buf16[(count+16384)&32767]=(float32_t)accum*65536.0f; // changed OCT!
#ifdef PCSIM 
      //      printf("%c",buffer[start+count]%255);
      //      printf("%d\n",start+count);
#endif
      //    }
      count+=step;
      if (count>start+wrap) count=start;
    }
  }
   }
  vill->position=count;
  }

//////////////////////////////////////////////////////////

// convolve

void runconv(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  u16 y,tmp;
  float32_t tmppp;
  float32_t c0=(float32_t)buf16[0]/16384.0;
  float32_t c1=(float32_t)buf16[1]/16384.0;
  float32_t c2=(float32_t)buf16[2]/16384.0;

   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    //    
    y=count+16384;
    y=y&32767;
    tmp=count-1;
    //    buf16[y]=((float32_t)buf16[tmp&32767]*c0)+((float32_t)buf16[count&32767]*c1)+((float32_t)buf16[(count+1)&32767]*c2);
  tmppp=(((float)(buf16[tmp&32767])/65536.0f)*c0)+(((float)(buf16[count&32767])/65536.0f)*c1)+(((float)(buf16[(count+1)&32767])/65536.0f)*c2);
  buf16[y]=tmppp*65536.0f;

#ifdef PCSIM
    //    //    //    printf("%d %d %d %d %d\n",tmp, count,(count+1)&32767, y, buf16[(count+start)&32767]);
    //    //    printf("%c",buf16[start+count]%255);
#endif
    //  }
   }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// sine

void runsine(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

static u8 cc=0;
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    //    
    buf16[count&32767]=sin_data[cc]; 
#ifdef PCSIM
    //    //    printf("%c",buf16[(count+start)&32767]);
#endif
    cc++;
      }
  vill->position=count;
}

void runnoise(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;
// TSTY!
  static u16 xx;
   for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;

    //    
    buf16[count&32767]=xx++; 
   }
  vill->position=count;
}


//////////////////////////////////////////////////////////
/* chunking 

- copy one buffer chunk to another chunk
- do some kind of walker

*/

#ifdef PCSIM
signed char direction[2]={-1,1};
//extern signed char direction[2];
#else
extern signed char direction[2];
#endif


void runchunk(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

static u16 othercount;
  u16 otherstart=buf16[0]>>1;
  u16 otherwrap=buf16[1]>>1;
  u16 dirr=buf16[2]&1;

   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    othercount+=direction[dirr];

    if (othercount<otherwrap && othercount>0){
    buf16[count&32767]=buf16[(othercount+otherstart)&32767];
    }
    else {
    if (dirr==1)  othercount=0;
    else othercount=otherwrap;
    }
#ifdef PCSIM
    //    //    printf("%c",buf16[(count+start)&32767]);
#endif
   }
  vill->position=count;
}

void runderefchunk(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

static u16 othercount;
  u16 otherstart=buf16[0]>>1;
  u16 otherwrap=buf16[1]>>1;
  u16 dirr=buf16[2]&1;

   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    //    
    othercount+=direction[dirr];
    if (othercount<otherwrap && othercount>0){
      buf16[count&32767]=buf16[buf16[(othercount+otherstart)&32767]>>1];
    }
    else {
    if (dirr==1)  othercount=0;
    else othercount=otherwrap;
    }
#ifdef PCSIM
    //    //    printf("%c",buf16[(count+start)&32767]);
#endif
   }
  vill->position=count;
}

void runwalkerchunk(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

static u16 othercount;
  static u16 otherstart=0;
  static u16 otherwrap=32768;
  static u8 dirr=0;
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    //    
    othercount+=direction[dirr];
        if (othercount>otherwrap || othercount<1){
      // get new start and wrap - from where? would need other counter
      otherstart=buf16[0]>>1;
      otherwrap=buf16[1]>>1;
      dirr=buf16[2]&1;
      if (dirr==1)  othercount=0;
      else       othercount=otherwrap;
      }
    buf16[count&32767]=buf16[(othercount+otherstart)&32767];

#ifdef PCSIM
    //    //        printf("%c",buf16[(count+start)&32767]);
    //    //    printf("x: %d\n",othercount);
#endif
      }
  vill->position=count;
}

void runswapchunk(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

static u16 othercount;
  static u16 otherstart=0;
  static u16 otherwrap=32768;
  u16 dirr=1;
  u16 tmp;
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    //    
    othercount+=direction[dirr];
    if (othercount>otherwrap || othercount<1){
      otherstart=buf16[0]>>1;
      otherwrap=buf16[1]>>1;
      dirr=buf16[2]&1;
  if (dirr==1)  othercount=0;
  else othercount=otherwrap;
    }

    tmp=buf16[count&32767];
    buf16[count&32767]=buf16[(othercount+otherstart)&32767];
    buf16[(othercount+otherstart)&32767]=tmp;
#ifdef PCSIM
    //    //    //           printf("%c",buf16[(count+start)&32767]);
#endif
      }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// generic arithmetik datagens

void runinc(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  u16 cop=buf16[0]; 
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;


    buf16[count&32767]=cop++;
#ifdef PCSIM
	//	//printf("%c",buf16[count&32767]);
#endif
      }
buf16[0]=cop;
  vill->position=count;
}

void rundec(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
  u16 cop=buf16[0]; 
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;

    buf16[count&32767]=cop--;
#ifdef PCSIM
    //    //printf("%c",buf16[count&32767]);
#endif
      }
buf16[0]=cop;
  vill->position=count;
}

void runleft(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;


    buf16[count&32767]=buf16[count&32767]<<1;
#ifdef PCSIM
    //    //    //    //    printf("%d %d\n",count,buf16[count&32767]);
    //printf("%c", buf16[count&32767]);
#endif
      }
  vill->position=count;
}

void runright(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (i=0; i<howmuch; i++) {
          count+=step;
      if (count>start+wrap) count=start;


    buf16[count&32767]=buf16[count&32767]>>1;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",buf16[count&32767]);
    //printf("%c", buf16[count&32767]);
#endif
      }
  vill->position=count;
}

void runswap(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

u16 temp;
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap-1) count=start;

      //    if (count>=(wrap-1)) count=start;
    temp=buf16[count&32767];
    buf16[count&32767]=buf16[(count+1)&32767];
    buf16[(count+1)&32767]=temp;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",buf16[count&32767]);
    //printf("%c", buf16[count&32767]);
#endif
      }
  vill->position=count;
}

void runnextinc(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

   for (u8 xx=0;xx<vill->howmany;xx++){
  
          count+=step;
      if (count>=start+wrap-1) count=start;
 
      //    if (count>=(wrap-1)) count=0;
    buf16[count&32767]=buf16[(count+1)&32767]+1;
   }
  vill->position=count;
}

void runnextdec(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap-1) count=start;

    //    if (count>=32766) count=0;
      //    if (count>=(wrap-1)) count=0;
    buf16[count&32767]=buf16[(count+1)&32767]-1;
     }
  vill->position=count;
}

void runnextmult(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>-start+wrap-1) count=start;

      //    if (count>=(wrap-1)) count=0;
    buf16[count&32767]*=buf16[(count+1)&32767];
     }
  vill->position=count;
}

void runnextdiv(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap-1) count=start;

      //    if (count>=(wrap-1)) count=0;
    if ((buf16[(count+1)&32767])>0)   buf16[count&32767]/=buf16[(count+1)&32767];
     }
  vill->position=count;
}

void runcopy(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap-1) count=start;
 
    //    if (count>=32766) count=0;
      //    if (count>=(wrap-1)) count=0;
    buf16[(count+1)&32767]=buf16[count&32767];
     }
  vill->position=count;
}

void runzero(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;
    buf16[count&32767]=0;
     }
  vill->position=count;
}

void runfull(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;
   for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;
    buf16[count&32767]=65535;
   }
  vill->position=count;
}

void runrand(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  u8 *workingbuffeur=(u8 *)buf16;
  //  for (i=0; i<howmuch*2; i++) {
   for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;

      workingbuffeur[count]=randi()&255;// TESTY!!!???
          count+=step;
      if (count>start+wrap) count=start;

    workingbuffeur[count]=randi()&255;
   }
  vill->position=count;
}

void runknob(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  //  u8 *workingbuffeur=(u8 *)buffer;
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;
#ifdef TENE
	//    workingbuffeur[count&32767]=adc_buffer[2]>>4; // TOP knob in either case!
    buf16[count&32767]=adc_buffer[2]<<4; // TOP knob in either case!
    //    //    //    printf("%d\n",buf16[count&32767]);
#else
    //        workingbuffeur[count&32767]=adc_buffer[3]>>4; // 8 bits
        buf16[count&32767]=adc_buffer[3]<<4; // 16 bits
#endif
     }
  vill->position=count;
}

//////////////////////////////////////////////////////////
// swap datagen 16 bits to and from audio buffer

void runswapaudio(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

u16 temp;
     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;

    // convert signed to unsigned how? 
	temp=(uint16_t)audio_buffer[count&32767];
	audio_buffer[count&32767]=(int16_t)buf16[count&32767];
	buf16[count&32767]=temp;
     }
  vill->position=count;
}

//////////////////////////////////////////////////////////
// OR/XOR/AND/other ops datagen 16 bits to and from audio buffer

void runORaudio(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

u16 temp;
      

   for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;

    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[count&32767];
    u16 cop=buf16[0]; 
    switch(cop%5){
    case 0:
    audio_buffer[count&32767]|=(int16_t)buf16[count&32767];
    buf16[count&32767]|=temp;
    break;
    case 1:
    audio_buffer[count&32767]^=(int16_t)buf16[count&32767];
    buf16[count&32767]^=temp;
    break;
    case 2:
    audio_buffer[count&32767]&=(int16_t)buf16[count&32767];
    buf16[count&32767]&=temp;
    break;
    case 3:
    audio_buffer[count&32767]-=(int16_t)buf16[count&32767];
    buf16[count&32767]+=temp;
    break;
    case 4:
    audio_buffer[count&32767]+=(int16_t)buf16[count&32767];
    buf16[count&32767]-=temp;
    break;
    }
    }
  vill->position=count;
}


//////////////////////////////////////////////////////////

// first SIR:

/*

This is the C version of program 2.1 from page 19 of
"Modeling Infectious Disease in humans and animals"
by Keeling & Rohani.

It is the simple SIR epidemic without births or deaths.

see also: http://homepages.warwick.ac.uk/~masfz/ModelingInfectiousDiseases/

 */

static float32_t dPop[3];
static float32_t beta;//=520.0/365.0;
static float32_t gamm;//=1.0/7.0;
static float32_t S0;//=1.0-1e-6;
static float32_t I0;//=1e-6;
static float32_t SS,II,R;
static float32_t stepp;
static float32_t steppp;
static float32_t stepppp;

void Diff(float32_t Pop[3])
{
  float32_t tmpS, tmpI, tmpR;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpR=Pop[2];

  dPop[0] = - beta*tmpS*tmpI;              // dS/dt
  dPop[1] = beta*tmpS*tmpI - gamm*tmpI;   // dI/dt
  dPop[2] = gamm*tmpI;                    // dR/dt
}

void Runge_Kutta(void)
{
  unsigned char i;
  float32_t dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float32_t tmpPop[3], initialPop[3];

  /* Integrates the equations one step, using Runge-Kutta 4
     Note: we work with arrays rather than variables to make the
     coding easier */

  initialPop[0]=SS; initialPop[1]=II; initialPop[2]=R;

  Diff(initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=dPop[i];
      tmpPop[i]=initialPop[i]+stepp*dPop1[i]/2;
    }

  Diff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=dPop[i];
      tmpPop[i]=initialPop[i]+stepp*dPop2[i]/2;  
    }

  Diff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=dPop[i];
      tmpPop[i]=initialPop[i]+stepp*dPop3[i]; 
    }

  Diff(tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=dPop[i];
      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*stepp;
    }

  SS=tmpPop[0]; II=tmpPop[1]; R=tmpPop[2];
  //  //  //  printf("%g %g %g\n",S,II,R);

  return;
}



void runsimplesir(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  beta=(float32_t)buf16[0]/65536.0f;
  gamm=(float32_t)buf16[1]/65536.0f;
  S0=1.0-1e-6;
  I0=1e-6;
  stepp=0.01f/((beta+gamm)*S0);
  SS=S0; II=I0; R=1-SS-II;

   for (u8 xx=0;xx<vill->howmany;xx++){
  
          count+=step;
      if (count>start+wrap) count=start;
	Runge_Kutta();//  t+=step;
	buf16[count&32767]=(u16)((float)II*1000000000.0f);
   }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// SEIR. SIR

static u16 n;
static u16 m;
static float32_t mu;
static float32_t S00,I00;
static float32_t S,I[MAX_GROUPS]; // 4x16=64bytes
static float32_t dPopp[MAX_GROUPS+1];//4x9=36bytes

void seirinit(u16 *buf16){
  u8 i;
  //  beta=17/5;
  //  gamm=1.0/13;
  n=13;
  m=8;
  mu=1.0f/(55.0f*365.0f);
  S00=0.05f;
  I00=0.00001f;

  S=S00;
  for(i=0;i<n;i++)
    {
      I[i]=I00/(float)n;
    }
    }

void seirDiff(float32_t Pop[MAX_GROUPS+1])
{
  int i;
  float32_t Inf, tmpS;

  /* Set up some temporary variables to make things easier.
     Note I_i = Pop[i]  */
  
  tmpS=Pop[0];
  Inf=0;
  for(i=m+1;i<=n;i++)
    {
      Inf+=Pop[i];
    }
  
  dPopp[0]= mu - beta*Inf*tmpS - mu*tmpS;
  dPopp[1]= beta*Inf*tmpS - gamm*n*Pop[1] - mu*Pop[1];

  for(i=2;i<=n;i++)
    {
      dPopp[i]= gamm*n*Pop[i-1] - gamm*n*Pop[i] - mu*Pop[i];
      //      //      //  printf("dp: %g\n",dPopp[i]);
    }

  return;
}

void seir_Runge_Kutta(void)
{
  u8 i;
  float32_t dPop1[MAX_GROUPS], dPop2[MAX_GROUPS], dPop3[MAX_GROUPS], dPop4[MAX_GROUPS];
  float32_t tmpPop[MAX_GROUPS],InitialPop[MAX_GROUPS];

  /* Integrates the equations one step, using Runge-Kutta 4
     Note: we work with arrays rather than variables to make the
     coding easier */

  InitialPop[0]=S;
  for(i=0;i<n;i++)
    {
      InitialPop[i+1]=I[i];
    }

  seirDiff(InitialPop);
  for(i=0;i<=n;i++)
    {
      dPop1[i]=dPopp[i];
      tmpPop[i]=InitialPop[i]+steppp*dPop1[i]/2.0f;
    }

  seirDiff(tmpPop);
  for(i=0;i<=n;i++)
    {
      dPop2[i]=dPopp[i];
      tmpPop[i]=InitialPop[i]+steppp*dPop2[i]/2.0f;  
    }

  seirDiff(tmpPop);
  for(i=0;i<=n;i++)
    {
      dPop3[i]=dPopp[i];
      tmpPop[i]=InitialPop[i]+steppp*dPop3[i]; 
    }

  seirDiff(tmpPop);

  for(i=0;i<n;i++)
    {
      dPop4[i+1]=dPopp[i+1];
      I[i]=I[i]+(dPop1[i+1]/6.0f + dPop2[i+1]/3.0f + dPop3[i+1]/3.0f + dPop4[i+1]/6.0f)*steppp;
    }
  dPop4[0]=dPop[0];
  S=S+(dPop1[0]/6.0f + dPop2[0]/3.0f + dPop3[0]/3.0f + dPop4[0]/6.0f)*steppp;
  //  //  //  printf("S: %g\n",S);
  return;
}


void runseir(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;


  
  beta=(float32_t)buf16[0]/65536.0f;
  gamm=(float32_t)buf16[1]/65536.0f;
  steppp=0.01f/(beta+gamm*n+mu);

     for (u8 xx=0;xx<vill->howmany;xx++){
          count+=step;
      if (count>start+wrap) count=start;
    seir_Runge_Kutta();//  t+=step;
    buf16[count&32767]=S*65536.0f;
     }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// SICR. SIR

static float32_t betaa;
static float32_t epsilon;
static float32_t gammm;
static float32_t Gamm; 
static float32_t muu;
static float32_t q;
static float32_t S000;
static float32_t I000;
static float32_t C000;
static float32_t t,SSS,III,C,RRR;
static float32_t dPop[3];

void sicrinit(u16 *buf16){
  //betaa=0.2;
  //epsilon=0.1;
//gamm=1.0/100.0;
//Gamm=1.0/1000.0;

muu=1.0f/(50.0f*365.0f);
q=0.4f;
S000=0.1f;
I0=1e-4;
C000=1e-3;

SSS=S000; III=I0; C=C000; RRR=1-SSS-III-C000;
}

void sicrdiff(float32_t Pop[3])
{
  float32_t tmpS, tmpI, tmpC;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpC=Pop[2];
  dPop[0] = muu - betaa*tmpS*(tmpI + epsilon*tmpC) - muu*tmpS;
  dPop[1] = betaa*tmpS*(tmpI + epsilon*tmpC) - gammm*tmpI -muu*tmpI;
  dPop[2] = gammm*q*tmpI - Gamm*tmpC - muu*tmpC;
  return;
}

void sicr_Runge_Kutta(void)
{
  
  float32_t dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float32_t tmpPop[3], initialPop[3];
  u8 i;
  initialPop[0]=SSS; initialPop[1]=III; initialPop[2]=C;

  sicrdiff(initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=dPop[i];
      tmpPop[i]=initialPop[i]+stepppp*dPop1[i]/2;
    }

  sicrdiff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=dPop[i];
      tmpPop[i]=initialPop[i]+stepppp*dPop2[i]/2;  
    }

  sicrdiff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=dPop[i];
      tmpPop[i]=initialPop[i]+stepppp*dPop3[i]; 
    }

  sicrdiff(tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=dPop[i];

      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*stepppp;
    }

  SSS=tmpPop[0]; III=tmpPop[1]; C=tmpPop[2];  RRR=1-SSS-III-C;
 
  return;
}

void runsicr(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

betaa=(float32_t)buf16[0]/65536.0f;
epsilon=(float32_t)buf16[1]/65536.0f;
gammm=(float32_t)buf16[2]/65536.0f;
Gamm=(float32_t)buf16[3]/65536.0f;
stepppp=0.01/((betaa+gammm+muu+Gamm)*S000);

   for (u8 xx=0;xx<vill->howmany;xx++){
  
          count+=step;
      if (count>start+wrap) count=start;

    sicr_Runge_Kutta();//  t+=step;
    buf16[count&32767]=(u16)(SSS*1000000.0f);
   }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// IFS

static float32_t prob[5];
static float32_t coeff[4][6];
static Point p1,p2;

void ifsinit(u16 *buf16){
  u8 column = 6, row = 4;
  u8 iter,i;
  p1.x=0.1f;
  p1.y=0.1f;         
  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){

      coeff[iter][i]=(float32_t)randi()/4096.0f;
      if ((float32_t)randi()/4096.0f>0.5f) coeff[iter][i]= coeff[iter][i]-1.0f;
      prob[iter]=(float32_t)randi()/4096.0f;

    }
    /*        prob[0]=0.00f;
  prob[1]=0.01f; 
  prob[2]=0.85f; 
  prob[3]=0.07f; 
  prob[4]=0.07f; 
    */
    prob[0]=(float32_t)buf16[0]/65536.0f;
    prob[1]=(float32_t)buf16[1]/65536.0f;
    prob[2]=(float32_t)buf16[2]/65536.0f;
    prob[3]=(float32_t)buf16[3]/65536.0f;
    prob[4]=(float32_t)buf16[4]/65536.0f;
  }
  }

void runifs(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;


  float32_t randiom_num;
   
  u8 column = 6, row = 4,i;

  randiom_num = (float32_t)randi()/4096.0f;

#ifdef PCSIM
  //  printf("p1=%d\n",p1);
#endif
   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (x=0;x<howmuch;x++){
          count+=step;
	  if (count>start+wrap) {
	    count=start;
	    prob[0]=(float32_t)buf16[0]/65536.0f;
	    prob[1]=(float32_t)buf16[1]/65536.0f;
	    prob[2]=(float32_t)buf16[2]/65536.0f;
	    prob[3]=(float32_t)buf16[3]/65536.0f;
	    prob[4]=(float32_t)buf16[4]/65536.0f;
	  }

  for(i = 0; i < row; i++){
    if ( BET(randiom_num,prob[i],prob[i+1]) ){
      p2.x = coeff[i][0]*p1.x + coeff[i][1]*p1.y + coeff[i][4];
      p2.y = coeff[i][2]*p1.x + coeff[i][3]*p1.y + coeff[i][5];
      break;
    }
      }
  p1=p2;  
  buf16[count&32767]=(u16)p2.y;
#ifdef PCSIM
  printf("%c",buf16[count&32767]);
#endif
  /*    iter=randi()%row;
    i=randi()%column;
    coeff[iter][i]=((float32_t)randi()/(float32_t)(RANDI_MAX));
    if (((float32_t)randi()/(float32_t)(RANDI_MAX))>0.5) coeff[iter][i]= coeff[iter][i]-1;
    prob[iter]=((float32_t)randi()/(float32_t)(RANDI_MAX));
    p1.x=0.5;
    p1.y=0.5;*/
  }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// ROSSLER

//float32_t h,a,b,c;

static float32_t h,a,b,c;

void rosslerinit(void){
h = (float32_t)buf16[0]/120536.0;
a = (float32_t)buf16[1]/122536.0;
b = (float32_t)buf16[2]/100536.0;
}

void runrossler(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  float32_t lx1,ly1,lz1;
  
  /* which unit to vary according to buf16 */
  // leave as so!
 static float32_t   lx0 = 0.1f;
 static float32_t   ly0 = 0.0f;
 static float32_t   lz0 = 0.0f;

  c = 5.8f;

   for (u8 xx=0;xx<vill->howmany;xx++){
  
          count+=step;
	  if (count>start+wrap) {
	    count=start;
	    lx0 = 0.1f;
	    ly0 = 0.0f;
	    lz0 = 0.0f;
	    h = (float32_t)buf16[0]/120536.0;
	    a = (float32_t)buf16[1]/122536.0;
	    b = (float32_t)buf16[2]/100536.0;
	  }

  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  lx0 = lx1;
  ly0 = ly1;
  lz0 = lz1;
  
      buf16[count&32767]=(u16)(lz1*1024.0f);

#ifdef PCSIM
      //    printf("%c",buf16[count&32767]);
#endif

        }

  vill->position=count;
}

//////////////////////////////////////////////////////////

//float32_t x0,yy0,z0;

// 2nd rossler from: MCLDChaosUGens.cpp

static float32_t hh,aa,bb,cc;

void secondrosllerinit(void){
  aa = (float32_t)buf16[0]/65536.0f;
  bb = (float32_t)buf16[1]/65536.0f;
  cc = (float32_t)buf16[2]/65536.0f;
  hh = (float32_t)buf16[3]/65536.0f;
}

void runsecondrossler(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  // what these should be?

 static float32_t xn=0.1f;
 static float32_t yn=0.1f;
 static float32_t zn=0.1f;
 static float32_t xnm1=0.0f;
 static float32_t ynm1=0.0f;
 static float32_t znm1=0.0f;

  float32_t dx = xn - xnm1;
  float32_t dy = yn - ynm1;
  float32_t dz = zn - znm1;
  //  secondrosslerinit(buf16);

   for (u8 xx=0;xx<vill->howmany;xx++){
  //	for (i=0; i<howmuch; ++i) {
          count+=step;
	  if (count>start+wrap) {
	    count=start;

  aa = (float32_t)buf16[0]/65536.0f;
  bb = (float32_t)buf16[1]/65536.0f;
  cc = (float32_t)buf16[2]/65536.0f;
  hh = (float32_t)buf16[3]/65536.0f;
	  }

    xnm1 = xn;
    ynm1 = yn;
    znm1 = zn;

    float32_t k1x, k2x, k3x, k4x,
      k1y, k2y, k3y, k4y,
      k1z, k2z, k3z, k4z,
      kxHalf, kyHalf, kzHalf;

			// 4th order Runge-Kutta approximate solution for differential equations
			k1x = - (hh * (ynm1 + znm1));
			k1y = hh * (xnm1 + aa * ynm1);
			k1z = hh * (bb + znm1 * (xnm1 - cc));
			kxHalf = k1x * 0.5;
			kyHalf = k1y * 0.5;
			kzHalf = k1z * 0.5;

			k2x = - (hh * (ynm1 + kyHalf + znm1 + kzHalf));
			k2y = hh * (xnm1 + kxHalf + aa * (ynm1 + kyHalf));
			k2z = hh * (bb + (znm1 + kzHalf) * (xnm1 + kxHalf - cc));
			kxHalf = k2x * 0.5;
			kyHalf = k2y * 0.5;
			kzHalf = k2z * 0.5;

			k3x = - (hh * (ynm1 + kyHalf + znm1 + kzHalf));
			k3y = hh * (xnm1 + kxHalf + aa * (ynm1 + kyHalf));
			k3z = hh * (bb + (znm1 + kzHalf) * (xnm1 + kxHalf - cc));

			k4x = - (hh * (ynm1 + k3y + znm1 + k3z));
			k4y = hh * (xnm1 + k3x + aa * (ynm1 + k3y));
			k4z = hh * (bb + (znm1 + k3z) * (xnm1 + k3x - cc));

			xn = xn + (k1x + 2.0*(k2x + k3x) + k4x) * ONESIXTH;
			yn = yn + (k1y + 2.0*(k2y + k3y) + k4y) * ONESIXTH;
			zn = zn + (k1z + 2.0*(k2z + k3z) + k4z) * ONESIXTH;

			dx = xn - xnm1;
			dy = yn - ynm1;
			dz = zn - znm1;
	
	/*		ZXP(xout) = (xnm1 + dx) * 0.5f;
		ZXP(yout) = (ynm1 + dy) * 0.5f;
		ZXP(zout) = (znm1 + dz) * 1.0f;*/
	    

		buf16[count&32767]=(u16)((xnm1+dx)*1024.0f);
   }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

#define FACT 32768.0f

static float32_t delta,muuu,muplusone,gammar;    

void brusselinit(void){
  delta = (float32_t)buf16[0]/FACT;
  muuu = (float32_t)buf16[1]/FACT;
  muplusone = 1.0f+muuu; 
  gammar = (float32_t)buf16[2]/FACT;
}

void runbrussel(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  float32_t dx, dy; 
  
  static float32_t x= 0.5f; 
  static float32_t y= 0.5f;  
  delta = (float32_t)buf16[0]/FACT;
  muuu = (float32_t)buf16[1]/FACT;
  muplusone = 1.0f+muuu; 
  gammar = (float32_t)buf16[2]/FACT;

   for (u8 xx=0;xx<vill->howmany;xx++){
  //    for (i=0; i<howmuch; ++i) {
	          count+=step;
		  if (count>start+wrap) {
		    count=start;
  delta = (float32_t)buf16[0]/FACT;
  muuu = (float32_t)buf16[1]/FACT;
  muplusone = 1.0f+muuu; 
  gammar = (float32_t)buf16[2]/FACT;
  x=0.5f;y=0.5f;
		  }
	
      float32_t temp = x*x*y; 
        
        dx = temp - (muplusone*x) + gammar;
        dy =  (muuu*x)  - temp; 
        
        x += delta*dx; 
        y += delta*dy; 
	

	buf16[count&32767]=y*32768.0f;
   }
  vill->position=count;
}

//////////////////////////////////////////////////////////

// spruceworm

static float32_t k1,k2,alpha,betaa,muuu,rho,delta;

void spruceinit(void){
  k1 = (float32_t)buf16[0]/FACT;
  k2 = (float32_t)buf16[1]/FACT;
  alpha = (float32_t)buf16[2]/FACT;
  betaa = (float32_t)buf16[3]/FACT;
  muuu = (float32_t)buf16[4]/FACT;
  rho = (float32_t)buf16[5]/FACT;
  delta = (float32_t)buf16[6]/FACT;
}

void runspruce(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  float32_t dx, dy; 
  
  static float32_t x= 0.9f; 
  static float32_t y= 0.1f;  

   for (u8 xx=0;xx<vill->howmany;xx++){
  //	for (i=0; i<howmuch; ++i) {
	          count+=step;
		  if (count>start+wrap) {
		    count=start;
  k1 = (float32_t)buf16[0]/FACT;
  k2 = (float32_t)buf16[1]/FACT;
  alpha = (float32_t)buf16[2]/FACT;
  betaa = (float32_t)buf16[3]/FACT;
  muuu = (float32_t)buf16[4]/FACT;
  rho = (float32_t)buf16[5]/FACT;
  delta = (float32_t)buf16[6]/FACT;
  x=0.9f;y=0.1f;
		  }

        float32_t temp = y*y; 
        float32_t temp2 = betaa*x;
        
        dx = (k1* x* (1.0-x)) - (muuu*y);
        dy = (k2*y*(1.0- (y/(alpha*x))))  - (rho*(temp/(temp2*temp2 +  temp))); 
        x += delta*dx; 
        y += delta*dy; 
	buf16[count&32767]=(u16)(x*65536.0f);

		}

#ifdef PCSIM
	//	printf("%c",buf16[count&32767]);
#endif
  vill->position=count;
}

//////////////////////////////////////////////////////////

// OREGONATOR

static float32_t deltaaa,epsilonnn,qqq,muuuu;

void oregoninit(void){
  deltaaa = (float32_t)buf16[0]/FACT;
  epsilonnn = (float32_t)buf16[1]/FACT;
  muuuu = (float32_t)buf16[2]/FACT;
  qqq = (float32_t)buf16[3]/FACT;
}

void runoregon(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  float32_t dx, dy, dz; 
  
  static float32_t x= 0.5f; 
  static float32_t y= 0.5f; 
  static float32_t z= 0.5f; 
  

   for (u8 xx=0;xx<vill->howmany;xx++){
    //	for (i=0; i<howmuch; ++i) {
	          count+=step;
		  if (count>start+wrap) {
		    count=start;
		    deltaaa = (float32_t)buf16[0]/FACT;
		    epsilonnn = (float32_t)buf16[1]/FACT;
		    muuuu = (float32_t)buf16[2]/FACT;
		    qqq = (float32_t)buf16[3]/FACT;
		    x=0.5f;y=0.5f;z=0.5f;
		  }
        dx = epsilonnn*((qqq*y) -(x*y) + (x*(1-x))); 
	dy = muuuu* (-(qqq*y) -(x*y) + z); 
        dz = x-y; 
        
        x += delta*dx; 
        y += delta*dy; 
        z += delta*dz; 

	

		buf16[count&32767]=x*65536.0f;
			}
#ifdef PCSIM
	//	printf("%c",buf16[count&32767]);
#endif

  vill->position=count;
}

//////////////////////////////////////////////////////////

// FITZHUGH - writes into buffer 3xhowmuch, how to store local float32_ts?

  static float32_t b0;//= 1.4;
  static float32_t b1;//= 1.1;


void fitzinit(void){
  b0=(float32_t)buf16[0]/32768.0;
  b1=(float32_t)buf16[1]/32768.0;
}

void runfitz(villager_generic* vill){
  u8 step=vill->step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;

  /* SETTINGS */

  float32_t urate= 0.7f;
  float32_t wrate= 1.7f;
  static float32_t u=0.0f,ww=0.0f; 

   for (u8 xx=0;xx<vill->howmany;xx++){
  //  for (x=0;x<howmuch;x++){
          count+=step;
	  if (count>start+wrap) {
	    count=start;
  b0=(float32_t)buf16[0]/32768.0;
  b1=(float32_t)buf16[1]/32768.0;
  u=0.0f, ww=0.0f;
	  }

    //    if (count>=MAX_SAM) count=0;
    float32_t dudt= urate*(u-(0.33333*u*u*u)-ww);
    float32_t dwdt= wrate*(b0+b1*u-ww);
	  
    u+=dudt;
    ww+=dwdt;
    //assumes fmod works correctly for negative values
        if ((u>1.0) || (u<-1.0)) u=fabsf(fmodf(u-1.0,4.0)-2.0)-1.0;
    //    if ((u>1.0) || (u<-1.0)) u=fabsf(u-2.0)-1.0;

	    int z=((float32_t)(u)*3600.0f);
    //    int zz=((float32_t)(w)*1500);

       buf16[count&32767]=(u16)z;//buf16[x+2]=zz;

#ifdef PCSIM
       //       //	//        printf("fitz: %c",u); 
       printf("%c",buf16[count&32767]); 
#endif

         }
  vill->position=count;
  }

#ifdef PCSIM

void main(int argc, char **argv)
{
  int x,count; 
  u16 howmuch,i;
  //   uint16_t xxx[MAX_SAM];
  //     u8 xxx[65536];
     srand(time(NULL)*rand());

  u16 AUDIO_BUFSZ=32768;
  u16 f0106erpos=0, F0106ERSTEP=1, F0106ERWRAP, F0106ERSTART, f0cons,tmp,wrapper;

  u8 stack_pos=0;
  struct stackey stackyy[STACK_SIZE];
  //  u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768

  for (x=0;x<65535;x++){
    buf16[x]=randi()%65536;
  }

  float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase;
  int sign_samp;
  w= 2*pi;
  w= w/256;
  for (i = 0; i <= 256; i++)
    {
      yi= 16383*sinf(phase); // was 2047
      phase=phase+w;
      sign_samp=16383+yi;     // dc offset translated for a 12 bit DAC - but is 16 bit?
      sin_data[i]=sign_samp; // write value into array
    }


  //    u16 *buf16 = (u16*) xxx;
  
  //  struct FORM *unity=malloc(sizeof(struct FORM));

  //  forminit(unity, xxx,0,3);

////  //  printf("test%d\n",256<<7);
//  	 for (x=0;x<1;x++){
  u16 addr=rand()&32767;u16 xx,xxxx;
  u16 which=(rand()&31)<<10;//
  ifsinit(buf16);// LEAVE IN!

  while(1){
    count=runifs(1,count,0,32000);
  }

				 }
#endif

