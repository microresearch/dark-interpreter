// gcc -DLINUX -std=gnu99 CA.c -o CA -lm -DPCSIM

// runkrum, runhodge, runhodgenet, runlife, runcel, runcel1d, runfire, runwire, runSIR, runSIR16

#ifdef PCSIM
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "CA.h"
#include "simulation.h"
#define randi() rand()
extern u8 table[21];
//u8 table[21];
extern u16 *stackery;//[48]; // 16*3 MAX
extern int16_t *audio_buffer;
//u16 stackery[48];
#else
#include "CA.h"
#include "audio.h"
#include "simulation.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern u8 table[21];
extern u16 stackery[STACK_SIZE*4]; // 16*4 MAX
extern int16_t audio_buffer[32768] __attribute__ ((section (".data")));
#endif

extern u8 *datagenbuffer;

//////////////////////////////////////////

void runnoney(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;
  for (u8 xx=0;xx<vill->howmany;xx++){
    x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  if (x>vill->start+vill->wrap) x=vill->start;
  datagenbuffer[x]=0;
}
  vill->position=x;
}

//////////////////////////////////////////

// hodge from microbd simplified with circular buffer and init

void runhodge(villager_generic* vill){
  
  u8 step=vill->step;
  u16 x=vill->position;
  u16 wrap=vill->wrap;
  //  u16 x=vill->position;


  u8 sum, numill, numinf;
  uint16_t y;
  u8 k1=datagenbuffer[1];
  u8 k2=datagenbuffer[2];
  if (k1==0) k1=1;
  if (k2==0) k2=1;

  for (u8 xx=0;xx<vill->howmany;xx++){
    sum=0; numill=0; numinf=0;
    y=x-datagenbuffer[4]-1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=datagenbuffer[4]-2;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=datagenbuffer[4]-2;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y+=1;
    sum+=datagenbuffer[y];
    if (datagenbuffer[y]==(datagenbuffer[0]-1)) numill++; else if (datagenbuffer[y]>0) numinf++;

    y=x+wrap;
  if(datagenbuffer[x] == 0)
    datagenbuffer[y] = floorf(numinf / k1) + floorf(numill / k2);
  else if(datagenbuffer[x] < datagenbuffer[0] - 1)
    datagenbuffer[y] = floorf(sum / (numinf + 1)) + datagenbuffer[3];
  else
    datagenbuffer[y] = 0;

  if(datagenbuffer[y] > datagenbuffer[0] - 1)
    datagenbuffer[y] = datagenbuffer[0] - 1;

    x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  }
#ifdef PCSIM  
  //printf("%c",datagenbuffer[y]);
#endif
  vill->position=x;

}

//////////////////////////////////////////

// hodge from hodgenet is pretty much same... but few
// differences... so here they are expressed (also could be faster this way)

void runhodgenet(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;

  u8 sum, numill, numinf; u16 place;
  uint16_t y;
  u8 k1=datagenbuffer[1];
  u8 k2=datagenbuffer[2];
  if (k1==0) k1=1;
  if (k2==0) k2=1;

  for (u8 xx=0;xx<vill->howmany;xx++){
    sum=0; numill=0; numinf=0;
    place=x-datagenbuffer[4]-1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=datagenbuffer[4]-2;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=datagenbuffer[4]-2;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

  place+=1;
  if (datagenbuffer[place]==datagenbuffer[0]) numill++; if (datagenbuffer[place]>0) numinf++;  
  sum+=datagenbuffer[place];

    y=x+32768;
  if(datagenbuffer[x] == 0)
    datagenbuffer[y] = floorf(numinf / k1) + floorf(numill / k2);
  else if(datagenbuffer[x] < datagenbuffer[0])
    {    if ((numinf+1)!=0)    datagenbuffer[y] = floorf(sum / (numinf + 1)) + datagenbuffer[3];}
  else
    datagenbuffer[y] = 0;

  if(datagenbuffer[y] > datagenbuffer[0])
    datagenbuffer[y] = datagenbuffer[0];

    x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  }
#ifdef PCSIM  
  //  printf("%c",datagenbuffer[x]);
#endif
  vill->position=x;

}

//////////////////////////////////////////

//life - 2d CA - these all now use CA struct

