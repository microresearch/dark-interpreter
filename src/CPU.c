// was leaky.c
//gcc -DLINUX -std=gnu99 leaky.c -o leaky

#include "leaky.h"

#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#else
#include <malloc.h>
#include <math.h>
#endif
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


/* TODO D.I:

- for each thread heap points into buffer - DONE

- stack of diff cpus/threads which can be added/subbed to by knob - TODO(each thread has m_CPU)
- re-init (per thread?)


- cpus link to grains...

push and pop CPUs onto the thread stack. also some variable/array for
exchange of grains/cpu start-end positions

- control of leakiness - add in mutation, cross-pollination, infection, plague, death so cpus can also infect/take over each other (meta)

- redo for overlap thread code (but should instruction pointer be int
  or?) how tricky - do we need mod?

- for wormcode steering buffer, also other cpus modded to read/write
  to that extra buffer - where to pass ref??? some kind of window

- add SPL to redcode

- add hodge.c

TOTAL so far: 23 CPUs

*/

//int HEAP_SIZE=20400; // see above (80*255)

void thread_create(thread *this, int start, uint8_t which) {
    this->m_CPU=which;
    this->m_start=start; // or overlap?
    this->m_pc=rand()%255;
    this->m_reg8bit1=rand()%255;
    this->m_reg8bit2=rand()%255;
    this->m_reg8bit3=rand()%255;
    this->m_reg8bit4=rand()%255;
    this->m_stack_pos=-1;
    //this->m_stack=(u8*)malloc(STACK_SIZE);

    for (int n=0; n<STACK_SIZE; n++)
      {
	this->m_stack[n]=0;
      }
}

u8 thread_peek(thread* this, machine *m, u8 addr) {
	return machine_peek(m,this->m_start+addr);
}

void thread_poke(thread* this, machine *m, u8 addr, u8 data) {
	machine_poke(m,this->m_start+addr,data);
}

u8 antrule(u8 dir,u8 inst, u8 rule){
  u8 index,x;
  // process state from rule 
  // rule in binary
  // inst is index into binary array - so inst must be lower than 0->7 - it is
  index=(rule>>inst)&1;
  if (index==0) return dir-1; 
  else return dir+1; 

}


void thread_run(thread* this, machine *m) {
  u8 instr;	
  u8 flag,other;
  u8 biotadir[8]={239,240,1,17,16,15,254,238}; 
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  //  printf("running: %d ",this->m_start);
  //  sleep(1);

	//			printf("%c",instr);
	// SWITCH for m_CPU!
  switch(this->m_CPU)
    {
    case 0: // :LEAKY STACK! - working!
      instr=thread_peek(this,m,this->m_pc);
      this->m_pc++;
      //			printf("%d", instr);
      switch(instr%25)
	{
	  
    case NOP: break;
    case ORG: this->m_start=this->m_start+this->m_pc-1; this->m_pc=1; break;
    case EQU: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)==thread_pop(this)); break;
    case JMP: this->m_pc=thread_peek(this,m,this->m_pc++); break;
    case JMPZ: if (thread_stack_count(this,1) && thread_pop(this)==0) this->m_pc=thread_peek(this,m,this->m_pc); else this->m_pc++; break;
    case PSHL: thread_push(this,thread_peek(this,m,this->m_pc++)); break;
    case PSH: thread_push(this,thread_peek(this,m,thread_peek(this,m,this->m_pc++))); break;
    case PSHI: thread_push(this,thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc++)))); break;
	case POP: if (thread_stack_count(this,1)) thread_poke(this,m,thread_peek(this,m,this->m_pc++),thread_pop(this)); break;
    case POPI: if (thread_stack_count(this,1)) thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc++)),thread_pop(this)); break;
    case ADD: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)+thread_pop(this)); break;
    case SUB: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)-thread_pop(this)); break;
    case INC: if (thread_stack_count(this,1)) thread_push(this,thread_pop(this)+1); break;
    case DEC: if (thread_stack_count(this,1)) thread_push(this,thread_pop(this)-1); break;
    case AND: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)&thread_pop(this)); break;
    case OR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)|thread_pop(this)); break;
    case XOR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)^thread_pop(this)); break;
    case NOT: if (thread_stack_count(this,1)) thread_push(this,~thread_pop(this)); break;
    case ROR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)>>(thread_peek(this,m,this->m_pc++)%8)); break;
    case ROL: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)<<(thread_peek(this,m,this->m_pc++)%8)); break;
    case PIP: 
    {
        u8 d=thread_peek(this,m,this->m_pc++); 
        thread_poke(this,m,d,thread_peek(this,m,d)+1); 
    } break;
    case PDP: 
    {
        u8 d=thread_peek(this,m,this->m_pc++); 
        thread_poke(this,m,d,thread_peek(this,m,d)-1); 
    } break;
    case DUP: if (thread_stack_count(this,1)) thread_push(this,thread_top(this)); break;
    case SAY: 
      //      printf("%c",thread_pop(this));
      //      thread_poke(this,m,thread_peek(this,m,this->m_pc++),rand()%255);      
        break;
	case INP:
	  thread_poke(this,m,thread_peek(this,m,this->m_pc++),rand()%255);      
	  break;

    default : break;
	}
      break;

