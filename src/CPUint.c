// was CPU.c but now with overlap, uint16_t blah

// void cpustackpush(machine *this, u16 address, u8 cputype, u8 delay)

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

/* TODO:

   cpustackpush(m,randi()<<4,randi()%CPU_TOTAL);
   // here we can also do so that they follow consequtively and
   // also pass wrap and other params? ???TODO???
   // also pass biotadir for 8bit 16bit

- cpus link to grains - some variable/array for exchange of grains/cpu
  start-end positions

- control of leakiness m->m_leakiness

- for wormcode steering buffer, also other cpus modded to read/write
  instruction pointer to that extra buffer - where to pass ref??? some
  kind of window or we just use sep. code for that?

- add input into some CPUs (perhaps adc_buffer[somereg%10]) )

- add hodge.c in case 16:

- whether to push off threads from stack when full?

TOTAL so far: 24 CPUs (0-23)

*/

#define CPU_TOTAL 24

void leak(machine *m);

void thread_create(thread *this, u16 address, uint8_t which, u8 delay) { // ??? or we steer each of these?
    this->m_CPU=which;
    this->m_del=delay; this->m_delc=0;
    this->m_start=address;

#ifdef PCSIM
    this->m_wrap=this->m_start+randi()%65536;
#else
    this->m_wrap=this->m_start+randi();
#endif
    this->m_pc=this->m_start;
    this->m_reg8bit1=randi()%255;
    this->m_reg8bit2=randi()%255;
    this->m_reg8bit3=randi()%255;
    this->m_reg8bit4=randi()%255;
    this->m_stack_pos=-1;
    //this->m_stack=(u8*)malloc(STACK_SIZE);

    for (int n=0; n<STACK_SIZE; n++)
      {
	this->m_stack[n]=0;
      }
}

void cpustackpush(machine *this, u16 address, u8 cputype, u8 delay){
  if (this->m_threadcount==MAX_THREADS) return;
  else {
    thread_create(&this->m_threads[this->m_threadcount], address, cputype,delay);// last is CPU type!
  this->m_threadcount++;
  }
}

void cpustackpop(machine *this){
  this->m_threadcount--;
  if (this->m_threadcount==0) this->m_threadcount=1;
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
  //  u8 biotadir[8]={239,240,1,17,16,15,254,238}; // now for 16 bit
  u16 biotadir[8]={65279,65280,1,257,256,254,65534,65278};
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  //  printf("running: %d ",this->m_start);
  //  sleep(1);

  // SWITCH for m_CPU!


#ifdef PCSIM
  //      printf("CPU: %d\n",this->m_CPU);
  printf("%c",machine_peek(m,this->m_pc));

#endif

  if (++this->m_delc==this->m_del){
  switch(this->m_CPU)
    {
    case 0: // :LEAKY STACK! - working!
      instr=machine_peek(m,this->m_pc);
      this->m_pc++;
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      //			printf("%d", instr);
      switch(instr%25)
	{
	  
    case NOP: break;
    case ORG: this->m_start=this->m_start+this->m_pc-1; this->m_pc=1; break;
    case EQU: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)==thread_pop(this)); break;
    case JMP: this->m_pc=machine_peek(m,this->m_pc++); break;
    case JMPZ: if (thread_stack_count(this,1) && thread_pop(this)==0) this->m_pc=machine_peek(m,this->m_pc); else this->m_pc++; break;
    case PSHL: thread_push(this,machine_peek(m,this->m_pc++)); break;
    case PSH: thread_push(this,machine_peek(m,machine_peek(m,this->m_pc++))); break;
    case PSHI: thread_push(this,machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc++)))); break;
	case POP: if (thread_stack_count(this,1)) machine_poke(m,machine_peek(m,this->m_pc++),thread_pop(this)); break;
    case POPI: if (thread_stack_count(this,1)) machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc++)),thread_pop(this)); break;
    case ADD: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)+thread_pop(this)); break;
    case SUB: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)-thread_pop(this)); break;
    case INC: if (thread_stack_count(this,1)) thread_push(this,thread_pop(this)+1); break;
    case DEC: if (thread_stack_count(this,1)) thread_push(this,thread_pop(this)-1); break;
    case AND: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)&thread_pop(this)); break;
    case OR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)|thread_pop(this)); break;
    case XOR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)^thread_pop(this)); break;
    case NOT: if (thread_stack_count(this,1)) thread_push(this,~thread_pop(this)); break;
    case ROR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)>>(machine_peek(m,this->m_pc++)%8)); break;
    case ROL: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)<<(machine_peek(m,this->m_pc++)%8)); break;
    case PIP: 
    {
        u8 d=machine_peek(m,this->m_pc++); 
        machine_poke(m,d,machine_peek(m,d)+1); 
    } break;
    case PDP: 
    {
        u8 d=machine_peek(m,this->m_pc++); 
        machine_poke(m,d,machine_peek(m,d)-1); 
    } break;
    case DUP: if (thread_stack_count(this,1)) thread_push(this,thread_top(this)); break;
    case SAY: 
      //      printf("%c",thread_pop(this));
      //      machine_poke(m,machine_peek(m,this->m_pc++),randi()%255);      
        break;
	case INP:
	  machine_poke(m,machine_peek(m,this->m_pc++),randi()%255);      
	  break;

    default : break;
	}
      break;

