//OCT2014 altered to ease porting to rev4.c!!!!! /////

/* 

Villager program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

 Villager program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with villager program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301 USA

Based in part on spork factory by Dave Griffiths.

*/

// was CPU.c but now with overlap, uint16_t blah

#ifdef LACH
#define SETSIZE 36
#define INFECTSIZE 740 
#define SETSHIFT 11
#define SHIFTY 7
#else
#define SETSIZE 66
#define INFECTSIZE 770 
#define SETSHIFT 10
#define SHIFTY 6
#endif


#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "CPUint.h"
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

void leak(machine *m);

// changes for non-stacked approach!

void thread_create(thread *villager, u16 address, u16 wrapaddress, uint8_t which, u8 delay) { // ??? or we steer each of these?
  villager->m_infection=0;
  villager->m_CPU=which;
  villager->m_del=delay; villager->m_delc=0;
  villager->start=address;
  villager->wrap=wrapaddress;

  villager->position=address;
  villager->m_reg8bit1=randi()%255;
  villager->m_reg8bit2=randi()%255;
  villager->m_reg16bit1=address;
  villager->m_stack_pos=-1;

    for (u8 n=0; n<STACK_SIZEE; n++)
      {
	villager->m_stack[n]=0;
      }
}

void cpustackpush(machine *villager, u16 address, u16 wrapaddress,u8 cputype, u8 delay){
  if (villager->m_threadcount>=MAX_THREADS) return;
  else {
    //    printf("Thread %d Thread buffer %ld\n",villager->m_threadcount,buffer);
    thread_create(&villager->m_threads[villager->m_threadcount], address, wrapaddress,cputype,delay);// last is CPU type!
  villager->m_threadcount++;
  }
}

void cpustackpop(machine *villager){
  villager->m_threadcount--;
  if (villager->m_threadcount<0) villager->m_threadcount=0;
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

void thread_run(thread* villager, machine *m) {
  u8 instr,temp;
  u16 y;
  u8 flag,other=0;// float other=0.0f;
  u8 deltastate[16] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};


  //  dircalc(biotadir,65536,256);
  if (++villager->m_delc>=villager->m_del){

    villager->m_delc=0;

#ifdef PCSIM

    //    printf("
    //    printf("CPU: %d pc: %d\n",villager->m_CPU, villager->position);
	    //    printf("%c",machine_p88k(villager->position));

#endif

    switch(villager->m_CPU)
      {
      case 0: // :LEAKY STACK! - working!
      instr=machine_p88k(villager->position);
      villager->position++;
      if (villager->position>villager->wrap) villager->position=villager->start;
      //			printf("%d", instr);
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
      break;

///////////////////////////////////////////////////////////////

    case 1:// BIOTA see: http://c2.com/cgi/wiki?BiotaLanguage
      if (villager->position>villager->wrap) villager->position=villager->start;

      //      instr=machine_peek(villager->position);
      instr=machine_p88k(villager->position);

      flag=0;
      //      printf("instr %d ",instr);
      switch(instr%8)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  villager->m_reg16bit1+=biotadir[villager->m_reg8bit1%8];
	  if (machine_p88k(villager->m_reg16bit1)==0) flag=1;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+4)%8];
	  if (machine_p88k(villager->m_reg16bit1)==0) flag=1;
	  break;
	case 2:
	  //* t -- turn DC right 45 degrees
	  villager->m_reg8bit1+=1;
	  break;
	case 3:
	  //* u -- unturn: turn DC left 45 degrees
	  villager->m_reg8bit1-=1;
	  break;
	case 4:
	  //    * g -- go to a non-empty character ahead (tries to move DC straight ahead, then right and left 45 degrees, then 90, then 135, then back).
	  temp=villager->m_reg8bit1%8;
	  villager->m_reg16bit1+=biotadir[temp];
	  if (machine_p88k(villager->m_reg16bit1)==0) {
	    villager->m_reg16bit1-=biotadir[temp]; // go back
	    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+1)%8]; // right 45
	    if (machine_p88k(villager->m_reg16bit1)==0) {
	      villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1+1)%8]; // go back
	      villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1-1)%8]; // left 45
	      if (machine_p88k(villager->m_reg16bit1)==0) {
		villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-1)%8]; // go back
		villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+2)%8]; // right 90
		if (machine_p88k(villager->m_reg16bit1)==0) {
		  villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1+2)%8]; // go back
		  villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1-2)%8]; // left 90
		  if (machine_p88k(villager->m_reg16bit1)==0) {
		    villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-2)%8]; // go back
		    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+3)%8]; // right 135
		    if (machine_p88k(villager->m_reg16bit1)==0) {
		    villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-3)%8]; // go back
		    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1-3)%8]; // left 135
		    if (machine_p88k(villager->m_reg16bit1)==0) {
		    villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-3)%8]; // go back
		    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+4)%8]; // back
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
	  if (machine_p88k(villager->m_reg16bit1)!=0) machine_poke(villager->m_reg16bit1,0); 
	  else flag=1;
	  break;
	case 6:
	  //    * d -- duplicate the current data into the cell left of the DC. Fails if source cell is empty or target cell is non-empty.
	  // left is -2

	  if (machine_p88k(villager->m_reg16bit1)==0 || (machine_p88k(villager->m_reg16bit1+(villager->m_reg16bit1+((villager->m_reg8bit1-4)%8))))!=0) flag=1;
	  else machine_poke(villager->m_reg16bit1+(villager->m_reg16bit1+((villager->m_reg8bit1-4)%8)),machine_p88k(villager->m_reg16bit1));
	  break;
	case 7:
	  //    * . -- no-op, a non-empty do nothing. 
	  break;
	}

      // - villager->position turns (where?) when it finds an empty location or a failing instruction
      // m_reg8bit3 is direction
      wormdir=villager->m_reg8bit2%8;
      if (machine_p88k(villager->position)==0 || flag==1){
	villager->m_reg8bit2-=1;
      }
      else villager->position+=biotadir[wormdir];
      //      printf("%c",villager->position);
      break;
      
