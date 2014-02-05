// was CPU.c but now with overlap, uint16_t blah

// void cpustackpush(machine *this, u16 address, u16 wrapaddress, u8 cputype, u8 delay)

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

- output for reddeath and few others (how?)

- fix 16/8 bit issue in poke and peek!!!!!!!!!!!!1

/////////////OLDER::::

- cpus link to grains - some variable/array for exchange of grains/cpu
  start-end positions (or is just machine)?

- control of leakiness m->m_leakiness and other params, infection and so onDONE

- for wormcode steering buffer, also other cpus modded to read/write
  instruction pointer to that extra buffer - where to pass ref??? some
  kind of window or we just use sep. code for that?
  
//this is wormdir extern u8 variable now!

- also that we should change wrap/jump in some CA inspired code ????

- add hodge.c in case 16:DONE

///

- whether to push off threads from stack when full?

*/

#define CPU_TOTAL 31

void leak(machine *m);

void thread_create(thread *this, u16 address, u16 wrapaddress, uint8_t which, u8 delay) { // ??? or we steer each of these?
  this->m_infection=0;
  this->m_CPU=which;
  this->m_del=delay; this->m_delc=0;
  this->m_start=address;
  this->m_wrap=wrapaddress;
  this->m_pc=address;
  this->m_reg8bit1=randi()%255;
  this->m_reg8bit2=randi()%255;
  this->m_reg16bit1=address;
  this->m_stack_pos=-1;
    //this->m_stack=(u8*)malloc(STACK_SIZE);

    for (int n=0; n<STACK_SIZE; n++)
      {
	this->m_stack[n]=0;
      }
}

void cpustackpush(machine *this, u16 address, u16 wrapaddress,u8 cputype, u8 delay){
  if (this->m_threadcount==MAX_THREADS) return;
  else {
    thread_create(&this->m_threads[this->m_threadcount], address, wrapaddress,cputype,delay);// last is CPU type!
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

void dircalc(u16 *mmm, u16 wrapper,u16 liner){
  mmm[0]=wrapper-liner;
  mmm[1]=(wrapper-liner)+1;
  mmm[2]=1;
  mmm[3]=liner+1;
  mmm[4]=liner;
  mmm[5]=liner-1;
  mmm[6]=wrapper-1;
  mmm[7]=wrapper-liner-1;
}

void thread_run(thread* this, machine *m) {
  u8 instr,temp;
  u16 y;
  u8 flag,other;
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  //  printf("running: %d ",this->m_start);
  //  sleep(1);
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

  //  dircalc(biotadir,65536,256);

  if (++this->m_delc==this->m_del){

#ifdef PCSIM
    //      printf("CPU: %d\n",this->m_CPU);
    //    printf("%c",machine_peek(m,this->m_pc));

#endif
    this->m_CPU=5;
    switch(this->m_CPU)
      {
      case 0: // :LEAKY STACK! - working!
      instr=machine_p88k(m,this->m_pc);
      this->m_pc++;
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      //			printf("%d", instr);
      switch(instr%25)
	{
	  
	case NOP: break;
	case ORG: this->m_start=this->m_pc-1; this->m_pc=this->m_start+1; break;
	case EQU: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)==thread_pop(this)); break;
	case JMP: this->m_pc=machine_peek(m,this->m_pc++); break;
	case JMPZ: if (thread_stack_count(this,1) && thread_pop(this)==0) this->m_pc=machine_peek(m,this->m_pc); else this->m_pc++; break;
    case PSHL: thread_push(this,machine_peek(m,this->m_pc++)); break;
	case PSH: thread_push(this,machine_p88k(m,machine_peek(m,this->m_pc++))); break;
	case PSHI: thread_push(this,machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc++)))); break;
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
    case ROR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)>>(machine_p88k(m,this->m_pc++)%8)); break;
    case ROL: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)<<(machine_p88k(m,this->m_pc++)%8)); break;
    case PIP: 
    {
        u16 d=machine_peek(m,this->m_pc++); 
        machine_poke(m,d,machine_peek(m,d)+1); 
    } break;
    case PDP: 
    {
        u16 d=machine_peek(m,this->m_pc++); 
        machine_poke(m,d,machine_peek(m,d)-1); 
    } break;
    case DUP: if (thread_stack_count(this,1)) thread_push(this,thread_top(this)); break;
    case SAY: 
      //      printf("%c",thread_pop(this));
      //      machine_poke(m,machine_peek(m,this->m_pc++),randi()%255);      
        break;
#ifndef PCSIM
	case INP:
	  machine_poke(m,machine_peek(m,this->m_pc++),adc_buffer[thread_pop(this)%10]);      
#endif
	  this->m_pc++;
	  break;

    default : break;
	}
      break;

