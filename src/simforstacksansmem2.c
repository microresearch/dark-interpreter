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
#define randi() (rand()%4096)
#define float32_t float


extern u16 sin_data[256];
extern u16 *stacker;//[256]; // 16*3 MAX
extern uint16_t adc_buffer[10];
extern int16_t* audio_buffer;
/* 
u16 sin_data[256];
u16 stacker[256]; 
uint16_t adc_buffer[10];
int16_t* audio_buffer;
*/
#else
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
extern u16 stacker[STACK_SIZE*4]; // 16*4 MAX
#endif

extern u16 *buf16;

//////////////////////////////////////////////////////////

u16 runnone(uint8_t howmuch, u16* buffer, u16 count, u16 start, u16 wrap){
  for (u8 s = 0; s < howmuch; s++ ) {
      count++;
  }
  return count;
}

// formantz

const u16 SAMPLE_FREQUENCY = 48000;
const float32_t Pi = 3.1415926535f;
const float32_t PI_2 = 6.28318531f;

u8 freq[3];
u8 w[3];

u16 runform(u8 step, u16 count, u16 start, u16 wrap){

  freq[0]=(u8)buf16[0];
  freq[1]=(u8)buf16[1];
  freq[2]=(u8)buf16[2];
  w[0]=(u8)buf16[3];
  w[1]=(u8)buf16[4];
  w[2]=(u8)buf16[5];

  //  u8 *buf16=(u8*)buffer;
  float32_t accum; float32_t x; 

  for (u8 f = 0; f < 3; f++ ) {
  u8 ff = freq[f]; // the three freqs

  float32_t freqq = (float32_t)ff*(50.0f/SAMPLE_FREQUENCY);

  float32_t buf1Res = 0, buf2Res = 0;
  float32_t q = 1.0f - w[f] * (Pi * 10.0f / SAMPLE_FREQUENCY);
  float32_t xp = 0;
  
  //  for (u8 s = 0; s < 8; s++ ) {
    x=(float32_t)(buf16[count%32768])/65536.0f;
    x = x + 2.0f * cosf ( PI_2 * freqq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;
    if (f==0) accum=x; 
    else
      accum+=x; // as float32_t
    if (f==2){
      buf16[(count+16384)%32768]=(float32_t)accum*65536.0f; // changed OCT!
#ifdef PCSIM 
      //      printf("%c",buffer[start+count]%255);
      //      printf("%d\n",start+count);
#endif
      //    }
      count+=step;
      //      
    }
  }
  return count;
  }

//////////////////////////////////////////////////////////

// convolve

u16 runconv(u8 step, u16 count, u16 start, u16 wrap){
  u16 y,tmp;
  float32_t tmppp;
  float32_t c0=(float32_t)buf16[0]/16384.0;
  float32_t c1=(float32_t)buf16[1]/16384.0;
  float32_t c2=(float32_t)buf16[2]/16384.0;

  //  for (i=0; i<howmuch; i++) {
    count+=step;
    //    
    y=count+16384;
    y=y%32768;
    tmp=count-1;
    //    buf16[y]=((float32_t)buf16[tmp%32768]*c0)+((float32_t)buf16[count%32768]*c1)+((float32_t)buf16[(count+1)%32768]*c2);
  tmppp=(((float)(buf16[tmp%32768])/65536.0f)*c0)+(((float)(buf16[count%32768])/65536.0f)*c1)+(((float)(buf16[(count+1)%32768])/65536.0f)*c2);
  buf16[y]=tmppp*65536.0f;

#ifdef PCSIM
    //    //    //    printf("%d %d %d %d %d\n",tmp, count,(count+1)%32768, y, buf16[(count+start)%32768]);
    //    //    printf("%c",buf16[start+count]%255);
#endif
    //  }
  return count;
}

//////////////////////////////////////////////////////////

// sine

u16 runsine(u8 step, u16 count, u16 start, u16 wrap){
static u8 cc=0;

  //  for (i=0; i<howmuch; i++) {
    count+=step;
    //    
    buf16[count%32768]=sin_data[cc]; 
#ifdef PCSIM
    //    //    printf("%c",buf16[(count+start)%32768]);
#endif
    cc++;
    //  }
  return count;
}

u16 runnoise(u8 step, u16 count, u16 start, u16 wrap){// TSTY!
  static u16 xx;
    count+=step;
    //    
    buf16[count%32768]=xx++; 
  return count;
}


//////////////////////////////////////////////////////////
/* chunking 

- copy one buffer chunk to another chunk
- do some kind of walker

*/

#ifdef PCSIM
//signed char direction[2]={-1,1};
extern signed char direction[2];
#else
extern signed char direction[2];
#endif


u16 runchunk(u8 step, u16 count, u16 start, u16 wrap){
static u16 othercount;
  u16 otherstart=buf16[0]>>1;
  u16 otherwrap=buf16[1]>>1;
  u16 dirr=buf16[2]&1;

  //  for (i=0; i<howmuch; i++) {
    count+=step;
    
    othercount+=direction[dirr];

    if (othercount<otherwrap && othercount>0){
    buf16[count%32768]=buf16[(othercount+otherstart)%32768];
    }
    else {
    if (dirr==1)  othercount=0;
    else othercount=otherwrap;
    }
#ifdef PCSIM
    //    //    printf("%c",buf16[(count+start)%32768]);
#endif
    //  }
  return count;
}

u16 runderefchunk(u8 step, u16 count, u16 start, u16 wrap){
static u16 othercount;
  u16 otherstart=buf16[0]>>1;
  u16 otherwrap=buf16[1]>>1;
  u16 dirr=buf16[2]&1;

  //  for (i=0; i<howmuch; i++) {
    count+=step;
    //    
    othercount+=direction[dirr];
    if (othercount<otherwrap && othercount>0){
      buf16[count%32768]=buf16[buf16[(othercount+otherstart)%32768]>>1];
    }
    else {
    if (dirr==1)  othercount=0;
    else othercount=otherwrap;
    }
#ifdef PCSIM
    //    //    printf("%c",buf16[(count+start)%32768]);
#endif
    //  }
  return count;
}

u16 runwalkerchunk(u8 step, u16 count, u16 start, u16 wrap){
static u16 othercount;
  static u16 otherstart=0;
  static u16 otherwrap=32768;
  static u8 dirr=0;

  //  for (i=0; i<howmuch; i++) {
    count+=step;
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
    buf16[count%32768]=buf16[(othercount+otherstart)%32768];

#ifdef PCSIM
    //    //        printf("%c",buf16[(count+start)%32768]);
    //    //    printf("x: %d\n",othercount);
#endif
    //  }
  return count;
}

u16 runswapchunk(u8 step, u16 count, u16 start, u16 wrap){
static u16 othercount;
  static u16 otherstart=0;
  static u16 otherwrap=32768;
  u16 dirr=1;
  u16 tmp;

  //  for (i=0; i<howmuch; i++) {
    count+=step;
    //    
    othercount+=direction[dirr];
    if (othercount>otherwrap || othercount<1){
      otherstart=buf16[0]>>1;
      otherwrap=buf16[1]>>1;
      dirr=buf16[2]&1;
  if (dirr==1)  othercount=0;
  else othercount=otherwrap;
    }

    tmp=buf16[count%32768];
    buf16[count%32768]=buf16[(othercount+otherstart)%32768];
    buf16[(othercount+otherstart)%32768]=tmp;
#ifdef PCSIM
    //    //    //           printf("%c",buf16[(count+start)%32768]);
#endif
    //  }
  return count;
}

//////////////////////////////////////////////////////////

// generic arithmetik datagens

u16 runinc(u8 step, u16 count, u16 start, u16 wrap){
  u16 cop=buf16[0]; 

  //  for (i=0; i<howmuch; i++) {
    count+=step;

    buf16[count%32768]=cop++;
#ifdef PCSIM
	//	//printf("%c",buf16[count%32768]);
#endif
    //  }
buf16[0]=cop;
return count;
}

u16 rundec(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  u16 cop=buf16[0]; 
  //  for (i=0; i<howmuch; i++) {
    count+=step;
    buf16[count%32768]=cop--;
#ifdef PCSIM
    //    //printf("%c",buf16[count%32768]);
#endif
    //  }
buf16[0]=cop;
return count;
}

u16 runleft(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  //  for (i=0; i<howmuch; i++) {
    count+=step;

    buf16[count%32768]=buf16[count%32768]<<1;
#ifdef PCSIM
    //    //    //    //    printf("%d %d\n",count,buf16[count%32768]);
    //printf("%c", buf16[count%32768]);
#endif
    //  }
return count;
}

u16 runright(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  //  for (i=0; i<howmuch; i++) {
    count+=step;

    buf16[count%32768]=buf16[count%32768]>>1;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",buf16[count%32768]);
    //printf("%c", buf16[count%32768]);
#endif
    //  }
  return count;
}

u16 runswap(u8 step, u16 count, u16 start, u16 wrap){
u16 temp;
  
    count+=step;
    if (count>=(wrap-1)) count=start;
    temp=buf16[count%32768];
    buf16[count%32768]=buf16[(count+1+start)%32768];
    buf16[(count+1+start)%32768]=temp;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",buf16[count%32768]);
    //printf("%c", buf16[count%32768]);
#endif
    //  }
  return count;
}

u16 runnextinc(u8 step, u16 count, u16 start, u16 wrap){

  
    count+=step; 
    if (count>=(wrap-1)) count=0;
    buf16[count%32768]=buf16[(count+1+start)%32768]+1;
  return count;
}

u16 runnextdec(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  
    count+=step;
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    buf16[count%32768]=buf16[(count+1+start)%32768]-1;
  return count;
}

u16 runnextmult(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  
    count+=step;
    if (count>=(wrap-1)) count=0;
    buf16[count%32768]*=buf16[(count+1+start)%32768];
  return count;
}

u16 runnextdiv(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  
    count+=step;
    if (count>=(wrap-1)) count=0;
    if ((buf16[(count+1+start)%32768])>0)   buf16[count%32768]/=buf16[(count+1+start)%32768];
  return count;
}

u16 runcopy(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  
    count+=step; 
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    buf16[(count+1+start)%32768]=buf16[count%32768];
  return count;
}

u16 runzero(u8 step, u16 count, u16 start, u16 wrap){
  u8 i;
  
    count+=step;

    buf16[count%32768]=0;
  return count;
}

u16 runfull(u8 step, u16 count, u16 start, u16 wrap){
    count+=step;

    buf16[count%32768]=65535;
  return count;
}

u16 runrand(u8 step, u16 count, u16 start, u16 wrap){
  u8 *workingbuffeur=(u8 *)buf16;
  //  for (i=0; i<howmuch*2; i++) {
    count+=step;
    workingbuffeur[count%32768]=randi()%255;
    count+=step;
    workingbuffeur[count%32768]=randi()%255;
  return count;
}

u16 runknob(u8 step, u16 count, u16 start, u16 wrap){
  //  u8 *workingbuffeur=(u8 *)buffer;
  
    count+=step;
    

#ifdef TENE
	//    workingbuffeur[count%32768]=adc_buffer[2]>>4; // TOP knob in either case!
    buf16[count%32768]=adc_buffer[2]<<4; // TOP knob in either case!
    //    //    //    printf("%d\n",buf16[count%32768]);
#else
    //        workingbuffeur[count%32768]=adc_buffer[3]>>4; // 8 bits
        buf16[count%32768]=adc_buffer[3]<<4; // 16 bits
#endif

  return count;
}

//////////////////////////////////////////////////////////
// swap datagen 16 bits to and from audio buffer

u16 runswapaudio(u8 step, u16 count, u16 start, u16 wrap){
u16 temp;
  
    count+=step;
    // convert signed to unsigned how? 
	temp=(uint16_t)audio_buffer[count%32768];
	audio_buffer[count%32768]=(int16_t)buf16[count%32768];
	buf16[count%32768]=temp;
  return count;
}

//////////////////////////////////////////////////////////
// OR/XOR/AND/other ops datagen 16 bits to and from audio buffer

u16 runORaudio(u8 step, u16 count, u16 start, u16 wrap){
u16 temp;
      
    count+=step;
    

    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[count%32768];
    u16 cop=buf16[0]; 
    switch(cop%5){
    case 0:
    audio_buffer[count%32768]|=(int16_t)buf16[count%32768];
    buf16[count%32768]|=temp;
    break;
    case 1:
    audio_buffer[count%32768]^=(int16_t)buf16[count%32768];
    buf16[count%32768]^=temp;
    break;
    case 2:
    audio_buffer[count%32768]&=(int16_t)buf16[count%32768];
    buf16[count%32768]&=temp;
    break;
    case 3:
    audio_buffer[count%32768]-=(int16_t)buf16[count%32768];
    buf16[count%32768]+=temp;
    break;
    case 4:
    audio_buffer[count%32768]+=(int16_t)buf16[count%32768];
    buf16[count%32768]-=temp;
    break;
    }
    //}
  return count;
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

float32_t dPop[3];
float32_t beta;//=520.0/365.0;
float32_t gamm;//=1.0/7.0;
float32_t S0;//=1.0-1e-6;
float32_t I0;//=1e-6;
float32_t step;
float32_t S,II,R;

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

  initialPop[0]=S; initialPop[1]=II; initialPop[2]=R;

  Diff(initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=dPop[i];
      tmpPop[i]=initialPop[i]+step*dPop1[i]/2;
    }

  Diff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=dPop[i];
      tmpPop[i]=initialPop[i]+step*dPop2[i]/2;  
    }

  Diff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=dPop[i];
      tmpPop[i]=initialPop[i]+step*dPop3[i]; 
    }

  Diff(tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=dPop[i];
      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*step;
    }

  S=tmpPop[0]; II=tmpPop[1]; R=tmpPop[2];
  //  //  //  printf("%g %g %g\n",S,II,R);

  return;
}