/////////////////////////////////////////////////////////////////

    case 2:
      // brainfuck: add in output???

      if (villager->position>villager->wrap) villager->position=villager->start;

      //      instr=machine_peek(villager->position);
      instr=machine_p88k(villager->position);

      //      printf("instr %d ",instr);
      switch(instr%7)
	{
	case 0:
	  villager->m_reg16bit1+=1;
	  villager->position++;
	  break;
	case 1:
	  villager->m_reg16bit1-=1;
	  villager->position++;
	  break;
	case 2:
	  //	    cells[omem]=cells[omem]++; 
	  machine_poke(villager->m_reg16bit1,machine_peek(villager->m_reg16bit1)+1);
	  villager->position++;
	  break;
	case 3:
	  machine_poke(villager->m_reg16bit1,machine_peek(villager->m_reg16bit1)-1);
	  villager->position++;
	  break;
	case 4:
	  villager->m_reg8bit2+=2;
	  if (villager->m_reg8bit2>=14) villager->m_reg8bit2=0;
	  villager->m_stack[villager->m_reg8bit2%STACK_SIZEE]= villager->position>>8;
	  villager->m_stack[(villager->m_reg8bit2+1)%STACK_SIZEE]= villager->position&255;
	  villager->position++;
	  break;
	case 5:
	  if (villager->m_reg8bit2>=14) villager->m_reg8bit2=0;
	  if (machine_p88k(villager->m_reg16bit1)!=0) villager->position=((villager->m_stack[villager->m_reg8bit2%STACK_SIZEE])<<8)+((villager->m_stack[(villager->m_reg8bit2+1)%STACK_SIZEE]));
	  villager->m_reg8bit2-=2;
	  if (villager->m_reg8bit2==0) villager->m_reg8bit2=14;
	  break;
	case 6:
	  //  cells[omem] = adcread(3); 
	  machine_poke(villager->m_reg16bit1,adc_buffer[villager->m_reg16bit1%10]);
	  villager->position++;
	  break;
	}
      //      printf("%c",villager->position);
      break;    
///////////////////////////////////////////////////////////////

    case 3:
      // masque red death: 

      if (villager->position>=villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);

      //      instr=machine_peek(villager->position);
      //      printf("instr %d ",instr);
      switch(instr%7){
      case 0:
	if (villager->m_reg8bit2==12){
	  machine_poke(villager->position+1,machine_p88k(villager->position));
	  if (machine_p88k(villager->position)==255) villager->m_reg8bit2=13;
	  villager->position++;
	}
	else villager->position++;
	break;
      case 1:
	if (villager->m_reg8bit2==13){
	  villager->m_reg16bit1++;
	  machine_poke(villager->m_reg16bit1,audio_buffer[villager->position%32768]); //READ IN
	  villager->position++;
	}
	else villager->position++;
	break;
      case 2:
	villager->m_reg8bit2++;
	if (villager->m_reg8bit2==60){
	  villager->m_reg8bit2++;
	  machine_poke(villager->m_reg16bit1,machine_p88k(villager->m_reg16bit1)^255);
	  villager->m_reg8bit2=0;
	}
	else villager->position++;
	break;
	case 3:
	  //	  seven rooms: 
	  temp= (villager->position)%7;
	  for (u8 x=11;x<22;x++){//11-21
	    //	    settingsarray[x]=settingsarray[x]>>temp;
	  } // CONSTRAINT
	  villager->position++;
	  break;
	case 4:
	  machine_poke(villager->m_reg16bit1-1,machine_p88k(villager->m_reg8bit1-1)^255);
	  machine_poke(villager->m_reg16bit1+1,machine_p88k(villager->m_reg8bit1+1)^255);
	  villager->position++;
	  break;
	case 5:
	  flag=randi()%4;
	  if (flag==0) 	  villager->m_reg8bit2++;
	  if (flag==1) 	  villager->m_reg8bit2--;
	  if (flag==2) 	  villager->m_reg8bit2+=16;
	  if (flag==3) 	  villager->m_reg8bit2-=16;
	  villager->position++;
	  break;
	case 6:
	  machine_poke(villager->m_reg16bit1+2,adc_buffer[((villager->m_reg16bit1)>>8)%10]);
	  villager->position++;
	  break;
      }
      break;    

      //      printf("%c",villager->position);
///////////////////////////////////////////////////////////////

    case 4:
      // plague: add in output???

      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);

      //      instr=machine_peek(villager->position);
      //            printf("instr %d\n",instr%5);
      switch(instr%5){
      case 0:
	machine_poke(villager->position,255);
	machine_poke(villager->position+1,255);
	//	villager->position+=2;
	villager->position+=biotadir[villager->m_reg8bit2%8];

	break;
      case 1:
	if (machine_p88k(villager->position)<128){
	  machine_poke(villager->position-1,machine_p88k(villager->position));
	  machine_poke(villager->position+1,machine_p88k(villager->position));
	}
	villager->position+=biotadir[villager->m_reg8bit2%8];
	break;
      case 2:
	machine_poke(villager->position-1,0);
	machine_poke(villager->position+1,0);
	villager->position+=biotadir[villager->m_reg8bit2%8];
	break;
      case 3:
	if ((machine_p88k(villager->position)%0x03)==1) villager->m_reg8bit2+=4;
	else villager->m_reg8bit2*=machine_p88k(villager->position)>>4;
	villager->position+=biotadir[villager->m_reg8bit2%8];
	break;
      case 4:
	machine_poke(villager->position+1,adc_buffer[(villager->m_reg8bit1>>8)%10]);
	villager->position+=biotadir[villager->m_reg8bit2%8];
	  break;
      }

      if (machine_p88k(villager->position)==255) villager->m_reg8bit2+=4;
      	wormdir=villager->m_reg8bit2%8;
	//      printf("%c",villager->position);
      break;    
	
