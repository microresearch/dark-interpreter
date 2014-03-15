// gcc simulation.c -osim -lm
// was datagentest.c previously

/* All simulation data generators: IFS, rossler, secondrossler, fitz,
   oregon, spruce, brussel, simpleSIR, seir, conv, sine */

//+ inc,dec,left,right and so on


/* TODO:

/////resolved/////

[- fix misunderstanding (as we want 64k with buffer so buffer is 32768
  samples long with u16) DONE]
[- somehow declare offset for settings, or we store this somewhere]
[- should howmuch be int or is u8 ok? probably leave as u8]
[- return count+i or i? - count+i in all cases]

*/


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
#else
#include "simulation.h"
#include <malloc.h>
#include <math.h>
#include <audio.h>
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));;
#endif

//////////////////////////////////////////////////////////

// convolve

void convinit(void* unity, uint16_t *workingbuffer){
  struct CONV* unit=unity;
  unit->del=0;
  unit->c0=(float)workingbuffer[0]/16384.0;
  unit->c1=(float)workingbuffer[1]/16384.0;
  unit->c2=(float)workingbuffer[2]/16384.0;
}


uint16_t runconv(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0; u16 y,tmp;
  struct CONV *unit=unity;

  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    y=count+16384;
    y=y%32768;
    if (count==0) tmp=32767;
    else tmp=count-1;
			
    workingbuffer[y]=((float)workingbuffer[tmp]*unit->c0)+((float)workingbuffer[count]*unit->c1)+((float)workingbuffer[(count+1)%32768]*unit->c2);

#ifdef PCSIM
    //    printf("%d %d %d %d %d\n",tmp, count,(count+1)%32768, y, workingbuffer[count]);
    //    printf("%f %f %f\n",unit->c0,unit->c1,unit->c2);
                printf("%c",workingbuffer[count]%255);

#endif
  }
    unit->del=0;
  }
  return count;
}


//////////////////////////////////////////////////////////

// sine

void sineinit(void* unity, uint16_t *workingbuffer){
  struct siney* unit=unity;
  unit->del=unit->cc=0;
  float pi= 3.141592;
  float w;    // ψ
  float yi;
  float phase;
  int sign_samp,i;
  w= 2*pi;
  w= w/256;
    for (i = 0; i <= 256; i++)
    {
      yi= 2047*sinf(phase); // was 2047
      phase=phase+w;
      sign_samp=2047+yi;     // dc offset translated for a 12 bit DAC
      unit->sin_data[i]=sign_samp; // write value into array
    }
}

uint16_t runsine(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct siney *unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=unit->sin_data[unit->cc%256];
#ifdef PCSIM
    printf("%c",workingbuffer[count]);
#endif
    unit->cc++;
  }
    unit->del=0;
  }
  return count;
}


//////////////////////////////////////////////////////////

// generic arithmetik datagens

void geninit(void* unity, uint16_t *workingbuffer){
  struct generik* unit=unity;
  unit->del=0;
  unit->cop=workingbuffer[0]; 
}

