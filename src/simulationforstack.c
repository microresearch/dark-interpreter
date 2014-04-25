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
#define randi() rand()
u16 sin_data[256];
#else
#include "simulation.h"
#include <malloc.h>
#include <math.h>
#include <audio.h>
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));;
extern u16 sin_data[256];
#endif

u16 stacker[64]; // 16*4 MAX

//////////////////////////////////////////////////////////

// formantz

void forminit(void* unity, u16 *workingbuffer){
  struct FORM* unit=unity;
  unit->freq[0]=(u8)workingbuffer[0];
  unit->freq[1]=(u8)workingbuffer[1];
  unit->freq[2]=(u8)workingbuffer[2];
  unit->w[0]=(u8)workingbuffer[3];
  unit->w[1]=(u8)workingbuffer[4];
  unit->w[2]=(u8)workingbuffer[5];
  unit->buffer=(u16*)workingbuffer;
}

const u16 SAMPLE_FREQUENCY = 48000;
const float Pi = 3.1415926535f;
const float PI_2 = 6.28318531f;

//////////////////////////////////////////////////////////SPEED!

u16 runform(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  struct FORM *unit=unity;
  u8 *workingbuffer=(u8 *)unit->buffer;
  float buff[255]; float x; 
  // samples to float
  //  u16 count=count;

  for (int f = 0; f < 3; f++ ) {
  u8 ff = unit->freq[f]; // the three freqs

  float freq = (float)ff*(50.0f/SAMPLE_FREQUENCY);

  float buf1Res = 0, buf2Res = 0;
  float q = 1.0f - unit->w[f] * (Pi * 10.0f / SAMPLE_FREQUENCY);
  float xp = 0;
  
  for (u8 s = 0; s < howmuch; s++ ) {
    // x is our float sample
    // Apply formant filter
    x=(float)(workingbuffer[(start+count)%32768])/32768.0f;
       x = x + 2.0f * cosf ( PI_2 * freq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;
    buff[s]+=x; // as float
    if (f==2){
      workingbuffer[(start+count)%32768]=(float)buff[s]*32768.0f;
#ifdef PCSIM
	    //            printf("%c",workingbuffer[count]%255);
      //    if (tmyyp>32767) printf("FDORMCRASH%d\n",tmp);

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

void convinit(void* unity, uint16_t *workingbuffer){
  struct CONV* unit=unity;
  unit->c0=(float)workingbuffer[0]/16384.0;
  unit->c1=(float)workingbuffer[1]/16384.0;
  unit->c2=(float)workingbuffer[2]/16384.0;
  unit->buffer=workingbuffer;
}


u16 runconv(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0; u16 y,tmp;
  struct CONV *unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    y=start+count+16384;
    y=y%32768;
    if (count==0) tmp=32766;
    else tmp=count-1;
			
        workingbuffer[y]=((float)workingbuffer[tmp%32768]*unit->c0)+((float)workingbuffer[(count+start)%32768]*unit->c1)+((float)workingbuffer[(count+1)%32768]*unit->c2);

#ifdef PCSIM
    //    printf("%d %d %d %d %d\n",tmp, count,(count+1)%32768, y, workingbuffer[(count+start)%32768]);
    //    printf("%f %f %f\n",unit->c0,unit->c1,unit->c2);
	//                    printf("%c",workingbuffer[(count+start)%32768]%255);
    //    if (count>32767) printf("CONVCRASH%d\n",count);

#endif
  }
  return count;
}


//////////////////////////////////////////////////////////

// sine

void sineinit(void* unity, uint16_t *workingbuffer){
  struct siney* unit=unity;
  unit->cc=0;
  unit->buffer=workingbuffer;
}

u16 runsine(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct siney *unit=unity;
  u16 *workingbuffer=unit->buffer;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=sin_data[(unit->cc)%256];
#ifdef PCSIM
    //        printf("%c",workingbuffer[(count+start)%32768]);
    //if (count>32767) printf("SINECRASH%d\n",count);
#endif
    unit->cc++;
  }
  return count;
}

//////////////////////////////////////////////////////////
/* chunking 

- copy one buffer chunk to another chunk
- do some kind of walker

*/

void chunkinit(void* unity, uint16_t *workingbuffer){
  struct chunkey* unit=unity; u16 tmp;
  unit->buffer=workingbuffer;
  unit->otherstart=workingbuffer[0]>>1;
  unit->otherwrap=workingbuffer[1]>>1;
  unit->dirr=workingbuffer[2]%4;
  if (unit->otherstart>unit->otherwrap) {
    u16 tmp=unit->otherstart;
    unit->otherstart=unit->otherwrap;
    unit->otherwrap=tmp;
  }
  if (unit->dirr==1 || unit->dirr==2)  unit->othercount=unit->otherstart;
  else unit->othercount=unit->otherwrap;
  tmp=sqrtf((float)unit->otherwrap);unit->newdir[0]=-tmp;unit->newdir[2]=tmp;
  unit->newdir[1]=1;
  unit->newdir[3]=-1;
}

u16 runchunk(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct chunkey* unit=unity;
  u16 *workingbuffer=unit->buffer;
  u16 othercount=unit->othercount;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;

    othercount+=unit->newdir[unit->dirr];
    if (unit->dirr==1 || unit->dirr==2)    { if (othercount>unit->otherwrap) othercount=0;}
      else if (othercount<=unit->otherstart || othercount>unit->otherwrap) othercount=unit->otherwrap;
    workingbuffer[(count+start)%32768]=workingbuffer[(othercount+unit->otherstart)%32768];

#ifdef PCSIM
    //          printf("%c",workingbuffer[(count+start)%32768]);
    //        if (count>32767) printf("CONVCRASH%d\n",count);

#endif
  }
  unit->othercount=othercount;
  return count;
}

u16 runderefchunk(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct chunkey* unit=unity;
  u16 *workingbuffer=unit->buffer;
  u16 othercount=unit->othercount;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=unit->newdir[unit->dirr];
    if (unit->dirr==1 || unit->dirr==2)   {  if (othercount>=unit->otherwrap) othercount=0;
    }
      else if (othercount<=unit->otherstart || othercount>unit->otherwrap) othercount=unit->otherwrap;
    workingbuffer[(count+start)%32768]=workingbuffer[workingbuffer[(othercount+unit->otherstart)%32768]>>1];
#ifdef PCSIM
    //        printf("%c",workingbuffer[(count+start)%32768]);
    //    if (count>32767) printf("CONVCRASH%d\n",count);

#endif
  }
  unit->othercount=othercount;
  return count;
}


u16 runwalkerchunk(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct chunkey* unit=unity;
  u16 *workingbuffer=unit->buffer;
  u16 tmp;
  u16 othercount=unit->othercount;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=unit->newdir[unit->dirr];
    if (othercount>unit->otherwrap || othercount<unit->otherstart){
      // get new start and wrap - from where? would need other counter
      unit->otherstart=workingbuffer[0]>>1;
      unit->otherwrap=workingbuffer[1]>>1;
      unit->dirr=workingbuffer[2]%4;
  if (unit->otherstart>unit->otherwrap) {
    u16 tmp=unit->otherstart;
    unit->otherstart=unit->otherwrap;
    unit->otherwrap=tmp;
  }
  if (unit->dirr==1 || unit->dirr==2)  othercount=0;
  else othercount=unit->otherwrap;
  tmp=sqrtf((float)unit->otherwrap);unit->newdir[0]=-tmp;unit->newdir[2]=tmp;
    }
    workingbuffer[(count+start)%32768]=workingbuffer[(othercount+unit->otherstart)%32768];

#ifdef PCSIM
    //        printf("%c",workingbuffer[(count+start)%32768]);
    //    if (count>32767) printf("CONVCRASH%d\n",count);

#endif
  }
  unit->othercount=othercount;
  return count;
}