///////////////////////////////////////////////////////////////

    case 1:// BIOTA see: http://c2.com/cgi/wiki?BiotaLanguage
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      //      instr=machine_peek(m,this->m_pc);
      instr=machine_peek(m,this->m_pc);

      flag=0;
      //      printf("instr %d ",instr);
      switch(instr%8)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  this->m_reg8bit2+=biotadir[this->m_reg8bit1%8];
	  if (machine_peek(m,this->m_reg8bit2)==0) flag=1;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  this->m_reg8bit2+=biotadir[(this->m_reg8bit1+4)%8];
	  if (machine_peek(m,this->m_reg8bit2)==0) flag=1;
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
	  if (machine_peek(m,this->m_reg8bit2)==0) {
	    this->m_reg8bit2-=biotadir[this->m_reg8bit1%8]; // go back
	    this->m_reg8bit2+=biotadir[(this->m_reg8bit1+1)%8]; // right 45
	    if (machine_peek(m,this->m_reg8bit2)==0) {
	      this->m_reg8bit2-=biotadir[(this->m_reg8bit1+1)%8]; // go back
	      this->m_reg8bit2+=biotadir[(this->m_reg8bit1-1)%8]; // left 45
	      if (machine_peek(m,this->m_reg8bit2)==0) {
		this->m_reg8bit2-=biotadir[(this->m_reg8bit1-1)%8]; // go back
		this->m_reg8bit2+=biotadir[(this->m_reg8bit1+2)%8]; // right 90
		if (machine_peek(m,this->m_reg8bit2)==0) {
		  this->m_reg8bit2-=biotadir[(this->m_reg8bit1+2)%8]; // go back
		  this->m_reg8bit2+=biotadir[(this->m_reg8bit1-2)%8]; // left 90
		  if (machine_peek(m,this->m_reg8bit2)==0) {
		    this->m_reg8bit2-=biotadir[(this->m_reg8bit1-2)%8]; // go back
		    this->m_reg8bit2+=biotadir[(this->m_reg8bit1+3)%8]; // right 135
		  if (machine_peek(m,this->m_reg8bit2)==0) {
		    this->m_reg8bit2-=biotadir[(this->m_reg8bit1-3)%8]; // go back
		    this->m_reg8bit2+=biotadir[(this->m_reg8bit1-3)%8]; // left 135
		  if (machine_peek(m,this->m_reg8bit2)==0) {
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
	  if (machine_peek(m,this->m_reg8bit2)!=0) machine_poke(m,this->m_reg8bit2,0); 
	  else flag=1;
	  break;
	case 6:
	  //    * d -- duplicate the current data into the cell left of the DC. Fails if source cell is empty or target cell is non-empty.
	  // left is -2

	  if (machine_peek(m,this->m_reg8bit2)==0 || (machine_peek(m,this->m_reg8bit2+(this->m_reg8bit2+((this->m_reg8bit1-4)%8))))!=0) flag=1;
	  else machine_poke(m,this->m_reg8bit2+(this->m_reg8bit2+((this->m_reg8bit1-4)%8)),machine_peek(m,this->m_reg8bit2));
	  break;
	case 7:
	  //    * . -- no-op, a non-empty do nothing. 
	  break;
	}

      // - this->m_pc turns (where?) when it finds an empty location or a failing instruction
      // m_reg8bit3 is direction
      if (machine_peek(m,this->m_pc)==0 || flag==1){
	this->m_reg8bit3-=1;
      }
      else this->m_pc+=biotadir[this->m_reg8bit3%8];
      //      printf("%c",this->m_pc);
      //      break;
///////////////////////////////////////////////////////////////

    case 2:
      // brainfuck: add in input and output???

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      //      instr=machine_peek(m,this->m_pc);
      instr=machine_peek(m,this->m_pc);

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
	  machine_poke(m,this->m_reg8bit1,machine_peek(m,this->m_reg8bit1)+1);
	  this->m_pc++;
	  break;
	case 3:
	  machine_poke(m,this->m_reg8bit1,machine_peek(m,this->m_reg8bit1)-1);
	  this->m_pc++;
	  break;
	case 4:
	  this->m_reg8bit2++;
	  if (this->m_reg8bit2>=16) this->m_reg8bit2=0;
	  this->m_stack[this->m_reg8bit2]= this->m_pc;
	  this->m_pc++;
	  break;
	case 5:
	  if (machine_peek(m,this->m_reg8bit1)!=0) this->m_pc=this->m_stack[this->m_reg8bit2]-1;
	  this->m_reg8bit2-=1;
	  if (this->m_reg8bit2==0) this->m_reg8bit2=16;
	  break;
	}
      //      printf("%c",this->m_pc);
    
///////////////////////////////////////////////////////////////

    case 3:
      // masque red death: add in input and output???

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);

      //      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%7){
      case 0:
	if (this->m_reg8bit2==12){
	  machine_poke(m,this->m_pc+1,machine_peek(m,this->m_pc));
	  if (this->m_pc==255) this->m_reg8bit2=13;
	  this->m_pc++;
	}
	else this->m_pc++;
	break;
      case 1:
	if (this->m_reg8bit2==13){
	  this->m_reg8bit1++;
	  //	  machine_poke(m,this->m_reg8bit1,machine_peek(m,this->m_pc)); READ IN! TODO!
	  //	  this->m_pc++;
	}
	else this->m_pc++;
	break;
      case 2:
	this->m_reg16bit1++;
	if ((this->m_reg16bit1%60)==0){
	  this->m_reg8bit2++;
	  machine_poke(m,this->m_reg8bit1,machine_peek(m,this->m_reg8bit1)^255);
	}
	else this->m_pc++;
	break;
	case 3:
	  //	  seven rooms: divide cellspace into 7 - 7 layers with filter each: TODO
	  break;
	case 4:
	  machine_poke(m,this->m_reg8bit1-1,machine_peek(m,this->m_reg8bit1-1)^255);
	  machine_poke(m,this->m_reg8bit1+1,machine_peek(m,this->m_reg8bit1+1)^255);
	  this->m_pc++;
	  break;
	case 5:
	  flag=randi()%4;
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

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);

      //      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%4){
      case 0:
	machine_poke(m,this->m_pc,255);
	machine_poke(m,this->m_pc+1,255);
	this->m_pc+=2;
	break;
      case 1:
	if (machine_peek(m,this->m_pc)<128){
	  machine_poke(m,this->m_pc-1,machine_peek(m,this->m_pc));
	  machine_poke(m,this->m_pc+1,machine_peek(m,this->m_pc));
	}
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      case 2:
	machine_poke(m,this->m_pc-1,0);
	machine_poke(m,this->m_pc+1,0);
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      case 3:
	if ((machine_peek(m,this->m_pc)%0x03)==1) this->m_reg8bit3+=4;
	else this->m_reg8bit3*=machine_peek(m,this->m_pc)>>4;
	this->m_pc+=biotadir[this->m_reg8bit3%8];
	break;
      }
	if (machine_peek(m,this->m_pc)==255) this->m_reg8bit3+=4;
	//      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 5:
      // first from micro: add in input and output???

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      instr=machine_peek(m,this->m_pc);
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
	machine_poke(m,this->m_reg8bit1,machine_peek(m,this->m_reg8bit1)+1);
	this->m_pc++;
	break;
      case 3:	  
	machine_poke(m,this->m_reg8bit1,machine_peek(m,this->m_reg8bit1)-1);
	this->m_pc++;
	break;
      case 4:	  
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)+1);
	this->m_pc++;
	break;
      case 5:	  
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)-1);
	this->m_pc++;
	break;
      case 6:	  
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)<<1);
	this->m_pc++;
	break;
      case 7:	  
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)<<2);
	this->m_pc++;
	break;
      case 8:	  
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)<<3);
	this->m_pc++;
	break;
      case 9:	  
	if (machine_peek(m,this->m_pc+1)==0) this->m_pc=this->m_reg8bit1;
	//	this->m_pc++;
	break;
      case 10:	  
	if (machine_peek(m,this->m_pc+1)<128) this->m_pc+=machine_peek(m,this->m_pc+1);
	else 	this->m_pc++;
	break;
      case 11:	  
	if (machine_peek(m,this->m_pc-1)<128) machine_poke(m,this->m_pc+1,machine_peek(m,this->m_pc));
	this->m_pc++;
	break;
      case 12:	  
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc+1));
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