///////////////////////////////////////////////////////////////

    case 1:// BIOTA see: http://c2.com/cgi/wiki?BiotaLanguage
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      //      instr=machine_peek(m,this->m_pc);
      instr=machine_p88k(m,this->m_pc);

      flag=0;
      //      printf("instr %d ",instr);
      switch(instr%8)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  this->m_reg16bit1+=biotadir[this->m_reg8bit1%8];
	  if (machine_p88k(m,this->m_reg16bit1)==0) flag=1;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  this->m_reg16bit1+=biotadir[(this->m_reg8bit1+4)%8];
	  if (machine_p88k(m,this->m_reg16bit1)==0) flag=1;
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
	  temp=this->m_reg8bit1%8;
	  this->m_reg16bit1+=biotadir[temp];
	  if (machine_p88k(m,this->m_reg16bit1)==0) {
	    this->m_reg16bit1-=biotadir[temp]; // go back
	    this->m_reg16bit1+=biotadir[(this->m_reg8bit1+1)%8]; // right 45
	    if (machine_p88k(m,this->m_reg16bit1)==0) {
	      this->m_reg16bit1-=biotadir[(this->m_reg8bit1+1)%8]; // go back
	      this->m_reg16bit1+=biotadir[(this->m_reg8bit1-1)%8]; // left 45
	      if (machine_p88k(m,this->m_reg16bit1)==0) {
		this->m_reg16bit1-=biotadir[(this->m_reg8bit1-1)%8]; // go back
		this->m_reg16bit1+=biotadir[(this->m_reg8bit1+2)%8]; // right 90
		if (machine_p88k(m,this->m_reg16bit1)==0) {
		  this->m_reg16bit1-=biotadir[(this->m_reg8bit1+2)%8]; // go back
		  this->m_reg16bit1+=biotadir[(this->m_reg8bit1-2)%8]; // left 90
		  if (machine_p88k(m,this->m_reg16bit1)==0) {
		    this->m_reg16bit1-=biotadir[(this->m_reg8bit1-2)%8]; // go back
		    this->m_reg16bit1+=biotadir[(this->m_reg8bit1+3)%8]; // right 135
		    if (machine_p88k(m,this->m_reg16bit1)==0) {
		    this->m_reg16bit1-=biotadir[(this->m_reg8bit1-3)%8]; // go back
		    this->m_reg16bit1+=biotadir[(this->m_reg8bit1-3)%8]; // left 135
		    if (machine_p88k(m,this->m_reg16bit1)==0) {
		    this->m_reg16bit1-=biotadir[(this->m_reg8bit1-3)%8]; // go back
		    this->m_reg16bit1+=biotadir[(this->m_reg8bit1+4)%8]; // back
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
	  if (machine_p88k(m,this->m_reg16bit1)!=0) machine_poke(m,this->m_reg16bit1,0); 
	  else flag=1;
	  break;
	case 6:
	  //    * d -- duplicate the current data into the cell left of the DC. Fails if source cell is empty or target cell is non-empty.
	  // left is -2

	  if (machine_p88k(m,this->m_reg16bit1)==0 || (machine_p88k(m,this->m_reg16bit1+(this->m_reg16bit1+((this->m_reg8bit1-4)%8))))!=0) flag=1;
	  else machine_poke(m,this->m_reg16bit1+(this->m_reg16bit1+((this->m_reg8bit1-4)%8)),machine_p88k(m,this->m_reg16bit1));
	  break;
	case 7:
	  //    * . -- no-op, a non-empty do nothing. 
	  break;
	}

      // - this->m_pc turns (where?) when it finds an empty location or a failing instruction
      // m_reg8bit3 is direction
      wormdir=this->m_reg8bit2%8;
      if (machine_p88k(m,this->m_pc)==0 || flag==1){
	this->m_reg8bit2-=1;
      }
      else this->m_pc+=biotadir[wormdir];
      //      printf("%c",this->m_pc);
      //      break;
      
/////////////////////////////////////////////////////////////////

    case 2:
      // brainfuck: add in output???

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      //      instr=machine_peek(m,this->m_pc);
      instr=machine_p88k(m,this->m_pc);

      //      printf("instr %d ",instr);
      switch(instr%7)
	{
	case 0:
	  this->m_reg16bit1+=1;
	  this->m_pc++;
	  break;
	case 1:
	  this->m_reg16bit1-=1;
	  this->m_pc++;
	  break;
	case 2:
	  //	    cells[omem]=cells[omem]++; 
	  machine_poke(m,this->m_reg16bit1,machine_peek(m,this->m_reg16bit1)+1);
	  this->m_pc++;
	  break;
	case 3:
	  machine_poke(m,this->m_reg16bit1,machine_peek(m,this->m_reg16bit1)-1);
	  this->m_pc++;
	  break;
	case 4:
	  this->m_reg8bit2+=2;
	  if (this->m_reg8bit2>=14) this->m_reg8bit2=0;
	  this->m_stack[this->m_reg8bit2]= this->m_pc>>8;
	  this->m_stack[this->m_reg8bit2+1]= this->m_pc&255;
	  this->m_pc++;
	  break;
	case 5:
	  if (this->m_reg8bit2>=14) this->m_reg8bit2=0;
	  if (machine_p88k(m,this->m_reg16bit1)!=0) this->m_pc=(this->m_stack[this->m_reg8bit2])<<8+((this->m_stack[this->m_reg8bit2+1]));
	  this->m_reg8bit2-=2;
	  if (this->m_reg8bit2==0) this->m_reg8bit2=14;
	  break;
#ifndef PCSIM
	case 6:
	  //  cells[omem] = adcread(3); 
	  machine_poke(m,this->m_reg16bit1,adc_buffer[this->m_reg16bit1%10]);
#endif
	  this->m_pc++;
	  break;
	}
      //      printf("%c",this->m_pc);
      break;    
///////////////////////////////////////////////////////////////

    case 3:
      // masque red death: add in output???

      if (this->m_pc>=this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);

      //      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%7){
      case 0:
	if (this->m_reg8bit2==12){
	  machine_poke(m,this->m_pc+1,machine_p88k(m,this->m_pc));
	  if (machine_p88k(m,this->m_pc)==255) this->m_reg8bit2=13;
	  this->m_pc++;
	}
	else this->m_pc++;
	break;
      case 1:
	if (this->m_reg8bit2==13){
	  this->m_reg16bit1++;
	  machine_poke(m,this->m_reg16bit1,machine_p88k(m,this->m_pc)); //READ IN! TODO!
	  this->m_pc++;
	}
	else this->m_pc++;
	break;
      case 2:
	this->m_reg8bit2++;
	if (this->m_reg8bit2==60){
	  this->m_reg8bit2++;
	  machine_poke(m,this->m_reg16bit1,machine_p88k(m,this->m_reg16bit1)^255);
	  this->m_reg8bit2=0;
	}
	else this->m_pc++;
	break;
	case 3:
	  //	  seven rooms: divide cellspace into 7 - 7 layers with filter each: TODO
	  break;
	case 4:
	  machine_poke(m,this->m_reg16bit1-1,machine_p88k(m,this->m_reg8bit1-1)^255);
	  machine_poke(m,this->m_reg16bit1+1,machine_p88k(m,this->m_reg8bit1+1)^255);
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
#ifndef PCSIM
	case 6:
	  //cells[omem+1]=adcread(3); rEADIN TODO
	  machine_poke(m,this->m_reg16bit1+2,adc_buffer[((this->m_reg16bit1)>>8)%10]);
#endif
	  this->m_pc++;
	  break;
      }
      break;    

      //      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////

    case 4:
      // plague: add in output???

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);

      //      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%5){
      case 0:
	machine_poke(m,this->m_pc,255);
	machine_poke(m,this->m_pc+1,255);
	this->m_pc+=2;
	break;
      case 1:
	if (machine_p88k(m,this->m_pc)<128){
	  machine_poke(m,this->m_pc-1,machine_p88k(m,this->m_pc));
	  machine_poke(m,this->m_pc+1,machine_p88k(m,this->m_pc));
	}
	this->m_pc+=biotadir[this->m_reg8bit2%8];
	break;
      case 2:
	machine_poke(m,this->m_pc-1,0);
	machine_poke(m,this->m_pc+1,0);
	this->m_pc+=biotadir[this->m_reg8bit2%8];
	break;
      case 3:
	if ((machine_p88k(m,this->m_pc)%0x03)==1) this->m_reg8bit2+=4;
	else this->m_reg8bit2*=machine_p88k(m,this->m_pc)>>4;
	this->m_pc+=biotadir[this->m_reg8bit2%8];
	break;
#ifndef PCSIM
      case 4:
	machine_poke(m,this->m_pc+1,adc_buffer[(this->m_reg8bit1>>8)%10]);
#endif
	  break;

      }
      if (machine_p88k(m,this->m_pc)==255) this->m_reg8bit2+=4;
	wormdir=this->m_reg8bit2%8;
	//      printf("%c",this->m_pc);
      break;    
	
///////////////////////////////////////////////////////////////

      case 5:
      // first from micro: add in output???

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      instr=machine_p88k(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%15){
      case 0:
	this->m_reg16bit1+=2;
	this->m_pc++;
	break;
      case 1:
	this->m_reg16bit1-=2;
	this->m_pc++;
	break;
      case 2:
	machine_poke(m,this->m_reg16bit1,machine_p88k(m,this->m_reg16bit1)+1);
	this->m_pc++;
	break;
      case 3:	  
	machine_poke(m,this->m_reg16bit1,machine_p88k(m,this->m_reg16bit1)-1);
	this->m_pc++;
	break;
      case 4:	  
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)+1);
	this->m_pc++;
	break;
      case 5:	  
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)-1);
	this->m_pc++;
	break;
      case 6:	  
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)>>1);
	this->m_pc++;
	break;
      case 7:	  
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)>>2);
	this->m_pc++;
	break;
      case 8:	  
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)>>3);
	this->m_pc++;
	break;
      case 9:	  
	if (machine_peek(m,this->m_pc+1)==0) this->m_pc=this->m_start+this->m_reg16bit1;
	//	this->m_pc++;
	break;
      case 10:	  
	if (machine_p88k(m,this->m_pc+1)<128) this->m_pc+=machine_p88k(m,this->m_pc+1);
	else 	this->m_pc++;
	break;
      case 11:	  
	if (machine_p88k(m,this->m_pc-1)<128) machine_poke(m,this->m_pc+1,machine_p88k(m,this->m_pc));
	this->m_pc++;
	break;
      case 12:	  
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc+1));
	this->m_pc++;
	break;
      case 13:	  
	this->m_pc++;
	break;
