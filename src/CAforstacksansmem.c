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
//u16 stackery[48];
#else
#include "CA.h"
#include "simulation.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern u8 table[21];
extern u16 stackery[STACK_SIZE*4]; // 16*4 MAX
#endif

//////////////////////////////////////////

// hodge from microbd simplified with circular buffer and init

/*void hodgeinit(void* unity, u8* cells){
  struct hodge* unit=unity;
  unit->q=cells[0];unit->k1=cells[1];unit->k2=cells[2];unit->g=cells[3];
  cells[4]=cells[4];
  unit->cells=cells;
  if (unit->k1==0) unit->k1=1;
  if (cells[2]==0) cells[2]=1;
  }*/

u16 runhodge(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){

  u8 sum=0, numill=0, numinf=0;
  uint16_t y; u8 i;
  u8 k1=cells[1];
  u8 k2=cells[2];
  if (k1==0) k1=1;
  if (k2==0) k2=1;

  for (i=0;i<howmuch;i++){
    y=x-cells[4]-1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=cells[4]-2;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=cells[4]-2;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y+=1;
    sum+=cells[y];
    if (cells[y]==(cells[0]-1)) numill++; else if (cells[y]>0) numinf++;

    y=x+wrap;
  if(cells[x] == 0)
    cells[y] = floorf(numinf / k1) + floorf(numill / k2);
  else if(cells[x] < cells[0] - 1)
    cells[y] = floorf(sum / (numinf + 1)) + cells[3];
  else
    cells[y] = 0;

  if(cells[y] > cells[0] - 1)
    cells[y] = cells[0] - 1;

  x++;
  if (x>(start+wrap)) x=start;
#ifdef PCSIM  
  //printf("%c",cells[y]);
#endif
  }
  return x;
}

//////////////////////////////////////////

// hodge from hodgenet is pretty much same... but few
// differences... so here they are expressed (also could be faster this way)

u16 runhodgenet(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){

  u8 sum=0, numill=0, numinf=0; u16 place;
  uint16_t y;
  u8 i;
  u8 k1=cells[1];
  u8 k2=cells[2];
  if (k1==0) k1=1;
  if (k2==0) k2=1;

  for (i=0;i<howmuch;i++){

  place=x-cells[4]-1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=cells[4]-2;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=cells[4]-2;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

  place+=1;
  if (cells[place]==cells[0]) numill++; if (cells[place]>0) numinf++;  
  sum+=cells[place];

    y=x+32768;
  if(cells[x] == 0)
    cells[y] = floorf(numinf / k1) + floorf(numill / k2);
  else if(cells[x] < cells[0])
    {    if ((numinf+1)!=0)    cells[y] = floorf(sum / (numinf + 1)) + cells[3];}
  else
    cells[y] = 0;

  if(cells[y] > cells[0])
    cells[y] = cells[0];

  x++;
  if (x>(start+wrap)) x=start;

#ifdef PCSIM  
  //  printf("%c",cells[x]);
#endif
  }
  return x;
}

//////////////////////////////////////////

//life - 2d CA - these all now use CA struct

//void inittable(u8 r, u8 k, int rule);

/*void cainit(void* unity, u8* cells){
  struct CA* unit=unity;
  cells[4]=cells[0]+1;
  unit->rule=cells[1];
  unit->cells=cells;
  }*/

u16 runlife(uint8_t howmuch, u8 *cells, u16 x, u16 start, u16 wrap){

  u8 sum=0;
  uint16_t y; u8 i;

  for (i=0;i<howmuch;i++){

    //    sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-cells[4]]&1)+(cells[x+cells[4]]&1)+(cells[x-cells[4]-1]&1)+(cells[x-cells[4]+1]&1)+(cells[x+cells[4]-1]&1)+(cells[x+cells[4]+1]&1);

    y=x-cells[4]-1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=cells[4]-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=cells[4]-2;
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
  if (x>(start+wrap)) x=start;

  }
  return x;
}

//////////////////////////////////////////