REF: http://vyznev.net/corewar/guide.html#start_instr

http://www.koth.org/info/akdewdney/images/Redcode.jpg

 */

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      instr=machine_peek(m,this->m_pc);
      switch(instr%28){
      case 0:
	// MOV # to direct.
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2),machine_peek(m,this->m_pc+1));
	this->m_pc+=3;
	break;
      case 1:	  
	// MOV indirect to direct.
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2),machine_peek(m,machine_peek(m,this->m_pc+1)));
	this->m_pc+=3;
	break;
      case 2:
	// MOV # to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_peek(m,this->m_pc+1));
	this->m_pc+=3;
	break;
      case 3:
	//MOV indirect to indirect
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_peek(m,machine_peek(m,this->m_pc+1)));
	this->m_pc+=3;
	break;
      case 4:
	//ADD # to direct
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2),machine_peek(m,this->m_pc+1)+machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 5:
	// ADD indirect to direct.
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2),machine_peek(m,machine_peek(m,this->m_pc+1)+machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))));
	this->m_pc+=3;
	break;
      case 6:
	// ADD # to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_peek(m,this->m_pc+1)+machine_peek(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 7:
	//ADD indirect to indirect
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_peek(m,machine_peek(m,this->m_pc+1))+machine_peek(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 8:
	//SUB # to direct
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2),machine_peek(m,this->m_pc+1)-machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 9:
	// indirect to direct.
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2),machine_peek(m,machine_peek(m,this->m_pc+1)-machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))));
	this->m_pc+=3;
	break;
      case 10:
	// # to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_peek(m,this->m_pc+1)-machine_peek(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 11:
	// indirect to indirect
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_peek(m,machine_peek(m,this->m_pc+1))-machine_peek(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 12:
	// jmp to direct
	this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	break;
      case 13:
	// jmp to indirect
	this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	break;
      case 14:
	// JMZdirect to direct
	if (machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))==0) 	this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 15:
	// JMZdirect to indirect
	if (machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))==0) 	this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 16:	
	// JMZindirect to direct
	if (machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) 	this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 17:
	// JMZindirect to indirect
	if (machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) 	this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 18:
	// JMGdirect to direct
	if (machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))>0) 	this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 19:
	// JMGdirect to indirect
	if (machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))>0) 	this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 20:	
	// JMGindirect to direct
	if (machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+2)))>0) 	this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 21:
	// JMGindirect to indirect
	if (machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+2)))>0) 	this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 22:
	// Dec, jump if zero... sub 1 from b. jump to a if b is zero
	// DJZ dir to dir
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2), machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))-1);
	if (machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))==0)	  this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	else this->m_pc+=3;
	break;
      case 23:
	// DJZ dir to indir
	machine_poke(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2), machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))-1);
	if (machine_peek(m,this->m_pc+(unsigned char)machine_peek(m,this->m_pc+2))==0)	  this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	else this->m_pc+=3;
	break;
      case 24:
	// DJZ indir to dir
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)), machine_peek(m,machine_peek(m,this->m_pc+2)-1));
	if (machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0)this->m_pc+=(unsigned char)machine_peek(m,this->m_pc+1);
	else this->m_pc+=3;
	break;
      case 25:
	// DJZ indir to indir
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)), machine_peek(m,machine_peek(m,this->m_pc+2)-1));
	if (machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) this->m_pc=machine_peek(m,machine_peek(m,machine_peek(m,this->m_pc+1)));
	else this->m_pc+=3;
	break;
      case 26:
	// SPL
	//- add new thread at address x
	cpustackpush(m,(u16)m->m_memory[this->m_pc+1],6,this->m_del);
	break;
      case 27:
	this->m_pc+=3;
	break;
      }
      //      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 7:
      // SIR: add in input and output??? untested for audio but...
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%4){
      case 0:
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
	if (machine_peek(m,this->m_pc+1)<128) machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)+1);
	this->m_pc++;
	break;
      case 1:	  
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128))    if (randi()%10 < 4) cells[IP] = dead;     
	if (machine_peek(m,this->m_pc+1)<128)  if (randi()%10 < 4) machine_poke(m,this->m_pc,255);     
	this->m_pc++;
	break;
      case 2:
	//  if (cells[(IP+1)] >= 128) cells[IP] = recovered;
	if (machine_peek(m,this->m_pc+1)>=128)  machine_poke(m,this->m_pc,129);
	this->m_pc++;
	break;
      case 3:
	if (machine_peek(m,this->m_pc+1)==0){
	  if ((machine_peek(m,this->m_pc-1)>0 && machine_peek(m,this->m_pc-1)<128) ||
	      (machine_peek(m,this->m_pc+1)>0 && machine_peek(m,this->m_pc+1)<128))
	    {
	if (randi()%10 < 4) machine_poke(m,this->m_pc,129);
	    }
	}
	this->m_pc++;
	break;
      }

