//  gcc datagentest.c -odatagen -lm
// ./datagen CPU PLAGUE

/* testing of all datagens */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define true 1
#define false 0


/* TODO:

- question -> should also be how much data to return in call?

proto: uint16_t SIR(uint16_t delay, uint16_t speed, uint16_t *workingbuffer)

any settings are taken from first bytesof buffer

- test again as all borrowed from microstripped.c - REDO... starting most from scratch

- each datagen should return a value
- add new datagens and change also for uint16_t
- add stepsizing and how is organised for buffers etc.

older: void (*plag[])(unsigned char* cells) = {mutate,SIR,hodge,cel,hodge,SIR,life,mutate};

divide into:

1- CPU/instruction sets from microbd (inc. corewars?) RETEST

2- cellular automata: 1d,2d, classic, flexible - test all

3- worms, langton's ants,turmites

4- esoteric:brainfuck

5- simulations: hodge - hodge.c, rossler, orbital, ifs, fitzhughnagumo - see
fitz.c, oreganotor, brusselator 

6- SIR simple, SIR using equations,
more ambitious models...

see:

latestresearch/sc3-plugins-src-2012-05-26/source/SLUGens/SLUGens.cpp

http://doc.sccode.org/Classes/FitzHughNagumo.html

*/



#define MAX_SAM 1024
#define MAXDIV2 512
#define CELLLEN 128
#define floor(x) ((int)(x))
#define HEX__(n) 0x##n##UL
#define recovered 255
#define susceptible 0

uint8_t k,p;


//6-SIR section

// SIR: we we could resolve edges?

uint16_t SIR(uint16_t delay, uint16_t speed, uint16_t *workingbuffer){ // sudden death!
  static uint8_t flag=0;
  uint16_t cell,cell1,cell2,cell3,sum=0;
  uint16_t *newcells, *cells;
  int16_t x,y;

  if (flag==0) {
    cells=workingbuffer; newcells=&workingbuffer[MAXDIV2];
  }
  else {
    cells=&workingbuffer[MAXDIV2]; newcells=workingbuffer;
  }      

  /* cell = cells[MAXDIV2]>>8; */
  /* newcells[0]=cell<<8; */
  /* if (cell >= k) newcells[0] = recovered<<8;                                                  */
  /* else if ((cell>32 && cell<k)) newcells[0]+=255; */
  /* else if (cell == susceptible) { */
  /*   cell = cells[MAXDIV2]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[CELLLEN]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[1]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[MAXDIV2-CELLLEN]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /* 	} */

  /////

  for (x=1;x<MAXDIV2;x++){
        cell = cells[x]>>8;
	newcells[x]=cells[x];
	if (cell >= k) newcells[x] = recovered<<8;                                                 
	else if (cell>8){
	  newcells[x]+=255; // aging                                                       
	}
	else { // susceptible
	  sum++;
	  y=x-1;
	  cell = cells[y]>>8;
	  y=(x+CELLLEN)%MAXDIV2;
	  cell1 = cells[y]>>8;
	  y=(x+1)%MAXDIV2;
	  cell2 = cells[y]>>8;
	  if ((x-CELLLEN)<0) y=MAXDIV2-(x+CELLLEN);
	  else y=x-CELLLEN;
	  cell3 = cells[y]>>8;

      if ( (cell>0 && cell<k) ||
	   (cell1>0 && cell1<k) ||
	   (cell2>0 && cell2<k) ||
	   (cell3>0 && cell3<k))
	{
	if (rand()%10 < p) newcells[x] = 9<<8;       
	}
	}}

  //SPEED UP: can also pull out loop for 0->celllen and for last celllen

  flag^=0x01;
  return sum;
}

void main(int argc, char **argv)
{
  int cuu=atoi(argv[1]), pll=atoi(argv[2]);
  int x;
  uint16_t xxx[MAX_SAM+12],result;

  srand(time(NULL));
  k=(rand()%240)+8; p=(rand()%10);

  for (x=0;x<MAX_SAM;x++){
    xxx[x]=rand()%65535;
  }

  //  x=1<<8;
  //  printf("%d",x);
        while(1){ 
    result=SIR(10,10,xxx);
    printf("%d\n",result); 
    }
}
