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
#define max_cpus 31

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

#define CPU buffer[offset]
#define DELAY buffer[offset+1]
#define DELC buffer[offset+2]
#define ADDRHI buffer[offset+3]
#define ADDRLO buffer[offset+4]
#define WRAPADDRHI buffer[offset+5]
#define WRAPADDRLO buffer[offset+6]
#define PCADDRHI buffer[offset+7]
#define PCADDRLO buffer[offset+8]
#define BIT81 buffer[offset+9]
#define BIT82 buffer[offset+10]
#define BIT83 buffer[offset+11]
#define STACK buffer[offset+12]

u16 thread_createee(u8 *buffer, u16 address, u16 wrapaddress,u8 which, u8 delay,u16 offset) { // ??? or we steer each of these?
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
  return offset+13+STACK_SIZE;
}

u16 machine_peekkk(u8* buffer, uint16_t addr);
void machine_pokeee(u8* buffer, uint16_t addr, u8 data);

u8 thread_stack_counttt(u8 *buffer, u8 c, u16 offset) { 
  return c<=STACK; // but now we start at 0
}

void thread_pushhh(u8 *buffer, u8 data, u16 offset) {
	if (STACK<STACK_SIZE)
	{
	  buffer[(++STACK)+offset]=data;
	}
}

u8 thread_poppp(u8* buffer, u16 offset) {
 	if (STACK>=1)
	{
		u8 ret=buffer[offset+STACK];
		STACK--;
		return ret;
	}
	//    printf("errorr\n");
	return 0;   
}

u8 thread_toppp(u8 *buffer, u16 offset) {
	if (STACK>=1)
	{
		return buffer[offset+STACK];
	}
	return 0;
}


void thread_runnn(u8* buffer, u16 offset) {
  u8 instr,temp;
  u16 y,addr;
  u8 flag,other;
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  //  printf("running: %d ",this->m_start);
  //  sleep(1);
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

  //  dircalc(biotadir,65536,256);
    CPU=0;
  if (++DELC==DELAY){
  switch(CPU%max_cpus)
    {

    case 0: // :LEAKY STACK! - working!
      //       this->m_pc++; PCADDRHI and LO!

#ifdef PCSIM
    //      printf("CPU: %d\n",this->m_CPU);
      printf("%c",instr);
#endif

      addr=((PCADDRHI<<8)+PCADDRLO);
      instr=machine_peekkk(buffer,addr)>>8;
      addr++;
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
  switch(instr%25)
    {
    case NOP: break;

    case ORG: //this->m_start=this->m_pc-1; this->m_pc=this->m_start+1; break;
      /////
      addr-=1;
      ADDRHI=addr>>8; // hi/lo
      ADDRLO=addr&255;
      addr+=1;
      break;
    case EQU: //if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)==thread_pop(this)); break;
      if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)==thread_poppp(buffer,offset),offset);
      break;
    case JMP: //this->m_pc=this->machine_peek(m,this->m_pc++); break;
      addr=machine_peekkk(buffer,addr++);
      break;      
    case JMPZ:// if (thread_stack_count(this,1) && thread_pop(this)==0) this->m_pc=machine_peek(m,this->m_pc); else this->m_pc++; break;
      if (thread_stack_counttt(buffer,1,offset) && thread_poppp(buffer,offset)==0) machine_peekkk(buffer,addr); else addr++; 
      break;
    case PSHL: //thread_push(this,machine_peek(m,this->m_pc++)); break;
      thread_pushhh(buffer,machine_peekkk(buffer,addr++)>>8,offset);
      break;

    case PSH: ///thread_push(this,machine_peek(m,machine_peek(m,this->m_pc++))); break;
      thread_pushhh(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr++))>>8,offset); break;

    case PSHI://  thread_push(this,machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc++)))); break;
      thread_pushhh(buffer,machine_peekkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr++)))>>8,offset); break;

    case POP:// if (thread_stack_count(this,1)) machine_poke(m,machine_peek(m,this->m_pc++),thread_pop(this)); break;
      if (thread_stack_counttt(buffer,1,offset)) machine_pokeee(buffer,machine_peekkk(buffer,addr),thread_poppp(buffer,offset)); break;

    case POPI: //if (thread_stack_count(this,1)) machine_poke(m,((ADDRHI<<8)+ADDRLO)+machine_peek(m,machine_peek(m,this->m_pc++)),thread_pop(this)); break;
      if (thread_stack_counttt(buffer,1,offset)) machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr++)),thread_poppp(buffer,offset)); break;

    case ADD: //if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)+thread_pop(this)); break;
      if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)+thread_poppp(buffer,offset),offset); break;

    case SUB: if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)-thread_poppp(buffer,offset),offset); break;

    case INC: if (thread_stack_counttt(buffer,1,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)+1,offset); break;

    case DEC: if (thread_stack_counttt(buffer,1,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)-1,offset); break;
      ////////// 
    case AND: if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)&thread_poppp(buffer,offset),offset); 