///////////////////////////////////////////////////////////////

    case 1:// BIOTA see: http://c2.com/cgi/wiki?BiotaLanguage
      instr=thread_peek(this,m,this->m_pc);
      flag=0;
      //      printf("instr %d ",instr);
      switch(instr%8)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  this->m_reg8bit2+=biotadir[this->m_reg8bit1%8];
	  if (thread_peek(this,m,this->m_reg8bit2)==0) flag=1;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  this->m_reg8bit2+=biotadir[(this->m_reg8bit1+4)%8];
	  if (thread_peek(this,m,this->m_reg8bit2)==0) flag=1;
	  break;
	case 2:
	  //* t -- turn DC right 45 degrees
	  this->m_reg8bit1+=1;
	  break;
	case 3:
	  //* u -- unturn: turn DC left 45 degrees
	  this->m_reg8bit1-=1;
	  break;
	case 4:
	  //    * g -- go to a non-empty character ahead (tries to move DC straight ahead, then right and left 45 degrees, then 90, then 135, then back).
	  this->m_reg8bit2+=biotadir[this->m_reg8bit1%8];
	  if (thread_peek(this,m,this->m_reg8bit2)==0) {
	    this->m_reg8bit2-=biotadir[this->m_reg8bit1%8]; // go back
	    this->m_reg8bit2+=biotadir[(this->m_reg8bit1+1)%8]; // right 45
	    if (thread_peek(this,m,this->m_reg8bit2)==0) {
	      this->m_reg8bit2-=biotadir[(this->m_reg8bit1+1)%8]; // go back
	      this->m_reg8bit2+=biotadir[(this->m_reg8bit1-1)%8]; // left 45
	      if (thread_peek(this,m,this->m_reg8bit2)==0) {
		this->m_reg8bit2-=biotadir[(this->m_reg8bit1-1)%8]; // go back
		this->m_reg8bit2+=biotadir[(this->m_reg8bit1+2)%8]; // right 90
		if (thread_peek(this,m,this->m_reg8bit2)==0) {
		  this->m_reg8bit2-=biotadir[(this->m_reg8bit1+2)%8]; // go back
		  this->m_reg8bit2+=biotadir[(this->m_reg8bit1-2)%8]; // left 90
		  if (thread_peek(this,m,this->m_reg8bit2)==0) {
		    this->m_reg8bit2-=biotadir[(this->m_reg8bit1-2)%8]; // go back
		    this->m_reg8bit2+=biotadir[(this->m_reg8bit1+3)%8]; // right 135
		  if (thread_peek(this,m,this->m_reg8bit2)==0) {
		    this->m_reg8bit2-=biotadir[(this->m_reg8bit1-3)%8]; // go back
		    this->m_reg8bit2+=biotadir[(this->m_reg8bit1-3)%8]; // left 135
		  if (thread_peek(this,m,this->m_reg8bit2)==0) {
		    this->m_reg8bit2-=biotadir[(this->m_reg8bit1-3)%8]; // go back
		    this->m_reg8bit2+=biotadir[(this->m_reg8bit1+4)%8]; // back
		  }
		  }
		  }
		}
	      }
	    }
	  }
	  break;
	case 5:
	  //    * c -- clear character at DC. Fails if already empty.
	  if (thread_peek(this,m,this->m_reg8bit2)!=0) thread_poke(this,m,this->m_reg8bit2,0); 
	  else flag=1;
	  break;
	case 6:
	  //    * d -- duplicate the current data into the cell left of the DC. Fails if source cell is empty or target cell is non-empty.
	  // left is -2

	  if (thread_peek(this,m,this->m_reg8bit2)==0 || (thread_peek(this,m,this->m_reg8bit2+(this->m_reg8bit2+((this->m_reg8bit1-4)%8))))!=0) flag=1;
	  else thread_poke(this,m,this->m_reg8bit2+(this->m_reg8bit2+((this->m_reg8bit1-4)%8)),thread_peek(this,m,this->m_reg8bit2));
	  break;
	case 7:
	  //    * . -- no-op, a non-empty do nothing. 
	  break;
	}

      // - this->m_pc turns (where?) when it finds an empty location or a failing instruction
      // m_reg8bit3 is direction
      if (thread_peek(this,m,this->m_pc)==0 || flag==1){
	this->m_reg8bit3-=1;
      }
      else this->m_pc+=biotadir[this->m_reg8bit3%8];
      //      printf("%c",this->m_pc);
      //      break;