uint16_t runinc(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=unit->cop++;
#ifdef PCSIM
    //    printf("%c",workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t rundec(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=unit->cop--;
#ifdef PCSIM
    printf("%c",workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runleft(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=workingbuffer[count]<<=1;
#ifdef PCSIM
    //    printf("%d %d\n",count,workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
  unit->del=0;
  }
  return count;
}

uint16_t runright(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=workingbuffer[count]>>=1;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runswap(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0; u16 temp,yy;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=32766) count=0;
    //    temp=workingbuffer[count];
    yy=count+1;
    //    workingbuffer[count]=workingbuffer[yy];
    //    workingbuffer[yy]=temp;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runnextinc(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0; u16 yy;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++; 
    if (count>=32766) count=0;
    workingbuffer[count]=workingbuffer[count+1]+1;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runnextdec(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=32766) count=0;
    workingbuffer[count]=workingbuffer[count+1]-1;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runnextmult(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=32766) count=0;
    workingbuffer[count]*=workingbuffer[count+1];
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runnextdiv(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count>=32766) count=0;
    if ((workingbuffer[count+1])>0)   workingbuffer[count]/=workingbuffer[count+1];
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}


uint16_t runcopy(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++; 
    if (count>=32766) count=0;
    workingbuffer[count+1]=workingbuffer[count];
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runzero(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=0;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runfull(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffer[count]=65535;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runrand(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  u8 *workingbuffeur=(u8 *)workingbuffer;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch*2; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    workingbuffeur[count]=randi()%255;
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

uint16_t runknob(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0;
  u8 *workingbuffeur=(u8 *)workingbuffer;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch*2; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    //    workingbuffeur[count]=randi()%255; **TODO: which knob????
#ifdef PCSIM
    //    printf("%d\n",workingbuffer[count]);
    printf("%c", workingbuffer[count]);
#endif
  }
    unit->del=0;
  }
  return count;
}

//////////////////////////////////////////////////////////
// swap datagen 16 bits to and from audio buffer

uint16_t runswapaudio(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0; u16 temp;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
#ifndef PCSIM
    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[count%AUDIO_BUFSZ];
    audio_buffer[count%AUDIO_BUFSZ]=(int16_t)workingbuffer[count];
    workingbuffer[count]=temp;
#endif
  }
    unit->del=0;
  }
  return count;
}

//////////////////////////////////////////////////////////
// OR/XOR/AND/other ops datagen 16 bits to and from audio buffer

uint16_t runORaudio(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  u8 i=0; u16 temp;
  struct generik* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
#ifndef PCSIM
    // convert signed to unsigned how? 

    temp=(uint16_t)audio_buffer[count%AUDIO_BUFSZ];
    switch(unit->cop%5){
    case 0:
    audio_buffer[count%AUDIO_BUFSZ]|=(int16_t)workingbuffer[count];
    workingbuffer[count]|=temp;
    break;
    case 1:
    audio_buffer[count%AUDIO_BUFSZ]^=(int16_t)workingbuffer[count];
    workingbuffer[count]^=temp;
    break;
    case 2:
    audio_buffer[count%AUDIO_BUFSZ]&=(int16_t)workingbuffer[count];
    workingbuffer[count]&=temp;
    break;
    case 3:
    audio_buffer[count%AUDIO_BUFSZ]-=(int16_t)workingbuffer[count];
    workingbuffer[count]+=temp;
    break;
    case 4:
    audio_buffer[count%AUDIO_BUFSZ]+=(int16_t)workingbuffer[count];
    workingbuffer[count]-=temp;
    break;
    }

#endif
  }
    unit->del=0;
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

uint16_t runsimplesir(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){

  u8 i=0;
  struct simpleSIR* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    Runge_Kutta(unit);//  unit->t+=step;
    workingbuffer[count]=unit->I;
#ifdef PCSIM
    printf("%c",unit->I);
#endif    
  }
  unit->del=0;
  }
  return count;

}

//////////////////////////////////////////////////////////

// SEIR. SIR

