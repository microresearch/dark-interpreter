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
u8 table[21];
#else
#include "CA.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern u8 table[21];
#endif

//////////////////////////////////////////

#define STACK_SIZE 16

struct stackey{
void (*functione) (u8 howmuch, void * unity);  
  u8 howmuch;
  void* unit;
  };

//////////////////////////////////////////

// hodge from microbd simplified with circular buffer and init

void hodgeinit(void* unity, u8* cells, u16 start, u16 wrap){
  struct hodge* unit=unity;
  unit->q=cells[0];unit->k1=cells[1];unit->k2=cells[2];unit->g=cells[3];
  unit->celllen=cells[4];
  unit->start=start;
  unit->wrap=wrap;
  unit->count=start;
  unit->cells=cells;
  if (unit->k1==0) unit->k1=1;
  if (unit->k2==0) unit->k2=1;
}

void runhodge(uint8_t howmuch, void* unity){

  u8 sum=0, numill=0, numinf=0;
  uint16_t y; u8 i=0;
  struct hodge *unit=unity;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=0;i<howmuch;i++){
    y=x-unit->celllen-1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=unit->celllen-2;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=unit->celllen-2;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(unit->q-1)) numill++; else if (cells[y]>0) numinf++;

    y=x+unit->wrap;
  if(cells[x] == 0)
    cells[y] = floorf(numinf / unit->k1) + floorf(numill / unit->k2);
  else if(cells[x] < unit->q - 1)
    cells[y] = floorf(sum / (numinf + 1)) + unit->g;
  else
    cells[y] = 0;

  if(cells[y] > unit->q - 1)
    cells[y] = unit->q - 1;

  x++;
  if (x>(unit->start+unit->wrap)) x=unit->start;
#ifdef PCSIM  
  //  //  //  //    printf("%c",cells[y]);
#endif
  }
  unit->count=x;
}

//////////////////////////////////////////

// hodge from hodgenet is pretty much same... but few
// differences... so here they are expressed (also could be faster this way)

void runhodgenet(uint8_t howmuch, void* unity){

  struct hodge* unit=unity;
  u8 sum=0, numill=0, numinf=0; u16 place;
  uint16_t y;
  u8 i=0;
  u16 x= unit->count;
  u8 *cells=unit->cells;

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

  place+=unit->celllen-2;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==unit->q) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=unit->celllen-2;
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
    cells[y] = floorf(numinf / unit->k1) + floorf(numill / unit->k2);
  else if(cells[x] < unit->q)
    cells[y] = floorf(sum / (numinf + 1)) + unit->g;
  else
    cells[y] = 0;

  if(cells[y] > unit->q)
    cells[y] = unit->q;

  x++;
  if (x>(unit->start+unit->wrap)) x=unit->start;

#ifdef PCSIM  
  //    printf("%c",cells[x]);
#endif
  }
  unit->count=x;
}

//////////////////////////////////////////

//life - 2d CA - these all now use CA struct

void inittable(u8 r, u8 k, int rule, u8 *table);

void cainit(void* unity, u8* cells, u16 start, u16 wrap){
  struct CA* unit=unity;
  unit->celllen=cells[0]+1;
  unit->rule=cells[1];
  unit->start=start;
  unit->wrap=wrap;
  unit->count=start;
  unit->cells=cells;
}

void runlife(uint8_t howmuch, void* unity){

  u8 sum;
  uint16_t y; u8 i=0;
  struct CA* unit=unity;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=0;i<howmuch;i++){

    //  sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    y=x-unit->celllen-1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=unit->celllen-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=unit->celllen-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    /*    if (sum==3 || (sum+(cells[x]&0x01)==3)) newcells[x]=255; /// 
	  else newcells[x]=0;*/
  y=x+32768;
  if ((cells[x]&1)==1 && sum<2) cells[y]=0;
  else if ((cells[x]&1)==1 && sum>3) cells[y]=0;
  else if ((cells[x]&1)==0 && sum==3) cells[y]=255;
  else cells[y]=cells[x];

#ifdef PCSIM  
  //    printf("%c",cells[x]);
#endif

  //  //  printf("%c",cells[x]);
  x++;
  if (x>(unit->start+unit->wrap)) x=unit->start;

  }
  unit->count=x;
}

//////////////////////////////////////////

//one dimensional - working line by line through buffer