#ifndef PCSIM
      case 14:
	machine_poke(m,this->m_pc,adc_buffer[((this->m_reg8bit1)>>8)%10]);
#endif
	break;
      }
      //      printf("%c",this->m_pc);
///////////////////////////////////////////////////////////////
      break;    

    case 6:
/* "real" corewars redcode SPL 

REF: http://vyznev.net/corewar/guide.html#start_instr

http://www.koth.org/info/akdewdney/images/Redcode.jpg

 */

      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;

      instr=machine_p88k(m,this->m_pc);
      switch(instr%30){
      case 0:
	// MOV # to direct.
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),machine_p88k(m,this->m_pc+1));
	this->m_pc+=3;
	break;
      case 1:	  
	// MOV indirect to direct.
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),machine_p88k(m,machine_peek(m,this->m_pc+1)));
	this->m_pc+=3;
	break;
      case 2:
	// MOV # to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_p88k(m,this->m_pc+1));
	this->m_pc+=3;
	break;
      case 3:
	//MOV indirect to indirect
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_p88k(m,machine_peek(m,this->m_pc+1)));
	this->m_pc+=3;
	break;
      case 4:
	//ADD # to direct
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),machine_p88k(m,this->m_pc+1)+machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
	//HERE      case 5:
	// ADD indirect to direct.
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),machine_p88k(m,machine_peek(m,this->m_pc+1))+(machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2)>>8)));
	this->m_pc+=3;
	break;
      case 6:
	// ADD # to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_p88k(m,this->m_pc+1)+machine_p88k(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 7:
	//ADD indirect to indirect
	machine_poke(m,machine_p88k(m,machine_peek(m,this->m_pc+2)),machine_p88k(m,machine_peek(m,this->m_pc+1))+machine_p88k(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 8:
	//SUB # to direct
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),machine_p88k(m,this->m_pc+1)-machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 9:
	// indirect to direct.
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),machine_p88k(m,machine_peek(m,this->m_pc+1)-machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))));
	this->m_pc+=3;
	break;
      case 10:
	// # to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_p88k(m,this->m_pc+1)-machine_p88k(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 11:
	// indirect to indirect
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),machine_p88k(m,machine_peek(m,this->m_pc+1))-machine_p88k(m,machine_peek(m,this->m_pc+2)));
	this->m_pc+=3;
	break;
      case 12:
	// jmp to direct
	this->m_pc+=machine_p88k(m,this->m_pc+1);
	break;
      case 13:
	// jmp to indirect
	this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	break;
      case 14:
	// JMZdirect to direct
	if (machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))==0) 	this->m_pc+=machine_p88k(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 15:
	// JMZdirect to indirect
	if (machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))==0) 	this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 16:	
	// JMZindirect to direct
	if (machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) 	this->m_pc+=machine_p88k(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 17:
	// JMZindirect to indirect
	if (machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) 	this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 18:
	// JMGdirect to direct
	if (machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))>0) 	this->m_pc+=machine_p88k(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 19:
	// JMGdirect to indirect
	if (machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))>0) 	this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 20:	
	// JMGindirect to direct
	if (machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+2)))>0) 	this->m_pc+=machine_p88k(m,this->m_pc+1);
	else 	this->m_pc+=3;
	break;
      case 21:
	// JMGindirect to indirect
	if (machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+2)))>0) 	this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc+=3;
	break;
      case 22:
	// Dec, jump if zero... sub 1 from b. jump to a if b is zero
	// DJZ dir to dir
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2), machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))-1);
	if (machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))==0)	  this->m_pc+=machine_p88k(m,this->m_pc+1);
	else this->m_pc+=3;
	break;
      case 23:
	// DJZ dir to indir
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2), machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))-1);
	if (machine_p88k(m,this->m_pc+machine_p88k(m,this->m_pc+2))==0)	  this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	else this->m_pc+=3;
	break;
      case 24:
	// DJZ indir to dir
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)), machine_p88k(m,machine_peek(m,this->m_pc+2)-1));
	if (machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) this->m_pc+=machine_p88k(m,this->m_pc+1);
	else this->m_pc+=3;
	break;
      case 25:
	// DJZ indir to indir
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)), machine_p88k(m,machine_peek(m,this->m_pc+2)-1));
	if (machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+2)))==0) this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,machine_peek(m,this->m_pc+1)));
	else this->m_pc+=3;
	break;
      case 26:
	// SPL
	//- add new thread at address x
	cpustackpush(m,(u16)m->m_memory[this->m_pc+1],(u16)m->m_memory[this->m_pc+2],6,this->m_del);
	break;
      case 27:
	this->m_pc+=3;
	break;
      case 28:
	// input to direct.
	machine_poke(m,this->m_pc+machine_p88k(m,this->m_pc+2),randi()%255);
	this->m_pc+=3;
	break;
      case 29:
	// to indirect.
	machine_poke(m,machine_peek(m,machine_peek(m,this->m_pc+2)),randi()%255);
	this->m_pc+=3;
	break;
      }
      //      printf("%c",this->m_pc);
      break;    