///////////////////////////////////////////////////////////////

      case 5:
      // first from micro: add in output???

      if (villager->position>villager->wrap) villager->position=villager->start;

      instr=machine_p88k(villager->position);
      //            printf("instr %d\n",instr%15);
      switch(instr%15){
      case 0:
	villager->m_reg16bit1+=2;
	villager->position++;
	break;
      case 1:
	villager->m_reg16bit1-=2;
	villager->position++;
	break;
      case 2:
	machine_poke(villager->m_reg16bit1,machine_p88k(villager->m_reg16bit1)+1);
	villager->position++;
	break;
      case 3:	  
	machine_poke(villager->m_reg16bit1,machine_p88k(villager->m_reg16bit1)-1);
	villager->position++;
	break;
      case 4:	  
	machine_poke(villager->position,machine_p88k(villager->position)+1);
	villager->position++;
	break;
      case 5:	  
	machine_poke(villager->position,machine_p88k(villager->position)-1);
	villager->position++;
	break;
      case 6:	  
	machine_poke(villager->position,machine_p88k(villager->position)>>1);
	villager->position++;
	break;
      case 7:	  
	machine_poke(villager->position,machine_p88k(villager->position)>>2);
	villager->position++;
	break;
      case 8:	  
	machine_poke(villager->position,machine_p88k(villager->position)>>3);
	villager->position++;
	break;
      case 9:	  
	if (machine_peek(villager->position+1)==0) {
	    villager->position=villager->start+villager->m_reg16bit1;
	}
	else villager->position++;
	break;
      case 10:	  
	if (machine_p88k(villager->position+1)<128 && machine_p88k(villager->position+1)>0) villager->position+=machine_p88k(villager->position+1);
	else 	villager->position++;
	break;
      case 11:	  
	if (machine_p88k(villager->position-1)<128) machine_poke(villager->position,machine_p88k(villager->position));
	villager->position++;
	break;
      case 12:	  
	machine_poke(villager->position,machine_p88k(villager->position+1));
	villager->position++;
	break;
      case 13:	  
	villager->position++;
	break;
      case 14:
	machine_poke(villager->position,adc_buffer[((villager->m_reg8bit1)>>8)%10]);
	villager->position++;
	break;
      }
      //      printf("%c",villager->position);
///////////////////////////////////////////////////////////////
      break;    

    case 6:
/* "real" corewars redcode SPL 

REF: http://vyznev.net/corewar/guide.html#start_instr

http://www.koth.org/info/akdewdney/images/Redcode.jpg

 */

      if (villager->position>villager->wrap) villager->position=villager->start;

      instr=machine_p88k(villager->position);
      //      printf("instr %d\n",instr%30);

      switch(instr%30){
      case 0:
	// MOV # to direct.
	machine_poke(villager->position+machine_p88k(villager->position+2),machine_p88k(villager->position+1));
	villager->position+=3;
	break;
      case 1:	  
	// MOV indirect to direct.
	machine_poke(villager->position+machine_p88k(villager->position+2),machine_p88k(machine_peek(villager->position+1)));
	villager->position+=3;
	break;
      case 2:
	// MOV # to indirect.
	machine_poke(machine_peek(machine_peek(villager->position+2)),machine_p88k(villager->position+1));
	villager->position+=3;
	break;
      case 3:
	//MOV indirect to indirect
	machine_poke(machine_peek(machine_peek(villager->position+2)),machine_p88k(machine_peek(villager->position+1)));
	villager->position+=3;
	break;
      case 4:
	//ADD # to direct
	machine_poke(villager->position+machine_p88k(villager->position+2),machine_p88k(villager->position+1)+machine_p88k(villager->position+machine_p88k(villager->position+2)));
	villager->position+=3;
	break;
	//HERE
      case 5:
	// ADD indirect to direct.
	machine_poke(villager->position+machine_p88k(villager->position+2),machine_p88k(machine_peek(villager->position+1))+(machine_p88k(villager->position+(machine_p88k(villager->position+2)>>8))));
	villager->position+=3;
	break;
      case 6:
	// ADD # to indirect.
	machine_poke(machine_peek(machine_peek(villager->position+2)),machine_p88k(villager->position+1)+machine_p88k(machine_peek(villager->position+2)));
	villager->position+=3;
	break;
      case 7:
	//ADD indirect to indirect
	machine_poke(machine_p88k(machine_peek(villager->position+2)),machine_p88k(machine_peek(villager->position+1))+machine_p88k(machine_peek(villager->position+2)));
	villager->position+=3;
	break;
      case 8:
	//SUB # to direct
	machine_poke(villager->position+machine_p88k(villager->position+2),machine_p88k(villager->position+1)-machine_p88k(villager->position+machine_p88k(villager->position+2)));
	villager->position+=3;
	break;
      case 9:
	// indirect to direct.
	machine_poke(villager->position+machine_p88k(villager->position+2),machine_p88k(machine_peek(villager->position+1)-machine_p88k(villager->position+machine_p88k(villager->position+2))));
	villager->position+=3;
	break;
      case 10:
	// # to indirect.
	machine_poke(machine_peek(machine_peek(villager->position+2)),machine_p88k(villager->position+1)-machine_p88k(machine_peek(villager->position+2)));
	villager->position+=3;
	break;
      case 11:
	// indirect to indirect
	machine_poke(machine_peek(machine_peek(villager->position+2)),machine_p88k(machine_peek(villager->position+1))-machine_p88k(machine_peek(villager->position+2)));
	villager->position+=3;
	break;
      case 12:
	// jmp to direct
	villager->position+=machine_p88k(villager->position+1);
	break;
      case 13:
	// jmp to indirect
	villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	break;
      case 14:
	// JMZdirect to direct
	if (machine_p88k(villager->position+machine_p88k(villager->position+2))==0) 	villager->position+=machine_p88k(villager->position+1);
	else 	villager->position+=3;
	break;
      case 15:
	// JMZdirect to indirect
	if (machine_p88k(villager->position+machine_p88k(villager->position+2))==0) 	villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	else 	villager->position+=3;
	break;
      case 16:	
	// JMZindirect to direct
	if (machine_p88k(machine_peek(machine_peek(villager->position+2)))==0) 	villager->position+=machine_p88k(villager->position+1);
	else 	villager->position+=3;
	break;
      case 17:
	// JMZindirect to indirect
	if (machine_p88k(machine_peek(machine_peek(villager->position+2)))==0) 	villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	else 	villager->position+=3;
	break;
      case 18:
	// JMGdirect to direct
	if (machine_p88k(villager->position+machine_p88k(villager->position+2))>0) 	villager->position+=machine_p88k(villager->position+1);
	else 	villager->position+=3;
	break;
      case 19:
	// JMGdirect to indirect
	if (machine_p88k(villager->position+machine_p88k(villager->position+2))>0) 	villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	else 	villager->position+=3;
	break;
      case 20:	
	// JMGindirect to direct
	if (machine_p88k(machine_peek(machine_peek(villager->position+2)))>0) 	villager->position+=machine_p88k(villager->position+1);
	else 	villager->position+=3;
	break;
      case 21:
	// JMGindirect to indirect
	if (machine_p88k(machine_peek(machine_peek(villager->position+2)))>0) 	villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	else 	villager->position+=3;
	break;
      case 22:
	// Dec, jump if zero... sub 1 from b. jump to a if b is zero
	// DJZ dir to dir
	machine_poke(villager->position+machine_p88k(villager->position+2), machine_p88k(villager->position+machine_p88k(villager->position+2))-1);
	if (machine_p88k(villager->position+machine_p88k(villager->position+2))==0)	  villager->position+=machine_p88k(villager->position+1);
	else villager->position+=3;
	break;
      case 23:
	// DJZ dir to indir
	machine_poke(villager->position+machine_p88k(villager->position+2), machine_p88k(villager->position+machine_p88k(villager->position+2))-1);
	if (machine_p88k(villager->position+machine_p88k(villager->position+2))==0)	  villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	else villager->position+=3;
	break;
      case 24:
	// DJZ indir to dir
	machine_poke(machine_peek(machine_peek(villager->position+2)), machine_p88k(machine_peek(villager->position+2)-1));
	if (machine_p88k(machine_peek(machine_peek(villager->position+2)))==0) villager->position+=machine_p88k(villager->position+1);
	else villager->position+=3;
	break;
      case 25:
	// DJZ indir to indir
	machine_poke(machine_peek(machine_peek(villager->position+2)), machine_p88k(machine_peek(villager->position+2)-1));
	if (machine_p88k(machine_peek(machine_peek(villager->position+2)))==0) villager->position=villager->start+machine_p88k(machine_peek(machine_peek(villager->position+1)));
	else villager->position+=3;
	break;
      case 26:
	// SPL
	//- add new thread at address x
	cpustackpush(machine_peek(villager->position+1),machine_peek(villager->position+2),6,villager->m_del);
	//	printf("adde\n");
	villager->position+=3;
	break;
      case 27:
	villager->position+=3;
	break;
      case 28:
	// input to direct.
	machine_poke(villager->position+machine_p88k(villager->position+2),randi()%255);
	villager->position+=3;
	break;
      case 29:
	// to indirect.
	machine_poke(machine_peek(machine_peek(villager->position+2)),randi()%255);
	villager->position+=3;
	break;
      }
      //      printf("%c",villager->position);
      break;    

