#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "CPUint.h"
#define randi() rand()
#else
#include <malloc.h>
#include "CPUint.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
#endif

#include <math.h>

#ifdef PCSIM
u8 wormdir; // worm direction
#else
extern u8 wormdir;
#endif

#define MAX_FRED 60

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

Based in part on spork factory by Dave Griffiths.

*/

#define CPU_TOTAL 31

#define CPU (buffer[offset])
#define DELAY (buffer[offset+1])
#define DELC (buffer[offset+2])
#define ADDRHI (buffer[offset+3])
#define ADDRLO (buffer[offset+4])
#define WRAPADDRHI (buffer[offset+5])
#define WRAPADDRLO (buffer[offset+6])
#define PCADDRHI (buffer[offset+7])
#define PCADDRLO (buffer[offset+8])
#define BIT81 (buffer[offset+9])
#define BIT82 (buffer[offset+10])
#define BIT83 (buffer[offset+11])
#define STACK (buffer[offset+12])
#define STACKSTART (buffer[offset+13])

u16 thread_create(u8* buffer, u16 address, u16 wrapaddress,u8 which, u8 delay,u16 offset) { // ??? or we steer each of these?
  u8 n;
  buffer[offset]=which; // cpu
  buffer[offset+1]=delay;
  buffer[offset+2]=0;
  buffer[offset+3]=address>>8; // hi/lo
  buffer[offset+4]=address&255;
  buffer[offset+5]=wrapaddress>>8; // hi/lo
  buffer[offset+6]=wrapaddress&255;
  buffer[offset+7]=address>>8; // hi/lo
  buffer[offset+8]=address&255;
  buffer[offset+9]=randi()%255;
  buffer[offset+10]=randi()%255;
  buffer[offset+11]=randi()%255;
  buffer[offset+12]=0; // stack_pos -1????

  for (n=0; n<STACK_SIZE; n++)
      {
	buffer[offset+13+n]=0;
	//	this->m_stack[n]=0;
      }
  return offset+13+n;
}


void thread_runnn(u8* buffer, u16 offset) {
  u8 instr,temp;
  u16 y;
  u8 flag,other;
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  //  printf("running: %d ",this->m_start);
  //  sleep(1);
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

  //  dircalc(biotadir,65536,256);

  if (++DELC==DELAY){

#ifdef PCSIM
    //      printf("CPU: %d\n",this->m_CPU);
    //    printf("%c",machine_peek(m,this->m_pc));

#endif

  switch(CPU)
    {
    case 0: // :LEAKY STACK! - working!
      //      instr=machine_peek(m,this->m_pc);
      //       this->m_pc++; PCADDRHI and LO!
      //      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      //			printf("%d", instr);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////

#ifdef PCSIM
int main(void)
{
  u8 x,threadcount; u16 addr,offset=0;
  u8 buffer[65536];// u16 *testi; u8 *testo;
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=randi()%255;
  }

  u16 threads[MAX_FRED];
  u8 flag,other;
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  u16 biotadir[8]={65280,65281,1,257,256,255,65535,65279}; //65536

  // inc threadcount, array of offsets
  for (x=0;x<MAX_FRED;x++){
    threadcount++;
    addr=randi()%65536;
    threads[x]=offset;
    offset+=thread_create(buffer, addr, addr+randi()%65536,randi()%31,randi()%255,offset);
  }

  // run threads
  

	/*	  while(1) {
          machine_run(m);
	  }*/
}
#endif