///////////////////////////////////////////////////////////////

    case 7:
      // SIR: add in input and output??? untested for audio but...
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%5){
      case 0:
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
	if (machine_p88k(m,this->m_pc+1)<128) machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)+1);
	this->m_pc++;
	break;
      case 1:	  
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128))    if (randi()%10 < 4) cells[IP] = dead;     
	if (machine_p88k(m,this->m_pc+1)<128)  if (randi()%10 < 4) machine_poke(m,this->m_pc,255);     
	this->m_pc++;
	break;
      case 2:
	//  if (cells[(IP+1)] >= 128) cells[IP] = recovered;
	if (machine_p88k(m,this->m_pc+1)>=128)  machine_poke(m,this->m_pc,129);
	this->m_pc++;
	break;
      case 3:
	if (machine_p88k(m,this->m_pc+1)==0){
	  if ((machine_p88k(m,this->m_pc-1)>0 && machine_p88k(m,this->m_pc-1)<128) ||
	      (machine_p88k(m,this->m_pc+1)>0 && machine_p88k(m,this->m_pc+1)<128))
	    {
	if (randi()%10 < 4) machine_poke(m,this->m_pc,129);
	    }
	}
	this->m_pc++;
	break;
#ifndef PCSIM
      case 4:
	machine_poke(m,this->m_pc,randi()%255);
