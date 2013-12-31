//gcc -DLINUX -std=gnu99 CA.c -o CA -lm

// runhodge, runhodgenet, runlife, runcel, runcel1d, runfire, runwire, runSIR

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "leaky.h"

/* TODO D.I:

ADD SIR code

*/

//////////////////////////////////////////

// hodge from microbd simplified with circular buffer and init

struct hodge{
  u8 q,k1,k2,g,celllen;
  u16 x;
};

void hodgeinit(struct hodge* unit, u8* cells){
  unit->q=cells[0];unit->k1=cells[1];unit->k2=cells[2];unit->g=cells[3];
  unit->celllen=cells[4];
  if (unit->k1==0) unit->k1=1;
  if (unit->k2==0) unit->k2=1;
}

uint16_t runhodge(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct hodge* unit){

  u8 sum=0, numill=0, numinf=0;
  uint16_t y; u8 i;

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
  printf("%c",cells[x]);
  }
  return x;
}

//////////////////////////////////////////

// hodge from hodgenet is pretty much same... but few
// differences... so here they are expressed (also could be faster this way)

uint16_t runhodgenet(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct hodge* unit){

  u8 sum=0, numill=0, numinf=0; u16 place;
  uint16_t y;
  u8 i;

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
  printf("%c",cells[y]);
  }
  return x;
}

//////////////////////////////////////////

//life - 2d CA - these all now use CA struct

struct CA{
  u8 celllen,rule;
};

void cainit(struct CA* unit, u8* cells){
  unit->celllen=cells[0];
  unit->rule=cells[1];
}

uint16_t runlife(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct CA* unit){

  u8 sum;
  uint16_t y; u8 i;

  for (i=0;i<howmuch;i++){
  sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    /*    if (sum==3 || (sum+(cells[x]&0x01)==3)) newcells[x]=255; /// 
	  else newcells[x]=0;*/
  y=x+32768;
  if ((cells[x]&1)==1 && sum<2) cells[y]=0;
  else if ((cells[x]&1)==1 && sum>3) cells[y]=0;
  else if ((cells[x]&1)==0 && sum==3) cells[y]=255;
  else cells[y]=cells[x];

  printf("%c",cells[x]);
  x++;
  }
  return i;
}

//////////////////////////////////////////

//one dimensional - working line by line through buffer

uint16_t runcel(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct CA* unit){

  u8 state,i;

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
      printf("%c",cells[x+i+unit->celllen]);
  }
  return i;
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

uint16_t runcel1d(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct CA* unit){

  u8 cell,sum; signed int z,zz;
  u8 radius=3, k=4, i;//k=states

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

    printf("%c",table[sum]<<4);
  }

  return i;
}

//////////////////////////////////////////

//forest fire

struct fire{
  u8 probB,probI,celllen;
};

void fireinit(struct fire* unit, u8* cells){
  unit->probB=cells[0]/32;
  unit->probI=cells[1]/10;
  unit->celllen=cells[2];
}

uint16_t runfire(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct fire* unit){

  u8 sum;
  uint16_t y; u8 i;

  for (i=0;i<howmuch;i++){

    sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    y=x+32768;

    if (cells[x]==0 || cells[x]==254) cells[y]=cells[x]; //empty or burnt
    // now deal with vegetation(bit1 empty) and burning(&1)
    else if ((cells[x]&1)==0 && rand()%255<=(sum*unit->probI)) cells[y]=cells[x]|1;  //veg->burning
    else if ((cells[x]&1)==1 && rand()%255<=unit->probB) cells[y]=254; // burning->burnt
    else cells[y]=cells[x];
    printf("%c",cells[y]);
    x++;
  }
  return i;
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

void wireinit(struct CA* unit, u8* cells){
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

uint16_t runwire(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct CA* unit){
  u8 sum;
  uint16_t y; u8 i;

  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==0) cells[y]=0; //blank
    else if (cells[x]==1) cells[y]=255;  // head to tail
    else if (cells[x]==255) cells[y]=129;  // tail to copper
    else if (cells[x]>128 && headcount(unit,cells,x)==1) cells[y]=1;
    else cells[y]=cells[x];
    printf("%c",cells[y]);
    x++;
  }
  return i;
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

struct SIR{
  u8 probI,probD,celllen;
};

void SIRinit(struct SIR* unit, u8* cells){
  unit->probD=cells[0]/32;
  unit->probI=cells[1]/10;
  unit->celllen=cells[2];
}

uint16_t runSIR(uint16_t x, uint16_t delay, uint16_t speed, u8 *cells, uint8_t howmuch, struct SIR* unit){

  uint16_t y; u8 i;

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
	if (rand()%100 <= unit->probI) cells[y] = 1;       
      }
      
      //calc probI
    }
    else if (cells[x]>1 && cells[x]<129){
      if (rand()%100<unit->probD) cells[y]=255; //dead
      else cells[y]=cells[x]+1;
      // if infected (>1 and <129) add day until recovered(129) or dprob dead(255)
    }

    else cells[y]=cells[x]; // blank cells


    printf("%c",cells[y]);
    x++;
  }
  return i;
}


//////////////////////////////////////////

int main(void)
{
  int x;
  u8 buffer[65536];
  uint16_t count=0;
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=rand()%255;
  }
  inittable(3,4,rand()%65536); //radius,states(k),rule - init with cell starter

  //  struct hodge *unit=malloc(sizeof(struct hodge));
  //    struct CA *unit=malloc(sizeof(struct CA));
    struct SIR *unit=malloc(sizeof(struct SIR));
  //struct fire *unit=malloc(sizeof(struct fire));
  //  hodgeinit(unit,buffer);
    //    cainit(unit,buffer);
    SIRinit(unit,buffer);
  //  fireinit(unit,buffer);
      while(1) {

	//	count+=runhodge(count,10,10,buffer,10,unit);
	//	count+=runcel1d(count,10,10,buffer,255,unit);
	//	count+=runfire(count,10,10,buffer,255,unit);
	//	count+=runwire(count,10,10,buffer,255,unit);
	count+=runSIR(count,10,10,buffer,255,unit);
	//	printf("%d",count);
	// runhodge, runhodgenet, runlife, runcel, runcel1d, runfire, runwire, runSIR

    }
}
