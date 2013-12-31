//gcc -DLINUX -std=gnu99 CA.c -o CA -lm

// runhodge, runhodgenet, runlife, runcel, runcel1d
// + CA SIR code?

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "leaky.h"

/* TODO D.I:

ADD forest fire and SIR code

-- init for all,, all types cleaned up - to test

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
  return x;
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

/*

all 2d automata (as in life)

*notes for forest fire:

cell states:

*for SIR:

*for wireworld:

4 states: blank, copper, head, tail

blank(0) stays blank(0)
head(<64) becomes tail(64->128)
tail(64->128) becomes copper(>128)
copper(>128) stays copper unless just 1 or 2 neighbours are heads(<64) then it becomes head(<64)

*/

//////////////////////////////////////////

int main(void)
{
  u16 x;
  u8 buffer[65536];
  uint16_t count=0;
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=rand()%255;
  }
  inittable(3,4,rand()%65536); //radius,states(k),rule - init with cell starter

  //  struct hodge *unit=malloc(sizeof(struct hodge));
  struct CA *unit=malloc(sizeof(struct CA));

  //  hodgeinit(unit,buffer);
  cainit(unit,buffer);
      while(1) {

	//	count+=runhodge(count,10,10,buffer,10,unit);
	count+=runlife(count,10,10,buffer,10,unit);
	  
	// runhodge, runhodgenet, runlife, runcel, runcel1d

    }
}
