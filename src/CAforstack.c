// gcc -DLINUX -std=gnu99 CA.c -o CA -lm -DPCSIM

// runhodge, runhodgenet, runlife, runcel, runcel1d, runfire, runwire, runSIR, runSIR16

#ifdef PCSIM
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "CA.h"
#define randi() rand()
#else
#include "CA.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
#endif

//////////////////////////////////////////

#define STACK_SIZE 16

#define HODGEY 0
#define HODGENETY 1
#define LIFEY 2
#define CELY 3
#define CEL1DY 4
#define FIREY 5
#define WIREY 6
#define SIRY 7
#define SIR16Y 8

#define NUM_FUNCS 9


char stack_posy;

//////////////////////////////////////////

// hodge from microbd simplified with circular buffer and init

void hodgeinit(void* unity, u8* cells){
  struct hodge* unit=unity;
  unit->q=cells[0];unit->k1=cells[1];unit->k2=cells[2];unit->g=cells[3];
  unit->celllen=cells[4];
  unit->del=0;
  if (unit->k1==0) unit->k1=1;
  if (unit->k2==0) unit->k2=1;
}

uint16_t runhodge(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void*  unity){


  u8 sum=0, numill=0, numinf=0;
  uint16_t y; u8 i=0;
  struct hodge *unit=unity;

  if (++unit->del==delay){
  for (i=0;i<howmuch;i++){
  sum=cells[x]+cells[x-1]+cells[x+1]+cells[x-unit->celllen]+cells[x+unit->celllen]+cells[x-unit->celllen-1]+cells[x-unit->celllen+1]+cells[x+unit->celllen-1]+cells[x+unit->celllen+1];

  if (cells[x-1]==(unit->q-1)) numill++; else if (cells[x-1]>0) numinf++;
  if (cells[x+1]==(unit->q-1)) numill++; else if (cells[x+1]>0) numinf++;
  if (cells[x-unit->celllen]==(unit->q-1)) numill++; else if (cells[x-unit->celllen]>0) numinf++;
  if (cells[x+unit->celllen]==(unit->q-1)) numill++; else if (cells[x+unit->celllen]>0) numinf++;
  if (cells[x-unit->celllen-1]==unit->q) numill++; else if (cells[x-unit->celllen-1]>0) numinf++;
  if (cells[x-unit->celllen+1]==unit->q) numill++; else if (cells[x-unit->celllen+1]>0) numinf++;
  if (cells[x+unit->celllen-1]==unit->q) numill++; else if (cells[x+unit->celllen-1]>0) numinf++;
  if (cells[x+unit->celllen+1]==unit->q) numill++; else if (cells[x+unit->celllen+1]>0) numinf++;

    y=x+32768;
  if(cells[x] == 0)
    cells[y] = floor(numinf / unit->k1) + floor(numill / unit->k2);
  else if(cells[x] < unit->q - 1)
    cells[y] = floor(sum / (numinf + 1)) + unit->g;
  else
    cells[y] = 0;

  if(cells[y] > unit->q - 1)
    cells[y] = unit->q - 1;

  x++;
  //  printf("%c",cells[x]);
  }
  unit->del=0;
  }
  return x;
}

//////////////////////////////////////////

// hodge from hodgenet is pretty much same... but few
// differences... so here they are expressed (also could be faster this way)

uint16_t runhodgenet(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){

  struct hodge* unit=unity;
  u8 sum=0, numill=0, numinf=0; u16 place;
  uint16_t y;
  u8 i=0;
  if (++unit->del==delay){
  for (i=0;i<howmuch;i++){
  place=x-unit->celllen-1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=unit->celllen-1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=2;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=unit->celllen-1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
  place+=1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];
    y=x+32768;
  if(cells[x] == 0)
    cells[y] = floor(numinf / unit->k1) + floor(numill / unit->k2);
  else if(cells[x] < unit->q)
    cells[y] = floor(sum / (numinf + 1)) + unit->g;
  else
    cells[y] = 0;

  if(cells[y] > unit->q)
    cells[y] = unit->q;

  x++;
  //  printf("%c",cells[y]);
  }
  unit->del=0;
  }
  return x;
}

//////////////////////////////////////////

//life - 2d CA - these all now use CA struct

void cainit(void* unity, u8* cells){
  struct CA* unit=unity;
  unit->celllen=cells[0];
  unit->del=0;
  unit->rule=cells[1];
}