///////////////////////////////////////////////////////////////

    case 2:
      // brainfuck: add in input and output???
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%6)
	{
	case 0:
	  this->m_reg8bit1++;
	  this->m_pc++;
	  break;
	case 1:
	  this->m_reg8bit1--;
	  this->m_pc++;
	  break;
	case 2:
	  //	    cells[omem]=cells[omem]++; 
	  thread_poke(this,m,this->m_reg8bit1,thread_peek(this,m,this->m_reg8bit1)+1);
	  this->m_pc++;
	  break;
	case 3:
	  thread_poke(this,m,this->m_reg8bit1,thread_peek(this,m,this->m_reg8bit1)-1);
	  this->m_pc++;
	  break;
	case 4:
	  this->m_reg8bit2++;
	  if (this->m_reg8bit2>=16) this->m_reg8bit2=0;
	  this->m_stack[this->m_reg8bit2]= this->m_pc;
	  this->m_pc++;
	  break;
	case 5:
	  if (thread_peek(this,m,this->m_reg8bit1)!=0) this->m_pc=this->m_stack[this->m_reg8bit2]-1;
	  this->m_reg8bit2-=1;
	  if (this->m_reg8bit2==0) this->m_reg8bit2=16;
	  break;
	}
      //      printf("%c",this->m_pc);
    
