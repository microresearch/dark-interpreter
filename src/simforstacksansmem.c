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
      if (count>=wrap) count=0;
  }
  return count;
}

// formantz

const u16 SAMPLE_FREQUENCY = 48000;
const float32_t Pi = 3.1415926535f;
const float32_t PI_2 = 6.28318531f;

u8 freq[3];
u8 w[3];

u16 runform(uint8_t howmuch, u16* buffer, u16 count, u16 start, u16 wrap){

  freq[0]=(u8)buffer[0];
  freq[1]=(u8)buffer[1];
  freq[2]=(u8)buffer[2];
  w[0]=(u8)buffer[3];
  w[1]=(u8)buffer[4];
  w[2]=(u8)buffer[5];

  //  u8 *workingbuffer=(u8*)buffer;
  float32_t buff[255]; float32_t x; 

  for (u8 f = 0; f < 3; f++ ) {
  u8 ff = freq[f]; // the three freqs

  float32_t freqq = (float32_t)ff*(50.0f/SAMPLE_FREQUENCY);

  float32_t buf1Res = 0, buf2Res = 0;
  float32_t q = 1.0f - w[f] * (Pi * 10.0f / SAMPLE_FREQUENCY);
  float32_t xp = 0;
  
  for (u8 s = 0; s < howmuch; s++ ) {
    x=(float32_t)(buffer[(start+count)%32768])/32768.0f;
       x = x + 2.0f * cosf ( PI_2 * freqq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;
    if (f==0) buff[s]=x; 
    else
      buff[s]+=x; // as float32_t
    if (f==2){
      buffer[(start+count)%32768]=(float32_t)buff[s]*32768.0f;
#ifdef PCSIM 
      //      printf("%c",buffer[start+count]%255);
      //      printf("%d\n",start+count);
#endif
    }
      count++;
      if (count>=wrap) count=0;
  }
  }
  return count;
  }

//////////////////////////////////////////////////////////

// convolve

u16 runconv(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i=0; u16 y,tmp;

  float32_t c0=(float32_t)workingbuffer[0]/16384.0;
  float32_t c1=(float32_t)workingbuffer[1]/16384.0;
  float32_t c2=(float32_t)workingbuffer[2]/16384.0;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    y=start+count+16384;
    y=y%32768;
    tmp=count+start-1;
    workingbuffer[y]=((float32_t)workingbuffer[tmp%32768]*c0)+((float32_t)workingbuffer[(count+start)%32768]*c1)+((float32_t)workingbuffer[(count+start+1)%32768]*c2);

#ifdef PCSIM
    //    //    //    printf("%d %d %d %d %d\n",tmp, count,(count+1)%32768, y, workingbuffer[(count+start)%32768]);
    //    //    printf("%c",workingbuffer[start+count]%255);
#endif
  }
  return count;
}

//////////////////////////////////////////////////////////

// sine

u16 runsine(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; static u8 cc=0;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=sin_data[cc%256]; 
#ifdef PCSIM
    //    //    printf("%c",workingbuffer[(count+start)%32768]);
#endif
    cc++;
  }
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


u16 runchunk(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; static u16 othercount;
  u16 otherstart=workingbuffer[0]>>1;
  u16 otherwrap=workingbuffer[1]>>1;
  u16 dirr=workingbuffer[2]&1;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=direction[dirr];

    if (othercount<otherwrap && othercount>0){
    workingbuffer[(count+start)%32768]=workingbuffer[(othercount+otherstart)%32768];
    }
    else {
    if (dirr==1)  othercount=0;
    else othercount=otherwrap;
    }
#ifdef PCSIM
    //    //    printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runderefchunk(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; static u16 othercount;
  u16 otherstart=workingbuffer[0]>>1;
  u16 otherwrap=workingbuffer[1]>>1;
  u16 dirr=workingbuffer[2]&1;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=direction[dirr];
    if (othercount<otherwrap && othercount>0){
      workingbuffer[(count+start)%32768]=workingbuffer[workingbuffer[(othercount+otherstart)%32768]>>1];
    }
    else {
    if (dirr==1)  othercount=0;
    else othercount=otherwrap;
    }
#ifdef PCSIM
    //    //    printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runwalkerchunk(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; static u16 othercount;
  static u16 otherstart=0;
  static u16 otherwrap=32768;
  static u8 dirr=0;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=direction[dirr];
        if (othercount>otherwrap || othercount<1){
      // get new start and wrap - from where? would need other counter
      otherstart=workingbuffer[0]>>1;
      otherwrap=workingbuffer[1]>>1;
      dirr=workingbuffer[2]&1;
      if (dirr==1)  othercount=0;
      else       othercount=otherwrap;
      }
    workingbuffer[(count+start)%32768]=workingbuffer[(othercount+otherstart)%32768];

#ifdef PCSIM
    //    //        printf("%c",workingbuffer[(count+start)%32768]);
    //    //    printf("x: %d\n",othercount);
#endif
  }
  return count;
}