///////////////////////////////////////////////////////////////

    case 8:
      // WOrmcode: but was with overlaps and also we need extra buffer to steer!
      //      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);

      //      this->m_reg8bit3=biotadir[randi()%8]; // replace with buffer steering TODO or:
      this->m_reg8bit3=biotadir[machine_peek(m,this->m_pc)%8];
      this->m_pc+=this->m_reg8bit3;
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
      switch(instr%12){
      case 0:
	break;
      case 1:
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)+1);
	break;
      case 2:
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)-1);
	break;
      case 3:
	this->m_pc+=machine_peek(m,this->m_pc+this->m_reg8bit3);
	break;
      case 4:
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)+machine_peek(m,this->m_pc+this->m_reg8bit3));
	break;
      case 5:
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)-machine_peek(m,this->m_pc+this->m_reg8bit3));
	break;
      case 6:
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)<<1);
	break;
      case 7:
	machine_poke(m,this->m_pc,machine_peek(m,this->m_pc)>>1);
	break;
      case 8:
	if (machine_peek(m,this->m_pc+(this->m_reg8bit3*2))==0) this->m_pc+=machine_peek(m,this->m_pc+this->m_reg8bit3);
	break;
      case 9:
	flag=machine_peek(m,this->m_pc);
	machine_poke(m,this->m_pc-this->m_reg8bit3,flag);
	machine_poke(m,this->m_pc+this->m_reg8bit3,flag);
	break;
      case 10:
	thread_push(this,machine_peek(m,this->m_pc+this->m_reg8bit3));
	break;
      case 11:
	machine_poke(m,this->m_pc+this->m_reg8bit3,thread_pop(this));
	break;
      }
      //      printf("%c",instr);