#endif
	break;

      }
      break;    

///////////////////////////////////////////////////////////////

    case 8:
      // WOrmcode: but was with overlaps and also we need extra buffer to steer!
      //      instr=machine_peek(m,this->m_pc);
      //      printf("instr %d ",instr);

      //      this->m_reg8bit2=biotadir[randi()%8]; // replace with buffer steering TODO or:
      wormdir=randi()%8;
      this->m_reg16bit1=biotadir[wormdir];
      this->m_pc+=this->m_reg16bit1;
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
      switch(instr%13){
      case 0:
	break;
      case 1:
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)+1);
	break;
      case 2:
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)-1);
	break;
      case 3:
	this->m_pc+=machine_p88k(m,this->m_pc+this->m_reg16bit1);
	break;
      case 4:
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)+machine_p88k(m,this->m_pc+this->m_reg16bit1));
	break;
      case 5:
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)-machine_p88k(m,this->m_pc+this->m_reg16bit1));
	break;
      case 6:
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)<<1);
	break;
      case 7:
	machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc)>>1);
	break;
      case 8:
	if (machine_p88k(m,this->m_pc+(this->m_reg8bit2*2))==0) this->m_pc+=machine_p88k(m,this->m_pc+this->m_reg16bit1);
	break;
      case 9:
	flag=machine_p88k(m,this->m_pc);
	machine_poke(m,this->m_pc-this->m_reg16bit1,flag);
	machine_poke(m,this->m_pc+this->m_reg16bit1,flag);
	break;
      case 10:
	thread_push(this,machine_p88k(m,this->m_pc+this->m_reg16bit1));
	break;
      case 11:
	machine_poke(m,this->m_pc+this->m_reg16bit1,thread_pop(this));
	break;
#ifndef PCSIM
      case 12:
	machine_poke(m,this->m_pc+this->m_reg16bit1,adc_buffer[thread_pop(this)%10]);      
#endif
	break;
      }

      //      printf("%c",instr);
      break;    

///////////////////////////////////////////////////////////////

    case 9:
      // stack machine a la: http://www.ece.cmu.edu/~koopman/stack_computers/sec3_2.html#321
      // but sans return stack
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%16){
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
	thread_push(this,machine_p88k(m,thread_pop(this)));
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
	if (thread_pop(this)==0) this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	else 	this->m_pc++;
	break;
      case 12:
	//sub call
	thread_push(this,this->m_pc);
	this->m_pc=this->m_start+machine_p88k(m,machine_peek(m,this->m_pc+1));
	break;
      case 13:
	//sub return
	this->m_pc=this->m_start+thread_pop(this);
	break;
      case 14:
	thread_push(this,machine_p88k(m,this->m_pc+1));
	this->m_pc++;
	break;
#ifndef PCSIM
      case 15:
	machine_poke(m,machine_peek(m,this->m_pc+1),adc_buffer[thread_pop(this)%10]);      
#endif
	break;

      }
      //      printf("%c",this->m_pc);
      break;    

///////////////////////////////////////////////////////////////

    case 10:
      // befunge: http://en.wikipedia.org/wiki/Befunge
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%31){
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
	this->m_reg8bit2=2;
	break;
      case 18: // left
	this->m_reg8bit2=6;
	break;
      case 19: // up
	this->m_reg8bit2=0;
	break;
      case 20: // down
	this->m_reg8bit2=4;
	break;
      case 21:
	this->m_reg8bit2=(randi()%4)*2;
	break;
      case 22:
	if (thread_pop(this)==0)	this->m_reg8bit2=2;
	else 	this->m_reg8bit2=6;
	break;
      case 23:
	if (thread_pop(this)==0)	this->m_reg8bit2=4;
	else 	this->m_reg8bit2=0;
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
	this->m_pc+=biotadir[this->m_reg8bit2%8];
	break;
      case 28:
	machine_poke(m,(thread_pop(this))*(thread_pop(this)),thread_pop(this));
	break;
      case 29:
	thread_push(this,machine_p88k(m,thread_pop(this)*thread_pop(this)));
	break;
#ifndef PCSIM
      case 30:
	machine_poke(m,(thread_pop(this))*(thread_pop(this)),adc_buffer[thread_pop(this)%10]);      
#endif
	break;

      }
      wormdir=this->m_reg8bit2%8;
      this->m_pc+=biotadir[wormdir];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 11:
      // kind of langton's ant as CPU - each thread has direction and
      // changes state of cell according to stack?
      // turn right/left/flip and move on
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
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
	  this->m_reg8bit2+=2;
	  machine_poke(m,this->m_pc,instr^thread_pop(this));
	  break;
	case 13:
	  this->m_reg8bit2-=2;
	  machine_poke(m,this->m_pc,instr^thread_pop(this));
	  break;
	}
      wormdir=this->m_reg8bit2%8;
      this->m_pc+=biotadir[wormdir];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 12:
      // turmites code: turmite has state,direction,position...
      // reg8bit1 is state. reg8bit2 is direction index.
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
      machine_poke(m,this->m_pc,instr+this->m_reg8bit1);
      //delta = dmove[(instr - this->reg8bit1) & 0xf];
      flag=instr - this->m_reg8bit1;
      //tm->dir = (tm->dir + delta) & 3;
      this->m_reg8bit2=(this->m_reg8bit2+flag)&8;
      //do move and wrap
      wormdir=this->m_reg8bit2;
      this->m_pc+=biotadir[wormdir];
      // finally
      this->m_reg8bit1 += deltastate[instr%16];
      //      printf("%c",this->m_pc);
      break;