void runcel(uint8_t howmuch, void* unity){

  u8 state,i=0; u16 y;
  struct CA* unit=unity;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=1;i<howmuch;i++){
      state = 0;
      y=x+i+1;
      if (cells[y]>128)
	state |= 0x4;
      y-=1;
      if (cells[y]>128)
	state |= 0x2;
      y-=1;
      if (cells[y]>128)
	state |= 0x1;
      y=x+i+unit->celllen;
      if ((unit->rule >> state) & 1){
	cells[y] = 255;
      }
      else{
	cells[y] = 0;
      } 
#ifdef PCSIM  
      //    printf("%c",cells[x]);
#endif

  }
  x+=i;
  if (x>(unit->start+unit->wrap)) x=unit->start;
  unit->count=x;
}

///////////////

void inittable(u8 r, u8 k, int rule, u8 *table){
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

void runcel1d(uint8_t howmuch, void* unity){

  u8 cell,sum; int16_t z,zz;
  u8 k=4, i;//k=states
  struct CA* unit=unity;
  u16 y;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=1;i<howmuch;i++){
    sum=0;
    
    // sum of cells in radius - not looping!
    for (z=-3;z<3;z++){ // hardcode sans radius...
      zz=x+i+z;
      if (zz>=unit->celllen) zz=zz-unit->celllen;
      if (zz<0) zz=unit->celllen+zz;
            sum+=(cells[(u16)zz]>>4)%4; // crash here?
    }

    y=x+i+unit->celllen;
        cells[y]= table[sum]<<4;  

#ifdef PCSIM  
	//    printf("%c",cells[x]);
#endif
    
  }


  x=x+i;
 if (x>(unit->start+unit->wrap)) x=unit->start;
 unit->count=x;

}

//////////////////////////////////////////

//forest fire

void fireinit(void* unity, u8* cells,  u16 start, u16 wrap){
  struct fire* unit=unity;
  unit->probB=cells[0]/32;
  unit->probI=cells[1]/10;
  unit->celllen=cells[2];
  unit->start=start;
  unit->wrap=wrap;
  unit->count=start;
  unit->cells=cells;
}

void runfire(uint8_t howmuch,void* unity){

  u8 sum;
  uint16_t y; u8 i=0;
  struct fire* unit=unity;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=0;i<howmuch;i++){

    y=x-unit->celllen-1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=unit->celllen-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=unit->celllen-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    //    sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    y=x+32768;

    if (cells[x]==0 || cells[x]==254) cells[y]=cells[x]; //empty or burnt
    // now deal with vegetation(bit1 empty) and burning(&1)
    else if ((cells[x]&1)==0 && randi()%255<=(sum*unit->probI)) cells[y]=cells[x]|1;  //veg->burning
    else if ((cells[x]&1)==1 && randi()%255<=unit->probB) cells[y]=254; // burning->burnt
    else cells[y]=cells[x];

#ifdef PCSIM  
    //    printf("%c",cells[x]);
#endif

    x++;
    if (x>(unit->start+unit->wrap)) x=unit->start;
  }
  unit->count=x;
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

void wireinit(void* unity, u8* cells,  u16 start, u16 wrap){
  struct CA* unit=unity;
  unit->celllen=cells[0];
  unit->start=start;
  unit->wrap=wrap;
  unit->count=start;
  unit->cells=cells;
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

void runwire(uint8_t howmuch, void* unity){
  u8 sum;
  uint16_t y; u8 i=0;
  struct CA* unit=unity;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==0) cells[y]=0; //blank
    else if (cells[x]==1) cells[y]=255;  // head to tail
    else if (cells[x]==255) cells[y]=129;  // tail to copper
    else if (cells[x]>128 && headcount(unit,cells,x)==1) cells[y]=1;
    else cells[y]=cells[x];

#ifdef PCSIM  
    //    printf("%c",cells[x]);
#endif

    x++;
  if (x>(unit->start+unit->wrap)) x=unit->start;
  }
  unit->count=x;
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

void SIRinit(void* unity, u8* cells, u16 start, u16 wrap){
    struct SIR* unit=unity;
  unit->probD=cells[0]/32;
  unit->probI=cells[1]/10;
  unit->celllen=cells[2];
  unit->start=start;
  unit->wrap=wrap;
  unit->count=start;
  unit->cells=cells;
}

