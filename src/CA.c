//gcc -DLINUX -std=gnu99 CA.c -o CA -lm

//CA...runlife, runce, runcell1d
//runhodge(micro), 

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "leaky.h"

/* TODO D.I:

ADD rest of hodge code...

*/

#define CELLLEN 255

//2- cellular automata: 1d,2d, classic, flexible - re-do all (again states 8 bits)
// working with 64k buffers

// 2d automata works on 128*128 buffer (could be larger)

#define MAX_SAM 65536

// Hodge-podge from microbd

void runhodge(unsigned char* cellies){
  int sum=0, numill=0, numinf=0;
  unsigned char q,k1,k2,g;
  static unsigned int x=CELLLEN+1;
  static unsigned char flag=0;
  unsigned char *newcells, *cells;

  if (flag&0x01==0) {
    cells=cellies; newcells=&cellies[MAX_SAM/2];
  }
  else {
    cells=&cellies[MAX_SAM/2]; newcells=cellies;
  }      

  q=cells[0];k1=cells[1];k2=cells[2];g=cells[3];
  if (k1==0) k1=1;
  if (k2==0) k2=1;

  sum=cells[x]+cells[x-1]+cells[x+1]+cells[x-CELLLEN]+cells[x+CELLLEN]+cells[x-CELLLEN-1]+cells[x-CELLLEN+1]+cells[x+CELLLEN-1]+cells[x+CELLLEN+1];

  if (cells[x-1]==(q-1)) numill++; else if (cells[x-1]>0) numinf++;
  if (cells[x+1]==(q-1)) numill++; else if (cells[x+1]>0) numinf++;
  if (cells[x-CELLLEN]==(q-1)) numill++; else if (cells[x-CELLLEN]>0) numinf++;
  if (cells[x+CELLLEN]==(q-1)) numill++; else if (cells[x+CELLLEN]>0) numinf++;
  if (cells[x-CELLLEN-1]==q) numill++; else if (cells[x-CELLLEN-1]>0) numinf++;
  if (cells[x-CELLLEN+1]==q) numill++; else if (cells[x-CELLLEN+1]>0) numinf++;
  if (cells[x+CELLLEN-1]==q) numill++; else if (cells[x+CELLLEN-1]>0) numinf++;
  if (cells[x+CELLLEN+1]==q) numill++; else if (cells[x+CELLLEN+1]>0) numinf++;

  if(cells[x] == 0)
    newcells[x%(MAX_SAM/2)] = floor(numinf / k1) + floor(numill / k2);
  else if(cells[x] < q - 1)
    newcells[x%(MAX_SAM/2)] = floor(sum / (numinf + 1)) + g;
  else
    newcells[x%(MAX_SAM/2)] = 0;

  if(newcells[x%(MAX_SAM/2)] > q - 1)
    newcells[x%(MAX_SAM/2)] = q - 1;

  x++;
  printf("%c",newcells[x]);
  if (x>((MAX_SAM/2)-CELLLEN-1)) {
    x=CELLLEN+1;
    flag^=0x01;
  }
}


unsigned char runlife(unsigned char* cellies){
  unsigned char sum;
  unsigned int x;
  static unsigned char flag=0;
  unsigned char *newcells, *cells;

  if ((flag&0x01)==0) {
    cells=(unsigned char*)cellies; newcells=(unsigned char*)cellies+16384;
  }
  else {
    cells=(unsigned char*)cellies+16384; newcells=(unsigned char*)cellies;
  }      

  for (x=CELLLEN+1;x<(16384-CELLLEN-1);x++){
    sum=(cells[x-1]&0x01)+(cells[x+1]&0x01)+(cells[x-CELLLEN]&0x01)+(cells[x+CELLLEN]&0x01)+(cells[x-CELLLEN-1]&0x01)+(cells[x-CELLLEN+1]&0x01)+(cells[x+CELLLEN-1]&0x01)+(cells[x+CELLLEN+1]&0x01);

    /*    if (sum==3 || (sum+(cells[x]&0x01)==3)) newcells[x]=255; /// 
	  else newcells[x]=0;*/
    if ((cells[x]&1)==1 && sum<2) newcells[x]=0;
    else if ((cells[x]&1)==1 && sum>3) newcells[x]=0;
    else if ((cells[x]&1)==0 && sum==3) newcells[x]=255;
    else newcells[x]=cells[x];

    printf("%c",cells[x]);
  }
  
  flag^=0x01;
  return sum;
}

////one dimensional - working line by line through buffer
/// celllen for this one could be 255!

unsigned char runcel(unsigned char* cells){

  static unsigned char l=0; unsigned char cell, state, res;
  unsigned char rule=cells[0];
  res=0;
  l++;
  l%=CELLLEN;

  for (cell = 1; cell < CELLLEN; cell++){ 
      state = 0;
      if (cells[cell + 1+ (l*CELLLEN)]>128)
	state |= 0x4;
      if (cells[cell+(CELLLEN*l)]>128)
	state |= 0x2;
      if (cells[cell - 1 +(CELLLEN*l)]>128)
	state |= 0x1;
                     
      if ((rule >> state) & 1){
	res += 1; 
	cells[cell+(((l+1)%CELLLEN)*CELLLEN)] = 255;
      }
      else{
	cells[cell+(((l+1)%CELLLEN)*CELLLEN)] = 0;
      } 
      printf("%c",cells[cell+(l*CELLLEN)]);

  }
  return res;
}

unsigned char *table;

///how much memory does table take?

void inittable(unsigned char r, unsigned char k, int rule){
  int max, z, summ;

  free(table);
  max = (k-1)*((r*2)+1);
  table= (unsigned char *)malloc(max+1);
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

// 1d with rules

int runcell1d(unsigned char *cells){

  static unsigned char l=0; unsigned char cell; signed int sum,ssum,z,zz;
  int radius=3, k=4;//k=states

  l++; ssum=0;
  l%=CELLLEN;
  for (cell = 0; cell < CELLLEN; cell++){ 
    sum=0;
    
    // sum of cells in radius - not looping!
    for (z=-radius;z<radius;z++){
      zz=cell+z;
      if (zz>=CELLLEN) zz=zz-CELLLEN;
      if (zz<0) zz=CELLLEN+zz;
      sum+=cells[zz+(l*CELLLEN)]%k;
      // printf("%d : %d\n",zz,sum);  
    }
    cells[cell+(((l+1)%CELLLEN)*CELLLEN)]= table[sum]; 
    printf("%c",table[sum]<<4);

    ssum+=sum;
  }
  return ssum;
}


int main(void)
{
  int x;
  u8 buffer[65536];
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=rand()%255;
  }
  inittable(3,4,rand()%65536); //radius,states(k),rule - init with cell starter

      while(1) {
	//    x=lifer(buffer);
	runhodge(buffer);
    }
}