///////////////////////////////////////////////////////////////

    case 7:
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      //      printf("instr %d ",instr);
      //      printf("instr %d\n",instr%5);

      switch(instr%5){
      case 0:
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
	if (machine_p88k(villager->position+1)<128) machine_poke(villager->position,machine_p88k(villager->position)+1);
	villager->position++;
	break;
      case 1:	  
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128))    if (randi()%10 < 4) cells[IP] = dead;     
	if (machine_p88k(villager->position+1)<128)  if (randi()%10 < 4) machine_poke(villager->position,255);     
	villager->position++;
	break;
      case 2:
	//  if (cells[(IP+1)] >= 128) cells[IP] = recovered;
	if (machine_p88k(villager->position+1)>=128)  machine_poke(villager->position,129);
	villager->position++;
	break;
      case 3:
	if (machine_p88k(villager->position+1)==0){
	  if ((machine_p88k(villager->position-1)>0 && machine_p88k(villager->position-1)<128) ||
	      (machine_p88k(villager->position+1)>0 && machine_p88k(villager->position+1)<128))
	    {
	if (randi()%10 < 4) machine_poke(villager->position,129);
	    }
	}
	villager->position++;
	break;
      case 4:
	machine_poke(villager->position,randi()%255);
	//	villager->position++;
	break;
      }
      break;    

///////////////////////////////////////////////////////////////

    case 8:
      // WOrmcode: but was with overlaps and also we need extra buffer to steer!
      //      instr=machine_peek(villager->position);
      //      printf("instr %d ",instr);
      
      wormdir=randi()%8;
      villager->m_reg16bit1=biotadir[wormdir];
      villager->position+=villager->m_reg16bit1;
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      switch(instr%13){
      case 0:
	break;
      case 1:
	machine_poke(villager->position,machine_p88k(villager->position)+1);
	break;
      case 2:
	machine_poke(villager->position,machine_p88k(villager->position)-1);
	break;
      case 3:
	villager->position+=machine_p88k(villager->position+villager->m_reg16bit1);
	break;
      case 4:
	machine_poke(villager->position,machine_p88k(villager->position)+machine_p88k(villager->position+villager->m_reg16bit1));
	break;
      case 5:
	machine_poke(villager->position,machine_p88k(villager->position)-machine_p88k(villager->position+villager->m_reg16bit1));
	break;
      case 6:
	machine_poke(villager->position,machine_p88k(villager->position)<<1);
	break;
      case 7:
	machine_poke(villager->position,machine_p88k(villager->position)>>1);
	break;
      case 8:
	if (machine_p88k(villager->position+(villager->m_reg8bit2*2))==0) villager->position+=machine_p88k(villager->position+villager->m_reg16bit1);
	break;
      case 9:
	flag=machine_p88k(villager->position);
	machine_poke(villager->position-villager->m_reg16bit1,flag);
	machine_poke(villager->position+villager->m_reg16bit1,flag);
	break;
      case 10:
	thread_push(villager,machine_p88k(villager->position+villager->m_reg16bit1));
	break;
      case 11:
	machine_poke(villager->position+villager->m_reg16bit1,thread_pop(villager));
	break;
      case 12:
	machine_poke(villager->position+villager->m_reg16bit1,adc_buffer[thread_pop(villager)%10]);      
	break;
      }

      //      printf("%c",instr);
      break;    