u16 runswapchunk(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct chunkey* unit=unity;
  u16 *workingbuffer=unit->buffer;
  u16 tmp;
  u16 othercount=unit->othercount;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    othercount+=unit->newdir[unit->dirr];
    if (othercount>unit->otherwrap || othercount<unit->otherstart){
      // get new start and wrap - from where? would need other counter
      unit->otherstart=workingbuffer[0]>>1;
      unit->otherwrap=workingbuffer[1]>>1;
      unit->dirr=workingbuffer[2]%4;
  if (unit->otherstart>unit->otherwrap) {
    u16 tmp=unit->otherstart;
    unit->otherstart=unit->otherwrap;
    unit->otherwrap=tmp;
  }
  if (unit->dirr==1 || unit->dirr==2)  othercount=0;
  else othercount=unit->otherwrap;
  tmp=sqrtf((float)unit->otherwrap);unit->newdir[0]=-tmp;unit->newdir[2]=tmp;
    }

    tmp=workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]=workingbuffer[(othercount+unit->otherstart)%32768];
    workingbuffer[(othercount+unit->otherstart)%32768]=tmp;
#ifdef PCSIM
    //        printf("%c",workingbuffer[(count+start)%32768]);
    //    if (count>32767) printf("CONVCRASH%d\n",count);

#endif
  }
  unit->othercount=othercount;
  return count;
}



//////////////////////////////////////////////////////////

// generic arithmetik datagens

void geninit(void* unity, uint16_t *workingbuffer){
  struct generik* unit=unity;
  unit->cop=workingbuffer[0]; 
  unit->buffer=workingbuffer;
}

u16 runinc(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;

  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
        workingbuffer[(count+start)%32768]=unit->cop++;
#ifdef PCSIM
	//        printf("%c",workingbuffer[(count+start)%32768]);
    //    if (count>32767) printf("CONVCRASH%d\n",count);

#endif
  }
return count;
}

u16 rundec(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=unit->cop--;
#ifdef PCSIM
    //        printf("%c",workingbuffer[(count+start)%32768]);
#endif
  }
return count;
}

u16 runleft(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+start)%32768]<<=1;
#ifdef PCSIM
    //    printf("%d %d\n",count,workingbuffer[(count+start)%32768]);
    //      printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
return count;
}

u16 runright(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+start)%32768]>>=1;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runswap(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0; u16 temp,yy;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=(wrap-1)) count=start;
    temp=workingbuffer[(count+start)%32768];
    yy=count+1;
    workingbuffer[(count+start)%32768]=workingbuffer[yy%32768];
    workingbuffer[yy%32768]=temp;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextinc(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0; u16 yy;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++; 
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+1+start)%32768]+1;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextdec(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+start)%32768]=workingbuffer[(count+1+start)%32768]-1;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextmult(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+start)%32768]*=workingbuffer[(count+1+start)%32768];
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runnextdiv(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    if ((workingbuffer[(count+1+start)%32768])>0)   workingbuffer[(count+start)%32768]/=workingbuffer[(count+1+start)%32768];
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}