///////////////////////////////////////////////////////////////

    case 13:
      // linear CA/life code. copies to 128 steps ahead new state and keeps going(?)
      if (this->m_pc>this->m_wrap) this->m_pc=this->m_start;
      instr=machine_p88k(m,this->m_pc);
      other=(machine_p88k(m,this->m_pc-1)&1)+(machine_p88k(m,this->m_pc+1)&1)+(machine_p88k(m,this->m_pc-32)&1)+(machine_p88k(m,this->m_pc+32)&1)+(machine_p88k(m,this->m_pc-31)&1)+(machine_p88k(m,this->m_pc-33)&1)+(machine_p88k(m,this->m_pc+31)&1)+(machine_p88k(m,this->m_pc+33)&1);

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
      instr=machine_p88k(m,this->m_pc);
      //      machine_poke(m,this->m_pc,instr+1);
      machine_poke(m,this->m_pc,instr+biotadir[this->m_reg8bit1%8]);
      this->m_reg8bit1=antrule(this->m_reg8bit1,instr%8,machine_p88k(m,0));//last is rule
      this->m_pc+=biotadir[this->m_reg8bit1%8];
      //      printf("%c",this->m_pc);
      break;

    case 15:
      // second CA from CA.c
      flag = 0;
      instr=machine_p88k(m,this->m_pc);
      if (machine_p88k(m,this->m_pc+1)>128)	flag |= 0x4;
      if (instr>128) flag |= 0x2;
      if (machine_p88k(m,this->m_pc-1)>128)	flag |= 0x1;
                     
      if ((machine_p88k(m,0) >> flag) & 1)	machine_poke(m,this->m_pc+128,instr+129);
      else machine_poke(m,this->m_pc+128,instr-129); // or we stay with 255 and 0 as poked

      this->m_pc++; 
      if (this->m_pc==0) this->m_pc=1;
      //      printf("%c",instr);
      break;

    case 16:
      // **TODO** port of hodge - but we need larger 256*128 (32768) cellspace in two halves
      // numill and numinf
      flag=temp=0;
      temp=machine_p88k(m,this->m_pc)+machine_p88k(m,this->m_pc-1)+machine_p88k(m,this->m_pc+1)+machine_p88k(m,this->m_pc-256)+machine_p88k(m,this->m_pc+256)+machine_p88k(m,this->m_pc-255)+machine_p88k(m,this->m_pc-257)+machine_p88k(m,this->m_pc+255)+machine_p88k(m,this->m_pc+257);

      if (machine_p88k(m,this->m_pc-1)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc+1)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc-256)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc+256)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc-255)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc-257)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc+255)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;
      if (machine_p88k(m,this->m_pc+257)==machine_p88k(m,0)-1) flag++; else if (machine_p88k(m,this->m_pc-1)>0) other++;

      y=this->m_pc+32768;
      if (y<4) y=4;

  if(machine_p88k(m,this->m_pc) == 0)
    machine_poke(m,this->m_pc,floor(other / (machine_p88k(m,1)+1)) + floor(flag/(machine_p88k(m,2)+1)));
  else if(machine_p88k(m,this->m_pc) < machine_p88k(m,0)-1)
    machine_poke(m,y,floor(temp / (other + 1)) + machine_p88k(m,3));
  else
    machine_poke(m,y,0);

  if(machine_p88k(m,this->m_pc) > machine_p88k(m,0)-1)
    machine_poke(m,y,machine_p88k(m,0)-1);

  this->m_pc++; 
  if (this->m_pc<4) this->m_pc=4;

  break;
    case 17:
      // start generic - add (add/sub/zero/copy/invert/swap)
      instr=machine_p88k(m,this->m_pc);
      machine_poke(m,this->m_pc,instr+1);
      this->m_pc++; 
      break;
    case 18:
      // generic - sub (add/sub/zero/copy/invert/swap)
      instr=machine_p88k(m,this->m_pc);
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
      machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc+1));
      this->m_pc++; 
      break;
    case 21:
      // generic - copy (add/sub/zero/copy/invert/swap)
      machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc^255));
      this->m_pc++; 
      break;
    case 22:
      // generic - swap (add/sub/zero/copy/invert/swap)
      instr=machine_p88k(m,this->m_pc+1);
      machine_poke(m,this->m_pc+1,machine_p88k(m,this->m_pc));
      machine_poke(m,this->m_pc,instr);
      this->m_pc++; 
      break;
#ifndef PCSIM
    case 23:
      machine_poke(m,machine_peek(m,this->m_pc++),adc_buffer[machine_p88k(m,this->m_pc)%10]);     
      break;