void seirinit(void* unity, uint16_t *workingbuffer){
  unsigned char i;
  struct SEIR* unit=unity;
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
  int i;
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


uint16_t runseir(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch,void* unity){

  u8 i=0;
  struct SEIR* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    seir_Runge_Kutta(unit);//  unit->t+=step;
    workingbuffer[count]=unit->S;
#ifdef PCSIM
        printf("%c",unit->S);
#endif
  }
  unit->del=0;
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

uint16_t runsicr(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){

  u8 i=0;
  struct SICR* unit=unity;
  if (++unit->del==delay){
  for (i=0; i<howmuch; i++) {
    count++;
    if (count==MAX_SAM) count=0;
    sicr_Runge_Kutta(unit);//  unit->t+=step;
    workingbuffer[count]=unit->S;
#ifdef PCSIM
    printf("%c",unit->S);
#endif
  }
  unit->del=0;
  }
  return count;
}




//////////////////////////////////////////////////////////

// IFS

void ifsinit(void* unity, uint16_t *workingbuffer){
  u8 i,iter;
  u8 column = 6, row = 4;
  struct IFS* unit=unity;
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

uint16_t runifs(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){

  float randiom_num;
  u8 iter,i,it,x;
  u8 column = 6, row = 4;
  struct IFS* unit=unity;
  /*  ifs->prob[0]=0.0;
  ifs->prob[1]=0.85; 
  ifs->prob[2]=0.92; 
  ifs->prob[3]=0.99; 
  ifs->prob[4]=1.0; 
  */
  if (++unit->del==delay){

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
  if (count==MAX_SAM) count=0;
  workingbuffer[count]=unit->p2.x;
#ifdef PCSIM
    printf("%c",unit->p2.x);
#endif
  /*    iter=randi()%row;
    i=randi()%column;
    unit->coeff[iter][i]=((float)randi()/(float)(RANDI_MAX));
    if (((float)randi()/(float)(RANDI_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
    unit->prob[iter]=((float)randi()/(float)(RANDI_MAX));
    unit->p1.x=0.5;
    unit->p1.y=0.5;*/
}
  unit->del=0;
  }
  return count;
}

//////////////////////////////////////////////////////////

// ROSSLER

void rosslerinit(void* unity, uint16_t *workingbuffer) {
  /*  unit->h = 0.1;
  unit->a = 0.3;
  unit->b = 0.2;
  unit->c = 5.8;*/
  struct Rossler* unit=unity;
  unit->h = (float)workingbuffer[0]/120536.0;
  unit->a = (float)workingbuffer[1]/122536.0;
  unit->b = (float)workingbuffer[2]/100536.0;
  unit->lx0 = 0.1;
  unit->ly0 = 0;
  unit->lz0 = 0;
}

uint16_t runrossler(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  float lx0,ly0,lz0,lx1,ly1,lz1;
  //  float h,a,b,c;
  u8 i=0;
  struct Rossler* unit=unity;
  /* which unit to vary according to workingbuffer */
  // leave as so!

  float h = unit->h;
  float a = unit->a;
  float b = unit->b;
  float c = unit->c;

  if (++unit->del==delay){

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
    printf("%c",lx1);
#endif
  if (count==MAX_SAM) count=0;
  workingbuffer[count]=lx1;
  //  workingbuffer[i+1]=ly1;
  //  workingbuffer[i+2]=lz1;
  }
  unit->del=0;
  }

  return count;
}

//////////////////////////////////////////////////////////

// 2nd rossler from: MCLDChaosUGens.cpp

void secondrosslerinit(void* unity, uint16_t *workingbuffer){
  struct secondRossler* unit=unity;
  unit->a = (float)workingbuffer[0]/65536.0;
  unit->b = (float)workingbuffer[1]/65536.0;
  unit->c = (float)workingbuffer[2]/65536.0;
  unit->h = (float)workingbuffer[3]/65536.0;
  unit->x0 = (float)workingbuffer[4]/65536.0;
  unit->y0 = (float)workingbuffer[5]/65536.0;
  unit->z0 = (float)workingbuffer[6]/65536.0;

  }

uint16_t runsecondrossler(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){

  u8 i=0;
  struct secondRossler* unit=unity;
  if (++unit->del==delay){
    
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
	    if (count==MAX_SAM) count=0;
	    workingbuffer[count]=xnm1+dx;
#ifdef PCSIM
	    printf("%c",(xnm1+dx));
#endif
	}
	unit->xn = xn;
	unit->yn = yn;
	unit->zn = zn;
	unit->xnm1 = xnm1;
	unit->ynm1 = ynm1;
	unit->znm1 = znm1;
	unit->del=0;
  }
  return count;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