u16 runcopy(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++; 
    //    if (count>=32766) count=0;
    if (count>=(wrap-1)) count=0;
    workingbuffer[(count+1+start)%32768]=workingbuffer[(count+start)%32768];
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runzero(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;

    workingbuffer[(count+start)%32768]=0;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runfull(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;

    workingbuffer[(count+start)%32768]=65535;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
    //        printf("%c", workingbuffer[(count+start)%32768]);
#endif
  }
  return count;
}

u16 runrand(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u16 i=0;
  //  u8 *workingbuffeur=(u8 *)workingbuffer;
  struct generik* unit=unity;
  u8 *workingbuffeur=(u8 *)unit->buffer;
  for (i=0; i<howmuch*2; i++) {
    count++;
        if (count>=wrap) count=0;

           workingbuffeur[count]=randi()%255;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
	   //	       printf("%c", workingbuffeur[count]);
#endif
  }
  return count;
}

u16 runknob(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u16 i=0;
  struct generik* unit=unity;
  u8 *workingbuffeur=(u8 *)unit->buffer;
  for (i=0; i<howmuch*2; i++) {
    count++;
        if (count>=wrap) count=0;

#ifndef PCSIM

#if defined(SUSP) || defined(LACH)
        workingbuffeur[count]=adc_buffer[3]<<4; // TOP knob in either case!
    //    printf("%d\n",workingbuffer[(count+start)%32768]);
#else
        workingbuffeur[count]=adc_buffer[2]<<4;
#endif
#endif
  }
  return count;
}

//////////////////////////////////////////////////////////
// swap datagen 16 bits to and from audio buffer

u16 runswapaudio(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0; u16 temp;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

#ifndef PCSIM
    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[count];
    audio_buffer[count]=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]=temp;
#endif
}
  return count;
}

//////////////////////////////////////////////////////////
// OR/XOR/AND/other ops datagen 16 bits to and from audio buffer

u16 runORaudio(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  u8 i=0; u16 temp;
  struct generik* unit=unity;
  u16 *workingbuffer=unit->buffer;
      for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

#ifndef PCSIM
    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[count];
    switch(unit->cop%5){
    case 0:
    audio_buffer[count]|=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]|=temp;
    break;
    case 1:
    audio_buffer[count]^=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]^=temp;
    break;
    case 2:
    audio_buffer[count]&=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]&=temp;
    break;
    case 3:
    audio_buffer[count]-=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]+=temp;
    break;
    case 4:
    audio_buffer[count]+=(int16_t)workingbuffer[(count+start)%32768];
    workingbuffer[(count+start)%32768]-=temp;
    break;
    }

#endif
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

void Diff(struct simpleSIR* unit,float Pop[3])
{
  float tmpS, tmpI, tmpR;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpR=Pop[2];

  unit->dPop[0] = - unit->beta*tmpS*tmpI;              // dS/dt
  unit->dPop[1] = unit->beta*tmpS*tmpI - unit->gamm*tmpI;   // dI/dt
  unit->dPop[2] = unit->gamm*tmpI;                    // dR/dt
}

void Runge_Kutta(struct simpleSIR* unit)
{
  unsigned char i;
  float dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float tmpPop[3], initialPop[3];

  /* Integrates the equations one step, using Runge-Kutta 4
     Note: we work with arrays rather than variables to make the
     coding easier */

  initialPop[0]=unit->S; initialPop[1]=unit->I; initialPop[2]=unit->R;

  Diff(unit,initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop1[i]/2;
    }

  Diff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop2[i]/2;  
    }

  Diff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop3[i]; 
    }

  Diff(unit,tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*unit->step;
    }

  unit->S=tmpPop[0]; unit->I=tmpPop[1]; unit->R=tmpPop[2];
  //printf("%g %g %g\n",unit->S,unit->I,unit->R);

  return;
}

void simplesirinit(void* unity, uint16_t *workingbuffer){
  struct simpleSIR* unit=unity;
  //  unit->t=0;
  unit->buffer=workingbuffer;

  //  unit->beta=520.0/365.0;
  //  unit->gamm=1.0/7.0;
  unit->beta=(float)workingbuffer[0]/65536.0;
  unit->gamm=(float)workingbuffer[1]/65536.0;
  unit->S0=1.0-1e-6;
  unit->I0=1e-6;
  unit->step=0.01/((unit->beta+unit->gamm)*unit->S0);
  unit->S=unit->S0; unit->I=unit->I0; unit->R=1-unit->S-unit->I;
  // what else in init?
}

u16 runsimplesir(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  u8 i=0;
  struct simpleSIR* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

    Runge_Kutta(unit);//  unit->t+=step;
        workingbuffer[(count+start)%32768]=unit->I;
#ifdef PCSIM
	//        printf("%c",unit->I);
	//if (count>32767) printf("SIRCRASH%d\n",count);
#endif    
  }

return count;
}

//////////////////////////////////////////////////////////

// SEIR. SIR

void seirinit(void* unity, uint16_t *workingbuffer){
  unsigned char i;
  struct SEIR* unit=unity;
  unit->buffer=workingbuffer;
  //unit->beta=17/5;
  //unit->gamm=1.0/13;
  unit->beta=(float)workingbuffer[0]/65536.0;
  unit->gamm=(float)workingbuffer[1]/65536.0;
  unit->n=13;
  unit->m=8;
  unit->mu=1.0/(55*365);
  unit->S0=0.05;
  unit->I0=0.00001;
  unit->step=0.01/(unit->beta+unit->gamm*unit->n+unit->mu);

  unit->S=unit->S0;
  for(i=0;i<unit->n;i++)
    {
      unit->I[i]=unit->I0/unit->n;
    }

}