#endif
///////////////////////////////////////////////////////////////
    case 24:
      // from wormcode.c
      instr=machine_p88k(m,this->m_pc);
      switch(instr%15)
	{
	case 0:
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 1:
	  //inc
	  machine_poke(m,this->m_pc,instr+1);
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 2:
	  machine_poke(m,this->m_pc,instr-1);
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 3:
	  this->m_pc=this->m_start+machine_p88k(m,this->m_pc);
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 4:
	  machine_poke(m,this->m_pc,randi()%255);
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 5:
	  machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc+biotadir[randi()%8]));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 6:
	  machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc+biotadir[randi()%8]));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 7:
	  machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc-biotadir[randi()%8]));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 8:
	  machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc<<biotadir[randi()%8]));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 9:
	  machine_poke(m,this->m_pc,machine_p88k(m,this->m_pc>>biotadir[randi()%8]));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 10:
	  if (machine_p88k(m,this->m_pc+(biotadir[randi()%8]*2))==0){
	    wormdir=biotadir[randi()%8];
	    this->m_pc+=wormdir;
	  }
	  break;
	case 11:
	  machine_poke(m,(this->m_pc-biotadir[randi()%8]),instr);
	  machine_poke(m,(this->m_pc+biotadir[randi()%8]),instr);
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 12:
	  thread_push(this, machine_p88k(m,this->m_pc+biotadir[randi()%8]));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
	case 13:
	  machine_poke(m,(this->m_pc+=biotadir[randi()%8]),thread_pop(this));
	  wormdir=biotadir[randi()%8];
	  this->m_pc+=wormdir;
	  break;
#ifndef PCSIM
      case 14:
	machine_poke(m,(this->m_pc+=biotadir[randi()%8]),adc_buffer[thread_pop(this)%10]);      
#endif
	break;
	}
      break;
///////////////////////////////////////////////////////////////
    case 25:
      // 16 bit increment
      instr=machine_p88k(m,this->m_pc);
      y=(instr<<8)+machine_p88k(m,this->m_pc+1)+1;
      machine_poke(m,this->m_pc,y>>8);      
      machine_poke(m,this->m_pc+1,y&255);      
      this->m_pc+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 26:
      // 16 bit decrement
      instr=machine_p88k(m,this->m_pc);
      y=(instr<<8)+machine_p88k(m,this->m_pc+1)-1;
      machine_poke(m,this->m_pc,y>>8);      
      machine_poke(m,this->m_pc+1,y&255);      
      this->m_pc+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 27:
      // 16 bit left
      instr=machine_p88k(m,this->m_pc);
      y=(instr<<9)+machine_p88k(m,this->m_pc+1)<<1;
      machine_poke(m,this->m_pc,y>>8);      
      machine_poke(m,this->m_pc+1,y&255);      
      this->m_pc+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 28:
      // 16 bit right
      instr=machine_p88k(m,this->m_pc);
      y=(instr<<7)+machine_p88k(m,this->m_pc+1)>>1;
      machine_poke(m,this->m_pc,y>>8);      
      machine_poke(m,this->m_pc+1,y&255);      
      this->m_pc+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 29:
      // pure leakage - push instr onto stack. when stack is full pull off...
      if (thread_stack_count(this,STACK_SIZE)) machine_poke(m,this->m_pc,thread_pop(this));
      else thread_push(this,machine_p88k(m,this->m_pc));
      this->m_pc++;
      break;
///////////////////////////////////////////////////////////////
    case 30:
      // convolution
      temp=(machine_p88k(m,this->m_pc-1)*machine_p88k(m,0))+(machine_p88k(m,this->m_pc)*machine_p88k(m,1))+(machine_p88k(m,this->m_pc+1)*machine_p88k(m,2));
      y=this->m_pc+32768;
      if (y<3) y=3;
      machine_poke(m,y,temp);
      this->m_pc++; 
      if (this->m_pc<3) this->m_pc=3;

    }
      this->m_delc=0;
  } // if del
}

u8 thread_stack_count(thread* this, u8 c) { 
    return (c-1)<=this->m_stack_pos; 
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
  //  int count=0;
  //    this->m_heap = (u8*)malloc(sizeof(u8)*HEAP_SIZE);
  this->m_threadcount=0;
  this->m_memory=buffer;
    this->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED
}

u16 machine_peek(const machine* this, uint16_t addr) {
  //	return this->m_heap[addr%HEAP_SIZE];
  return (this->m_memory[addr]<<8)+this->m_memory[addr+1];
}

u8 machine_p88k(const machine* this, uint16_t addr) {
  //	return this->m_heap[addr%HEAP_SIZE];
  return this->m_memory[addr];
}


void machine_poke(machine* this, uint16_t addr, u8 data) {
  //	this->m_heap[addr%HEAP_SIZE]=data;
  this->m_memory[addr]=data;
}

void machine_run(machine* this) {
	for (unsigned char n=0; n<this->m_threadcount; n++) {
		thread_run(&this->m_threads[n],this);
	}
	
	
	if ((randi()%this->m_leakiness)==0) {
	    	leak(this);
  	}
	
	// do infection/mutation
	
	if ((randi()%this->m_infectprob)==0) {
	  infectcpu(this,randi()%16,randi()%16,randi()%this->m_threadcount);
	  //void infectcpu(machine *m, u8 probI, u8 probD, u8 infected){
	}
	
	if ((randi()%this->m_mutateprob)==0) {

	  //void mutate(machine *m, u8 which, u8 identifier){
	  mutate(this,randi()%this->m_threadcount,randi()%16);

	  //	  infectcpu(this,randi()%16,randi()%16,randi()%this->m_threadcount);
	  //void infectcpu(machine *m, u8 probI, u8 probD, u8 infected){
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
      for (count=xx->m_stack_pos;count>0;count--){
	    xx->m_stack[count-1]=xx->m_stack[count];
	    //	    printf("xxxxleaky %d\n", count);
      }
		 xx->m_stack_pos--;
  }
}