///////////////////////////////////////////////////////////////

    case 9:
      // stack machine a la: http://www.ece.cmu.edu/~koopman/stack_computers/sec3_2.html#321
      // but sans return stack
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      //            printf("instr %d\n",instr%16);
      switch(instr%16){
      case 0:
	flag=thread_pop(villager);
	machine_poke(thread_pop(villager),flag);
	villager->position++;
	break;
      case 1:
	thread_push(villager,thread_pop(villager)+thread_pop(villager));
	villager->position++;
	break;
      case 2:
	thread_push(villager,thread_pop(villager)-thread_pop(villager));
	villager->position++;
	break;
      case 3:
	thread_push(villager,machine_p88k(thread_pop(villager)));
	villager->position++;
	break;
      case 4:
	thread_push(villager,thread_pop(villager)&thread_pop(villager));
	villager->position++;
	break;
      case 5:
	thread_pop(villager);
	villager->position++;
	break;
      case 6:
	flag=thread_pop(villager);
	thread_push(villager,flag);
	thread_push(villager,flag);
	villager->position++;
	break;
      case 7:
	thread_push(villager,thread_pop(villager)|thread_pop(villager));
	villager->position++;
	break;
      case 8:
	//Push a copy of the second element on the stack, N1, onto the top of the stack
	villager->m_reg8bit1=thread_pop(villager);
	villager->m_reg8bit2=thread_pop(villager);
	thread_push(villager,villager->m_reg8bit2);
	thread_push(villager,villager->m_reg8bit1);
	thread_push(villager,villager->m_reg8bit2);
	villager->position++;
	break;
      case 9:
	villager->m_reg8bit1=thread_pop(villager);
	villager->m_reg8bit2=thread_pop(villager);
	thread_push(villager,villager->m_reg8bit1);
	thread_push(villager,villager->m_reg8bit2);
	villager->position++;
	break;
      case 10:
	thread_push(villager,thread_pop(villager)^thread_pop(villager));
	villager->position++;
	break;
      case 11:
	if (thread_pop(villager)==0) villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	else 	villager->position++;
	break;
      case 12:
	//sub call
	thread_push(villager,villager->position);
	villager->position=villager->start+machine_p88k(machine_peek(villager->position+1));
	break;
      case 13:
	//sub return
	villager->position=villager->start+thread_pop(villager);
	break;
      case 14:
	thread_push(villager,machine_p88k(villager->position+1));
	villager->position++;
	break;
      case 15:
	machine_poke(machine_peek(villager->position+1),adc_buffer[thread_pop(villager)%10]);      
	villager->position++;
	break;
      }
      //      printf("%c",villager->position);
      break;    
     
///////////////////////////////////////////////////////////////

    case 10:
      // befunge: http://en.wikipedia.org/wiki/Befunge
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
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
	thread_push(villager,instr%30);
	break;
      case 10:
	thread_push(villager,thread_pop(villager)-thread_pop(villager));
	break;
      case 11:
	flag=thread_pop(villager);
	thread_push(villager,thread_pop(villager)-flag);
	break;
      case 12:
	thread_push(villager,thread_pop(villager)*thread_pop(villager));
	break;
      case 13:
	flag=thread_pop(villager);
	if (flag!=0) thread_push(villager,thread_pop(villager)/flag);
	break;
        case 14:
	flag=thread_pop(villager);
	if (flag!=0) thread_push(villager,thread_pop(villager)%flag);
	break;
      case 15:
	flag=thread_pop(villager);
	if (flag==0) thread_push(villager,1);
	else thread_push(villager,0);
	break;
      case 16:
	flag=thread_pop(villager);
	if (thread_pop(villager)>flag) thread_push(villager,1);
	else thread_push(villager,0);
	break;
      case 17: // right
	  villager->m_reg8bit2=2;
  	break;
      case 18: // left
	villager->m_reg8bit2=6;
	break;
      case 19: // up
	villager->m_reg8bit2=0;
	break;
      case 20: // down
	villager->m_reg8bit2=4;
	break;
      case 21:
	villager->m_reg8bit2=(randi()%4)*2;
	break;
      case 22:
	if (thread_pop(villager)==0)	villager->m_reg8bit2=2;
	else 	villager->m_reg8bit2=6;
	break;
      case 23:
	if (thread_pop(villager)==0)	villager->m_reg8bit2=4;
	else 	villager->m_reg8bit2=0;
	break;
      case 24:
	flag=thread_pop(villager);
	thread_push(villager,flag);
	thread_push(villager,flag);
	break;
      case 25:
	villager->m_reg8bit1=thread_pop(villager);
	villager->m_reg8bit2=thread_pop(villager);
	thread_push(villager,villager->m_reg8bit1);
	thread_push(villager,villager->m_reg8bit2);
	break;
      case 26:
	thread_pop(villager);
	break;
      case 27:
	villager->position+=biotadir[villager->m_reg8bit2%8];
	break;
      case 28:
	machine_poke((thread_pop(villager))*(thread_pop(villager)),thread_pop(villager));
	break;
      case 29:
	thread_push(villager,machine_p88k(thread_pop(villager)*thread_pop(villager)));
	break;
      case 30:
	machine_poke((thread_pop(villager))*(thread_pop(villager)),adc_buffer[thread_pop(villager)%10]);      
	break;
      }
      wormdir=villager->m_reg8bit2%8;
      villager->position+=biotadir[wormdir];
      break;
///////////////////////////////////////////////////////////////

    case 11:
      // kind of langton's ant as CPU - each thread has direction and
      // changes state of cell according to stack?
      // turn right/left/flip and move on
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
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
	  thread_push(villager,instr);
	  break;
	case 12:
	  villager->m_reg8bit2+=2;
	  machine_poke(villager->position,instr^thread_pop(villager));
	  break;
	case 13:
	  villager->m_reg8bit2-=2;
	  machine_poke(villager->position,instr^thread_pop(villager));
	  break;
	}
      wormdir=villager->m_reg8bit2%8;
      villager->position+=biotadir[wormdir];
      //      printf("%c",villager->position);
      break;