break;
    case OR: if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)|thread_poppp(buffer,offset),offset); break;
    case XOR: if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)^thread_poppp(buffer,offset),offset); break;
    case NOT: if (thread_stack_counttt(buffer,1,offset)) thread_pushhh(buffer,~thread_poppp(buffer,offset),offset); break;

    case ROR: if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)>>(machine_peekkk(buffer,addr)%8),offset); break;

    case ROL: if (thread_stack_counttt(buffer,2,offset)) thread_pushhh(buffer,thread_poppp(buffer,offset)<<(machine_peekkk(buffer,addr)%8),offset); break;
    case PIP: 
    {
      u16 d=machine_peekkk(buffer,addr++);
      //        machine_poke(m,d,machine_peek(m,d)+1); 

      machine_pokeee(buffer,d,machine_peekkk(buffer,d)+1); 
    } break;
    case PDP: 
    {
      u16 d=machine_peekkk(buffer,addr++); 
      machine_pokeee(buffer,d,machine_peekkk(buffer,d)-1); 
    } break;
    case DUP: if (thread_stack_counttt(buffer,1,offset)) thread_pushhh(buffer,thread_toppp(buffer,offset),offset); break;
    case SAY: 
      //      printf("%c",thread_poppp(buffer));
      //      machine_poke(m,machine_peek(m,buffer->m_pc++),randi()%255);      
        break;
	case INP:
#ifndef PCSIM
	  machine_pokeee(buffer,machine_peekkk(buffer,addr++),adc_buffer[thread_poppp(buffer,offset)%10]);      
#endif
	  addr++;
	  break;

    default : break;
    }
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
    }
  }
}

u16 machine_peekkk(u8* buffer, uint16_t addr) {
  //	return buffer->m_heap[addr%HEAP_SIZE];
  return (buffer[addr]<<8)+buffer[addr+1];
}

void machine_pokeee(u8* buffer, uint16_t addr, u8 data) {
  //	buffer->m_heap[addr%HEAP_SIZE]=data;
  buffer[addr]=data;
}



///////////////////////////////////////////////////////////////

#ifdef PCSIM
int main(void)
{
  u16 x; u16 addr,offset=0;
  u8 buffer[65536];// u16 *testi; u8 *testo;
  srandom(time(0));
  for (x=0;x<65535;x++){
    buffer[x]=randi()%255;
  }

  u16 threads[MAX_FRED];
  u8 flag,other;
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  u16 biotadir[8]={65280,65281,1,257,256,255,65535,65279}; //65536

  // inc threadcount, array of offsets
  for (x=0;x<MAX_FRED;x++){
    addr=randi()%65536;
    threads[x]=offset;
    offset=thread_createee(buffer, addr, addr+randi()%65536,randi()%31,randi()%255,offset);

    //u16 thread_create(u8 *buffer, u16 address, u16 wrapaddress,u8 which, u8 delay,u16 offset) { // ??? or we steer each of these?
  }

  // run threads
  
  
  while(1) {
        for (x=0;x<MAX_FRED;x++){
          thread_runnn(buffer,threads[x]);
    	  }
  }
}
#endif