void runSIR(uint8_t howmuch, void* unity){
  struct SIR* unit=unity;
  uint16_t y,yy; u8 i=0;
  u16 x= unit->count;
  u8 *cells=unit->cells;

  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==129 || cells[x]==255) cells[y]=cells[x]; //dead or recovered
    //
    else if (cells[x]==0){
      // do count of surroundings
      /*      if ( (cells[x-unit->celllen]>0 && cells[x-unit->celllen]<129) ||
	   (cells[x+unit->celllen]>0 && cells[x+unit->celllen]<129) ||
	   (cells[x-1]>0 && cells[x-1]<129) ||
	   (cells[x+1]>0 && cells[x+1]<129))
	{
	if (randi()%100 <= unit->probI) cells[y] = 1;       
	}*/
      yy=x-unit->celllen;
      if (cells[yy]>0 && cells[yy]<129) 	if (randi()%100 <= unit->probI) cells[y] = 1;       
	else {
	  yy+=unit->celllen-1;
	  if (cells[yy]>0 && cells[yy]<129) 	if (randi()%100 <= unit->probI) cells[y] = 1;       
	    else {
	      yy+=2;
	      if (cells[yy]>0 && cells[yy]<129) 	if (randi()%100 <= unit->probI) cells[y] = 1;       
		else {
		  yy+=1;
		  if (cells[yy]>0 && cells[yy]<129) 	if (randi()%100 <= unit->probI) cells[y] = 1;       
		}
	    }
	}      
      //calc probI
    }
    else if (cells[x]>1 && cells[x]<129){
      if (randi()%100<unit->probD) cells[y]=255; //dead
      else cells[y]=cells[x]+1;
      // if infected (>1 and <129) add day until recovered(129) or dprob dead(255)
    }

    else cells[y]=cells[x]; // blank cells

#ifdef PCSIM  
    //    printf("%c",cells[x]);
#endif

    x++;
  if (x>(unit->start+unit->wrap)) x=unit->start;

  }
  unit->count=x;

}

//////////////////////////////////////////

/* 16 bit SIR with 4x 4-bit fields:

-total pop
-S pop
-I pop
-R pop

infection radius???, max population=15

 */

void SIR16init(void* unity, u8* cells, u16 start, u16 wrap){
  struct SIR16* unit=unity;
  u16 i,y; u8 total,suscept,infected;

  /*with probabilities fixed for:

- movement=probM
- morbidity=probR
- contact infection=probC
- recovery=probV
*/
  unit->cells=cells;
  unit->probM=cells[0];
  unit->probR=cells[1];
  unit->probC=cells[2];
  unit->probV=cells[3];
  // TODO/IGNORE fix this constraint!
  // distribute totals and SIR - total must be sum of S.I.R in other bits
  /*  for (i=0;i<65534;i+=2){
    total=(cells[i]>>4)|1;
    suscept=cells[i]%total;
    infected=total-suscept;
    cells[i]=(total<<4)+suscept;
    y=i+1;
    cells[y]=(infected<<4);
    }*/
  unit->start=start;
  unit->wrap=wrap;
  unit->count=start;

}


u16 biotadir[8]={65279,65280,1,257,256,254,65534,65278};

void runSIR16(uint8_t howmuch, void* unity){
  u8 i,ii; u16 y,dest,yy;
  u8 totalhost,totaldest,which,sirhost,sirdest,futuretotal,futurerecovered,futuresuscept,futureinfected,sutureinfected,infected,suscept;
  struct SIR16* unit=unity;
  u16 x= unit->count;
  u8 *cells=unit->cells;

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
	  yy=dest+1;
	  sirdest=cells[yy]>>4;
	  yy=y+1;
	  sirhost=cells[yy]>>4;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  // total
	  cells[dest]=(cells[dest]&15)+(totaldest<<4);
	  cells[y]=(cells[y]&15)+(totalhost<<4);
	  yy=dest+1;
	  cells[yy]=(cells[yy]^240)+(sirdest<<4);
	  yy=y+1;
	  cells[yy]=(cells[yy]^240)+(sirhost<<4);
	  break;
	case 2:
	  totaldest+=1;
	  totalhost-=1;
	  yy=dest+1;
	  sirdest=cells[yy]&15;
	  yy=y+1;
	  sirhost=cells[yy]&15;
	  sirdest+=1;
	  sirhost-=1;
	  // updating
	  cells[dest]=(cells[dest]&15)+(totaldest<<4);
	  cells[y]=(cells[y]&15)+(totalhost<<4);
	  yy=dest+1;
	  cells[yy]=(cells[yy]^15)+sirdest;
	  yy=y+1;
	  cells[yy]=(cells[yy]^15)+sirhost;
	  break;
	}
    }
  }

  for (i=0;i<howmuch;i++){
    y=x+32768;

    // leave recovered as they are 
    // 16 bits: top/lower top/lower as-> total/S/I/R
    futuretotal=cells[x]>>4;
    yy=x+1;
    futurerecovered=cells[yy]&15;
    infected=futureinfected=cells[yy]>>4;

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
    yy=y+1;
    cells[yy]=(futureinfected<<4)+futurerecovered;
    x+=2;

#ifdef PCSIM  
    //    printf("%c",cells[x]);
#endif

  if (x>(unit->start+unit->wrap)) x=unit->start;
  }
    unit->count=x;
}