///////////////////////////////////////////////////////////////

    case 12:
      // turmites code: turmite has state,direction,position...
      // reg8bit1 is state. reg8bit2 is direction index.
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      machine_poke(villager->position,instr+villager->m_reg8bit1);
      //delta = dmove[(instr - villager->reg8bit1) & 0xf];
      flag=instr - villager->m_reg8bit1;
      //tm->dir = (tm->dir + delta) & 3;
      villager->m_reg8bit2=(villager->m_reg8bit2+flag)&8;
      //do move and wrap
      wormdir=(villager->m_reg8bit2)%8;
      villager->position+=biotadir[wormdir];
      // finally
      villager->m_reg8bit1 += deltastate[instr%16];
      //      printf("%c",villager->position);
      break;
///////////////////////////////////////////////////////////////

    case 13:
      // linear CA/life code. copies to 128 steps ahead new state and keeps going(?)
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      other=(machine_p88k(villager->position-1)&1)+(machine_p88k(villager->position+1)&1)+(machine_p88k(villager->position-32)&1)+(machine_p88k(villager->position+32)&1)+(machine_p88k(villager->position-31)&1)+(machine_p88k(villager->position-33)&1)+(machine_p88k(villager->position+31)&1)+(machine_p88k(villager->position+33)&1);

    if ((instr&1)==1 && other<2) flag=0;
    else if ((instr&1)==1 && other>3) flag=0;
    else if ((instr&1)==0 && other==3) flag=instr+1;
    else flag=instr;

    machine_poke(villager->position+128,flag);
      villager->position++; 
      //      printf("%c",instr);
      break;

    case 14:
      // ants revisited... how to do multiple ants using stack?
      // read cell//process rule string//change cell//move ant
      // rule string is from cell[0]
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      //      machine_poke(villager->position,instr+1);
      machine_poke(villager->position,instr+biotadir[villager->m_reg8bit1%8]);
      villager->m_reg8bit1=antrulee(villager->m_reg8bit1,instr%8,machine_p88k(0));//last is rule
      villager->position+=biotadir[villager->m_reg8bit1%8];
      //      printf("%c",villager->position);
      break;

    case 15:
      // second CA from CA.c
      flag = 0;
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      if (machine_p88k(villager->position+1)>128)	flag |= 0x4;
      if (instr>128) flag |= 0x2;
      if (machine_p88k(villager->position-1)>128)	flag |= 0x1;
                     
      if ((machine_p88k(0) >> flag) & 1)	machine_poke(villager->position+128,instr+129);
      else machine_poke(villager->position+128,instr-129); // or we stay with 255 and 0 as poked

      villager->position++; 
      if (villager->position==0) villager->position=1;
      //      printf("%c",instr);
      break;

    case 16:
      // port of hodge - larger 256*128 (32768) cellspace in two halves
      if (villager->position>villager->wrap) villager->position=villager->start;
      flag=temp=0;
      temp=machine_p88k(villager->position)+machine_p88k(villager->position-1)+machine_p88k(villager->position+1)+machine_p88k(villager->position-256)+machine_p88k(villager->position+256)+machine_p88k(villager->position-255)+machine_p88k(villager->position-257)+machine_p88k(villager->position+255)+machine_p88k(villager->position+257);

      if (machine_p88k(villager->position-1)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position-1)>0) other+=1.0;
      if (machine_p88k(villager->position+1)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position+1)>0) other+=1.0;
      if (machine_p88k(villager->position-256)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position-256)>0) other+=1.0;
      if (machine_p88k(villager->position+256)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position+256)>0) other+=1.0;
      if (machine_p88k(villager->position-255)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position-255)>0) other+=1.0;
      if (machine_p88k(villager->position-257)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position-257)>0) other+=1.0;
      if (machine_p88k(villager->position+255)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position+255)>0) other+=1.0;
      if (machine_p88k(villager->position+257)==machine_p88k(0)-1) flag++; else if (machine_p88k(villager->position+257)>0) other+=1.0;

      y=villager->position+32768;
      if (y<4) y=4;

      //      y=floor((double)other / (double)(machine_p88k(1)>>4));
      //      printf("floor: %d other: %d peek: %d\n",y,other,machine_p88k(2));

      //      printf("peek: %d\n",machine_p88k(2));


      if(machine_p88k(villager->position) == 0){
	u8 zx=floorf(other / ((machine_p88k(1)>>4)+1)) + floorf(flag/((machine_p88k(2)>>4)+1));
	    machine_poke(villager->position,zx);
      }
      else if(machine_p88k(villager->position) < machine_p88k(0)-1){
	u8 zx=floorf(temp / (other + 1)) + machine_p88k(3);
	machine_poke(villager->position,zx);
      }
  else
    machine_poke(y,0);

  if(machine_p88k(villager->position) > machine_p88k(0)-1)
    machine_poke(y,machine_p88k(0)-1);

  villager->position++; 
  if (villager->position<4) villager->position=4;

  break;
    case 17:
      // start generic - add (add/sub/zero/copy/invert/swap)
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      machine_poke(villager->position,instr+1);
      villager->position++;
      if (villager->position>villager->wrap) villager->position=villager->start;
      break;
    case 18:
      // generic - sub (add/sub/zero/copy/invert/swap)
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      machine_poke(villager->position,instr-1);
      villager->position++; 
      break;
    case 19:
      // generic - zero (add/sub/zero/copy/invert/swap)
      if (villager->position>villager->wrap) villager->position=villager->start;
      machine_poke(villager->position,0);
      villager->position++; 
      break;
    case 20:
      // generic - copy (add/sub/zero/copy/invert/swap)
      if (villager->position>villager->wrap) villager->position=villager->start;
      machine_poke(villager->position,machine_p88k(villager->position+1));
      villager->position++; 
      break;
    case 21:
      // generic - copy (add/sub/zero/copy/invert/swap)
      if (villager->position>villager->wrap) villager->position=villager->start;
      machine_poke(villager->position,machine_p88k(villager->position^255));
      villager->position++; 
      break;
      ////////////////////////////////////////////////////??????
    case 22:
      // generic - swap (add/sub/zero/copy/invert/swap)
      instr=machine_p88k(villager->position+1);
      machine_poke(villager->position+1,machine_p88k(villager->position));
      machine_poke(villager->position,instr);
      villager->position+=2; 
      if (villager->position>villager->wrap) villager->position=villager->start;
      break;
    case 23:
      if (villager->position>villager->wrap) villager->position=villager->start;
      machine_poke(machine_peek(villager->position++),adc_buffer[machine_p88k(villager->position)%10]);    
      break;