u16 runkrum(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){

  u16 i,y,place;
  u8 n=cells[0];
  if (n==0) n=1;
  //  n=4;
  for (i=0;i<howmuch;i++){
    y=x+32768;

    // for each neighbour if neighbour=(src+1) mod n
    // copy old neighbour to new/y

  place=x-cells[4];
  if (cells[place]==(cells[x]+1)%n) cells[y]=cells[place];

  place+=cells[4]-1;
  if (cells[place]==(cells[x]+1)%n) cells[y]=cells[place];

  place+=2;
  if (cells[place]==(cells[x]+1)%n) cells[y]=cells[place];

  place+=cells[4]-1;
  if (cells[place]==(cells[x]+1)%n) cells[y]=cells[place];

  x++;
  //  if (x>(start+wrap)) x=start;

#ifdef PCSIM  
  //     printf("%c",cells[x]);
#endif

  }
  return x;
}

//////////////////////////////////////////

//one dimensional - working line by line through buffer

u16 runcel(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){

  u8 state,i=0;u16 y;

  for (i=0;i<howmuch;i++){
    state = 0;
    x++;
  if (x>(start+wrap)) x=start;
  if (cells[(x +1+ (cells[0]))%65536]>128)
      state |= 0x4;
  if (cells[(x+(cells[0]))%65536]>128)
      state |= 0x2;
    if (cells[(x - 1 +(cells[0]))%65536]>128)
      state |= 0x1;

    y=(x+(cells[0]*2))%65536;// next row but one!

      if ((cells[1] >> state) & 1){
	cells[y] = 0;
      }
      else{
	cells[y] = 255;
      } 
#ifdef PCSIM  
      //      printf("%c",cells[y]);
#endif
  }
  return x;
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

u16 runcel1d(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){

  u8 sum; int16_t z; u16 zz;
  u8 i;//k=states
  u16 y;

  for (i=3;i<howmuch;i++){

    sum=0;
    
    // sum of cells in radius - not looping!
    for (z=-3;z<3;z++){ // hardcode sans radius...
      zz=x+i+z;
      if (zz>=cells[4]) zz=zz-cells[4];
      //      if (zz<0) zz=cells[4]+zz;
      sum+=(cells[zz]>>4)%4; // crash here? and it did/// not now !
    }

    y=x+i+cells[4];
    cells[y]=table[sum]<<4;  

#ifdef PCSIM  
    //	printf("%c",cells[y]);
#endif
    
  }
  x=x+i;
 if (x>(start+wrap)) x=start;
 return x;
}

//////////////////////////////////////////

//forest fire

/*void fireinit(void* unity, u8* cells){
  struct fire* unit=unity;
  unit->probB=cells[0]/32;
  probI=cells[1]/10;
  cells[4]=cells[2];
  unit->cells=cells;  
  }*/

u16 runfire(uint8_t howmuch,u8* cells, u16 x, u16 start, u16 wrap){

  u8 sum=0;
  uint16_t y; u8 i;
  u8 probB=cells[0]/32;
  u8 probI=cells[1]/10;
  cells[4]=cells[2];


  for (i=0;i<howmuch;i++){

    y=x-cells[4]-1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=cells[4]-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=cells[4]-2;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    y+=1;
    sum+=cells[y]&1;

    //    sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-cells[4]]&1)+(cells[x+cells[4]]&1)+(cells[x-cells[4]-1]&1)+(cells[x-cells[4]+1]&1)+(cells[x+cells[4]-1]&1)+(cells[x+cells[4]+1]&1);

    y=x+32768;

    if (cells[x]==0 || cells[x]==254) cells[y]=cells[x]; //empty or burnt
    // now deal with vegetation(bit1 empty) and burning(&1)
    else if ((cells[x]&1)==0 && randi()%255<=(sum*probI)) cells[y]=cells[x]|1;  //veg->burning
    else if ((cells[x]&1)==1 && randi()%255<=probB) cells[y]=254; // burning->burnt
    else cells[y]=cells[x];

#ifdef PCSIM  
    //      printf("%c",cells[x]);
#endif

    x++;
    if (x>(start+wrap)) x=start;
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

/*void wireinit(void* unity, u8* cells){
  struct CA* unit=unity;
  cells[4]=cells[0];
  }*/

u8 headcount(u8 *cells,u16 place){
  u8 counter=0;
  cells[4]=cells[0];
  place-=cells[4]-1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  place+=cells[4]-2;
  if (cells[place]==1) counter++;
  place+=2;
  if (cells[place]==1) counter++;
  place+=cells[4]-1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  place+=1;
  if (cells[place]==1) counter++;
  if (counter<3 && counter!=0) return 1;
  else return 0;
}

u16 runwire(uint8_t howmuch, u8 *cells, u16 x, u16 start, u16 wrap){
  uint16_t y; u8 i;

  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==0) cells[y]=0; //blank
    else if (cells[x]==1) cells[y]=255;  // head to tail
    else if (cells[x]==255) cells[y]=129;  // tail to copper
    else if (cells[x]>128 && headcount(cells,x)==1) cells[y]=1;
    else cells[y]=cells[x];

#ifdef PCSIM  
    //       printf("%c",cells[x]);
#endif

    x++;
  if (x>(start+wrap)) x=start;
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

		 /*void SIRinit(void* unity, u8* cells){
    struct SIR* unit=unity;
  unit->probD=cells[0]/32;
  probI=cells[1]/10;
  cells[4]=cells[2];
  unit->cells=cells;
  }*/

u16 runSIR(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){
  uint16_t y,yy; u8 i;

  u8 probD=cells[0]/32;
  u8 probI=cells[1]/10;
  cells[4]=cells[2];


  for (i=0;i<howmuch;i++){

    y=x+32768;

    if (cells[x]==129 || cells[x]==255) cells[y]=cells[x]; //dead or recovered
    //
    else if (cells[x]==0){
      // do count of surroundings
      /*      if ( (cells[x-cells[4]]>0 && cells[x-cells[4]]<129) ||
	   (cells[x+cells[4]]>0 && cells[x+cells[4]]<129) ||
	   (cells[x-1]>0 && cells[x-1]<129) ||
	   (cells[x+1]>0 && cells[x+1]<129))
	{
	if (randi()%100 <= probI) cells[y] = 1;       
	}*/
      yy=x-cells[4];
      if (cells[yy]>0 && cells[yy]<129) {	if (randi()%100 <= probI) cells[y] = 1;       }
	else {
	  yy+=cells[4]-1;
	  if (cells[yy]>0 && cells[yy]<129){ 	if (randi()%100 <= probI) cells[y] = 1;       }
	    else {
	      yy+=2;
	      if (cells[yy]>0 && cells[yy]<129){ 	if (randi()%100 <= probI) cells[y] = 1;}       
		else {
		  yy+=1;
		  if (cells[yy]>0 && cells[yy]<129){ 	if (randi()%100 <= probI) cells[y] = 1;       }
		}
	    }
	}      
      //calc probI
    }
    else if (cells[x]>1 && cells[x]<129){
      if (randi()%100<probD) cells[y]=255; //dead
      else cells[y]=cells[x]+1;
      // if infected (>1 and <129) add day until recovered(129) or dprob dead(255)
    }

    else cells[y]=cells[x]; // blank cells

#ifdef PCSIM  
    //       printf("%c",cells[x]);
#endif

    x++;
  if (x>(start+wrap)) x=start;

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

/*void SIR16init(void* unity, u8* cells){
  //  struct SIR16* unit=unity;
  //  u16 i,y; u8 total,suscept,infected;

  //with probabilities fixed for:

- movement=probM
- morbidity=probR
- contact infection=probC
- recovery=probV
}*/


u16 biotadir[8]={65279,65280,1,257,256,254,65534,65278};

u16 runSIR16(uint8_t howmuch, u8* cells, u16 x, u16 start, u16 wrap){
  u8 i,ii; u16 y,dest,yy;
  u8 totalhost,totaldest,which,sirhost,sirdest,futuretotal,futurerecovered,futuresuscept,futureinfected,infected,suscept;

  u8 probM=cells[0];
  u8 probR=cells[1];
  u8 probC=cells[2];
  u8 probV=cells[3];

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
    if (totaldest<15 &&totalhost>0 && randi()%255<=probM){
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
      if ((randi()%255)<=probR) {futureinfected--; futuretotal--;} // dead
      if ((randi()%255)<=probV) {futurerecovered++; futureinfected--;} // recovered
    }
    suscept=futuresuscept=cells[i]&15;
    for (ii=0;ii<suscept;ii++){
    // for each of suscept:
    // compute contact infections based on how many infected in cell
      if (((randi()%16)*infected)>probC) {futureinfected++; futuresuscept--;} // dead
    }

    // put all back together???
    cells[y]=(futuretotal<<4)+futuresuscept;
    yy=y+1;
    cells[yy]=(futureinfected<<4)+futurerecovered;
    x+=2;

#ifdef PCSIM  
    //        printf("%c",cells[x]);
#endif

  if (x>(start+wrap)) x=start;
  }
    return x;
}

//////////////////////////////////////////

//stack_posy=ca_pushn(stack,0,datagenbuffer,stack_posy,1,10); // delay,howmany);

signed char ca_pushn(struct stackeyyy stack[STACK_SIZE], u8 typerr, u8* buffer, u8 stack_posy, u8 howmuch, u16 start, u16 wrap){
  if (stack_posy<STACK_SIZE)
    {

      if (howmuch==0) howmuch=1;
      u8 tmp=stack_posy*4;
      stackery[tmp]=start;
      stack[stack_posy].buffer=buffer;
      stack[stack_posy].count=start;
      stackery[tmp+1]=wrap;
      stackery[tmp+2]=howmuch;
      stackery[tmp+3]=typerr;
      stack_posy++;
    }  
  return stack_posy;
}

void ca_runall(struct stackeyyy stack[STACK_SIZE], u8 stack_posy){
  u8 i,x; 
  for (i=0;i<stack_posy;i++){
	u8 tmp=i*4;

	//	stack[i].count=stack[i].functione(stackery[tmp+1],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+2]);
	x=stackery[tmp+3]%NUM_CA;
      switch(x){
      case KRUMMY:
	stack[i].count=runkrum(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case HODGEY:
	//	stack[stack_posy].unit=malloc(sizeof(struct hodge));
	//	if (stack[stack_posy].unit==NULL) return stack_posy; TODO????
	//	hodgeinit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runhodge;
	stack[i].count=runhodge(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case HODGENETY:
	//	stack[stack_posy].unit=malloc(sizeof(struct hodge));
	//	hodgeinit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runhodgenet;
	stack[i].count=runhodgenet(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);

	break;
      case LIFEY:
	//	stack[stack_posy].unit=malloc(sizeof(struct CA));
	//	cainit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runlife;
	stack[i].count=runlife(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case CELY:
	//	stack[stack_posy].unit=malloc(sizeof(struct CA));
	//	cainit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runcel;
	stack[i].count=runcel(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case CEL1DY:
	//	stack[stack_posy].unit=malloc(sizeof(struct CA));
	//	cainit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runcel1d;
	stack[i].count=runcel1d(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case FIREY:
	//	stack[stack_posy].unit=malloc(sizeof(struct fire));
	//	fireinit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runfire;
	stack[i].count=runfire(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case WIREY:
	//	stack[stack_posy].unit=malloc(sizeof(struct CA));
	//	cainit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runwire;
	stack[i].count=runwire(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case SIRY:
	//	stack[stack_posy].unit=malloc(sizeof(struct SIR));
	//	SIRinit(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runSIR;
	stack[i].count=runSIR(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      case SIR16Y:
	//	stack[stack_posy].unit=malloc(sizeof(struct SIR16));
	//	SIR16init(stack[stack_posy].unit,buffer);
	//	stack[stack_posy].functione=runSIR16;
	stack[i].count=runSIR16(stackery[tmp+2],stack[i].buffer,stack[i].count,stackery[tmp],stackery[tmp+1]);
	break;
      }
  }
}

signed char ca_pop(u8 stack_posy){
 	if (stack_posy>0)
	{
	  stack_posy--;
	  //	  if (stack[stack_posy].unit!=NULL){
	  //	  free(stack[stack_posy].unit);
		  //	  stack[stack_posy].unit=NULL;
		  //	  	  }
	  //	  stack_posy--;
	}
  return stack_posy;
}


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