//////////////////////////////////////////

//stack_posy=ca_pushn(stack,0,datagenbuffer,stack_posy,1,10); // delay,howmany);

signed char ca_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u8* buffer, u8 stack_posy, u8 howmuch, u16 start, u16 wrap){
  if (stack_posy<STACK_SIZE)
    {
      //      //   printf("%d\n",stack_posy);
      stack[stack_posy].howmuch=howmuch;

      switch(typerr){
      case HODGEY:
	stack[stack_posy].unit=malloc(sizeof(struct hodge));
	hodgeinit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runhodge;
	break;
      case HODGENETY:
	stack[stack_posy].unit=malloc(sizeof(struct hodge));
	hodgeinit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runhodgenet;
	break;
      case LIFEY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runlife;
	break;
      case CELY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runcel;
	break;
      case CEL1DY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runcel1d;
	break;
      case FIREY:
	stack[stack_posy].unit=malloc(sizeof(struct fire));
	fireinit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runfire;
	break;
      case WIREY:
	stack[stack_posy].unit=malloc(sizeof(struct CA));
	cainit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runwire;
	break;
      case SIRY:
	stack[stack_posy].unit=malloc(sizeof(struct SIR));
	SIRinit(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runSIR;
	break;
      case SIR16Y:
	stack[stack_posy].unit=malloc(sizeof(struct SIR16));
	SIR16init(stack[stack_posy].unit,buffer,start,wrap);
	stack[stack_posy].functione=runSIR16;
	break;
      }
      stack_posy++;
    }  
  return stack_posy;
}

void ca_runall(struct stackey stack[STACK_SIZE], u8 stack_posy){
  static u16 count; u8 i;
  for (i=0;i<stack_posy;i++){
    if (stack[stack_posy].unit!=NULL){
      stack[i].functione(stack[i].howmuch,stack[i].unit);
        }
  }
}

signed char ca_pop(struct stackey stack[STACK_SIZE], u8 stack_posy){
 	if (stack_posy>0)
	{
	  stack_posy--;
	  //	  if (stack[stack_posy].unit!=NULL){
	  	  free(stack[stack_posy].unit);
		  //	  stack[stack_posy].unit=NULL;
		  //	  	  }
	  //	  stack_posy--;
	}
  return stack_posy;
}


//////////////////////////////////////////

#ifdef PCSIM
int main(void)
{
  u16 x,xx;
  u16 buffer[32768];
  uint16_t count=0;
  srandom(time(0));

  u8 stack_posy=0;
  struct stackey stack[STACK_SIZE];
  void *malloced[NUM_CA];


  for (x=0;x<32768;x++){
    buffer[x]=randi()%65536;
  }

  inittable(3,4,randi()%65536,table); //radius,states(k),rule - init with cell starter

    for (x=0;x<STACK_SIZE;x++){
      xx=rand()%32768;
      stack_posy=ca_pushn(stack,rand()%9,buffer,stack_posy,100,xx,xx+rand()%16000); // last as howmany, start.,wrap
    }

    //    //    printf("stackposy: %d\n", stack_posy);


               while(1){
		 ca_runall(stack,stack_posy);     

  //		 	 	 else stack_posy=ca_pop(stack,stack_posy);
		 //	 printf("stackposy: %d\n", stack_posy);
		 //signed char ca_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u8* buffer, u8 stack_posy, u8 howmuch, u16 start, u16 wrap){
		     printf("%c",buffer[x%32768]>>8);
		     //		     which=buf16[x%32768]>>8;
		   x++;

    	 }

}
#endif