///////////////////////////////////////////////////////////////

    case 3:
      // masque red death: add in input and output???
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%7){
      case 0:
	if (this->m_reg8bit2==12){
	  thread_poke(this,m,this->m_pc+1,thread_peek(this,m,this->m_pc));
	  if (this->m_pc==255) this->m_reg8bit2=13;
	  this->m_pc++;
	}
	else this->m_pc++;
	break;
      case 1:
	if (this->m_reg8bit2==13){
	  this->m_reg8bit1++;
	  //	  thread_poke(this,m,this->m_reg8bit1,thread_peek(this,m,this->m_pc)); READ IN! TODO!
	  //	  this->m_pc++;
	}
	else this->m_pc++;
	break;
      case 2:
	this->m_reg16bit1++;
	if ((this->m_reg16bit1%60)==0){
	  this->m_reg8bit2++;
	  thread_poke(this,m,this->m_reg8bit1,thread_peek(this,m,this->m_reg8bit1)^255);
	}
	else this->m_pc++;
	break;
	case 3:
	  //	  seven rooms: divide cellspace into 7 - 7 layers with filter each: TODO
	  break;
	case 4:
	  thread_poke(this,m,this->m_reg8bit1-1,thread_peek(this,m,this->m_reg8bit1-1)^255);
	  thread_poke(this,m,this->m_reg8bit1+1,thread_peek(this,m,this->m_reg8bit1+1)^255);
	  this->m_pc++;
	  break;
	case 5:
	  flag=rand()%4;
	  if (flag==0) 	  this->m_reg8bit2++;
	  if (flag==1) 	  this->m_reg8bit2--;
	  if (flag==2) 	  this->m_reg8bit2+=16;
	  if (flag==3) 	  this->m_reg8bit2-=16;
	  this->m_pc++;
	  break;
	case 6:
	  //cells[omem+1]=adcread(3); rEADIN TODO
	  this->m_pc++;
	  break;
	}
      //      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 4:
      // plague: add in input and output???
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%4){
      case 0:
	thread_poke(this,m,this->m_pc,255);
	thread_poke(this,m,this->m_pc+1,255);
	this->m_pc+=2;
	break;
      case 1:
	if (thread_peek(this,m,this->m_pc)<128){
	  thread_poke(this,m,this->m_pc-1,thread_peek(this,m,this->m_pc));
	  thread_poke(this,m,this->m_pc+1,thread_peek(this,m,this->m_pc));
	}
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      case 2:
	thread_poke(this,m,this->m_pc-1,0);
	thread_poke(this,m,this->m_pc+1,0);
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      case 3:
	if ((thread_peek(this,m,this->m_pc)%0x03)==1) this->m_reg8bit3+=4;
	else this->m_reg8bit3*=thread_peek(this,m,this->m_pc)>>4;
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      }
	if (thread_peek(this,m,this->m_pc)==255) this->m_reg8bit3+=4;
	//      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 5:
      // first from micro: add in input and output???
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%14){
      case 0:
	this->m_reg8bit1++;
	this->m_pc++;
	break;
      case 1:
	this->m_reg8bit1--;
	this->m_pc++;
	break;
      case 2:
	thread_poke(this,m,this->m_reg8bit1,thread_peek(this,m,this->m_reg8bit1)+1);
	this->m_pc++;
	break;
      case 3:	  
	thread_poke(this,m,this->m_reg8bit1,thread_peek(this,m,this->m_reg8bit1)-1);
	this->m_pc++;
	break;
      case 4:	  
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)+1);
	this->m_pc++;
	break;
      case 5:	  
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)-1);
	this->m_pc++;
	break;
      case 6:	  
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)<<1);
	this->m_pc++;
	break;
      case 7:	  
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)<<2);
	this->m_pc++;
	break;
      case 8:	  
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)<<3);
	this->m_pc++;
	break;
      case 9:	  
	if (thread_peek(this,m,this->m_pc+1)==0) this->m_pc=this->m_reg8bit1;
	//	this->m_pc++;
	break;
      case 10:	  
	if (thread_peek(this,m,this->m_pc+1)<128) this->m_pc+=thread_peek(this,m,this->m_pc+1);
	else 	this->m_pc++;
	break;
      case 11:	  
	if (thread_peek(this,m,this->m_pc-1)<128) thread_poke(this,m,this->m_pc+1,thread_peek(this,m,this->m_pc));
	this->m_pc++;
	break;
      case 12:	  
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc+1));
	this->m_pc++;
	break;
      case 13:	  
	this->m_pc++;
	break;
      }
      //      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 6:
/* "real" corewars redcode SPL 

TODO: SPL for branchings... - add new thread at address x

(add new threads till max and change leaks only for total threads)

REF: http://vyznev.net/corewar/guide.html#start_instr

http://www.koth.org/info/akdewdney/images/Redcode.jpg

 */
      instr=thread_peek(this,m,this->m_pc);
      switch(instr%30){
      case 0:
	// MOV # to direct.
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2),thread_peek(this,m,this->m_pc+1));
	this->m_pc+=3;
	break;
      case 1:	  
	// MOV indirect to direct.
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2),thread_peek(this,m,thread_peek(this,m,this->m_pc+1)));
	this->m_pc+=3;
	break;
      case 2:
	// MOV # to indirect.
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)),thread_peek(this,m,this->m_pc+1));
	this->m_pc+=3;
	break;
      case 3:
	//MOV indirect to indirect
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)),thread_peek(this,m,thread_peek(this,m,this->m_pc+1)));
	this->m_pc+=3;
	break;
      case 4:
	//ADD # to direct
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2),thread_peek(this,m,this->m_pc+1)+thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 5:
	// ADD indirect to direct.
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2),thread_peek(this,m,thread_peek(this,m,this->m_pc+1)+thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))));
	this->m_pc+=3;
	break;
      case 6:
	// ADD # to indirect.
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)),thread_peek(this,m,this->m_pc+1)+thread_peek(this,m,thread_peek(this,m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 7:
	//ADD indirect to indirect
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)),thread_peek(this,m,thread_peek(this,m,this->m_pc+1))+thread_peek(this,m,thread_peek(this,m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 8:
	//SUB # to direct
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2),thread_peek(this,m,this->m_pc+1)-thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 9:
	// indirect to direct.
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2),thread_peek(this,m,thread_peek(this,m,this->m_pc+1)-thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))));
	this->m_pc+=3;
	break;
      case 10:
	// # to indirect.
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)),thread_peek(this,m,this->m_pc+1)-thread_peek(this,m,thread_peek(this,m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 11:
	// indirect to indirect
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)),thread_peek(this,m,thread_peek(this,m,this->m_pc+1))-thread_peek(this,m,thread_peek(this,m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 12:
	// jmp to direct
	this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	break;
      case 13:
	// jmp to indirect
	this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	break;
      case 14:
	// JMZdirect to direct
	if (thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))==0) 	this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 15:
	// JMZdirect to indirect
	if (thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))==0) 	this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 16:	
	// JMZindirect to direct
	if (thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)))==0) 	this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 17:
	// JMZindirect to indirect
	if (thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)))==0) 	this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 18:
	// JMGdirect to direct
	if (thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))>0) 	this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 19:
	// JMGdirect to indirect
	if (thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))>0) 	this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 20:	
	// JMGindirect to direct
	if (thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)))>0) 	this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 21:
	// JMGindirect to indirect
	if (thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)))>0) 	this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 22:
	// Dec, jump if zero... sub 1 from b. jump to a if b is zero
	// DJZ dir to dir
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2), thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))-1);
	if (thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))==0)	  this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	else this->m_pc+=3;
	break;
      case 23:
	// DJZ dir to indir
	thread_poke(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2), thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))-1);
	if (thread_peek(this,m,this->m_pc+(unsigned char)thread_peek(this,m,this->m_pc+2))==0)	  this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	else this->m_pc+=3;
	break;
      case 24:
	// DJZ indir to dir
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)), thread_peek(this,m,thread_peek(this,m,this->m_pc+2)-1));
	if (thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)))==0)this->m_pc+=(unsigned char)thread_peek(this,m,this->m_pc+1);
	else this->m_pc+=3;
	break;
      case 25:
	// DJZ indir to indir
	thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)), thread_peek(this,m,thread_peek(this,m,this->m_pc+2)-1));
	if (thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+2)))==0) this->m_pc=thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->m_pc+1)));
	else this->m_pc+=3;
	break;
      case 26:
	this->m_pc+=3;
	break;
      }
      //      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 7:
      // SIR: add in input and output??? untested for audio but...
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%4){
      case 0:
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
	if (thread_peek(this,m,this->m_pc+1)<128) thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)+1);
	this->m_pc++;
	break;
      case 1:	  
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128))    if (rand()%10 < 4) cells[IP] = dead;     
	if (thread_peek(this,m,this->m_pc+1)<128)  if (rand()%10 < 4) thread_poke(this,m,this->m_pc,255);     
	this->m_pc++;
	break;
      case 2:
	//  if (cells[(IP+1)] >= 128) cells[IP] = recovered;
	if (thread_peek(this,m,this->m_pc+1)>=128)  thread_poke(this,m,this->m_pc,129);
	this->m_pc++;
	break;
      case 3:
	if (thread_peek(this,m,this->m_pc+1)==0){
	  if ((thread_peek(this,m,this->m_pc-1)>0 && thread_peek(this,m,this->m_pc-1)<128) ||
	      (thread_peek(this,m,this->m_pc+1)>0 && thread_peek(this,m,this->m_pc+1)<128))
	    {
	if (rand()%10 < 4) thread_poke(this,m,this->m_pc,129);
	    }
	}
	this->m_pc++;
	break;
      }

///////////////////////////////////////////////////////////////

    case 8:
      // WOrmcode: but was with overlaps and also we need extra buffer to steer!
      //      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);

      //      this->m_reg8bit3=biotadir[rand()%8]; // replace with buffer steering TODO or:
      this->m_reg8bit3=biotadir[thread_peek(this,m,this->m_pc)%8];
      this->m_pc+=this->m_reg8bit3;
      instr=thread_peek(this,m,this->m_pc);
      switch(instr%12){
      case 0:
	break;
      case 1:
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)+1);
	break;
      case 2:
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)-1);
	break;
      case 3:
	this->m_pc+=thread_peek(this,m,this->m_pc+this->m_reg8bit3);
	break;
      case 4:
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)+thread_peek(this,m,this->m_pc+this->m_reg8bit3));
	break;
      case 5:
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)-thread_peek(this,m,this->m_pc+this->m_reg8bit3));
	break;
      case 6:
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)<<1);
	break;
      case 7:
	thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc)>>1);
	break;
      case 8:
	if (thread_peek(this,m,this->m_pc+(this->m_reg8bit3*2))==0) this->m_pc+=thread_peek(this,m,this->m_pc+this->m_reg8bit3);
	break;
      case 9:
	flag=thread_peek(this,m,this->m_pc);
	thread_poke(this,m,this->m_pc-this->m_reg8bit3,flag);
	thread_poke(this,m,this->m_pc+this->m_reg8bit3,flag);
	break;
      case 10:
	thread_push(this,thread_peek(this,m,this->m_pc+this->m_reg8bit3));
	break;
      case 11:
	thread_poke(this,m,this->m_pc+this->m_reg8bit3,thread_pop(this));
	break;
      }
      //      printf("%c",instr);