///////////////////////////////////////////////////////////////

    case 9:
      // stack machine a la: http://www.ece.cmu.edu/~koopman/stack_computers/sec3_2.html#321
      // but sans return stack
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%15){
      case 0:
	flag=thread_pop(this);
	machine_poke(m,thread_pop(this),flag);
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
	thread_push(this,machine_peek(m,thread_pop(this)));
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
	if (thread_pop(this)==0) this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc++;
	break;
      case 12:
	//sub call
	thread_push(this,this->m_pc);
	this->m_pc=machine_peek(m,machine_peek(m,this->m_pc+1));
	break;
      case 13:
	//sub return
	this->m_pc=thread_pop(this);
	break;
      case 14:
	thread_push(this,machine_peek(m,this->m_pc+1));
	this->m_pc++;
	break;
      }
      //      printf("%c",this->m_pc);

///////////////////////////////////////////////////////////////

    case 10:
      // befunge: http://en.wikipedia.org/wiki/Befunge
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
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
	this->m_reg8bit3=(randi()%4)*2;
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
	machine_poke(m,(thread_pop(this)&16)*(thread_pop(this)&16),thread_pop(this));
	break;
      case 29:
	thread_push(this,machine_peek(m,thread_pop(this)&16)*(thread_pop(this)&16));
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
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
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
	  machine_poke(m,this->m_pc,instr^thread_pop(this));
	  break;
	case 13:
	  this->m_reg8bit3-=2;
	  machine_poke(m,this->m_pc,instr^thread_pop(this));
	  break;
	}
      this->m_pc+=biotadir[this->m_reg8bit3%8];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 12:
      // turmites code: turmite has state,direction,position...
      // reg8bit1 is state. reg8bit2 is direction index.
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
      machine_poke(m,this->m_pc,instr+this->m_reg8bit1);
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
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_peek(m,this->m_pc);
      other=(machine_peek(m,this->m_pc-1)&1)+(machine_peek(m,this->m_pc+1)&1)+(machine_peek(m,this->m_pc-32)&1)+(machine_peek(m,this->m_pc+32)&1)+(machine_peek(m,this->m_pc-31)&1)+(machine_peek(m,this->m_pc-33)&1)+(machine_peek(m,this->m_pc+31)&1)+(machine_peek(m,this->m_pc+33)&1);

    if ((instr&1)==1 && other<2) flag=0;
    else if ((instr&1)==1 && other>3) flag=0;
    else if ((instr&1)==0 && other==3) flag=instr+1;
    else flag=instr;

    machine_poke(m,this->m_pc+128,flag);
      this->m_pc++; 
      //      printf("%c",instr);
      break;

    case 14:
      // ants revisited... how to do multiple ants using stack?
      // read cell//process rule string//change cell//move ant
      // rule string is from cell[0]
      instr=machine_peek(m,this->m_pc);
      machine_poke(m,this->m_pc,instr+1);
      this->m_reg8bit1=antrule(this->m_reg8bit1,instr%8,machine_peek(m,0));//last is rule
      this->m_pc+=biotadir[this->m_reg8bit1%8];
      //      printf("%c",this->m_pc);
      break;

    case 15:
      // second CA from CA.c
      flag = 0;
      instr=machine_peek(m,this->m_pc);
      if (machine_peek(m,this->m_pc+1)>128)	flag |= 0x4;
      if (instr>128) flag |= 0x2;
      if (machine_peek(m,this->m_pc-1)>128)	flag |= 0x1;
                     
      if ((machine_peek(m,0) >> flag) & 1)	machine_poke(m,this->m_pc+128,instr+129);
      else machine_poke(m,this->m_pc+128,instr-129); // or we stay with 255 and 0 as poked

      this->m_pc++; 
      if (this->m_pc==0) this->m_pc=1;
      //      printf("%c",instr);
      break;

    case 16:
      // **TODO** port of hodge - but we need larger 256*128 (32768) cellspace in two halves
      // numill and numinf

      break;
    case 17:
      // start generic - add (add/sub/zero/copy/invert/swap)
      instr=machine_peek(m,this->m_pc);
      machine_poke(m,this->m_pc,instr+1);
      this->m_pc++; 
      break;
    case 18:
      // generic - sub (add/sub/zero/copy/invert/swap)
      instr=machine_peek(m,this->m_pc);
      machine_poke(m,this->m_pc,instr-1);
      this->m_pc++; 
      break;
    case 19:
      // generic - zero (add/sub/zero/copy/invert/swap)
      machine_poke(m,this->m_pc,0);
      this->m_pc++; 
      break;
    case 20:
      // generic - copy (add/sub/zero/copy/invert/swap)
      machine_poke(m,this->m_pc,machine_peek(m,this->m_pc+1));
      this->m_pc++; 
      break;
    case 21:
      // generic - copy (add/sub/zero/copy/invert/swap)
      machine_poke(m,this->m_pc,machine_peek(m,this->m_pc^255));
      this->m_pc++; 
      break;
    case 22:
      // generic - swap (add/sub/zero/copy/invert/swap)
      instr=machine_peek(m,this->m_pc+1);
      machine_poke(m,this->m_pc+1,machine_peek(m,this->m_pc));
      machine_poke(m,this->m_pc,instr);
      this->m_pc++; 
      break;