void seirDiff(struct SEIR* unit,float Pop[MAX_GROUPS+1])
{
  int i;
  float Inf, tmpS;

  /* Set up some temporary variables to make things easier.
     Note I_i = Pop[i]  */
  
  tmpS=Pop[0];
  Inf=0;
  for(i=unit->m+1;i<=unit->n;i++)
    {
      Inf+=Pop[i];
    }
  
  unit->dPop[0]= unit->mu - unit->beta*Inf*tmpS - unit->mu*tmpS;
  unit->dPop[1]= unit->beta*Inf*tmpS - unit->gamm*unit->n*Pop[1] - unit->mu*Pop[1];

  for(i=2;i<=unit->n;i++)
    {
      unit->dPop[i]= unit->gamm*unit->n*Pop[i-1] - unit->gamm*unit->n*Pop[i] - unit->mu*Pop[i];
    }

  return;
}

void seir_Runge_Kutta(struct SEIR* unit)
{
  u8 i;
  float dPop1[MAX_GROUPS], dPop2[MAX_GROUPS], dPop3[MAX_GROUPS], dPop4[MAX_GROUPS];
  float tmpPop[MAX_GROUPS],InitialPop[MAX_GROUPS];

  /* Integrates the equations one step, using Runge-Kutta 4
     Note: we work with arrays rather than variables to make the
     coding easier */

  InitialPop[0]=unit->S;
  for(i=0;i<unit->n;i++)
    {
      InitialPop[i+1]=unit->I[i];
    }

  seirDiff(unit,InitialPop);
  for(i=0;i<=unit->n;i++)
    {
      dPop1[i]=unit->dPop[i];
      tmpPop[i]=InitialPop[i]+unit->step*dPop1[i]/2;
    }

  seirDiff(unit,tmpPop);
  for(i=0;i<=unit->n;i++)
    {
      dPop2[i]=unit->dPop[i];
      tmpPop[i]=InitialPop[i]+unit->step*dPop2[i]/2;  
    }

  seirDiff(unit,tmpPop);
  for(i=0;i<=unit->n;i++)
    {
      dPop3[i]=unit->dPop[i];
      tmpPop[i]=InitialPop[i]+unit->step*dPop3[i]; 
    }

  seirDiff(unit,tmpPop);

  for(i=0;i<unit->n;i++)
    {
      dPop4[i+1]=unit->dPop[i+1];
      unit->I[i]=unit->I[i]+(dPop1[i+1]/6 + dPop2[i+1]/3 + dPop3[i+1]/3 + dPop4[i+1]/6)*unit->step;
    }
  dPop4[0]=unit->dPop[0];
  unit->S=unit->S+(dPop1[0]/6 + dPop2[0]/3 + dPop3[0]/3 + dPop4[0]/6)*unit->step;

  return;
}


u16 runseir(uint8_t howmuch,void* unity, u16 count, u16 start, u16 wrap){

  u8 i=0;
  struct SEIR* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=wrap) count=0;

    seir_Runge_Kutta(unit);//  unit->t+=step;
        workingbuffer[(count+start)%32768]=unit->S;
#ifdef PCSIM
	//      printf("%c",unit->S);
    //    if (count>32767) printf("SEIRCRASH%d\n",count);
#endif
  }
return count;
}

//////////////////////////////////////////////////////////

// SICR. SIR

void sicrinit(void* unity, uint16_t *workingbuffer){
  //unit->beta=0.2;
  //unit->epsilon=0.1;
//unit->gamm=1.0/100.0;
//unit->Gamm=1.0/1000.0;
  struct SICR* unit=unity;
  unit->buffer=workingbuffer;
unit->beta=(float)workingbuffer[0]/65536.0;
unit->epsilon=(float)workingbuffer[1]/655360.0;
unit->gamm=(float)workingbuffer[2]/655360.0;
unit->Gamm=(float)workingbuffer[3]/655360.0;
unit->mu=1.0/(50.0*365.0);
unit->q=0.4;
unit->S0=0.1;
unit->I0=1e-4;
unit->C0=1e-3;

unit->S=unit->S0; unit->I=unit->I0; unit->C=unit->C0; unit->R=1-unit->S-unit->I-unit->C0;
unit->step=0.01/((unit->beta+unit->gamm+unit->mu+unit->Gamm)*unit->S0);


}

void sicrdiff(struct SICR* unit,float Pop[3])
{
  float tmpS, tmpI, tmpC;
  tmpS=Pop[0]; tmpI=Pop[1]; tmpC=Pop[2];
  unit->dPop[0] = unit->mu - unit->beta*tmpS*(tmpI + unit->epsilon*tmpC) - unit->mu*tmpS;
  unit->dPop[1] = unit->beta*tmpS*(tmpI + unit->epsilon*tmpC) - unit->gamm*tmpI -unit->mu*tmpI;
  unit->dPop[2] = unit->gamm*unit->q*tmpI - unit->Gamm*tmpC - unit->mu*tmpC;
  return;
}