///////////////////////////////////////////////////////////////

    case 9:
      // stack machine a la: http://www.ece.cmu.edu/~koopman/stack_computers/sec3_2.html#321
      // but sans return stack
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%15){
      case 0:
	flag=thread_pop(this);
	thread_poke(this,m,thread_pop(this),flag);
	this->m_pc++;
	break;
      case 1:
	thread_push(this,thread_pop(this)+thread_pop(this));
	this->m_pc++;
	break;
      case 2:
	thread_push(this,thread_pop(this)-thread_pop(this));
	this->m_pc++;
	break;
      case 3:
	thread_push(this,thread_peek(this,m,thread_pop(this)));
	this->m_pc++;
	break;
      case 4:
	thread_push(this,thread_pop(this)&thread_pop(this));
	this->m_pc++;
	break;
      case 5:
	thread_pop(this);
	this->m_pc++;
	break;
      case 6:
	flag=thread_pop(this);
	thread_push(this,flag);
	thread_push(this,flag);
	this->m_pc++;
	break;
      case 7:
	thread_push(this,thread_pop(this)|thread_pop(this));
	this->m_pc++;
	break;
      case 8:
	//Push a copy of the second element on the stack, N1, onto the top of the stack
	this->m_reg8bit1=thread_pop(this);
	this->m_reg8bit2=thread_pop(this);
	thread_push(this,this->m_reg8bit2);
	thread_push(this,this->m_reg8bit1);
	thread_push(this,this->m_reg8bit2);
	this->m_pc++;
	break;
      case 9:
	this->m_reg8bit1=thread_pop(this);
	this->m_reg8bit2=thread_pop(this);
	thread_push(this,this->m_reg8bit1);
	thread_push(this,this->m_reg8bit2);
	this->m_pc++;
	break;
      case 10:
	thread_push(this,thread_pop(this)^thread_pop(this));
	this->m_pc++;
	break;
      case 11:
	if (thread_pop(this)==0) this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	else 	this->m_pc++;
	break;
      case 12:
	//sub call
	thread_push(this,this->m_pc);
	this->m_pc=thread_peek(this,m,thread_peek(this,m,this->m_pc+1));
	break;
      case 13:
	//sub return
	this->m_pc=thread_pop(this);
	break;
      case 14:
	thread_push(this,thread_peek(this,m,this->m_pc+1));
	this->m_pc++;
	break;
      }
      //      printf("%c",this->m_pc);