void mutate(machine *m, u8 which, u8 identifier){
  // select CPU and flip bits within contraints
  // what are identifiers? del, wrap, pc also registers or not?
  u8 x; u16 temp;
  //  x=which%m->m_threadcount;
  thread *this=&m->m_threads[which];
  switch (identifier%4){
  case 0:
    this->m_CPU^=(1<<randi()%4);
    break;
  case 1:
    temp=this->m_wrap-this->m_start;
    temp^=(1<<randi()%16);
    this->m_wrap=this->m_start+temp;
    break;
  case 2:
    this->m_del^=(1<<randi()%8);
    break;
  case 3:
    this->m_pc^=(1<<randi()%16);
    this->m_wrap=this->m_start+randi();
    break;
  }
}

void swapcpu(machine *m, u8 which, u8 identifier){
  u8 x,y,temp; u16 tempi;
  x=randi()%m->m_threadcount;
  y=(x+1)%m->m_threadcount;
  thread *this=&m->m_threads[x];
  thread *that=&m->m_threads[y];
  switch (identifier%3){
  case 0:
    temp=this->m_CPU;
    this->m_CPU=that->m_CPU;
    that->m_CPU=temp;
    break;
  case 1:
    tempi=this->m_start;
    this->m_start=that->m_start;
    that->m_start=tempi;
    tempi=this->m_wrap;
    this->m_wrap=that->m_wrap;
    that->m_wrap=tempi;
    break;
  case 2:
    temp=this->m_del;
    this->m_del=that->m_del;
    that->m_del=temp;
    break;
  }
}

void killcpu(machine *m, u8 killed);

void infectcpu(machine *m, u8 probI, u8 probD, u8 infected){
  // chances of infecting neighbouring cell which in time is killed or recovers
  thread *this=&m->m_threads[(infected+1)%m->m_threadcount];

  // if is recovered leave alone
  if (this->m_infection==129) return;

  // if is infected calc whether to kill or inc to recovery
  else if (this->m_infection>1) {
    if (randi()%255 <=probD) killcpu(m,(infected+1)%m->m_threadcount);
    else this->m_infection++;
  }

  // if not infected then prob to infect it
  else if (this->m_infection==0){
    if (randi()%255 <= probI) this->m_infection=1;
  }
}

void killcpu(machine *m, u8 killed){
  u8 x;
  // or should we run through and kill one - how re-org list.
  //  cpustackpop(m);
  //  thread *this=&m->m_threads[which];
  //this->m_threads[this->m_threadcount]
  for (x=killed;x<m->m_threadcount-1;x++){
    m->m_threads[x]=m->m_threads[x+1];
  }
  m->m_threadcount--;
}



#ifdef PCSIM
int main(void)
{
  int x; u16 addr;
  u8 buffer[65536];// u16 *testi; u8 *testo;
  srandom(time(0));
  for (x=0;x<65536;x++){
    buffer[x]=randi()%255;
  }


  /*
  testo=(u8 *)buffer+1;
  testi=(u16 *)buffer+1;
  //  (u8 *)buffer+=1;
  //  (u16 *)testi;
  printf("buffer %p\n",(long *)testi);
  printf("buffer %p\n",(long *)testo); // 
  */

  machine *m=(machine *)malloc(sizeof(machine));
  machine_create(m,buffer); // this just takes care of pointer to machine and malloc for threads
  m->m_leakiness=randi()%255;
  m->m_infectprob=randi()%255;
  m->m_mutateprob=randi()%255;

  // what about swapping????

  u8 flag,other;
  u8 deltastate[ ] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};	/* change in state indexed by color */
  //  printf("running: %d ",this->m_start);
  //  sleep(1);
  u16 biotadir[8]={65280,65281,1,257,256,255,65535,65279}; //65536

  //  dircalc(biotadir,65536,256);

	for (unsigned char n=0; n<100; n++)
	{
	  // 	  cpustackpush(m,randi()%65536,randi()%CPU_TOTAL);

// void cpustackpush(machine *this, u16 address, u16 wrapaddress, u8 cputype, u8 delay)

/*
#ifdef PCSIM
    this->m_wrap=this->m_start+randi()%65536;
#else
    this->m_wrap=this->m_start+randi();
#endif
*/
	  // 	  cpustackpush(m,addr,addr+randi()%65536,randi()%25,randi()%255);


  // SWITCH for m_CPU!

	  addr=randi()%65536;
	  // 	  cpustackpush(m,addr,addr+randi()%65536,randi()%25,randi()%255);
	  	  cpustackpush(m,addr,addr+randi()%65536,randi()%31,randi()%255);
		  //	  	  cpustackpush(m,addr,addr+randi()%65536,30,randi()%255);
	  //	  cpustackpush(m,addr,addr+randi()%65536,26,randi()%255);
	}

	while(1) {
          machine_run(m);
	  }
}
#endif