void runlife(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;

  u8 sum;
  uint16_t y; u8 i;

  for (u8 xx=0;xx<vill->howmany;xx++){
    sum=0;

    y=x-datagenbuffer[4]-1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=datagenbuffer[4]-2;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=datagenbuffer[4]-2;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    /*    if (sum==3 || (sum+(datagenbuffer[x]&0x01)==3)) newdatagenbuffer[x]=255; /// 
	  else newdatagenbuffer[x]=0;*/
  y=x+32768;
  if ((datagenbuffer[x]&1)==1 && sum<2) datagenbuffer[y]=0;
  else if ((datagenbuffer[x]&1)==1 && sum>3) datagenbuffer[y]=0;
  else if ((datagenbuffer[x]&1)==0 && sum==3) datagenbuffer[y]=255;
  else datagenbuffer[y]=datagenbuffer[x];

#ifdef PCSIM  
  //    printf("%c",datagenbuffer[x]);
#endif

  //  //  printf("%c",datagenbuffer[x]);
    x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  }
  vill->position=x;
}

//////////////////////////////////////////

void runkrum(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;

  u16 y,place;
  u8 n=datagenbuffer[0];
  if (n==0) n=1;
  //  n=4;

  for (u8 xx=0;xx<vill->howmany;xx++){
    y=x+32768;

    // for each neighbour if neighbour=(src+1) mod n
    // copy old neighbour to new/y

  place=x-datagenbuffer[4];
  if (datagenbuffer[place]==(datagenbuffer[x]+1)%n) datagenbuffer[y]=datagenbuffer[place];

  place+=datagenbuffer[4]-1;
  if (datagenbuffer[place]==(datagenbuffer[x]+1)%n) datagenbuffer[y]=datagenbuffer[place];

  place+=2;
  if (datagenbuffer[place]==(datagenbuffer[x]+1)%n) datagenbuffer[y]=datagenbuffer[place];

  place+=datagenbuffer[4]-1;
  if (datagenbuffer[place]==(datagenbuffer[x]+1)%n) datagenbuffer[y]=datagenbuffer[place];

    x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  }
#ifdef PCSIM  
  //     printf("%c",datagenbuffer[x]);
#endif
  vill->position=x;

}

//////////////////////////////////////////

//one dimensional - working line by line through buffer

void runcel(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;

  u8 state;u16 y;

  for (u8 xx=0;xx<vill->howmany;xx++){
    state = 0;
      x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  if (datagenbuffer[(x +1+ (datagenbuffer[0]))&65535]>128)
      state |= 0x4;
  if (datagenbuffer[(x+datagenbuffer[0])&65535]>128)
      state |= 0x2;
  y=x-1;
  if (datagenbuffer[(y + datagenbuffer[0])&65535]>128)
      state |= 0x1;

    y=(x+(datagenbuffer[0]*2))%65536;// next row but one!

      if ((datagenbuffer[1] >> state) & 1){
	datagenbuffer[y] = 0;
      }
      else{
	datagenbuffer[y] = 255;
      } 
  }
#ifdef PCSIM  
      //      printf("%c",datagenbuffer[y]);
#endif
  vill->position=x;

}

///////////////

void inittable(u8 r, u8 k, int rule){
  u8 max; int z; u8 summ;

  //    free(table);
  max = (k-1)*((r*2)+1);
  //  //  printf("ttt %d\n",max);

  //  table= malloc(max+1);
  for (z=max;z>=0;z--){
    summ=0;
    while ((rule-powf(k,z))>=0) {
      summ++;
      rule=rule-powf(k,z);
    }
    if (summ>=1) {
      table[z]=summ;
    }
    else table[z]=0;
    //    //    printf("%d\n",table[z]);
  }
}

//////////////////////////////////////////

// 1d with rules

void runcel1d(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;

  u8 sum; int16_t z; u16 zz;
  u16 y;

  for (u8 xx=0;xx<vill->howmany;xx++){
    sum=0;
    
    // sum of datagenbuffer in radius - not looping!
    for (z=-3;z<3;z++){ // hardcode sans radius...
      zz=x+z;
      if (zz>=datagenbuffer[4]) zz=zz-datagenbuffer[4];
      //      if (zz<0) zz=datagenbuffer[4]+zz;
      sum+=(datagenbuffer[zz]>>4)%4; // crash here? and it did/// not now !
    }

    y=x+datagenbuffer[4];
    datagenbuffer[y]=table[sum]<<4;  

#ifdef PCSIM  
    //	printf("%c",datagenbuffer[y]);
#endif
  x=x+step;
  }
 vill->position= x;
}