///////////////////////////////////////////////////////////////

    case 10:
      // befunge: http://en.wikipedia.org/wiki/Befunge
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%30){
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
	thread_push(this,instr%30);
	break;
      case 10:
	thread_push(this,thread_pop(this)-thread_pop(this));
	break;
      case 11:
	flag=thread_pop(this);
	thread_push(this,thread_pop(this)-flag);
	break;
      case 12:
	thread_push(this,thread_pop(this)*thread_pop(this));
	break;
      case 13:
	flag=thread_pop(this);
	if (flag!=0) thread_push(this,thread_pop(this)/flag);
	break;
      case 14:
	flag=thread_pop(this);
	if (flag!=0) thread_push(this,thread_pop(this)%flag);
	break;
      case 15:
	flag=thread_pop(this);
	if (flag==0) thread_push(this,1);
	else thread_push(this,0);
	break;
      case 16:
	flag=thread_pop(this);
	if (thread_pop(this)>flag) thread_push(this,1);
	else thread_push(this,0);
	break;
      case 17: // right
	this->m_reg8bit3=2;
	break;
      case 18: // left
	this->m_reg8bit3=6;
	break;
      case 19: // up
	this->m_reg8bit3=0;
	break;
      case 20: // down
	this->m_reg8bit3=4;
	break;
      case 21:
	this->m_reg8bit3=(rand()%4)*2;
	break;
      case 22:
	if (thread_pop(this)==0)	this->m_reg8bit3=2;
	else 	this->m_reg8bit3=6;
	break;
      case 23:
	if (thread_pop(this)==0)	this->m_reg8bit3=4;
	else 	this->m_reg8bit3=0;
	break;
      case 24:
	flag=thread_pop(this);
	thread_push(this,flag);
	thread_push(this,flag);
	break;
      case 25:
	this->m_reg8bit1=thread_pop(this);
	this->m_reg8bit2=thread_pop(this);
	thread_push(this,this->m_reg8bit1);
	thread_push(this,this->m_reg8bit2);
	break;
      case 26:
	thread_pop(this);
	break;
      case 27:
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      case 28:
	thread_poke(this,m,(thread_pop(this)&16)*(thread_pop(this)&16),thread_pop(this));
	break;
      case 29:
	thread_push(this,thread_peek(this,m,thread_pop(this)&16)*(thread_pop(this)&16));
	break;
      }
      this->m_pc+=biotadir[this->m_reg8bit3%8];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 11:
      // kind of langton's ant as CPU - each thread has direction and
      // changes state of cell according to stack?
      // turn right/left/flip and move on
      instr=thread_peek(this,m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%14)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	  thread_push(this,instr);
	  break;
	case 12:
	  this->m_reg8bit3+=2;
	  thread_poke(this,m,this->m_pc,instr^thread_pop(this));
	  break;
	case 13:
	  this->m_reg8bit3-=2;
	  thread_poke(this,m,this->m_pc,instr^thread_pop(this));
	  break;
	}
      this->m_pc+=biotadir[this->m_reg8bit3%8];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 12:
      // turmites code: turmite has state,direction,position...
      // reg8bit1 is state. reg8bit2 is direction index.
      instr=thread_peek(this,m,this->m_pc);
      thread_poke(this,m,this->m_pc,instr+this->m_reg8bit1);
      //delta = dmove[(instr - this->reg8bit1) & 0xf];
      flag=instr - this->m_reg8bit1;
      //tm->dir = (tm->dir + delta) & 3;
      this->m_reg8bit2=(this->m_reg8bit2+flag)&8;
      //do move and wrap
      this->m_pc+=biotadir[this->m_reg8bit2];
      // finally
      this->m_reg8bit1 += deltastate[instr%16];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 13:
      // linear CA/life code. copies to 128 steps ahead new state and keeps going(?)
      instr=thread_peek(this,m,this->m_pc);
      other=(thread_peek(this,m,this->m_pc-1)&1)+(thread_peek(this,m,this->m_pc+1)&1)+(thread_peek(this,m,this->m_pc-32)&1)+(thread_peek(this,m,this->m_pc+32)&1)+(thread_peek(this,m,this->m_pc-31)&1)+(thread_peek(this,m,this->m_pc-33)&1)+(thread_peek(this,m,this->m_pc+31)&1)+(thread_peek(this,m,this->m_pc+33)&1);

    if ((instr&1)==1 && other<2) flag=0;
    else if ((instr&1)==1 && other>3) flag=0;
    else if ((instr&1)==0 && other==3) flag=instr+1;
    else flag=instr;

    thread_poke(this,m,this->m_pc+128,flag);
      this->m_pc++; 
      //      printf("%c",instr);
      break;

    case 14:
      // ants revisited... how to do multiple ants using stack?
      // read cell//process rule string//change cell//move ant
      // rule string is from cell[0]
      instr=thread_peek(this,m,this->m_pc);
      thread_poke(this,m,this->m_pc,instr+1);
      this->m_reg8bit1=antrule(this->m_reg8bit1,instr%8,thread_peek(this,m,0));//last is rule
      this->m_pc+=biotadir[this->m_reg8bit1%8];
      //      printf("%c",this->m_pc);
      break;

    case 15:
      // second CA from CA.c
      flag = 0;
      instr=thread_peek(this,m,this->m_pc);
      if (thread_peek(this,m,this->m_pc+1)>128)	flag |= 0x4;
      if (instr>128) flag |= 0x2;
      if (thread_peek(this,m,this->m_pc-1)>128)	flag |= 0x1;
                     
      if ((thread_peek(this,m,0) >> flag) & 1)	thread_poke(this,m,this->m_pc+128,instr+129);
      else thread_poke(this,m,this->m_pc+128,instr-129); // or we stay with 255 and 0 as poked

      this->m_pc++; 
      if (this->m_pc==0) this->m_pc=1;
      //      printf("%c",instr);
      break;

    case 16:
      // **TODO** port of hodgenet.c - but we need larger 255*128 cellspace in two halves...????
      break;
    case 17:
      // start generic - add (add/sub/zero/copy/invert/swap)
      instr=thread_peek(this,m,this->m_pc);
      thread_poke(this,m,this->m_pc,instr+1);
      break;
    case 18:
      // generic - sub (add/sub/zero/copy/invert/swap)
      instr=thread_peek(this,m,this->m_pc);
      thread_poke(this,m,this->m_pc,instr-1);
      break;
    case 19:
      // generic - zero (add/sub/zero/copy/invert/swap)
      thread_poke(this,m,this->m_pc,0);
      break;
    case 20:
      // generic - copy (add/sub/zero/copy/invert/swap)
      thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc+1));
      break;
    case 21:
      // generic - copy (add/sub/zero/copy/invert/swap)
      thread_poke(this,m,this->m_pc,thread_peek(this,m,this->m_pc^255));
      break;
    case 22:
      // generic - swap (add/sub/zero/copy/invert/swap)
      instr=thread_peek(this,m,this->m_pc+1);
      thread_poke(this,m,this->m_pc+1,thread_peek(this,m,this->m_pc));
      thread_poke(this,m,this->m_pc,instr);
      break;
    }
}