uint16_t runlife(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){

  u8 sum;
  uint16_t y; u8 i=0;
  struct CA* unit=unity;
  if (++unit->del==delay){
  for (i=0;i<howmuch;i++){
  sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    /*    if (sum==3 || (sum+(cells[x]&0x01)==3)) newcells[x]=255; /// 
	  else newcells[x]=0;*/
  y=x+32768;
  if ((cells[x]&1)==1 && sum<2) cells[y]=0;
  else if ((cells[x]&1)==1 && sum>3) cells[y]=0;
  else if ((cells[x]&1)==0 && sum==3) cells[y]=255;
  else cells[y]=cells[x];

  //  printf("%c",cells[x]);
  x++;
  }
  unit->del=0;
  }
  return x;
}

//////////////////////////////////////////

//one dimensional - working line by line through buffer

uint16_t runcel(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){

  u8 state,i=0;
  struct CA* unit=unity;
  if (++unit->del==delay){
  for (i=1;i<howmuch;i++){
      state = 0;

      if (cells[x+i+1]>128)
	state |= 0x4;
      if (cells[x+i]>128)
	state |= 0x2;
      if (cells[x+i-1]>128)
	state |= 0x1;
                     
      if ((unit->rule >> state) & 1){
	cells[x+i+unit->celllen] = 255;
      }
      else{
	cells[x+i+unit->celllen] = 0;
      } 
      //      printf("%c",cells[x+i+unit->celllen]);
  }
  unit->del=0;
  }

  return x;
}

///////////////

u8 *table;

///how much memory does table take?

void inittable(u8 r, u8 k, int rule){
  u8 max; int z; u8 summ;

  free(table);
  max = (k-1)*((r*2)+1);
  table= (u8 *)malloc(max+1);
  for (z=max;z>=0;z--){
    summ=0;
    while ((rule-pow(k,z))>=0) {
      summ++;
      rule=rule-pow(k,z);
    }
    if (summ>=1) {
      table[z]=summ;
    }
    else table[z]=0;
    //    printf("%d\n",table[z]);
  }
}

//////////////////////////////////////////

// 1d with rules

uint16_t runcel1d(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){

  u8 cell,sum; signed int z,zz;
  u8 radius=3, k=4, i;//k=states
  struct CA* unit=unity;

  for (i=1;i<howmuch;i++){
    sum=0;
    
    // sum of cells in radius - not looping!
    for (z=-radius;z<radius;z++){
      zz=x+i+z;
      if (zz>=unit->celllen) zz=zz-unit->celllen;
      if (zz<0) zz=unit->celllen+zz;
      sum+=(cells[zz]>>4)%k;
    }

    cells[x+i+unit->celllen]= table[sum]<<4; 

    //    printf("%c",table[sum]<<4);
  }

  return x+i;
}

//////////////////////////////////////////

//forest fire

void fireinit(void* unity, u8* cells){
  struct fire* unit=unity;
  unit->probB=cells[0]/32;
  unit->probI=cells[1]/10;
  unit->celllen=cells[2];
  unit->del=0;
}

uint16_t runfire(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch,void* unity){

  u8 sum;
  uint16_t y; u8 i=0;
  struct fire* unit=unity;
  if (++unit->del==delay){
  for (i=0;i<howmuch;i++){

    sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    y=x+32768;

    if (cells[x]==0 || cells[x]==254) cells[y]=cells[x]; //empty or burnt
    // now deal with vegetation(bit1 empty) and burning(&1)
    else if ((cells[x]&1)==0 && randi()%255<=(sum*unit->probI)) cells[y]=cells[x]|1;  //veg->burning
    else if ((cells[x]&1)==1 && randi()%255<=unit->probB) cells[y]=254; // burning->burnt
    else cells[y]=cells[x];
    //    printf("%c",cells[y]);
    x++;
  }
  unit->del=0;
  }
  return x;
}

//////////////////////////////////////////

/*wireworld:

4 states: blank, copper, head, tail

blank(0) stays blank(0)
head(1) becomes tail(255)
tail(255) becomes copper(128-254)
copper(?) stays copper unless just 1 or 2 neighbours are heads(1) then it becomes head(1)
*/

// use struct CA

void wireinit(void* unity, u8* cells){
  struct CA* unit=unity;
  unit->del=0;
  unit->celllen=cells[0];
}

u8 headcount(struct CA* unit,u8 *cells,u16 place){
  u8 counter=0;
  place-=unit->celllen-1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  place+=unit->celllen-2;
  if (cells[place]==1) counter++;
  place+=2;
  if (cells[place]==1) counter++;
  place+=unit->celllen-1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  if (counter<3 && counter!=0) return 1;
  else return 0;
}