void sicr_Runge_Kutta(struct SICR* unit)
{
  u8 i=0;
  float dPop1[3], dPop2[3], dPop3[3], dPop4[3];
  float tmpPop[3], initialPop[3];

  initialPop[0]=unit->S; initialPop[1]=unit->I; initialPop[2]=unit->C;

  sicrdiff(unit,initialPop);
  for(i=0;i<3;i++)
    {
      dPop1[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop1[i]/2;
    }

  sicrdiff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop2[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop2[i]/2;  
    }

  sicrdiff(unit,tmpPop);
  for(i=0;i<3;i++)
    {
      dPop3[i]=unit->dPop[i];
      tmpPop[i]=initialPop[i]+unit->step*dPop3[i]; 
    }

  sicrdiff(unit,tmpPop);

  for(i=0;i<3;i++)
    {
      dPop4[i]=unit->dPop[i];

      tmpPop[i]=initialPop[i]+(dPop1[i]/6 + dPop2[i]/3 + dPop3[i]/3 + dPop4[i]/6)*unit->step;
    }


  unit->S=tmpPop[0]; unit->I=tmpPop[1]; unit->C=tmpPop[2];  unit->R=1-unit->S-unit->I-unit->C;
 
  return;
}

u16 runsicr(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  u8 i=0;
  struct SICR* unit=unity;
  u16 *workingbuffer=unit->buffer;
  for (i=0; i<howmuch; i++) {
    count++;
        if (count>=wrap) count=0;

    sicr_Runge_Kutta(unit);//  unit->t+=step;
        workingbuffer[(count+start)%32768]=unit->S;
#ifdef PCSIM
	//        printf("%c",unit->S);
	//if (count>32767) printf("SICR2CRASH%d\n",count);
#endif
  }
return count;
}




//////////////////////////////////////////////////////////

// IFS

void ifsinit(void* unity, uint16_t *workingbuffer){
  u8 i,iter;
  u8 column = 6, row = 4;
  struct IFS* unit=unity;
  unit->buffer=workingbuffer;
  unit->p1.x=0.1;
  unit->p1.y=0.1;         

  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){
      //      iter=randi()%row;
      //      i=randi()%column;

#ifdef PCSIM
      unit->coeff[iter][i]=((float)randi()/(float)(RAND_MAX));
      if (((float)randi()/(float)(RAND_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
      unit->prob[iter]=((float)randi()/(float)(RAND_MAX));
#else
      unit->coeff[iter][i]=((float)randi()/4096.0f);
      if (((float)randi()/4096.0f)>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
      unit->prob[iter]=((float)randi()/4096.0f);
#endif

  unit->prob[0]=(float)workingbuffer[0]/65536.0;
  unit->prob[1]=(float)workingbuffer[1]/65536.0;
  unit->prob[2]=(float)workingbuffer[2]/65536.0;
  unit->prob[3]=(float)workingbuffer[3]/65536.0;
  unit->prob[4]=(float)workingbuffer[4]/65536.0;
    }
  }

}

u16 runifs(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  float randiom_num;
  u8 iter,i,it,x;
  u8 column = 6, row = 4;
  struct IFS* unit=unity;
  u16 *workingbuffer=unit->buffer;
  /*  ifs->prob[0]=0.0;
  ifs->prob[1]=0.85; 
  ifs->prob[2]=0.92; 
  ifs->prob[3]=0.99; 
  ifs->prob[4]=1.0; 
  */

#ifdef PCSIM
  randiom_num = (float)randi()/(float)(RAND_MAX);
#else
  randiom_num = (float)randi()/4096.0;
#endif
  for (x=0;x<howmuch;x++){
    count++;
  for(i = 0; i < row; i++){
    if ( BET(randiom_num,unit->prob[i],unit->prob[i+1]) ){
      unit->p2.x = unit->coeff[i][0]*unit->p1.x + unit->coeff[i][1]*unit->p1.y + unit->coeff[i][4];
      unit->p2.y = unit->coeff[i][2]*unit->p1.x + unit->coeff[i][3]*unit->p1.y + unit->coeff[i][5];
      break;
    }
  }
  unit->p1=unit->p2;  
					
  //  if (unit->p2.x>0.0)
    //    unit->returnvalx=(int)((unit->p2.x)*1024);
  //  if (unit->p2.y>0.0)
    //   unit->returnvaly=(int)((unit->p2.y)*1024);
      if (count>=wrap) count=0;

        workingbuffer[(count+start)%32768]=unit->p2.x;
#ifdef PCSIM
	//          printf("%c",unit->p2.x);
      //    if (count>32767) printf("IFSCRASH%d\n",count);

#endif
  /*    iter=randi()%row;
    i=randi()%column;
    unit->coeff[iter][i]=((float)randi()/(float)(RANDI_MAX));
    if (((float)randi()/(float)(RANDI_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
    unit->prob[iter]=((float)randi()/(float)(RANDI_MAX));
    unit->p1.x=0.5;
    unit->p1.y=0.5;*/
}
return count;
}

//////////////////////////////////////////////////////////

// ROSSLER

void rosslerinit(void* unity, uint16_t *workingbuffer){
  /*  unit->h = 0.1;
  unit->a = 0.3;
  unit->b = 0.2;
  unit->c = 5.8;*/
  struct Rossler* unit=unity;
  unit->buffer=workingbuffer;
  unit->h = (float)workingbuffer[0]/120536.0;
  unit->a = (float)workingbuffer[1]/122536.0;
  unit->b = (float)workingbuffer[2]/100536.0;
  unit->lx0 = 0.1;
  unit->ly0 = 0;
  unit->lz0 = 0;

}

u16 runrossler(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  float lx0,ly0,lz0,lx1,ly1,lz1;
  //  float h,a,b,c;
  u8 i=0;
  struct Rossler* unit=unity;
  u16 *workingbuffer=unit->buffer;
  /* which unit to vary according to workingbuffer */
  // leave as so!

  float h = unit->h;
  float a = unit->a;
  float b = unit->b;
  float c = unit->c;


  for (i=0; i<howmuch; i++) {
    count++;
  lx0 = unit->lx0;
  ly0 = unit->ly0;
  lz0 = unit->lz0;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  unit->lx0 = lx1;
  unit->ly0 = ly1;
  unit->lz0 = lz1;
#ifdef PCSIM
  //      printf("%c",lx1);
  //    if (count>32767) printf("ROSSCRASH%d\n",count);

#endif
      if (count>=wrap) count=0;

        workingbuffer[(count+start)%32768]=lx1;
  //  workingbuffer[i+1]=ly1;
  //  workingbuffer[i+2]=lz1;
  }

return count;
}

//////////////////////////////////////////////////////////

// 2nd rossler from: MCLDChaosUGens.cpp

void secondrosslerinit(void* unity, uint16_t *workingbuffer){
  struct secondRossler* unit=unity;
  unit->buffer=workingbuffer;
  unit->a = (float)workingbuffer[0]/65536.0;
  unit->b = (float)workingbuffer[1]/65536.0;
  unit->c = (float)workingbuffer[2]/65536.0;
  unit->h = (float)workingbuffer[3]/65536.0;
  unit->x0 = (float)workingbuffer[4]/65536.0;
  unit->y0 = (float)workingbuffer[5]/65536.0;
  unit->z0 = (float)workingbuffer[6]/65536.0;

  }

u16 runsecondrossler(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  u8 i=0;
  struct secondRossler* unit=unity;
      u16 *workingbuffer=unit->buffer;
    float a=unit->a;
    float b=unit->b;
    float c=unit->b;
    float h=unit->h;
    float x0=unit->x0;
    float y0=unit->y0;
    float z0=unit->z0;

      float xn = unit->xn;
    float yn = unit->yn;
    float zn = unit->zn;
  float xnm1 = unit->xnm1;
  float ynm1 = unit->ynm1;
  float znm1 = unit->znm1;
  float dx = xn - xnm1;
  float dy = yn - ynm1;
  float dz = zn - znm1;
	for (i=0; i<howmuch; ++i) {
    count++;
			xnm1 = xn;
			ynm1 = yn;
			znm1 = zn;

			float k1x, k2x, k3x, k4x,
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

			    workingbuffer[(count+start)%32768]=xnm1+dx;
#ifdef PCSIM
			    //			    printf("%c",(xnm1+dx));
			    //    if (count>32767) printf("ROSS2CRASH%d\n",count);

#endif
	}
	unit->xn = xn;
	unit->yn = yn;
	unit->zn = zn;
	unit->xnm1 = xnm1;
	unit->ynm1 = ynm1;
	unit->znm1 = znm1;
return count;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

void brusselinit(void* unity, uint16_t *workingbuffer){
  struct Brussel* unit=unity;   
  unit->buffer=workingbuffer;
  unit->x = 0.5f; 
  unit->y = 0.5f; 
  unit->delta = (float)workingbuffer[0]/65536.0;
  unit->mu = (float)workingbuffer[1]/65536.0;
  unit->gamma = (float)workingbuffer[2]/65536.0;
}

u16 runbrussel(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
    
  float dx, dy; 
  
  u8 i=0;
  struct Brussel* unit=unity;
  u16 *workingbuffer=unit->buffer;
  float muplusone = 1.0f+unit->mu; 
  float x= unit->x; 
  float y= unit->y;  
    for (i=0; i<howmuch; ++i) {
	    count++;	
      float temp = x*x*y; 
        
        dx = temp - (muplusone*x) + unit->gamma;
        dy =  (unit->mu*x)  - temp; 
        
        x += unit->delta*dx; 
        y += unit->delta*dy; 
	if (count>=wrap) count=0;

		workingbuffer[(count+start)%32768]=x*65536.0;
	}
#ifdef PCSIM
    //    printf("brussels: x %f y %f\n",x,y); 
    //        printf("%c",workingbuffer[(count+start)%32768]); 
    //    if (count>32767) printf("BRUSSCRASH%d\n",count);

#endif
    unit->x = x; 
    unit->y = y;
return count;
}

//////////////////////////////////////////////////////////

// spruceworm

void spruceinit(void *unity, uint16_t *workingbuffer){
  struct Spruce* unit=unity;
  unit->buffer=workingbuffer;
  unit->x = 0.9f; 
  unit->y = 0.1f; 
  unit->k1 = (float)workingbuffer[0]/65536.0;
  unit->k2 = (float)workingbuffer[1]/65536.0;
  unit->alpha = (float)workingbuffer[2]/65536.0;
  unit->beta = (float)workingbuffer[3]/65536.0;
  unit->mu = (float)workingbuffer[4]/65536.0;
  unit->rho = (float)workingbuffer[5]/65536.0;
  unit->delta = (float)workingbuffer[6]/65536.0;
}

u16 runspruce(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  float dx, dy; 
  u8 i=0;
  struct Spruce* unit=unity;
  u16 *workingbuffer=unit->buffer;
    float x= unit->x; 
    float y= unit->y;  
	for (i=0; i<howmuch; ++i) {
	    count++;	
        float temp = y*y; 
        float temp2 = unit->beta*x;
        
        dx = (unit->k1* x* (1.0-x)) - (unit->mu*y);
        dy = (unit->k2*y*(1.0- (y/(unit->alpha*x))))  - (unit->rho*(temp/(temp2*temp2 +  temp))); 
        x += unit->delta*dx; 
        y += unit->delta*dy; 

	if (count>=wrap) count=0;

		workingbuffer[(count+start)%32768]=x*65536.0;

	}
	
  unit->x = x; 
  unit->y = y;
#ifdef PCSIM
  //    	printf("%c",x*65536); 
  //    if (count>32767) printf("SPRUCECRASH%d\n",count);

#endif
return count;
}

//////////////////////////////////////////////////////////

// OREGONATOR

void oregoninit(void *unity, uint16_t *workingbuffer){
  struct Oregon* unit=unity;
  unit->buffer=workingbuffer;
  unit->x = 0.5f; 
    unit->y = 0.5f; 
    unit->z = 0.5f; 
    unit->delta = (float)workingbuffer[0]/65536.0;
    unit->epsilon = (float)workingbuffer[1]/65536.0;
    unit->mu = (float)workingbuffer[2]/65536.0;
    unit->q = (float)workingbuffer[3]/65536.0;

}

u16 runoregon(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){
  
  float dx, dy, dz; 
  u8 i=0;
  struct Oregon* unit=unity;
  u16 *workingbuffer=unit->buffer;
  float x= unit->x; 
  float y= unit->y; 
  float z= unit->z; 
	for (i=0; i<howmuch; ++i) {
	    count++;	
        dx = unit->epsilon*((unit->q*y) -(x*y) + (x*(1-x))); 
	dy = unit->mu* (-(unit->q*y) -(x*y) + z); 
        dz = x-y; 
        
        x += unit->delta*dx; 
        y += unit->delta*dy; 
        z += unit->delta*dz; 
        
	//	output1[i]= x; 
	//        output2[i]= y; 
	//        output3[i]= z; 
	if (count>=wrap) count=0;

		workingbuffer[(count+start)%32768]=x*65536.0;
	}
#ifdef PCSIM
	//	printf("Oregonator: x %f y %f z %f\n",x,y,z); 
	//	printf("%c",workingbuffer[(count+start)%32768]); 
	//    if (count>32767) printf("ORCRASH%d\n",count);

#endif
	unit->x = x; 
	unit->y = y;
	unit->z = z;
return count;
}

//////////////////////////////////////////////////////////

// FITZHUGH - writes into buffer 3xhowmuch, how to store local floats?

void fitzinit(void *unity, uint16_t *workingbuffer){
  struct Fitz* unit=unity;
  unit->buffer=workingbuffer;
  unit->u=0.0;
  unit->w=0.0;
  		unit->b0= 1.4;
  		unit->b1= 1.1;
  //    unit->b0=(float)workingbuffer[0]/32768.0;
  //    unit->b1=(float)workingbuffer[1]/32768.0;
}

u16 runfitz(uint8_t howmuch, void* unity, u16 count, u16 start, u16 wrap){

  /* SETTINGS */

  float urate= 0.7;
  float wrate= 1.7;
  float u,w;
  u8 x;
  struct Fitz* unit=unity;
  u16 *workingbuffer=unit->buffer;
  u=unit->u;
  w=unit->w;
  for (x=0;x<howmuch;x++){
    count++;
    //    if (count>=MAX_SAM) count=0;
    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(unit->b0+unit->b1*u-w);
	  
    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
        if ((u>1.0) || (u<-1.0)) u=fabs(fmodf(u-1.0,4.0)-2.0)-1.0;
    //    if ((u>1.0) || (u<-1.0)) u=fabs(u-2.0)-1.0;



	    int z=((float)(u)*3600);
    //    int zz=((float)(w)*1500);
    if (count>=wrap) count=0;

       workingbuffer[(count+start)%32768]=(u16)z;//workingbuffer[x+2]=zz;

#ifdef PCSIM
	//        printf("fitz: %c",u); 
       //     printf("%c",workingbuffer[(count+start)%32768]>>8); 
       // if (count>32767) printf("FITZCRASH%d\n",count);

#endif

  }
  unit->u=u;
  unit->w=w;
  return count;
  }


#ifdef PCSIM

void passingarraytest(uint8_t *buffer) {
  u8 *m_memory; u16 i;
  m_memory=buffer;
  for (i=0;i<65535;i++){
    printf("%d %d\n",i, m_memory[i]);
  }
}

#endif

signed char func_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u16* buffer, u8 stack_pos, u8 howmuch, u16 start, u16 wrap){
  if (stack_pos<STACK_SIZE)
    {

      if (howmuch==0) howmuch=1;
      u8 tmp=stack_pos<<2;
      stacker[tmp]=start;
      stacker[tmp+1]=start;
      stacker[tmp+2]=howmuch;
      stacker[tmp+3]=wrap;

      /*      stack[stack_pos].howmuch=howmuch;
      stack[stack_pos].start=start;
      stack[stack_pos].count=start;
      stack[stack_pos].wrap=wrap;*/

      switch(typerr){
      case CONVY:
	stack[stack_pos].unit=malloc(sizeof(struct CONV));
	convinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runconv;
	break;
      case FORMY:
	stack[stack_pos].unit=malloc(sizeof(struct FORM));
	forminit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runform;
	break;
      case SINEY:
	stack[stack_pos].unit=malloc(sizeof(struct siney));
	sineinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runsine;
	break;
      case INCY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runinc;
	break;
      case DECY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=rundec;
	break;
      case LEFTY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runleft;
	break;
      case RIGHTY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runright;
	break;
      case SWAPPY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runswap;
	break;
      case NEXTINCY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runnextinc;
	break;
      case NEXTDECY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runnextdec;
	break;
      case NEXTMULTY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runnextmult;
	break;
      case NEXTDIVY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runnextdiv;
	break;
      case COPYY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runcopy;
	break;
      case ZEROY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runzero;
	break;
      case FULLY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runfull;
	break;
      case RANDY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runrand;
	break;
      case KNOBY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runknob;
	break;
      case SWAPAUDIOY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runswapaudio;
	break;
      case ORAUDIOY:
	stack[stack_pos].unit=malloc(sizeof(struct generik));
	geninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runORaudio;
	break;
      case SIMPLESIRY:
	stack[stack_pos].unit=malloc(sizeof(struct simpleSIR));
	simplesirinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runsimplesir;
	break;
      case SEIRY:
	stack[stack_pos].unit=malloc(sizeof(struct SEIR));
	seirinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runseir;
	break;
      case SICRY:
	stack[stack_pos].unit=malloc(sizeof(struct SICR));
	sicrinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runsicr;
	break;
      case IFSY:
	stack[stack_pos].unit=malloc(sizeof(struct IFS));
	ifsinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runifs;
	break;
      case ROSSLERY:
	stack[stack_pos].unit=malloc(sizeof(struct Rossler));
	rosslerinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runrossler;
	break;
      case SECONDROSSLERY:
	stack[stack_pos].unit=malloc(sizeof(struct secondRossler));
	secondrosslerinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runsecondrossler;
	break;
      case BRUSSELY:
	stack[stack_pos].unit=malloc(sizeof(struct Brussel));
	brusselinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runbrussel;
	break;
      case SPRUCEY:
	stack[stack_pos].unit=malloc(sizeof(struct Spruce));
	spruceinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runspruce;
	break;
      case OREGONY:
	stack[stack_pos].unit=malloc(sizeof(struct Oregon));
	oregoninit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runoregon;
	break;
      case FITZY:
	stack[stack_pos].unit=malloc(sizeof(struct Fitz));
	fitzinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runfitz;
	break;
      case SWAPCHUNKY:
	stack[stack_pos].unit=malloc(sizeof(struct chunkey));
	chunkinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runswapchunk;
	break;
      case CHUNKY:
	stack[stack_pos].unit=malloc(sizeof(struct chunkey));
	chunkinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runchunk;
	break;
      case DEREFCHUNKY:
	stack[stack_pos].unit=malloc(sizeof(struct chunkey));
	chunkinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runderefchunk;
	break;
      case WALKERCHUNKY:
	stack[stack_pos].unit=malloc(sizeof(struct chunkey));
	chunkinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runwalkerchunk;
      }
      stack_pos++;
    }
  return stack_pos;
}


void func_runall(struct stackey stack[STACK_SIZE],u8 stack_pos){

  static u16 count; char i; signed char x;
      for (i=0;i<stack_pos;i++){
	u8 tmp=i<<2;
	//	stack[stack_pos].count=stack[i].functione(stack[i].howmuch,stack[i].unit,stack[stack_pos].count,stack[stack_pos].start,stack[stack_pos].wrap);

	stacker[tmp]=stack[i].functione(stacker[tmp+2],stack[i].unit,stacker[tmp],stacker[tmp+1],stacker[tmp+3]);
        }
}

signed char func_pop(struct stackey stack[STACK_SIZE], u8 stack_pos){
 	if (stack_pos>0)
	{
	  stack_pos--;
	    free(stack[stack_pos].unit);
	}
	return stack_pos;
	}

#ifdef PCSIM

void main(void)
{
  //  int cuu=atoi(argv[1]), pll=atoi(argv[2]);
  unsigned int x; u16 count=0;
  u8 howmuch,i;
  //   uint16_t xxx[MAX_SAM];
     u8 xxx[65536];
     srand(time(NULL)*rand());

  u16 AUDIO_BUFSZ=32768;
  u16 f0106erpos=0, F0106ERSTEP=1, F0106ERWRAP, F0106ERSTART, f0cons,tmp,wrapper;

  u8 stack_pos=0;
  struct stackey stackyy[STACK_SIZE];
   u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768

  for (x=0;x<65535;x++){
    xxx[x]=randi()%65536;
  }

  u16 *buf16 = (u16*) xxx;
  
  //  struct FORM *unity=malloc(sizeof(struct FORM));

  //  forminit(unity, xxx,0,3);

  //  printf("test%d\n",256<<7);
  	 for (x=0;x<16;x++){
	   u16 addr=rand()%32768;
	   u8 which=rand()%NUM_FUNCS;
	   //	   u8 which=rand()%9;
	   //	   printf("which: %d\n",which);
	   //	   stack_pos=func_pushn(stackyy,which,buf16,stack_pos,10,addr,addr+rand()%32768);//howmuch,start,wrap 
	   stack_pos=func_pushn(stackyy,which,buf16,stack_pos,1,0,addr);//howmuch,start,wrap //29-32

  	 	   }
  
	 u16 tmppp=4094,ooo=4096;u16 mirror,tmppushpull; u8 which;
	 ooo=8192;
	 //	 tmppp=(ooo>>13);
	 //	 printf("tmp %d\n",tmppp);
		 u8 stak=0;
	 //	 call function;

	 //	 calltest(tmppp+ooo);

		 //		 while(1){
		   // stak=1;
		 //		   tmppp+=64;
		 which=511>>8;
		 tmppp=1<<14;
		   printf("tmp %d\n",tmppp);
		   //		 }
		 //		 for (x=0;x<5000000;x++){
		 //		 		 while(1){
				   //			   if ((rand()%15)<10)			   stack_pos=func_pushn(stackyy,rand()%31,buf16,stack_pos,rand()%32760,0,rand()%32760);//29-32
	//			   else stack_pos=func_pop(stackyy,stack_pos);
		
		 //		   func_runall(stackyy,stack_pos); // simulations
		 //		   printf("%c",buf16[x%32768]>>8);
				   //				   x++;
		     //    which=buf16[x%32768]>>8;

		 //		 for (x=0;x<stak;x++){printf("xxxxx");}

		 //		    }
}

#endif
