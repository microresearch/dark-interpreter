//gcc -DLINUX -std=gnu99 CA.c -o CA

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "leaky.h"

/* TODO D.I:

*/

#define CELLLEN 128

//2- cellular automata: 1d,2d, classic, flexible - re-do all (again states 8 bits)
// working with 64k buffers

// 2d automata works on 128*128 buffer (could be larger)

unsigned char lifer(unsigned char* cellies){
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

    if (sum==3 || (sum+(cells[x]&0x01)==3)) newcells[x]=255;
    else newcells[x]=0;
    printf("%c",cells[x]);
  }
  
  flag^=0x01;
  return sum;
}

////one dimensional - working line by line through buffer
/// celllen for this one could be 255!

unsigned char cel(unsigned char* cells){

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


int main(void)
{
  int x;
  u8 buffer[65536];
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=rand()%255;
  }

    while(1) {
    x=lifer(buffer);
      }
}