const u8* thread_get_stack(thread* this) { 
    return this->m_stack; 
}

const u8 thread_stack_count(thread* this, u8 c) { 
    return (c-1)<=this->m_stack_pos; 
}

const int thread_get_stack_pos(thread* this) { 
    return this->m_stack_pos; 
}

void thread_push(thread* this, u8 data) {
	if (this->m_stack_pos<STACK_SIZE-1)
	{
		this->m_stack[++this->m_stack_pos]=data;
	}
}

u8 thread_pop(thread* this) {
 	if (this->m_stack_pos>=0)
	{
		u8 ret=this->m_stack[this->m_stack_pos];
		this->m_stack_pos--;
		return ret;
	}
	//    printf("errorr\n");
	return 0;   
}

u8 thread_top(thread* this) {
	if (this->m_stack_pos>=0)
	{
		return this->m_stack[this->m_stack_pos];
	}
	return 0;
}

///////////////////////////////////////////////////////////////

void machine_create(machine *this, uint8_t *buffer) {
  int count=0;
  //    this->m_heap = (u8*)malloc(sizeof(u8)*HEAP_SIZE);
  this->m_memory=buffer;
  this->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk

	for (unsigned char n=0; n<MAX_THREADS; n++)
	{
	  thread_create(&this->m_threads[n], count, 22);// last is CPU type!
	  this->m_threadcount++;
	  count+=255;
    }
}

u8 machine_peek(const machine* this, uint16_t addr) {
  //	return this->m_heap[addr%HEAP_SIZE];
  
  return this->m_memory[addr];
}

void machine_poke(machine* this, uint16_t addr, u8 data) {
  //	this->m_heap[addr%HEAP_SIZE]=data;
  this->m_memory[addr]=data;
}

void machine_run(machine* this) {
	for (unsigned char n=0; n<MAX_THREADS; n++) {
		thread_run(&this->m_threads[n],this);
	}
}

void write_mem(machine *m, int *a, uint16_t len) {
    for (uint16_t i=0; i<len; i++) {
        machine_poke(m,i,a[i]);
    }
}

void leak(machine *m){
  // leak bottom of stack x into top y
  unsigned char x=1, y=1, count;
    while (x==y){ 
  x=rand()%m->m_threadcount;
  y=rand()%m->m_threadcount;
    }

  thread *xx=&m->m_threads[x];
  thread *yy=&m->m_threads[y];
  //  printf("leak %d %d\n",x,y);
  
  // bottom of x stack is pushed onto y
  if (xx->m_stack_pos>1){
  thread_push(yy,xx->m_stack[0]);
  // how to remove from stack xx?
  for (count=xx->m_stack_pos;count>0; count--){
    xx->m_stack[count-1]=xx->m_stack[count];
  }
		 xx->m_stack_pos--;
  }
}

#ifdef PCSIM
int main(void)
{
  int x;
  u8 buffer[65536];
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=rand()%255;
  }

  machine *m=(machine *)malloc(sizeof(machine));
  machine_create(m,buffer);

  while(1) {
      machine_run(m);
      if (rand()%20==0) {
  	leak(m);
  	}

  }
}
#endif
