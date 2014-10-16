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

// was CPU.c but now with overlap, uint16_t blah

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include "CPUint.h"
extern int16_t* audio_buffer;
extern uint16_t* adc_buffer;
#define randi() rand()
#else
#include <malloc.h>
#include "CPUint.h"
#include "audio.h"
#define randi() (adc_buffer[9])
extern __IO uint16_t adc_buffer[10];
extern int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));;
//extern u8 *datagenbuffer;
#endif

#include <math.h>

extern u8 wormdir; // worm direction
extern u8 *datagenbuffer;

// changes for non-stacked approach!

u8 thread_stack_count(villager_generic *villager, u8 c) { 
    return (c-1)<=villager->m_stack_pos; 
}

void thread_push(villager_generic *villager, u8 data) {
  if (villager->m_stack_pos<14) // STACK_SIZEE-2
	{
	  villager->m_stack[++villager->m_stack_pos]=data;
	}
}

u8 thread_pop(villager_generic *villager) {
 	if (villager->m_stack_pos>=0 && villager->m_stack_pos<STACK_SIZEE)
	{
	  u8 ret=villager->m_stack[villager->m_stack_pos];
	  villager->m_stack_pos--;
				return ret;
		//		return 0;
	}
	//    printf("errorr\n");
	return 0;   
}

u8 thread_top(villager_generic *villager) {
	if (villager->m_stack_pos>=0 && villager->m_stack_pos<STACK_SIZEE)
	{
	  return villager->m_stack[villager->m_stack_pos];
	}
	return 0;
}

///////////////////////////////////////////////////////////////

inline u16 machine_peek(uint16_t addr) {
  //	return this->m_heap[addr%HEAP_SIZE];
  u16 y;
  y=addr+1;
  return (datagenbuffer[addr]<<8)+datagenbuffer[y];
}

inline u8 machine_p88k(uint16_t addr) {
  //	return this->m_heap[addr%HEAP_SIZE];
  return datagenbuffer[addr];
}


inline void machine_poke(uint16_t addr, u8 data) {
  //	this->m_heap[addr%HEAP_SIZE]=data;
  datagenbuffer[addr]=data;
  //      printf("%d\n",&this->m_memory[addr]);
}


u8 antrulee(u8 dir,u8 inst, u8 rule){
  u8 index;
  // process state from rule 
  // rule in binary
  // inst is index into binary array - so inst must be lower than 0->7 - it is
  index=(rule>>inst)&1;
  if (index==0) return dir-1; 
  else return dir+1; 
  }

u16 runleakystack(villager_generic *villager){
  u8 instr=datagenbuffer[villager->position];
  
      switch(instr%25)
	{
	case NOP: break;
	case ORG: villager->start=villager->position-1; villager->position=villager->start+1; break;
	case EQU: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)==thread_pop(villager)); break;
	case JMP: villager->position=machine_peek(villager->position++); break;
	case JMPZ: if (thread_stack_count(villager,1) && thread_pop(villager)==0) villager->position=machine_peek(villager->position); else villager->position++; break;
    case PSHL: thread_push(villager,machine_peek(villager->position++)); break;
	case PSH: thread_push(villager,machine_p88k(machine_peek(villager->position++))); break;
	case PSHI: thread_push(villager,machine_p88k(machine_peek(machine_peek(villager->position++)))); break;
	case POP: if (thread_stack_count(villager,1)) machine_poke(machine_peek(villager->position++),thread_pop(villager)); break;
    case POPI: if (thread_stack_count(villager,1)) machine_poke(machine_peek(machine_peek(villager->position++)),thread_pop(villager)); break;
    case ADD: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)+thread_pop(villager)); break;
    case SUB: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)-thread_pop(villager)); break;
    case INC: if (thread_stack_count(villager,1)) thread_push(villager,thread_pop(villager)+1); break;
    case DEC: if (thread_stack_count(villager,1)) thread_push(villager,thread_pop(villager)-1); break;
    case AND: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)&thread_pop(villager)); break;
    case OR: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)|thread_pop(villager)); break;
    case XOR: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)^thread_pop(villager)); break;
    case NOT: if (thread_stack_count(villager,1)) thread_push(villager,~thread_pop(villager)); break;
    case ROR: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)>>(machine_p88k(villager->position++)%8)); break;
    case ROL: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)<<(machine_p88k(villager->position++)%8)); break;
    case PIP: 
    {
        u16 d=machine_peek(villager->position++); 
        machine_poke(d,machine_peek(d)+1); 
    } break;
    case PDP: 
    {
        u16 d=machine_peek(villager->position++); 
        machine_poke(d,machine_peek(d)-1); 
    } break;
    case DUP: if (thread_stack_count(villager,1)) thread_push(villager,thread_top(villager)); break;
    case SAY: 
      //      printf("%c",thread_pop(villager));
      //      machine_poke(machine_peek(villager->position++),randi()%255);      
        break;
	case INP:
	  machine_poke(machine_peek(villager->position++),adc_buffer[thread_pop(villager)%10]);      
	  villager->position++;
	  break;

    default : break;
	}
}