uint16_t runwire(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){
  u8 sum;
  uint16_t y; u8 i=0;
  struct CA* unit=unity;
  if (++unit->del==delay){
  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==0) cells[y]=0; //blank
    else if (cells[x]==1) cells[y]=255;  // head to tail
    else if (cells[x]==255) cells[y]=129;  // tail to copper
    else if (cells[x]>128 && headcount(unit,cells,x)==1) cells[y]=1;
    else cells[y]=cells[x];
    //    printf("%c",cells[y]);
    x++;
  }
  unit->del=0;
  }
  return x;
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

void SIRinit(void* unity, u8* cells){
    struct SIR* unit=unity;
  unit->probD=cells[0]/32;
  unit->probI=cells[1]/10;
  unit->celllen=cells[2];
  unit->del=0;
}

uint16_t runSIR(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){
  struct SIR* unit=unity;
  uint16_t y; u8 i=0;
  if (++unit->del==delay){

  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==129 || cells[x]==255) cells[y]=cells[x]; //dead or recovered
    //
    else if (cells[x]==0){
      // do count of surroundings
      if ( (cells[x-unit->celllen]>0 && cells[x-unit->celllen]<129) ||
	   (cells[x+unit->celllen]>0 && cells[x+unit->celllen]<129) ||
	   (cells[x-1]>0 && cells[x-1]<129) ||
	   (cells[x+1]>0 && cells[x+1]<129))
	{
	if (randi()%100 <= unit->probI) cells[y] = 1;       
      }
      
      //calc probI
    }
    else if (cells[x]>1 && cells[x]<129){
      if (randi()%100<unit->probD) cells[y]=255; //dead
      else cells[y]=cells[x]+1;
      // if infected (>1 and <129) add day until recovered(129) or dprob dead(255)
    }

    else cells[y]=cells[x]; // blank cells


    //    printf("%c",cells[y]);
    x++;
  }
  unit->del=0;
  }
  return x;
}

//////////////////////////////////////////

/* 16 bit SIR with 4x 4-bit fields:

-total pop
-S pop
-I pop
-R pop

infection radius???, max population=15

 */

void SIR16init(void* unity, u8* cells){
  struct SIR16* unit=unity;
  unit->del=0; u16 i; u8 total,suscept,infected;

  /*with probabilities fixed for:

- movement=probM
- morbidity=probR
- contact infection=probC
- recovery=probV
*/

  unit->probM=cells[0];
  unit->probR=cells[1];
  unit->probC=cells[2];
  unit->probV=cells[3];
  // distribute totals and SIR - total must be sum of S.I.R in other bits
  for (i=0;i<65534;i+=2){
    total=(cells[i]>>4)|1;
    suscept=cells[i]%total;
    infected=total-suscept;
    cells[i]=(total<<4)+suscept;
    cells[i+1]=(infected<<4);
  }
}


u16 biotadir[8]={65279,65280,1,257,256,254,65534,65278};