//////////////////////////////////////////

//forest fire

void runfire(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;

  u8 sum;
  uint16_t y;
  u8 probB=datagenbuffer[0]/32;
  u8 probI=datagenbuffer[1]/10;

  for (u8 xx=0;xx<vill->howmany;xx++){
    sum=0;
  datagenbuffer[4]=datagenbuffer[2];

    y=x-datagenbuffer[4]-1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=datagenbuffer[4]-2;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=datagenbuffer[4]-2;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    y+=1;
    sum+=datagenbuffer[y]&1;

    //    sum=(datagenbuffer[x-1]&1)+(datagenbuffer[x+1]&1)+(datagenbuffer[x-datagenbuffer[4]]&1)+(datagenbuffer[x+datagenbuffer[4]]&1)+(datagenbuffer[x-datagenbuffer[4]-1]&1)+(datagenbuffer[x-datagenbuffer[4]+1]&1)+(datagenbuffer[x+datagenbuffer[4]-1]&1)+(datagenbuffer[x+datagenbuffer[4]+1]&1);

    y=x+32768;

    if (datagenbuffer[x]==0 || datagenbuffer[x]==254) datagenbuffer[y]=datagenbuffer[x]; //empty or burnt
    // now deal with vegetation(bit1 empty) and burning(&1)
    else if ((datagenbuffer[x]&1)==0 && randi()%255<=(sum*probI)) datagenbuffer[y]=datagenbuffer[x]|1;  //veg->burning
    else if ((datagenbuffer[x]&1)==1 && randi()%255<=probB) datagenbuffer[y]=254; // burning->burnt
    else datagenbuffer[y]=datagenbuffer[x];

#ifdef PCSIM  
    //      printf("%c",datagenbuffer[x]);
#endif

      x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  }
    vill->position=x;

}

//////////////////////////////////////////

/*wireworld:

4 states: blank, copper, head, tail

blank(0) stays blank(0)
head(1) becomes tail(255)
tail(255) becomes copper(128-254)
copper(?) stays copper unless just 1 or 2 neighbours are heads(1) then it becomes head(1)
*/

u8 headcount(u8 *datagenbuffer,u16 place){
  u8 counter=0;
  datagenbuffer[4]=datagenbuffer[0];
  place-=datagenbuffer[4]-1;
  if (datagenbuffer[place]==1) counter++;
  place+=1;
  if (datagenbuffer[place]==1) counter++;
  place+=1;
  if (datagenbuffer[place]==1) counter++;
  place+=datagenbuffer[4]-2;
  if (datagenbuffer[place]==1) counter++;
  place+=2;
  if (datagenbuffer[place]==1) counter++;
  place+=datagenbuffer[4]-1;
  if (datagenbuffer[place]==1) counter++;
  place+=1;
  if (datagenbuffer[place]==1) counter++;
  place+=1;
  if (datagenbuffer[place]==1) counter++;
  if (counter<3 && counter!=0) return 1;
  else return 0;
}

void runwire(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;
  uint16_t y; 

  for (u8 xx=0;xx<vill->howmany;xx++){
    y=x+32768;

    if (datagenbuffer[x]==0) datagenbuffer[y]=0; //blank
    else if (datagenbuffer[x]==1) datagenbuffer[y]=255;  // head to tail
    else if (datagenbuffer[x]==255) datagenbuffer[y]=129;  // tail to copper
    else if (datagenbuffer[x]>128 && headcount(datagenbuffer,x)==1) datagenbuffer[y]=1;
    else datagenbuffer[y]=datagenbuffer[x];

#ifdef PCSIM  
    //       printf("%c",datagenbuffer[x]);
#endif

      x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;

  }
  vill->position=x;
}

//////////////////////////////////////////

/*

SIR:

4 states 0=suscept/1=infected+days/255=D/-1=recovered?/dead

#define recovered 129
#define dead 255                                                                   
#define susceptible 0

Iprob = x # probability of transmission
Dprob = x # probability of death

- if dead or recovered then leave as are
- if infected (>1) add day until recovered(129) or dprob dead(255)
- if susceptible (0) then count surrounds and Iprob to be infected

[see also more complex models which include: population for each cell,
S.I.R pops (4 bits each as 16 bit CA), parameters for radius,movement
prob, birth death, virus morbidity, contact infection prob, vectored
infect prob, spontaneous infect prob, recovery prob, re-infection prob]

*/