u16 runsimplesir(u8 step, u16 count, u16 start, u16 wrap){

    beta=(float32_t)buf16[0]/65536.0f;
    gamm=(float32_t)buf16[1]/65536.0f;
  S0=1.0-1e-6;
  I0=1e-6;
  step=0.01f/((beta+gamm)*S0);
  S=S0; II=I0; R=1-S-II;


  
    count+=step;
    

	Runge_Kutta();//  t+=step;
	buf16[count%32768]=(u16)((float)II*1000000000.0f);
return count;
}

//////////////////////////////////////////////////////////

// SEIR. SIR

u16 n;
u16 m;
float32_t mu;
float32_t S0,I0;
float32_t S,I[MAX_GROUPS]; // 4x16=64bytes
float32_t dPopp[MAX_GROUPS+1];//4x9=36bytes

void seirinit(u16 *buf16){
  u8 i;
  //  beta=17/5;
  //  gamm=1.0/13;
  n=13;
  m=8;
  mu=1.0f/(55.0f*365.0f);
  S0=0.05f;
  I0=0.00001f;

  S=S0;
  for(i=0;i<n;i++)
    {
      I[i]=I0/(float)n;
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
      tmpPop[i]=InitialPop[i]+step*dPop1[i]/2.0f;
    }

  seirDiff(tmpPop);
  for(i=0;i<=n;i++)
    {
      dPop2[i]=dPopp[i];
      tmpPop[i]=InitialPop[i]+step*dPop2[i]/2.0f;  
    }

  seirDiff(tmpPop);
  for(i=0;i<=n;i++)
    {
      dPop3[i]=dPopp[i];
      tmpPop[i]=InitialPop[i]+step*dPop3[i]; 
    }

  seirDiff(tmpPop);

  for(i=0;i<n;i++)
    {
      dPop4[i+1]=dPopp[i+1];
      I[i]=I[i]+(dPop1[i+1]/6.0f + dPop2[i+1]/3.0f + dPop3[i+1]/3.0f + dPop4[i+1]/6.0f)*step;
    }
  dPop4[0]=dPop[0];
  S=S+(dPop1[0]/6.0f + dPop2[0]/3.0f + dPop3[0]/3.0f + dPop4[0]/6.0f)*step;
  //  //  //  printf("S: %g\n",S);
  return;
}