void brusselinit(void* unity, uint16_t *workingbuffer) {
  struct Brussel* unit=unity;   

  unit->x = 0.5f; 
  unit->y = 0.5f; 
  unit->delta = (float)workingbuffer[0]/65536.0;
  unit->mu = (float)workingbuffer[1]/65536.0;
  unit->gamma = (float)workingbuffer[2]/65536.0;
  unit->del=0;
}

uint16_t runbrussel(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
    
  float dx, dy; 
  
  u8 i=0;
  struct Brussel* unit=unity;
  if (++unit->del==delay){
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
	if (count==MAX_SAM) count=0;
	workingbuffer[count]=x*65536.0;
	}
#ifdef PCSIM
    //    printf("brussels: x %f y %f\n",x,y); 
    printf("%c",workingbuffer[count]); 
#endif
    unit->del=0;
    unit->x = x; 
    unit->y = y;
  }
  return count;
}

//////////////////////////////////////////////////////////

// spruceworm

void spruceinit(void *unity, uint16_t *workingbuffer) {
    struct Spruce* unit=unity;
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

uint16_t runspruce(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){

  float dx, dy; 
  u8 i=0;
  struct Spruce* unit=unity;
  if (++unit->del==delay){
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

	if (count==MAX_SAM) count=0;
	workingbuffer[count]=x*65536.0;

	}
	
  unit->del=0;
  unit->x = x; 
  unit->y = y;
#ifdef PCSIM
  	printf("%c",x*65536); 
#endif
  }
  return count;
}

//////////////////////////////////////////////////////////

// OREGONATOR

void oregoninit(void *unity, uint16_t *workingbuffer) {
  struct Oregon* unit=unity;
    unit->x = 0.5f; 
    unit->y = 0.5f; 
    unit->z = 0.5f; 
    unit->delta = (float)workingbuffer[0]/65536.0;
    unit->epsilon = (float)workingbuffer[1]/65536.0;
    unit->mu = (float)workingbuffer[2]/65536.0;
    unit->q = (float)workingbuffer[3]/65536.0;

}

uint16_t runoregon(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){
  
  float dx, dy, dz; 
  u8 i=0;
  struct Oregon* unit=unity;
  if (++unit->del==delay){

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
	if (count==MAX_SAM) count=0;
	workingbuffer[count]=x*65536.0;
	}
#ifdef PCSIM
	printf("Oregonator: x %f y %f z %f\n",x,y,z); 
#endif
	unit->x = x; 
	unit->y = y;
	unit->z = z;
	unit->del=0;
}
return count;
}

//////////////////////////////////////////////////////////

// FITZHUGH - writes into buffer 3xhowmuch, how to store local floats?

void fitzinit(void *unity, uint16_t *workingbuffer) {
  struct Fitz* unit=unity;
  unit->u=0.0;
  unit->w=0.0;
  		unit->b0= 1.4;
  		unit->b1= 1.1;
  //    unit->b0=(float)workingbuffer[0]/32768.0;
  //    unit->b1=(float)workingbuffer[1]/32768.0;
}