///////////////////////////////////////////////////////////////
    case 23:
      // from wormcode.c
      instr=machine_peek(m,this->m_pc);
      switch(instr%14)
	{
	case 0:
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 1:
	  //inc
	  machine_poke(m,this->m_pc,instr+1);
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 2:
	  machine_poke(m,this->m_pc,instr-1);
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 3:
	  this->m_pc=machine_peek(m,this->m_pc);
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 4:
	  machine_poke(m,this->m_pc,randi()%255);
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 5:
	  machine_poke(m,this->m_pc,machine_peek(m,this->m_pc+biotadir[randi()%8]));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 6:
	  machine_poke(m,this->m_pc,machine_peek(m,this->m_pc+biotadir[randi()%8]));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 7:
	  machine_poke(m,this->m_pc,machine_peek(m,this->m_pc-biotadir[randi()%8]));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 8:
	  machine_poke(m,this->m_pc,machine_peek(m,this->m_pc<<biotadir[randi()%8]));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 9:
	  machine_poke(m,this->m_pc,machine_peek(m,this->m_pc>>biotadir[randi()%8]));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 10:
	  if (machine_peek(m,this->m_pc+(biotadir[randi()%8]*2))==0) this->m_pc+=biotadir[randi()%8];
	  break;
	case 11:
	  machine_poke(m,(this->m_pc-biotadir[randi()%8]),instr);
	  machine_poke(m,(this->m_pc+biotadir[randi()%8]),instr);
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 12:
	  thread_push(this, machine_peek(m,this->m_pc+biotadir[randi()%8]));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	case 13:
	  machine_poke(m,(this->m_pc+=biotadir[randi()%8]),thread_pop(this));
	  this->m_pc+=biotadir[randi()%8];
	  break;
	}
    }
      this->m_delc=0;
  } // if del
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