void runSIR(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;
  uint16_t y,yy; 

  u8 probD=datagenbuffer[0]/32;
  u8 probI=datagenbuffer[1]/10;

  for (u8 xx=0;xx<vill->howmany;xx++){

  datagenbuffer[4]=datagenbuffer[2];

    y=x+32768;

    if (datagenbuffer[x]==129 || datagenbuffer[x]==255) datagenbuffer[y]=datagenbuffer[x]; //dead or recovered
    //
    else if (datagenbuffer[x]==0){
      // do count of surroundings
      /*      if ( (datagenbuffer[x-datagenbuffer[4]]>0 && datagenbuffer[x-datagenbuffer[4]]<129) ||
	   (datagenbuffer[x+datagenbuffer[4]]>0 && datagenbuffer[x+datagenbuffer[4]]<129) ||
	   (datagenbuffer[x-1]>0 && datagenbuffer[x-1]<129) ||
	   (datagenbuffer[x+1]>0 && datagenbuffer[x+1]<129))
	{
	if (randi()%100 <= probI) datagenbuffer[y] = 1;       
	}*/
      yy=x-datagenbuffer[4];
      if (datagenbuffer[yy]>0 && datagenbuffer[yy]<129) {	if (randi()%100 <= probI) datagenbuffer[y] = 1;       }
	else {
	  yy+=datagenbuffer[4]-1;
	  if (datagenbuffer[yy]>0 && datagenbuffer[yy]<129){ 	if (randi()%100 <= probI) datagenbuffer[y] = 1;       }
	    else {
	      yy+=2;
	      if (datagenbuffer[yy]>0 && datagenbuffer[yy]<129){ 	if (randi()%100 <= probI) datagenbuffer[y] = 1;}       
		else {
		  yy+=1;
		  if (datagenbuffer[yy]>0 && datagenbuffer[yy]<129){ 	if (randi()%100 <= probI) datagenbuffer[y] = 1;       }
		}
	    }
	}      
      //calc probI
    }
    else if (datagenbuffer[x]>1 && datagenbuffer[x]<129){
      if (randi()%100<probD) datagenbuffer[y]=255; //dead
      else datagenbuffer[y]=datagenbuffer[x]+1;
      // if infected (>1 and <129) add day until recovered(129) or dprob dead(255)
    }

    else datagenbuffer[y]=datagenbuffer[x]; // blank datagenbuffer

#ifdef PCSIM  
    //       printf("%c",datagenbuffer[x]);
#endif

      x+=step;
  if (x>vill->start+vill->wrap) x=vill->start;
  }
  vill->position=x;
  }

//////////////////////////////////////////

/* 16 bit SIR with 4x 4-bit fields:

-total pop
-S pop
-I pop
-R pop

infection radius???, max population=15

 */

u16 biotadir[8]={65279,65280,1,257,256,254,65534,65278};