uint16_t runfitz(uint16_t count, u8 delay, uint16_t *workingbuffer, uint8_t howmuch, void* unity){

  /* SETTINGS */

  float urate= 0.7;
  float wrate= 1.7;
  float u,w;
  u8 x;
  struct Fitz* unit=unity;
  if (++unit->del==delay){

  u=unit->u;
  w=unit->w;

  for (x=0;x<howmuch;x++){
    count++;
    if (count>=MAX_SAM) count=0;
    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(unit->b0+unit->b1*u-w);
	  
    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
        if ((u>1.0) || (u<-1.0)) u=fabs(fmodf(u-1.0,4.0)-2.0)-1.0;
    //    if ((u>1.0) || (u<-1.0)) u=fabs(u-2.0)-1.0;



	    int z=((float)(u)*3600);
    //    int zz=((float)(w)*1500);
	    workingbuffer[count]=(u16)z;//workingbuffer[x+2]=zz;

#ifdef PCSIM
	//        printf("fitz: %c",u); 
       printf("%c",workingbuffer[count]>>8); 
#endif


  }
  unit->u=u;
  unit->w=w;
  unit->del=0;
  }
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


signed char func_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u16* buffer, signed char stack_pos, u8 delay, u8 howmuch){
  if (stack_pos<(STACK_SIZE-1))
    {
      stack[stack_pos].howmuch=howmuch;
      stack[stack_pos].delay=delay;
      if (stack[stack_pos].howmuch==0) stack[stack_pos].howmuch=1;
      if (stack[stack_pos].delay==0) stack[stack_pos].delay=1;

      switch(typerr){
      case CONVY:
	stack[stack_pos].unit=malloc(sizeof(struct CONV));
	convinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runconv;
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
	stack[stack_pos].functione=runconv;
	break;
      case SPRUCEY:
	stack[stack_pos].unit=malloc(sizeof(struct Spruce));
	spruceinit(stack[stack_pos].unit,buffer);
	stack[stack_pos].functione=runconv;
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
      }
      stack_pos++;
    }
  return stack_pos;
}


void func_runall(struct stackey stack[STACK_SIZE], u16* buffer, signed char stack_pos){
  static u16 count; char i; signed char x;
      for (i=0;i<stack_pos;i++){
      count=stack[i].functione(count,stack[i].delay,buffer,stack[i].howmuch,stack[i].unit);// set delay and howmuch in struct!
        }
}

signed char func_pop(struct stackey stack[STACK_SIZE], signed char stack_pos){
 	if (stack_pos>=0)
	{
	  free(stack[stack_pos].unit);
	  stack_pos--;
	}
	return stack_pos;
}

#ifdef PCSIM

void main(void)
{
  //  int cuu=atoi(argv[1]), pll=atoi(argv[2]);
  u16 x;
  u8 howmuch,i;
  //   uint16_t xxx[MAX_SAM];
     u8 xxx[MAX_SAM*2];
  srand(time(NULL));

  char stack_pos=-1;

  for (x=0;x<MAX_SAM;x++){
    xxx[x]=randi()%65536;
  }
  u16 *buf16 = (u16*) xxx;

  struct stackey stack[STACK_SIZE];

struct dgenwalker{
  // xxxx(samp/hard/clocks)->step,position,direction(into array),speed,start,end 
  u8 step,dir,speed,del;
  u16 pos,start,wrap;
};

  
struct dgenwalker *lmer=malloc(sizeof(struct dgenwalker));

	lmer->step=10; lmer->speed=10; lmer->dir=0;lmer->pos=0;lmer->start=10;lmer->wrap=32000; lmer->del=0;

    for (x=0;x<STACK_SIZE;x++){
      stack_pos=func_pushn(stack,randi()%NUM_FUNCS,buf16,stack_pos,1,1);

      //      printf("%d\n", stack_pos);
      }
  
    u16 tmp;
    u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768

       while(1){
	 func_runall(stack,buf16,stack_pos);
	    if (++lmer->del==lmer->speed){
	    //lmer - set lmpwm
	    tmp=lmer->step*direction[lmer->dir];
	    if ((lmer->pos+tmp)>=lmer->wrap) lmer->pos=(lmer->pos+tmp)%(lmer->wrap+1);
	    else lmer->pos+=tmp;
	    x=(lmer->start+lmer->pos)%32768;
	    //	    tmp=(lmer->start+lmer->pos+1)%32768;
	    //	    	    setlmpwm(buf16[x],buf16[tmp]); 
	    //	    printf("where: %d value: %d\n",x,buf16[x]); 
	    lmer->del=0;
	    }


       }
}
#endif