u16 runswapchunk(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; static u16 othercount;
  static u16 otherstart=0;
  static u16 otherwrap=32768;
  u16 dirr=1;
  u16 tmp;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=direction[dirr];
    if (othercount>otherwrap || othercount<1){
      otherstart=workingbuffer[0]>>1;
      otherwrap=workingbuffer[1]>>1;
      dirr=workingbuffer[2]&1;
  if (dirr==1)  othercount=0;
  else othercount=otherwrap;
    }

    tmp=workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]=workingbuffer[(othercount+otherstart)%32768];
    workingbuffer[(othercount+otherstart)%32768]=tmp;
#ifdef PCSIM
    //    //    //           printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

//////////////////////////////////////////////////////////

// generic arithmetik datagens

u16 runinc(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  u16 cop=workingbuffer[0]; 

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
        workingbuffer[(count+start)%32768]=cop++;
#ifdef PCSIM
	//	//printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
workingbuffer[0]=cop;
return count;
}

u16 rundec(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  u16 cop=workingbuffer[0]; 
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=cop--;
#ifdef PCSIM
    //    //printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
workingbuffer[0]=cop;
return count;
}

u16 runleft(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+start)%32768]<<1;
#ifdef PCSIM
    //    //    //    //    printf("%d %d\n",count,workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
return count;
}

u16 runright(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+start)%32768]>>1;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runswap(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; u16 temp;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=(wrap-1)) count=start;
    temp=workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]=workingbuffer[(count+1+start)%32768];
    workingbuffer[(count+1+start)%32768]=temp;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextinc(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; 
  for (i=0; i<howmuch; i++) {
    count++; 
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+1+start)%32768]+1;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextdec(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+1+start)%32768]-1;
#ifdef PCSIM
    //    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextmult(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+start)%32768]*=workingbuffer[(count+1+start)%32768];
#ifdef PCSIM
    //    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextdiv(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=(wrap-1)) count=0;
    if ((workingbuffer[(count+1+start)%32768])>0)   workingbuffer[(count+start)%32768]/=workingbuffer[(count+1+start)%32768];
#ifdef PCSIM
    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runcopy(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++; 
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+1+start)%32768]=workingbuffer[(count+start)%32768];
#ifdef PCSIM
    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runzero(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=0;
#ifdef PCSIM
    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runfull(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=65535;
#ifdef PCSIM
    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runrand(uint8_t howmuch, u16 *buffer, u16 count, u16 start, u16 wrap){
  u16 i=0;
  u8 *workingbuffeur=(u8 *)buffer;
  for (i=0; i<howmuch*2; i++) {
    count++;
        if (count>=wrap) count=0;
           workingbuffeur[(count+start)%32768]=randi()%255;
#ifdef PCSIM
	   //	   //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
	   //printf("%c", workingbuffeur[count+start]);
#endif
  }
  return count;
}

u16 runknob(uint8_t howmuch, u16 *buffer, u16 count, u16 start, u16 wrap){
  u16 i=0;
  //  u8 *workingbuffeur=(u8 *)buffer;
  for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

#ifdef TENE
	//    workingbuffeur[(count+start)%32768]=adc_buffer[2]>>4; // TOP knob in either case!
    buffer[(count+start)%32768]=adc_buffer[2]<<4; // TOP knob in either case!
    //    //    //    printf("%d\n",workingbuffer[(count+start)%32768]);
#else
    //        workingbuffeur[(count+start)%32768]=adc_buffer[3]>>4; // 8 bits
        buffer[(count+start)%32768]=adc_buffer[3]<<4; // 16 bits
#endif

#ifdef PCSIM
	//    printf("%c",buffer[(count+start)%32768]);
#endif
  }
  return count;
}

//////////////////////////////////////////////////////////
// swap datagen 16 bits to and from audio buffer

u16 runswapaudio(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; u16 temp;
  for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

    // convert signed to unsigned how? 
	temp=(uint16_t)audio_buffer[(count+start)%32768];
	audio_buffer[(count+start)%32768]=(int16_t)workingbuffer[(count+start)%32768];
	workingbuffer[(count+start)%32768]=temp;
}
  return count;
}