u16 runseir(u8 step, u16 count, u16 start, u16 wrap){

  u8 i;
  beta=(float32_t)buf16[0]/65536.0f;
  gamm=(float32_t)buf16[1]/65536.0f;
  step=0.01f/(beta+gamm*n+mu);

  
    count+=step;


    seir_Runge_Kutta();//  t+=step;
    buf16[count%32768]=S*65536.0f;
return count;
}

//////////////////////////////////////////////////////////

// SICR. SIR

  float32_t beta;
  float32_t epsilon;
  float32_t gamm;
  float32_t Gamm; 
  float32_t mu;
  float32_t q;
  float32_t S0;
  float32_t I0;
  float32_t C0;
  float32_t t,S,III,C,R;
  float32_t dPop[3];
  float32_t step;


void sicrinit(u16 *buf16){
  //beta=0.2;
  //epsilon=0.1;
//gamm=1.0/100.0;
//Gamm=1.0/1000.0;

mu=1.0f/(50.0f*365.0f);
q=0.4f;
S0=0.1f;
I0=1e-4;
C0=1e-3;

S=S0; III=I0; C=C0; R=1-S-III-C0;
}

void sicrdiff(float32_t Pop[3])
{
  float32_t tmpS, tmpI, tmpC;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpC=Pop[2];
  dPop[0] = mu - beta*tmpS*(tmpI + epsilon*tmpC) - mu*tmpS;
  dPop[1] = beta*tmpS*(tmpI + epsilon*tmpC) - gamm*tmpI -mu*tmpI;
  dPop[2] = gamm*q*tmpI - Gamm*tmpC - mu*tmpC;
  return;
}