void runSIR16(villager_generic* vill){
  u8 step=vill->step;
  u16 x=vill->position;
  u8 ii; u16 y,dest,yy;
  u8 totalhost,totaldest,which,sirhost,sirdest,futuretotal,futurerecovered,futuresuscept,futureinfected,infected,suscept;

  u8 probM=datagenbuffer[0];
  u8 probR=datagenbuffer[1];
  u8 probC=datagenbuffer[2];
  u8 probV=datagenbuffer[3];


  for (u8 xx=0;xx<vill->howmany;xx++){
    // select random cell, for each of ind, select neighbour and move
    // it there based on probability and neighbour not full
#ifdef PCSIM
    y=randi()%65536;
#else
    y=randi()<<4;
#endif
    // choose random neighbour

    dest=y+biotadir[randi()%8];
    // 16 bits: top/lower top/lower total/S/I/R
    totaldest=datagenbuffer[dest]>>4;
    totalhost=datagenbuffer[y]>>4;
    if (totaldest<15 &&totalhost>0 && randi()%255<=probM){
    // from host choose S,I or R to shift over if motionP and not full
    // then update both host y and dest...
      which=randi()%3;
      switch(which)
	{
	case 0:
	  totaldest+=1;
	  totalhost-=1;
	  sirdest=datagenbuffer[dest]&15;
	  sirhost=datagenbuffer[y]&15;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  datagenbuffer[dest]=(totaldest<<4)+sirdest;
	  datagenbuffer[y]=(totalhost<<4)+sirhost;
	  break;
	case 1:
	  totaldest+=1;
	  totalhost-=1;
	  yy=dest+1;
	  sirdest=datagenbuffer[yy]>>4;
	  yy=y+1;
	  sirhost=datagenbuffer[yy]>>4;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  // total
	  datagenbuffer[dest]=(datagenbuffer[dest]&15)+(totaldest<<4);
	  datagenbuffer[y]=(datagenbuffer[y]&15)+(totalhost<<4);
	  yy=dest+1;
	  datagenbuffer[yy]=(datagenbuffer[yy]^240)+(sirdest<<4);
	  yy=y+1;
	  datagenbuffer[yy]=(datagenbuffer[yy]^240)+(sirhost<<4);
	  break;
	case 2:
	  totaldest+=1;
	  totalhost-=1;
	  yy=dest+1;
	  sirdest=datagenbuffer[yy]&15;
	  yy=y+1;
	  sirhost=datagenbuffer[yy]&15;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  datagenbuffer[dest]=(datagenbuffer[dest]&15)+(totaldest<<4);
	  datagenbuffer[y]=(datagenbuffer[y]&15)+(totalhost<<4);
	  yy=dest+1;
	  datagenbuffer[yy]=(datagenbuffer[yy]^15)+sirdest;
	  yy=y+1;
	  datagenbuffer[yy]=(datagenbuffer[yy]^15)+sirhost;
	  break;
	}
    }


    y=x+32768;

    // leave recovered as they are 
    // 16 bits: top/lower top/lower as-> total/S/I/R
    futuretotal=datagenbuffer[x]>>4;
    yy=x+1;
    futurerecovered=datagenbuffer[yy]&15;
    infected=futureinfected=datagenbuffer[yy]>>4;

    for (ii=0;ii<infected;ii++){
    // for each of infected:
    // deduct virus morbidity
    // recoveries
      if ((randi()%255)<=probR) {futureinfected--; futuretotal--;} // dead
      if ((randi()%255)<=probV) {futurerecovered++; futureinfected--;} // recovered
    }
    suscept=futuresuscept=datagenbuffer[x]&15;
    for (ii=0;ii<suscept;ii++){
    // for each of suscept:
    // compute contact infections based on how many infected in cell
      if (((randi()%16)*infected)>probC) {futureinfected++; futuresuscept--;} // dead
    }

    // put all back together???
    datagenbuffer[y]=(futuretotal<<4)+futuresuscept;
    yy=y+1;
    datagenbuffer[yy]=(futureinfected<<4)+futurerecovered;
    x+=step*2;
  if (x>vill->start+vill->wrap) x=vill->start;

#ifdef PCSIM  
    //        printf("%c",datagenbuffer[x]);
#endif
  }
    vill->position=x;

}

//////////////////////////////////////////

//////////////////////////////////////////

#ifdef PCSIM
/*
int main(void)
{
  u16 x,xx;
  u16 buffer[32768];
  uint16_t count=0;
  srandom(time(0));

  u8 stack_posy=0;
  struct stackeyyy stack[STACK_SIZE];
  //  void *malloced[NUM_CA];


  for (x=0;x<32768;x++){
    buffer[x]=randi()%65536;
  }

  inittable(3,4,randi()%65536); //radius,states(k),rule - init with cell starter

    for (x=0;x<1;x++){
      xx=rand()%65536;
      stack_posy=ca_pushn(stack,3,buffer,stack_posy,100,2,65535); // last as howmany, start.,wrap
    }

    //    //    printf("stackposy: %d\n", stack_posy);


               while(1){
		 ca_runall(stack,stack_posy);     

		 //		 		 if ((rand()%20)<10){stack_posy=ca_pushn(stack,rand()%9,buffer,stack_posy,rand()%65536,rand()%65536,rand()%65536);//29-32
				   //printf("pusn %d\n",stack_pos);
		 //		 }
		 //		 else stack_posy=ca_pop(stack,stack_posy);
		

		 //		   		   for(x=0;x<48;x++){
				     //		     stackery[x]=rand()%65536;
		 //		   }


		 //		 printf("%c",buffer[x]>>8);
		 //		 x++;
		 //		 if (x>=32768) x=0;
    	 }

}
*/
#endif