//////////////////////////////////////////////////////////
// OR/XOR/AND/other ops datagen 16 bits to and from audio buffer

u16 runORaudio(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  u8 i; u16 temp;
      for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[(count+start)%32768];
    u16 cop=workingbuffer[0]; 
    switch(cop%5){
    case 0:
    audio_buffer[(count+start)%32768]|=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]|=temp;
    break;
    case 1:
    audio_buffer[(count+start)%32768]^=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]^=temp;
    break;
    case 2:
    audio_buffer[(count+start)%32768]&=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]&=temp;
    break;
    case 3:
    audio_buffer[(count+start)%32768]-=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]+=temp;
    break;
    case 4:
    audio_buffer[(count+start)%32768]+=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]-=temp;
    break;
    }
}
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



u16 runsimplesir(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  u8 i;

    beta=(float32_t)workingbuffer[0]/65536.0f;
    gamm=(float32_t)workingbuffer[1]/65536.0f;
  S0=1.0-1e-6;
  I0=1e-6;
  step=0.01f/((beta+gamm)*S0);
  S=S0; II=I0; R=1-S-II;


  for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

	Runge_Kutta();//  t+=step;
	workingbuffer[(count+start)%32768]=(u16)((float)II*1000000000.0f);
#ifdef PCSIM
	//	//		printf("count %d %g %d\n",count,II,workingbuffer[(count+start)%32768] );
	//printf("%c",workingbuffer[(count+start)%32768]);
#endif    
  }

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