void sicr_Runge_Kutta(void)
{
  u8 i;
  float32_t dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float32_t tmpPop[3], initialPop[3];

  initialPop[0]=S; initialPop[1]=III; initialPop[2]=C;

  sicrdiff(initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=dPop[i];
      tmpPop[i]=initialPop[i]+step*dPop1[i]/2;
    }

  sicrdiff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=dPop[i];
      tmpPop[i]=initialPop[i]+step*dPop2[i]/2;  
    }

  sicrdiff(tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=dPop[i];
      tmpPop[i]=initialPop[i]+step*dPop3[i]; 
    }

  sicrdiff(tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=dPop[i];

      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*step;
    }

  S=tmpPop[0]; III=tmpPop[1]; C=tmpPop[2];  R=1-S-III-C;
 
  return;
}

u16 runsicr(u8 step, u16 count, u16 start, u16 wrap){

  u8 i;

beta=(float32_t)buf16[0]/65536.0f;
epsilon=(float32_t)buf16[1]/65536.0f;
gamm=(float32_t)buf16[2]/65536.0f;
Gamm=(float32_t)buf16[3]/65536.0f;
step=0.01/((beta+gamm+mu+Gamm)*S0);


  
    count+=step;


    sicr_Runge_Kutta();//  t+=step;
    buf16[count%32768]=(u16)(S*1000000.0f);
return count;
}