///////////////////////////////////////////////////////////////
    case 24:
      // from wormcode.c
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      //      printf("instr %d\n",instr%15);
      switch(instr%15)
	{
	case 0:
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 1:
	  //inc
	  machine_poke(villager->position,instr+1);
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 2:
	  machine_poke(villager->position,instr-1);
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 3:
	  villager->position=villager->start+machine_p88k(villager->position);
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 4:
	  machine_poke(villager->position,randi()%255);
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 5:
	  machine_poke(villager->position,machine_p88k(villager->position+biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 6:
	  machine_poke(villager->position,machine_p88k(villager->position+biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 7:
	  machine_poke(villager->position,machine_p88k(villager->position-biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 8:
	  machine_poke(villager->position,machine_p88k(villager->position<<biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 9:
	  machine_poke(villager->position,machine_p88k(villager->position>>biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 10:
	  if (machine_p88k(villager->position+(biotadir[randi()%8]*2))==0){
	    y=biotadir[randi()%8];
	    villager->position+=y;
	  }
	  else {  y=biotadir[randi()%8];
	  villager->position+=y;
	  }
	  break;
	case 11:
	  machine_poke((villager->position-biotadir[randi()%8]),instr);
	  machine_poke((villager->position+biotadir[randi()%8]),instr);
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 12:
	  thread_push(villager, machine_p88k(villager->position+biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
	case 13:
	  machine_poke((villager->position+=biotadir[randi()%8]),thread_pop(villager));
	  y=biotadir[randi()%8];
	  villager->position+=y;
	  break;
      case 14:
	machine_poke((villager->position+=biotadir[randi()%8]),adc_buffer[thread_pop(villager)%10]);      
	break;
	}
      break;
///////////////////////////////////////////////////////////////
    case 25:
      // 16 bit increment
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      y=(instr<<8)+machine_p88k(villager->position+1)+1;
      machine_poke(villager->position,y>>8);      
      machine_poke(villager->position+1,y&255);      
      villager->position+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 26:
      // 16 bit decrement
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      y=(instr<<8)+machine_p88k(villager->position+1)-1;
      machine_poke(villager->position,y>>8);      
      machine_poke(villager->position+1,y&255);      
      villager->position+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 27:
      // 16 bit left
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      y=(instr<<9)+(machine_p88k(villager->position+1)<<1);
      machine_poke(villager->position,y>>8);      
      machine_poke(villager->position+1,y&255);      
      villager->position+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 28:
      // 16 bit right
      if (villager->position>villager->wrap) villager->position=villager->start;
      instr=machine_p88k(villager->position);
      y=(instr<<7)+(machine_p88k(villager->position+1)>>1);
      machine_poke(villager->position,y>>8);      
      machine_poke(villager->position+1,y&255);      
      villager->position+=2;
      //      printf("%d\n",y);
      break;
///////////////////////////////////////////////////////////////
    case 29:
      // pure leakage - push instr onto stack. when stack is full pull off...
      if (villager->position>villager->wrap) villager->position=villager->start;
      if (thread_stack_count(villager,STACK_SIZEE)) machine_poke(villager->position,thread_pop(villager));
      else thread_push(villager,machine_p88k(villager->position));
      villager->position++;
      break;
///////////////////////////////////////////////////////////////
    case 30:
      // convolution
      if (villager->position>villager->wrap) villager->position=villager->start;
      temp=(machine_p88k(villager->position-1)*machine_p88k(0))+(machine_p88k(villager->position)*machine_p88k(1))+(machine_p88k(villager->position+1)*machine_p88k(2));
      y=villager->position+32768;
      //      if (y<3) y=3;
      machine_poke(y,temp);
      villager->position++; 
      //      if (villager->position<3) villager->position=3;
      break;
      case 31:
      if (villager->position>villager->wrap) villager->position=villager->start;
      villager->position++; 
      } // switch
  }// if del
}

u8 thread_stack_count(thread* villager, u8 c) { 
    return (c-1)<=villager->m_stack_pos; 
}

void thread_push(thread* villager, u8 data) {
  if (villager->m_stack_pos<14) // STACK_SIZEE-2
	{
	  villager->m_stack[++villager->m_stack_pos]=data;
	}
}

u8 thread_pop(thread* villager) {
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

u8 thread_top(thread* villager) {
	if (villager->m_stack_pos>=0 && villager->m_stack_pos<STACK_SIZEE)
	{
	  return villager->m_stack[villager->m_stack_pos];
	}
	return 0;
}

///////////////////////////////////////////////////////////////

inline u16 machine_peek(machine* villager, uint16_t addr) {
  //	return villager->m_heap[addr%HEAP_SIZE];
  u16 y;
  y=addr+1;
  return (villager->m_memory[addr]<<8)+villager->m_memory[y];
}

inline u8 machine_p88k(machine* villager, uint16_t addr) {
  //	return villager->m_heap[addr%HEAP_SIZE];
  return villager->m_memory[addr];
}


inline void machine_poke(machine* villager, uint16_t addr, u8 data) {
  //	villager->m_heap[addr%HEAP_SIZE]=data;
  villager->m_memory[addr]=data;
  //      printf("%d\n",&villager->m_memory[addr]);
}

void machine_run(machine* villager) {

  villager->m_threadcount=100; // TODO - fix villager 
  static u8 thrcount=0;
  //  static u8 n=0;
  thrcount++;
  //  if (threadcount>villager->m_threadcount) {
  //    threadcount=0;

  if (thrcount==254){
    thrcount=0;
  if (villager->m_leakiness==0) villager->m_leakiness=1;
  if (villager->m_infectprob==0) villager->m_infectprob=1;
  //  if (villager->m_mutateprob==0) villager->m_mutateprob=1;


  if ((randi()%villager->m_leakiness)==0  && villager->m_threadcount>0) {
    leak(villager);
  }
	
  if ((randi()%villager->m_infectprob)==0 && villager->m_threadcount>0) {
    infectcpu(villager,randi()%16,randi()%16,randi()%villager->m_threadcount);
  }
		
  }

    for (unsigned char n=0; n<villager->m_threadcount; n++) {
    thread_run(&villager->m_threads[n],villager);
    //        printf("running %d\n", n);
    //    n++; // TESTY!
    //    if (n>villager->m_threadcount) n=0;

    
      }
}

void leak(machine *m){
  // leak bottom of stack x into top y
  unsigned char x=1, y=1, count;
  x=randi()%m->m_threadcount;
  y=(x-1)%m->m_threadcount;

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
  u16 temp;
  //  x=which%m->m_threadcount;
  thread *villager=&m->m_threads[which];
  switch (identifier%4){
  case 0:
    villager->m_CPU^=(1<<randi()%4);
    break;
  case 1:
    temp=villager->wrap-villager->start;
    temp^=(1<<randi()%16);
    villager->wrap=villager->start+temp;
    break;
  case 2:
    villager->m_del^=(1<<randi()%8);
    break;
  case 3:
    villager->position^=(1<<randi()%16);
    villager->wrap=villager->start+randi();
    break;
  }
}

void swapcpu(machine *m, u8 identifier){// never called!
  u8 x,y,temp; u16 tempi;
  x=randi()%m->m_threadcount;
  y=(x+1)%m->m_threadcount;
  thread *villager=&m->m_threads[x];
  thread *that=&m->m_threads[y];
  switch (identifier%3){
  case 0:
    temp=villager->m_CPU;
    villager->m_CPU=that->m_CPU;
    that->m_CPU=temp;
    break;
  case 1:
    tempi=villager->start;
    villager->start=that->start;
    that->start=tempi;
    tempi=villager->wrap;
    villager->wrap=that->wrap;
    that->wrap=tempi;
    break;
  case 2:
    temp=villager->m_del;
    villager->m_del=that->m_del;
    that->m_del=temp;
    break;
  }
}

void killcpu(machine *m, u8 killed);

void infectcpu(machine *m, u8 probI, u8 probD, u8 infected){
  // chances of infecting neighbouring cell which in time is killed or recovers
  thread *villager=&m->m_threads[(infected+1)%m->m_threadcount];

  // if is recovered leave alone
  if (villager->m_infection==129) return;

  // if is infected calc whether to kill or inc to recovery
  else if (villager->m_infection>1) {
    if (randi()%255 <=probD) killcpu((infected+1)%m->m_threadcount);
    else villager->m_infection++;
  }

  // if not infected then prob to infect it
  else if (villager->m_infection==0){
    if (randi()%255 <= probI) villager->m_infection=1;
  }
}

void killcpu(machine *m, u8 killed){
  u8 x;
  // or should we run through and kill one - how re-org list.
  //  cpustackpop(m);
  //  thread *villager=&m->m_threads[which];
  //villager->m_threads[villager->m_threadcount]
  if (killed<m->m_threadcount && killed<(MAX_THREADS-1)){
  for (x=killed;x<m->m_threadcount-1;x++){
    m->m_threads[x]=m->m_threads[x+1];
  }
  m->m_threadcount--;}
}


#ifdef PCSIM
//int main(void)
/*int main(int argc, char **argv)
{
  u16 xx; u16 addr;
  u8 buffer[65536];// u16 *testi; u8 *testo;
  int number=atoi(argv[1]);
  srandom(time(0));
  for (xx=0;xx<65535;xx++){
    buffer[xx]=randi()%255;
    //    printf("%d\n",buffer[x]);
  }

  machine *m=(machine *)malloc(sizeof(machine));
  //  machine_create(buffer); // villager just takes care of pointer to machine and malloc for threads
   m->m_threadcount=0;
   m->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS); //PROBLEM with _sbrk FIXED

  m->m_leakiness=randi()%255;
  m->m_infectprob=randi()%255;
  m->m_mutateprob=randi()%255;

  u8 flag,other;
  int16_t right_buffer[64]; u8 sz=128;

	for (unsigned char n=0; n<120; n++)
	{
	  addr=randi()%65536;
	  //	  addr=0;
	  // 	  cpustackpush(addr,addr+randi()%65536,randi()%25,randi()%255);
	  //	  	  cpustackpush(addr,addr+randi()%65536,randi()%31,randi()%255);
	  	  cpustackpush(buffer,addr,addr+randi()%65536,randi()%31,randi()%10);
	  //	  	  cpustackpush(buffer, addr,addr+randi()%65536,randi()%31,);
	  //	  	  	  cpustackpush(buffer,addr,65535,number,100);
	}


//	u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768
	u16 tmp,any,counter,edge=0;
	u8 sampledir,samplestep,complexity;

	u8 sampdir=2,sampstep=1;
	u16 samp=0, samppos=0;
	u8 anydir, anyspeed, anystep;
	u16 anywrap,anystart,start,wrap;
	u8 inproc=1, anydel=0;
	u16 samplepos,anypos=0; u16 x;
	u8 del=0,speed=1;
	signed char dirry;
	u16 *buf16 = (u16*) buffer;


	anyspeed=1;anydir=2;anystart=0;anywrap=32; anystep=1;
	samplestep=1;sampledir=2;
	wrap=0;start=0;

	while(1) {
	  machine_run(m); //cpu - WRAP own speed
	  //	  	  printf("%c",buffer[x]);
	  	  x++;

	  //	  	  for (u8 y=0;y<m->m_threadcount;y++){
		    //	    m->m_threads[y].m_CPU=rand()%31;
	  //	    }

		  //	  	  if ((rand()%20)>10)	  cpustackpush(buffer,addr,addr+randi()%65536,6,1);
		  //	  	  else cpustackpop(m);

	}
	}
*/
#endif