uint16_t runSIR16(uint16_t x, uint16_t delay, u8 *cells, uint8_t howmuch, void* unity){
  u8 i,ii; u16 y,dest;
  u8 totalhost,totaldest,which,sirhost,sirdest,futuretotal,futurerecovered,futuresuscept,futureinfected,sutureinfected,infected,suscept;
  struct SIR16* unit=unity;
  if (++unit->del==delay){

    // select random cell, for each of ind, select neighbour and move
    // it there based on probability and neighbour not full
  for (i=0;i<howmuch;i++){
#ifdef PCSIM
    y=randi()%65536;
#else
    y=randi()<<4;
#endif
    // choose random neighbour
    dest=y+biotadir[randi()%8];
    // 16 bits: top/lower top/lower total/S/I/R
    totaldest=cells[dest]>>4;
    totalhost=cells[y]>>4;
    if (totaldest<15 &&totalhost>0 && randi()%255<=unit->probM){
    // from host choose S,I or R to shift over if motionP and not full
    // then update both host y and dest...
      which=randi()%3;
      switch(which)
	{
	case 0:
	  totaldest+=1;
	  totalhost-=1;
	  sirdest=cells[dest]&15;
	  sirhost=cells[y]&15;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  cells[dest]=(totaldest<<4)+sirdest;
	  cells[y]=(totalhost<<4)+sirhost;
	  break;
	case 1:
	  totaldest+=1;
	  totalhost-=1;
	  sirdest=cells[dest+1]>>4;
	  sirhost=cells[y+1]>>4;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  // total
	  cells[dest]=(cells[dest]&15)+(totaldest<<4);
	  cells[y]=(cells[y]&15)+(totalhost<<4);
	  cells[dest+1]=(cells[dest+1]^240)+(sirdest<<4);
	  cells[y+1]=(cells[y+1]^240)+(sirhost<<4);
	  break;
	case 2:
	  totaldest+=1;
	  totalhost-=1;
	  sirdest=cells[dest+1]&15;
	  sirhost=cells[y+1]&15;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  cells[dest]=(cells[dest]&15)+(totaldest<<4);
	  cells[y]=(cells[y]&15)+(totalhost<<4);
	  cells[dest+1]=(cells[dest+1]^15)+sirdest;
	  cells[y+1]=(cells[y+1]^15)+sirhost;
	  break;
	}
    }
  }

  for (i=0;i<howmuch;i++){
    y=x+32768;

    // leave recovered as they are 
    // 16 bits: top/lower top/lower as-> total/S/I/R
    futuretotal=cells[x]>>4;
    futurerecovered=cells[x+1]&15;
    infected=futureinfected=cells[x+1]>>4;

    for (ii=0;ii<infected;ii++){
    // for each of infected:
    // deduct virus morbidity
    // recoveries
      if ((randi()%255)<=unit->probR) {futureinfected--; futuretotal--;} // dead
      if ((randi()%255)<=unit->probV) {futurerecovered++; futureinfected--;} // recovered
    }
    suscept=futuresuscept=cells[i]&15;
    for (ii=0;ii<suscept;ii++){
    // for each of suscept:
    // compute contact infections based on how many infected in cell
      if (((randi()%16)*infected)>unit->probC) {futureinfected++; futuresuscept--;} // dead
    }

    // put all back together???
    cells[y]=(futuretotal<<4)+futuresuscept;
    cells[y+1]=(futureinfected<<4)+futurerecovered;
    x+=2;
#ifdef PCSIM
    printf("%c%c",cells[y],cells[y+1]);
#endif

  }
  unit->del=0;
  
  }
  return x;
}

//////////////////////////////////////////


void ca_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u8* buffer){
  if (stack_posy<STACK_SIZE-1)
    {
      ++stack_posy;
      stack[stack_posy].howmuch=randi()%255;
      stack[stack_posy].delay=randi()%255;

      switch(typerr){
      case HODGEY:
	stack[stack_posy].unit=malloc(sizeof(struct hodge));
	hodgeinit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runhodge;
	break;
      case HODGENETY:
	stack[stack_posy].unit=malloc(sizeof(struct hodge));
	hodgeinit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runhodgenet;
	break;
      case LIFEY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runhodge;
	break;
      case CELY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runcel;
	break;
      case CEL1DY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runcel1d;
	break;
      case FIREY:
	stack[stack_posy].unit=malloc(sizeof(struct fire));
	fireinit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runfire;
	break;
      case WIREY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runwire;
	break;
      case SIRY:
	stack[stack_posy].unit=malloc(sizeof(struct SIR));
	SIRinit(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runSIR;
	break;
      case SIR16Y:
	stack[stack_posy].unit=malloc(sizeof(struct SIR16));
	SIR16init(stack[stack_posy].unit,buffer);
	stack[stack_posy].functione=runSIR16;
	break;
      }
    }
}

void ca_runall(struct stackey stack[STACK_SIZE], u8* buffer){
  static u16 count; u8 i;
  for (i=0;i<(stack_posy+1);i++){
    count=stack[i].functione(count,stack[i].delay,buffer,stack[i].howmuch,stack[i].unit);// set delay and howmuch in struct!
  }
}

void ca_pop(struct stackey stack[STACK_SIZE]){
 	if (stack_posy>=0)
	{
	  free(stack[stack_posy].unit);
	  stack_posy--;
	}
}


//////////////////////////////////////////

#ifdef PCSIM
int main(void)
{
  u16 x;
  u8 buffer[65536];
  uint16_t count=0;
  srandom(time(0));

  stack_posy=-1;
  struct stackey stack[STACK_SIZE];

  for (x=0;x<65535;x++){
    buffer[x]=randi()%255;
  }

  inittable(3,4,randi()%65536); //radius,states(k),rule - init with cell starter

       while(1){
	 ca_runall(stack,buffer);
       }
}
#endif