//////////////////////////////////////////////////////////

// IFS

float32_t prob[5];
float32_t coeff[4][6];
Point p1,p2;

void ifsinit(u16 *buf16){
  u8 column = 6, row = 4;
  u8 iter,i;
  p1.x=0.1f;
  p1.y=0.1f;         
  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){

      coeff[iter][i]=(float32_t)randi()/4096.0f;
      //      if ((float32_t)randi()/4096.0f>0.5f) coeff[iter][i]= coeff[iter][i]-1.0f;
      prob[iter]=(float32_t)randi()/4096.0f;

      prob[0]=(float32_t)buf16[0]/65536.0f;
      prob[1]=(float32_t)buf16[1]/65536.0f;
      prob[2]=(float32_t)buf16[2]/65536.0f;
      prob[3]=(float32_t)buf16[3]/65536.0f;
      prob[4]=(float32_t)buf16[4]/65536.0f;
      /*  prob[0]=0.00f;
  prob[1]=0.01f; 
  prob[2]=0.85f; 
  prob[3]=0.07f; 
  prob[4]=0.07f; 
      */
    }
  }
  }

u16 runifs(u8 step, u16 count, u16 start, u16 wrap){

  float32_t randiom_num;
  u8 i; 
  u8 column = 6, row = 4;

  randiom_num = (float32_t)randi()/4096.0f;

  //  for (x=0;x<howmuch;x++){
    count+=step;
  for(i = 0; i < row; i++){
    if ( BET(randiom_num,prob[i],prob[i+1]) ){
      p2.x = coeff[i][0]*p1.x + coeff[i][1]*p1.y + coeff[i][4];
      p2.y = coeff[i][2]*p1.x + coeff[i][3]*p1.y + coeff[i][5];
      break;
    }
    //  }
  p1=p2;  
					
  

      buf16[count%32768]=(u16)p2.y;
#ifdef PCSIM
      //printf("%d\n",buf16[count%32768]);
      //      //      //    if (count>32767) printf("IFSCRASH%d\n",count);

#endif
  /*    iter=randi()%row;
    i=randi()%column;
    coeff[iter][i]=((float32_t)randi()/(float32_t)(RANDI_MAX));
    if (((float32_t)randi()/(float32_t)(RANDI_MAX))>0.5) coeff[iter][i]= coeff[iter][i]-1;
    prob[iter]=((float32_t)randi()/(float32_t)(RANDI_MAX));
    p1.x=0.5;
    p1.y=0.5;*/
  }
return count;
}