void machine_create(machine *this, u8 leakiness, uint8_t *buffer) {
  //  int count=0;
  //    this->m_heap = (u8*)malloc(sizeof(u8)*HEAP_SIZE);
  this->m_threadcount=0;
  this->m_memory=buffer;
  this->m_leakiness=leakiness;
    this->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk
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
	if ((randi()%this->m_leakiness)==0) {
  	leak(this);
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
  x=randi()%m->m_threadcount;
  y=randi()%m->m_threadcount;
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
    buffer[x]=randi()%255;
  }

  machine *m=(machine *)malloc(sizeof(machine));
  machine_create(m,randi()%255,buffer); // this just takes care of pointer to machine and malloc for threads

	for (unsigned char n=0; n<10; n++)
	{
#ifdef PCSIM
	  // 	  cpustackpush(m,randi()%65536,randi()%CPU_TOTAL);
	  cpustackpush(m,randi()%65536,23,1);
#else
	  cpustackpush(m,randi()<<4,randi()%CPU_TOTAL,1);
	  // here we can also do so that they follow consequtively and
	  // also pass wrap and other params? ???TODO???

#endif
	}

  while(1) {
      machine_run(m);
  }


  /*- add in mutation, cross-pollination, infection, plague, death so cpus
    can also infect/take over each other (meta)
  
    - mutation bit-flip a randiom CPU type
    - address exchange
    - infection
    - death, killing

  */

}
#endif