void seirinit(u16 *workingbuffer){
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


u16 runseir(uint8_t howmuch,u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  u8 i;
  beta=(float32_t)workingbuffer[0]/65536.0f;
  gamm=(float32_t)workingbuffer[1]/65536.0f;
  step=0.01f/(beta+gamm*n+mu);

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;

    seir_Runge_Kutta();//  t+=step;
    workingbuffer[(count+start)%32768]=S*65536.0f;
#ifdef PCSIM
    //    printf("%c",workingbuffer[(count+start)%32768]);
    //    //    //    if (count>32767) printf("SEIRCRASH%d\n",count);
#endif
  }
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


void sicrinit(u16 *workingbuffer){
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

u16 runsicr(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  u8 i;

beta=(float32_t)workingbuffer[0]/65536.0f;
epsilon=(float32_t)workingbuffer[1]/65536.0f;
gamm=(float32_t)workingbuffer[2]/65536.0f;
Gamm=(float32_t)workingbuffer[3]/65536.0f;
step=0.01/((beta+gamm+mu+Gamm)*S0);


  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;

    sicr_Runge_Kutta();//  t+=step;
    workingbuffer[(count+start)%32768]=(u16)(S*1000000.0f);
#ifdef PCSIM
    //    printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
return count;
}

//////////////////////////////////////////////////////////

// IFS

  float32_t prob[5];
  float32_t coeff[4][6];
  Point p1,p2;

void ifsinit(u16 *workingbuffer){
  
  p1.x=0.1f;
  p1.y=0.1f;         
  }

u16 runifs(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  float32_t randiom_num;
  u8 i,x,iter;
  u8 column = 6, row = 4;


  randiom_num = (float32_t)randi()/4096.0f;


  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){

      coeff[iter][i]=(float32_t)randi()/4096.0f;
      //      if ((float32_t)randi()/4096.0f>0.5f) coeff[iter][i]= coeff[iter][i]-1.0f;
      prob[iter]=(float32_t)randi()/4096.0f;

      prob[0]=(float32_t)workingbuffer[0]/65536.0f;
      prob[1]=(float32_t)workingbuffer[1]/65536.0f;
      prob[2]=(float32_t)workingbuffer[2]/65536.0f;
      prob[3]=(float32_t)workingbuffer[3]/65536.0f;
      prob[4]=(float32_t)workingbuffer[4]/65536.0f;
      /*  prob[0]=0.00f;
  prob[1]=0.01f; 
  prob[2]=0.85f; 
  prob[3]=0.07f; 
  prob[4]=0.07f; 
      */
    }
  }


  for (x=0;x<howmuch;x++){
    count++;
  for(i = 0; i < row; i++){
    if ( BET(randiom_num,prob[i],prob[i+1]) ){
      p2.x = coeff[i][0]*p1.x + coeff[i][1]*p1.y + coeff[i][4];
      p2.y = coeff[i][2]*p1.x + coeff[i][3]*p1.y + coeff[i][5];
      break;
    }
  }
  p1=p2;  
					
      if (count>=wrap) count=0;

      workingbuffer[(count+start)%32768]=(u16)p2.y;
#ifdef PCSIM
      //printf("%d\n",workingbuffer[(count+start)%32768]);
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

u16 runrossler(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  float32_t lx1,ly1,lz1;
  float32_t h,a,b,c;
  u8 i;
  /* which unit to vary according to workingbuffer */
  // leave as so!
 static float32_t   lx0 = 0.1f;
 static float32_t   ly0 = 0.0f;
 static float32_t   lz0 = 0.0f;

 // float32_t  h = (float32_t)workingbuffer[0]/120536.0;
 // float32_t  a = (float32_t)workingbuffer[1]/122536.0;
 // float32_t  b = (float32_t)workingbuffer[2]/100536.0;
 // h = 0.1f;
 //  a = 0.3f;
 //  b = 0.2f;
  
  h = (float32_t)workingbuffer[0]/120536.0;
  a = (float32_t)workingbuffer[1]/122536.0;
  b = (float32_t)workingbuffer[2]/100536.0;
  c = 5.8f;

  for (i=0; i<howmuch; i++) {
    count++;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  lx0 = lx1;
  ly0 = ly1;
  lz0 = lz1;
      if (count>=wrap) count=0;
      workingbuffer[(count+start)%32768]=(u16)(lz1*1024.0f);

#ifdef PCSIM
      //    printf("%c",workingbuffer[(count+start)%32768]);
#endif

  }

return count;
}

//////////////////////////////////////////////////////////

//float32_t x0,yy0,z0;

// 2nd rossler from: MCLDChaosUGens.cpp

u16 runsecondrossler(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  u8 i;
  float32_t h,a,b,c;
  
  a = (float32_t)workingbuffer[0]/65536.0f;
  b = (float32_t)workingbuffer[1]/65536.0f;
  c = (float32_t)workingbuffer[2]/65536.0f;
  h = (float32_t)workingbuffer[3]/65536.0f;

  //float32_t  a = (float32_t)workingbuffer[0]/65536.0f;
  //float32_t  b = (float32_t)workingbuffer[1]/65536.0f;
  //float32_t  c = (float32_t)workingbuffer[2]/65536.0f;
  //float32_t  h = (float32_t)workingbuffer[3]/65536.0f;

 static float32_t xn;
 static float32_t yn;
 static float32_t zn;
 static float32_t xnm1;
 static float32_t ynm1;
 static float32_t znm1;

  float32_t dx = xn - xnm1;
  float32_t dy = yn - ynm1;
  float32_t dz = zn - znm1;
  //  secondrosslerinit(workingbuffer);

	for (i=0; i<howmuch; ++i) {
    count++;
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
	        if (count>=wrap) count=0;

		workingbuffer[(count+start)%32768]=(u16)((xnm1+dx)*1024.0f);
#ifdef PCSIM
		//		printf("%c",workingbuffer[(count+start)%32768]);
		//		//			    //    if (count>32767) printf("ROSS2CRASH%d\n",count);

#endif
	}

return count;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

#define FACT 32768.0f

u16 runbrussel(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){
  float32_t delta,muuu,muplusone,gammar;    
  float32_t dx, dy; 
  u8 i;
  static float32_t x= 0.5f; 
  static float32_t y= 0.5f;  
  delta = (float32_t)workingbuffer[0]/FACT;
  muuu = (float32_t)workingbuffer[1]/FACT;
  muplusone = 1.0f+muuu; 
  gammar = (float32_t)workingbuffer[2]/FACT;

    for (i=0; i<howmuch; ++i) {
	    count++;	
      float32_t temp = x*x*y; 
        
        dx = temp - (muplusone*x) + gammar;
        dy =  (muuu*x)  - temp; 
        
        x += delta*dx; 
        y += delta*dy; 
	if (count>=wrap) count=0;

	workingbuffer[(count+start)%32768]=y*32768;
	}
#ifdef PCSIM
    //    //    //printf("brussels: x %f y %f\n",x,y); 
    //    printf("%d\n",workingbuffer[(count+start)%32768]); 
#endif
return count;
}

//////////////////////////////////////////////////////////

// spruceworm

u16 runspruce(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  float32_t k1,k2,alpha,betaa,muuu,rho,delta;
  float32_t dx, dy; 
  u8 i;
  static float32_t x= 0.9f; 
  static float32_t y= 0.1f;  
  k1 = (float32_t)workingbuffer[0]/FACT;
  k2 = (float32_t)workingbuffer[1]/FACT;
  alpha = (float32_t)workingbuffer[2]/FACT;
  betaa = (float32_t)workingbuffer[3]/FACT;
  muuu = (float32_t)workingbuffer[4]/FACT;
  rho = (float32_t)workingbuffer[5]/FACT;
  delta = (float32_t)workingbuffer[6]/FACT;

	for (i=0; i<howmuch; ++i) {
	    count++;	
        float32_t temp = y*y; 
        float32_t temp2 = betaa*x;
        
        dx = (k1* x* (1.0-x)) - (muuu*y);
        dy = (k2*y*(1.0- (y/(alpha*x))))  - (rho*(temp/(temp2*temp2 +  temp))); 
        x += delta*dx; 
        y += delta*dy; 

	if (count>=wrap) count=0;

	workingbuffer[(count+start)%32768]=(u16)(x*65536.0f);

	}

#ifdef PCSIM
	//	printf("%c",workingbuffer[(count+start)%32768]);
#endif
return count;
}

//////////////////////////////////////////////////////////

// OREGONATOR


u16 runoregon(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  float32_t delta,epsilonn,qq,muu;
  float32_t dx, dy, dz; 
  u8 i;
  static float32_t x= 0.5f; 
  static float32_t y= 0.5f; 
  static float32_t z= 0.5f; 
  
    delta = (float32_t)workingbuffer[0]/FACT;
    epsilonn = (float32_t)workingbuffer[1]/FACT;
    muu = (float32_t)workingbuffer[2]/FACT;
    qq = (float32_t)workingbuffer[3]/FACT;

	for (i=0; i<howmuch; ++i) {
	    count++;	
        dx = epsilonn*((qq*y) -(x*y) + (x*(1-x))); 
	dy = muu* (-(qq*y) -(x*y) + z); 
        dz = x-y; 
        
        x += delta*dx; 
        y += delta*dy; 
        z += delta*dz; 

	if (count>=wrap) count=0;

		workingbuffer[(count+start)%32768]=x*65536.0f;
	}
#ifdef PCSIM
	//	printf("%c",workingbuffer[(count+start)%32768]);
#endif

return count;
}

//////////////////////////////////////////////////////////

// FITZHUGH - writes into buffer 3xhowmuch, how to store local float32_ts?

u16 runfitz(uint8_t howmuch, u16 *workingbuffer, u16 count, u16 start, u16 wrap){

  /* SETTINGS */

  float32_t urate= 0.7f;
  float32_t wrate= 1.7f;
  static float32_t u=0.0f,ww=0.0f; 
  float32_t b0;//= 1.4;
  float32_t b1;//= 1.1;
  b0=(float32_t)workingbuffer[0]/32768.0;
  b1=(float32_t)workingbuffer[1]/32768.0;
  u8 x;

  for (x=0;x<howmuch;x++){
    count++;
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
    if (count>=wrap) count=0;
       workingbuffer[(count+start)%32768]=(u16)z;//workingbuffer[x+2]=zz;

#ifdef PCSIM
       //       //	//        printf("fitz: %c",u); 
       //            printf("%c",workingbuffer[(count+start)%32768]); 
#endif

  }
  return count;
  }

#ifdef PCSIM

void passingarraytest(uint8_t *buffer) {
  u8 *m_memory; u16 i;
  m_memory=buffer;
  for (i=0;i<65535;i++){
    //    //    printf("%d %d\n",i, m_memory[i]);
  }
}

#endif

signed char func_pushn(struct stackey stack[STACK_SIZE], u16 typerr, u16* buffer, u8 stack_pos, u16 howmuch, u16 start, u16 wrap){
  if (stack_pos<STACK_SIZE)
    {
      if (howmuch==0) howmuch=1;
      u8 tmp=stack_pos*4;
      stacker[tmp]=start;
      stack[stack_pos].count=start;
      stacker[tmp+1]=wrap;
      stacker[tmp+2]=howmuch;
      stacker[tmp+3]=typerr;
      stack[stack_pos].buffer=buffer;
      stack_pos++;
    }

  // re-inits:
  //  simplesirinit(buffer);
  seirinit(buffer);// LEAVE IN!
  sicrinit(buffer);// LEAVE IN!
  ifsinit(buffer);// LEAVE IN!
  ///////
  //  rosslerinit(buffer);
  //  secondrosslerinit(buffer);
  //  brusselinit(buffer);
  //  spruceinit(buffer);
  //  oregoninit(buffer);

  return stack_pos;
}


void func_runall(struct stackey stack[STACK_SIZE],u8 stack_pos){
  //  static u8 i=0; //TESTY!!!
    u8 i;
  u8 x,bufsel; u16 *buffer;
      for (i=0;i<stack_pos;i++){
	//	    if (i>=stack_pos) i=0; // TESTY!
	u8 tmp=i*4,howmuch; u16 start,wrap;

	//	stack[i].count=runconv(stacker[tmp+2],stack[i].buffer,stack[i].count,start,stacker[tmp+1]);
	start=stacker[tmp++]>>1;
	wrap=stacker[tmp++]>>1;
	howmuch=stacker[tmp]>>9; // 7 bits
	//	bufsel=stacker[tmp++]>>15; // last bit TESTY!
	bufsel=stacker[tmp++]&1; //AUG!
	//	bufsel=1;  // AUG..or...
	x=(stacker[tmp]>>10)%NUM_FUNCS; // 6 bits
	//	printf("running: %d start: %d wrap %d\n",x,start,wrap);

	if (bufsel) buffer=buf16;
	else buffer=(u16*)audio_buffer;

	switch(x){ // type
	case NUNNY:
	stack[i].count=runnone(howmuch,buffer,stack[i].count,start,wrap);
	break;
	case CONVY:
	stack[i].count=runconv(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case FORMY:
	stack[i].count=runform(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SINEY:
	stack[i].count=runsine(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case INCY:
	stack[i].count=runinc(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case DECY:
	stack[i].count=rundec(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case LEFTY:
	stack[i].count=runleft(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case RIGHTY:
	stack[i].count=runright(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SWAPPY:
	stack[i].count=runswap(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case NEXTINCY:
	stack[i].count=runnextinc(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case NEXTDECY:
	stack[i].count=runnextdec(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case NEXTMULTY:
	stack[i].count=runnextmult(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case NEXTDIVY:
	stack[i].count=runnextdiv(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case COPYY:
	stack[i].count=runcopy(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case ZEROY:
	stack[i].count=runzero(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case FULLY:
	stack[i].count=runfull(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case RANDY:
	stack[i].count=runrand(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case KNOBY:
	stack[i].count=runknob(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SWAPAUDIOY:
	stack[i].count=runswapaudio(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case ORAUDIOY:
	stack[i].count=runORaudio(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SIMPLESIRY:
	stack[i].count=runsimplesir(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SEIRY:
	stack[i].count=runseir(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SICRY:
	stack[i].count=runsicr(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case IFSY:
	stack[i].count=runifs(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case ROSSLERY:
	stack[i].count=runrossler(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SECONDROSSLERY:
	stack[i].count=runsecondrossler(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case BRUSSELY:
	stack[i].count=runbrussel(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SPRUCEY:
	stack[i].count=runspruce(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case OREGONY:
	stack[i].count=runoregon(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case FITZY:
	stack[i].count=runfitz(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case SWAPCHUNKY:
	stack[i].count=runswapchunk(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case CHUNKY:
	stack[i].count=runchunk(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case DEREFCHUNKY:
	stack[i].count=runderefchunk(howmuch,buffer,stack[i].count,start,wrap);
	break;
      case WALKERCHUNKY:
	stack[i].count=runwalkerchunk(howmuch,buffer,stack[i].count,start,wrap);
	}
	//		i++;
	}//TESTY!
}

signed char func_pop(u8 stack_pos){
 	if (stack_pos>0)
	{
	  stack_pos--;
	}
	return stack_pos;
	}

#ifdef PCSIM

void tester(u8 SAMPLEDIRR){
  int count=0; int dirry; static u16 sampleposread=100; u16 startread=100,wrapread=200, SAMPLESTARTREAD=100, SAMPLEWRAPREAD=200; 
  int dir[2]={-1,1};

 dirry=dir[SAMPLEDIRR]*10;
 // count+=dirry;
 count=((sampleposread-startread)+dirry);
 if (count<wrapread && count>0)
   {
     sampleposread+=dirry;//)%32768;
   }
 else {
   startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;
   if (SAMPLEDIRR==1) {
     sampleposread=startread; //forwards
     //   count=0;
   }
   else {
     sampleposread=startread+wrapread;
     //     count=wrapread;
   }
   //   sampleposread=startread;//+wrapread;
   //   count=0;
 }
 // // // printf("pos: %d\n", sampleposread);
}


/*void main(int argc, char **argv)
{
  int x; 
  u16 howmuch,i;
  //   uint16_t xxx[MAX_SAM];
     u8 xxx[65536];
     srand(time(NULL)*rand());

  u16 AUDIO_BUFSZ=32768;
  u16 f0106erpos=0, F0106ERSTEP=1, F0106ERWRAP, F0106ERSTART, f0cons,tmp,wrapper;

  u8 stack_pos=0;
  struct stackey stackyy[STACK_SIZE];
  //  u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768

  for (x=0;x<65535;x++){
    xxx[x]=randi()%65536;
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


    u16 *buf16 = (u16*) xxx;
  
  //  struct FORM *unity=malloc(sizeof(struct FORM));

  //  forminit(unity, xxx,0,3);

////  //  printf("test%d\n",256<<7);
//  	 for (x=0;x<1;x++){
  u16 addr=rand()%32768;u16 xx,xxxx;
  u16 which=(rand()%31)<<10;//
  //		   int which=atoi(argv[1])<<10;
  for (x=0;x<64;x++){
  stack_pos=func_pushn(stackyy,which,buf16,stack_pos,(100<<9),addr,rand()%32768);//howmuch,start,wrap
		   	 	   }
  


		   		   	 while(1){
		   	   func_runall(stackyy,stack_pos); // simulations
//	   //		   printf("%c",buf16[x%32768]>>8);
			   //		   x++;
		   	   	 }
				 }*/
#endif