//////////////////////////////////////////////////////////

// ROSSLER

//float32_t h,a,b,c;

u16 runrossler(u8 step, u16 count, u16 start, u16 wrap){
  float32_t lx1,ly1,lz1;
  float32_t h,a,b,c;
  u8 i;
  /* which unit to vary according to buf16 */
  // leave as so!
 static float32_t   lx0 = 0.1f;
 static float32_t   ly0 = 0.0f;
 static float32_t   lz0 = 0.0f;

 // float32_t  h = (float32_t)buf16[0]/120536.0;
 // float32_t  a = (float32_t)buf16[1]/122536.0;
 // float32_t  b = (float32_t)buf16[2]/100536.0;
 // h = 0.1f;
 //  a = 0.3f;
 //  b = 0.2f;
  
  h = (float32_t)buf16[0]/120536.0;
  a = (float32_t)buf16[1]/122536.0;
  b = (float32_t)buf16[2]/100536.0;
  c = 5.8f;

  
    count+=step;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  lx0 = lx1;
  ly0 = ly1;
  lz0 = lz1;
  
      buf16[count%32768]=(u16)(lz1*1024.0f);

#ifdef PCSIM
      //    printf("%c",buf16[count%32768]);
#endif

      //  }

return count;
}

//////////////////////////////////////////////////////////

//float32_t x0,yy0,z0;

// 2nd rossler from: MCLDChaosUGens.cpp

u16 runsecondrossler(u8 step, u16 count, u16 start, u16 wrap){

  u8 i;
  float32_t h,a,b,c;
  
  a = (float32_t)buf16[0]/65536.0f;
  b = (float32_t)buf16[1]/65536.0f;
  c = (float32_t)buf16[2]/65536.0f;
  h = (float32_t)buf16[3]/65536.0f;

  //float32_t  a = (float32_t)buf16[0]/65536.0f;
  //float32_t  b = (float32_t)buf16[1]/65536.0f;
  //float32_t  c = (float32_t)buf16[2]/65536.0f;
  //float32_t  h = (float32_t)buf16[3]/65536.0f;

 static float32_t xn;
 static float32_t yn;
 static float32_t zn;
 static float32_t xnm1;
 static float32_t ynm1;
 static float32_t znm1;

  float32_t dx = xn - xnm1;
  float32_t dy = yn - ynm1;
  float32_t dz = zn - znm1;
  //  secondrosslerinit(buf16);

  //	for (i=0; i<howmuch; ++i) {
    count+=step;
    xnm1 = xn;
    ynm1 = yn;
    znm1 = zn;

			float32_t k1x, k2x, k3x, k4x,
				k1y, k2y, k3y, k4y,
				k1z, k2z, k3z, k4z,
				kxHalf, kyHalf, kzHalf;

			// 4th order Runge-Kutta approximate solution for differential equations
			k1x = - (h * (ynm1 + znm1));
			k1y = h * (xnm1 + a * ynm1);
			k1z = h * (b + znm1 * (xnm1 - c));
			kxHalf = k1x * 0.5;
			kyHalf = k1y * 0.5;
			kzHalf = k1z * 0.5;

			k2x = - (h * (ynm1 + kyHalf + znm1 + kzHalf));
			k2y = h * (xnm1 + kxHalf + a * (ynm1 + kyHalf));
			k2z = h * (b + (znm1 + kzHalf) * (xnm1 + kxHalf - c));
			kxHalf = k2x * 0.5;
			kyHalf = k2y * 0.5;
			kzHalf = k2z * 0.5;

			k3x = - (h * (ynm1 + kyHalf + znm1 + kzHalf));
			k3y = h * (xnm1 + kxHalf + a * (ynm1 + kyHalf));
			k3z = h * (b + (znm1 + kzHalf) * (xnm1 + kxHalf - c));

			k4x = - (h * (ynm1 + k3y + znm1 + k3z));
			k4y = h * (xnm1 + k3x + a * (ynm1 + k3y));
			k4z = h * (b + (znm1 + k3z) * (xnm1 + k3x - c));

			xn = xn + (k1x + 2.0*(k2x + k3x) + k4x) * ONESIXTH;
			yn = yn + (k1y + 2.0*(k2y + k3y) + k4y) * ONESIXTH;
			zn = zn + (k1z + 2.0*(k2z + k3z) + k4z) * ONESIXTH;

			dx = xn - xnm1;
			dy = yn - ynm1;
			dz = zn - znm1;
	
	/*		ZXP(xout) = (xnm1 + dx) * 0.5f;
		ZXP(yout) = (ynm1 + dy) * 0.5f;
		ZXP(zout) = (znm1 + dz) * 1.0f;*/
	    

		buf16[count%32768]=(u16)((xnm1+dx)*1024.0f);

return count;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

#define FACT 32768.0f

u16 runbrussel(u8 step, u16 count, u16 start, u16 wrap){
  float32_t delta,muuu,muplusone,gammar;    
  float32_t dx, dy; 
  u8 i;
  static float32_t x= 0.5f; 
  static float32_t y= 0.5f;  
  delta = (float32_t)buf16[0]/FACT;
  muuu = (float32_t)buf16[1]/FACT;
  muplusone = 1.0f+muuu; 
  gammar = (float32_t)buf16[2]/FACT;

  //    for (i=0; i<howmuch; ++i) {
	    count+=step;	
      float32_t temp = x*x*y; 
        
        dx = temp - (muplusone*x) + gammar;
        dy =  (muuu*x)  - temp; 
        
        x += delta*dx; 
        y += delta*dy; 
	

	buf16[count%32768]=y*32768;
return count;
}

//////////////////////////////////////////////////////////

// spruceworm

u16 runspruce(u8 step, u16 count, u16 start, u16 wrap){

  float32_t k1,k2,alpha,betaa,muuu,rho,delta;
  float32_t dx, dy; 
  u8 i;
  static float32_t x= 0.9f; 
  static float32_t y= 0.1f;  
  k1 = (float32_t)buf16[0]/FACT;
  k2 = (float32_t)buf16[1]/FACT;
  alpha = (float32_t)buf16[2]/FACT;
  betaa = (float32_t)buf16[3]/FACT;
  muuu = (float32_t)buf16[4]/FACT;
  rho = (float32_t)buf16[5]/FACT;
  delta = (float32_t)buf16[6]/FACT;

  //	for (i=0; i<howmuch; ++i) {
	    count+=step;	
        float32_t temp = y*y; 
        float32_t temp2 = betaa*x;
        
        dx = (k1* x* (1.0-x)) - (muuu*y);
        dy = (k2*y*(1.0- (y/(alpha*x))))  - (rho*(temp/(temp2*temp2 +  temp))); 
        x += delta*dx; 
        y += delta*dy; 

	

	buf16[count%32768]=(u16)(x*65536.0f);

	//	}

#ifdef PCSIM
	//	printf("%c",buf16[count%32768]);
#endif
return count;
}

//////////////////////////////////////////////////////////

// OREGONATOR


u16 runoregon(u8 step, u16 count, u16 start, u16 wrap){

  float32_t delta,epsilonn,qq,muu;
  float32_t dx, dy, dz; 
  u8 i;
  static float32_t x= 0.5f; 
  static float32_t y= 0.5f; 
  static float32_t z= 0.5f; 
  
    delta = (float32_t)buf16[0]/FACT;
    epsilonn = (float32_t)buf16[1]/FACT;
    muu = (float32_t)buf16[2]/FACT;
    qq = (float32_t)buf16[3]/FACT;

    //	for (i=0; i<howmuch; ++i) {
	    count+=step;	
        dx = epsilonn*((qq*y) -(x*y) + (x*(1-x))); 
	dy = muu* (-(qq*y) -(x*y) + z); 
        dz = x-y; 
        
        x += delta*dx; 
        y += delta*dy; 
        z += delta*dz; 

	

		buf16[count%32768]=x*65536.0f;
		//	}
#ifdef PCSIM
	//	printf("%c",buf16[count%32768]);
#endif

return count;
}

//////////////////////////////////////////////////////////

// FITZHUGH - writes into buffer 3xhowmuch, how to store local float32_ts?

u16 runfitz(u8 step, u16 count, u16 start, u16 wrap){

  /* SETTINGS */

  float32_t urate= 0.7f;
  float32_t wrate= 1.7f;
  static float32_t u=0.0f,ww=0.0f; 
  float32_t b0;//= 1.4;
  float32_t b1;//= 1.1;
  b0=(float32_t)buf16[0]/32768.0;
  b1=(float32_t)buf16[1]/32768.0;
  u8 x;

  //  for (x=0;x<howmuch;x++){
    count+=step;
    //    if (count>=MAX_SAM) count=0;
    float32_t dudt= urate*(u-(0.33333*u*u*u)-ww);
    float32_t dwdt= wrate*(b0+b1*u-ww);
	  
    u+=dudt;
    ww+=dwdt;
    //assumes fmod works correctly for negative values
        if ((u>1.0) || (u<-1.0)) u=fabs(fmodf(u-1.0,4.0)-2.0)-1.0;
    //    if ((u>1.0) || (u<-1.0)) u=fabs(u-2.0)-1.0;

	    int z=((float32_t)(u)*3600);
    //    int zz=((float32_t)(w)*1500);

       buf16[count%32768]=(u16)z;//buf16[x+2]=zz;

#ifdef PCSIM
       //       //	//        printf("fitz: %c",u); 
       //            printf("%c",buf16[count%32768]); 
#endif

       //  }
  return count;
  }
