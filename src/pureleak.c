#ifdef PCSIM
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "CPUint.h"
#include "settings.h"
#define randi() rand()
extern uint16_t *settingsarray;
extern int16_t* audio_buffer;
extern uint16_t* adc_buffer;
/*uint16_t settingsarray[64];
int16_t audio_buffer[32768];
uint16_t adc_buffer[10];*/
#else
#include <malloc.h>
#include "CPUint.h"
#include "settings.h"
#define randi() (adc_buffer[9])
//#define randi() rand()
extern __IO uint16_t adc_buffer[10];
extern int16_t audio_buffer[32768] __attribute__ ((section (".data")));;
//extern u16 settingsarray[64];
#endif

#include <math.h>

extern u8 wormdir;

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

#define CPU buffer[offset]
#define DELAY buffer[delay]
#define DELC buffer[delc]
#define ADDRHI buffer[addrhi]
#define ADDRLO buffer[addrlo]
#define WRAPADDRHI buffer[wrapaddrhi]
#define WRAPADDRLO buffer[wrapaddrlo]
#define PCADDRHI buffer[pcaddrhi]
#define PCADDRLO buffer[pcaddrlo]
#define BITADDRHI buffer[bitaddrhi]
#define BITADDRLO buffer[bitaddrlo]
#define BIT81 buffer[bit81]
#define BIT82 buffer[bit82]
#define STACK buffer[stack]

void thread_createee(u8 *buffer, u16 address, u16 wrapaddress,u8 which, u8 delay,u16 offset) { // ??? or we steer each of these?
  buffer[offset]=which; // cpu
  buffer[offset+1]=delay;
  buffer[offset+2]=0;
  buffer[offset+3]=address>>8; // hi/lo
  buffer[offset+4]=address&255;
  buffer[offset+5]=wrapaddress>>8; // hi/lo
  buffer[offset+6]=wrapaddress&255;
  buffer[offset+7]=address>>8; // hi/lo
  buffer[offset+8]=address&255;
  buffer[offset+9]=address>>8; // hi/lo
  buffer[offset+10]=address&255;
  buffer[offset+11]=randi()%255;
  buffer[offset+12]=randi()%255;
  buffer[offset+13]=14; // stack_pos -1????
}

u16 machine_peekkk(u8* buffer, uint16_t addr);
u8 machine_p88kkk(u8* buffer, uint16_t addr);
void machine_pokeee(u8* buffer, uint16_t addr, u8 data);
void thread_runnn(u8* buffer, u8 threadcount);

void machine_runnn(u8* buffer){
  u8 x; 
  if (buffer[0]==0) buffer[0]=MAX_FRED*2;
  for (x=0;x<buffer[0];x+=2){ 
  thread_runnn(buffer,x);
 }
}

void cpustackpushhh(u8 *buffer,u16 addr,u16 wrapaddr,u8 cpuuu, u8 delayyy){
  u16 offset;
  offset=buffer[0]*30;
  buffer[buffer[0]]=offset>>8; // but offset can be over 255 - so top bit
  buffer[buffer[0]+1]=offset&255; // lower bit
  thread_createee(buffer, addr, wrapaddr,cpuuu,delayyy,offset);
  buffer[0]+=2;
}

void cpustackpoppp(u8 *buffer){
  buffer[0]--;
  }

u8 thread_stack_counttt(u8 *buffer, u8 c, u16 offset) { 
  offset+=13;
  return c<=(buffer[offset]-14); // but now we start at 0
}

u8 antrule(u8 dir,u8 inst, u8 rule);

u8 antrule(u8 dir,u8 inst, u8 rule){
  u8 index;
  // process state from rule 
  // rule in binary
  // inst is index into binary array - so inst must be lower than 0->7 - it is
  index=(rule>>inst)&1;
  if (index==0) return dir-1; 
  else return dir+1; 
}

void thread_pushhh(u8 *buffer, u8 data, u16 offset) {
  u16 offsetty=offset+13; u16 orf;
	if (buffer[offsetty]<30)
	{
	  orf=(++buffer[offsetty])+offset;
	  buffer[orf]=data;
	}
}

u8 thread_poppp(u8* buffer, u16 offset) {
  u16 offsetty=offset+13;
 	if (buffer[offsetty]>14)
	{
	  u16 orf=offset+buffer[offsetty];
		u8 ret=buffer[orf];
		buffer[offsetty]--;
		return ret;
	}
	return 0;   
}

u8 thread_toppp(u8 *buffer, u16 offset) {
  u16 offsetty=offset+13;
	if (buffer[offsetty]>14)
	{
	  u16 orf=offset+buffer[offsetty];
	  return buffer[orf];
	}
	return 0;
}

void thread_runnn(u8* buffer, u8 threadnum) {
  u8 instr,temp;
  u16 y,addr,temprr;
  u8 flag; u16 other=0;
  u16 offset=(buffer[threadnum+1]<<8)+buffer[threadnum+2]; 
  u16 delay=offset+1;
  u16 delc=offset+2;
  u16 addrhi=offset+3;
  u16 addrlo=offset+4;
  u16 wrapaddrhi=offset+5;
  u16 wrapaddrlo=offset+6;
  u16 pcaddrhi=offset+7;
  u16 pcaddrlo=offset+8;
  u16 bitaddrhi=offset+9;
  u16 bitaddrlo=offset+10;
  u16 bit81=offset+11;
  u16 bit82=offset+12;
  u16 stack=offset+13;

  //  printf("num: %d readoffset: %d cpu: %d\n",threadnum,offset,buffer[offset]);

  const u8 deltastate[16] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

      if (++DELC==DELAY){
	DELC=0;
#ifdef PCSIM
	//	printf("CPU: %d\n",CPU);
	//            printf("%c",machine_p88kkk(buffer,(PCADDRHI<<8)+PCADDRLO));
#endif

  switch(CPU%CPU_TOTAL)
    {
    case 0: // :LEAKY STACK! - working!
      addr=((PCADDRHI<<8)+PCADDRLO);
      instr=machine_p88kkk(buffer,addr);
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
    case JMP: //this->m_pc=this->machine_peekkk(buffer,this->m_pc++); break;
      addr=machine_peekkk(buffer,addr++);
      break;      
    case JMPZ:// if (thread_stack_count(this,1) && thread_pop(this)==0) this->m_pc=machine_peekkk(buffer,this->m_pc); else this->m_pc++; break;
      if (thread_stack_counttt(buffer,1,offset) && thread_poppp(buffer,offset)==0) machine_peekkk(buffer,addr); else addr++; 
      break;
    case PSHL: //thread_push(this,machine_peekkk(buffer,this->m_pc++)); break;
      thread_pushhh(buffer,machine_p88kkk(buffer,addr++),offset);
      break;

    case PSH: ///thread_push(this,machine_peekkk(buffer,machine_peekkk(buffer,this->m_pc++))); break;
      thread_pushhh(buffer,machine_p88kkk(buffer,machine_peekkk(buffer,addr++)),offset); break;

    case PSHI://  thread_push(this,machine_peekkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,this->m_pc++)))); break;
      thread_pushhh(buffer,machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr++))),offset); break;

    case POP:// if (thread_stack_count(this,1)) machine_poke(buffer,machine_peekkk(buffer,this->m_pc++),thread_pop(this)); break;
      if (thread_stack_counttt(buffer,1,offset)) machine_pokeee(buffer,machine_peekkk(buffer,addr),thread_poppp(buffer,offset)); break;

    case POPI: //if (thread_stack_count(this,1)) machine_poke(buffer,((ADDRHI<<8)+ADDRLO)+machine_peekkk(buffer,machine_peekkk(buffer,this->m_pc++)),thread_pop(this)); break;
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
      //        machine_poke(buffer,d,machine_peekkk(buffer,d)+1); 

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
      //      machine_poke(buffer,machine_peekkk(buffer,buffer->m_pc++),randi()%255);      
        break;
	case INP:
	  machine_pokeee(buffer,machine_peekkk(buffer,addr++),adc_buffer[thread_poppp(buffer,offset)%10]);      
	  addr++;
	  break;

    default : break;
    }
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////

    case 1:// BIOTA see: http://c2.com/cgi/wiki?BiotaLanguage

      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      flag=0;
      //      printf("instr %d ",instr);
      switch(instr%8)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  addr=((BITADDRHI<<8)+BITADDRLO);
	  addr+=biotadir[(BIT81)%8];
	  if (machine_p88kkk(buffer,addr)==0) flag=1;
	  BITADDRHI=addr>>8; // hi/lo
	  BITADDRLO=addr&255;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  //	  this->m_reg16bit1+=biotadir[(this->m_reg8bit1+4)%8];
	  //	  if (machine_p88k(buffer,this->m_reg16bit1)==0) flag=1;
	  addr=((BITADDRHI<<8)+BITADDRLO);
	  addr+=biotadir[((BIT81)+4)%8];
	  if (machine_p88kkk(buffer,addr)==0) flag=1;
	  BITADDRHI=addr>>8; // hi/lo
	  BITADDRLO=addr&255;
	  break;
	case 2:
	  //* t -- turn DC right 45 degrees
	  BIT81+=1;
	  break;
	case 3:
	  //* u -- unturn: turn DC left 45 degrees
	  BIT81-=1;
	  break;
	case 4:
	  //    * g -- go to a non-empty character ahead (tries to move DC straight ahead, then right and left 45 degrees, then 90, then 135, then back).
	  temp=BIT81%8;
	  addr=((BITADDRHI<<8)+BITADDRLO);
	  addr+=biotadir[temp];
	  if (machine_p88kkk(buffer,addr)==0) {
	    addr-=biotadir[temp]; // go back
	    addr+=biotadir[(BIT81+1)%8]; // right 45
	    if (machine_p88kkk(buffer,addr)==0) {
	      addr-=biotadir[(BIT81+1)%8]; // go back
	      addr+=biotadir[(BIT81-1)%8]; // left 45
	      if (machine_p88kkk(buffer,addr)==0) {
		addr-=biotadir[(BIT81-1)%8]; // go back
		addr+=biotadir[(BIT81+2)%8]; // right 90
		if (machine_p88kkk(buffer,addr)==0) {
		  addr-=biotadir[(BIT81+2)%8]; // go back
		  addr+=biotadir[(BIT81-2)%8]; // left 90
		  if (machine_p88kkk(buffer,addr)==0) {
		    addr-=biotadir[(BIT81-2)%8]; // go back
		    addr+=biotadir[(BIT81+3)%8]; // right 135
		    if (machine_p88kkk(buffer,addr)==0) {
		    addr-=biotadir[(BIT81-3)%8]; // go back
		    addr+=biotadir[(BIT81-3)%8]; // left 135
		    if (machine_p88kkk(buffer,addr)==0) {
		    addr-=biotadir[(BIT81-3)%8]; // go back
		    addr+=biotadir[(BIT81+4)%8]; // back
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
	  if (machine_p88kkk(buffer,addr)!=0) machine_pokeee(buffer,addr,0); 
	  else flag=1;
	  break;
	case 6:
	  //    * d -- duplicate the current data into the cell left of the DC. Fails if source cell is empty or target cell is non-empty.
	  // left is -2

	  if (machine_p88kkk(buffer,addr)==0 || (machine_p88kkk(buffer,addr+(addr+((BIT81-4)%8))))!=0) flag=1;
	  else machine_pokeee(buffer,addr+(addr+((BIT81-4)%8)),machine_p88kkk(buffer,addr));
	  break;
	case 7:
	  //    * . -- no-op, a non-empty do nothing. 
	  break;
	}

      // - this->m_pc turns (where?) when it finds an empty location or a failing instruction
      // m_reg8bit3 is direction

      // REDO ADDR
      addr=((PCADDRHI<<8)+PCADDRLO);

      wormdir=BIT82%8;
      if (machine_p88kkk(buffer,addr)==0 || flag==1){
	BIT82-=1;
      }
      else addr+=biotadir[wormdir];
      //      printf("%c",this->m_pc);
      //      break;
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
      
/////////////////////////////////////////////////////////////////

    case 2:
      // brainfuck: add in output???
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);

      switch(instr%7)
	{
	case 0:
	  y=((BITADDRHI<<8)+BITADDRLO);
	  y+=1;
	  addr++;
	  BITADDRHI=y>>8; // hi/lo
	  BITADDRLO=y&255;
	  break;
	case 1:
	  y=((BITADDRHI<<8)+BITADDRLO);
	  y-=1;
	  addr++;
	  BITADDRHI=y>>8; // hi/lo
	  BITADDRLO=y&255;
	  break;
	case 2:
	  //	    cells[omem]=cells[omem]++; 
	  y=((BITADDRHI<<8)+BITADDRLO);
	  machine_pokeee(buffer,y,machine_p88kkk(buffer,y)+1);
	  addr++;
	  break;
	case 3:
	  //	  machine_poke(buffer,y,machine_peekkk(buffer,y)-1);
	  y=((BITADDRHI<<8)+BITADDRLO);
	  machine_pokeee(buffer,y,machine_p88kkk(buffer,y)-1);
	  addr++;
	  break;
	case 4:
	  BIT81+=2;
	  if (BIT81>=14) BIT81=0;
	  //buffer[offset+STACK]
	  offset+=STACK+BIT81;
	  //	  buffer[offset]= addr>>8;
	  machine_pokeee(buffer,offset,addr>>8);
	  offset+=1;
	  //	  buffer[offset]= addr&255;
	  machine_pokeee(buffer,offset,addr&255);
	  addr++;
	  break;
	case 5:
	  y=((BITADDRHI<<8)+BITADDRLO);
	  if (BIT81>=14) BIT81=0;
	  if (machine_p88kkk(buffer,y)!=0) {
	    offset+=STACK+BIT81;
	    addr=buffer[offset]<<8;
	    offset+=1;
	    addr+=buffer[offset];
	  }
	  BIT81-=2;
	  if (BIT81==0) BIT81=14;
	  break;
	case 6:
	  //  cells[omem] = adcread(3); 
	  y=((BITADDRHI<<8)+BITADDRLO);
	  machine_pokeee(buffer,y,adc_buffer[(y>>8)%10]);
	  addr++;
	  break;
	}
      //      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    
///////////////////////////////////////////////////////////////

    case 3:
      // masque red death:
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);

      //      instr=machine_peekkk(buffer,this->m_pc);
      //      printf("instr %d ",instr);
      switch(instr%7){
      case 0:
	if (BIT81==12){
	  machine_pokeee(buffer,addr+1,machine_p88kkk(buffer,addr));
	  if (machine_p88kkk(buffer,addr)==255) BIT81=13;
	  addr++;
	}
	else addr++;
	break;
      case 1:
	if (BIT81==13){
	  y=((BITADDRHI<<8)+BITADDRLO);
	  y++;
	  machine_pokeee(buffer,y,audio_buffer[addr%32768]); //READ IN
	  BITADDRHI=y>>8; // hi/lo
	  BITADDRLO=y&255;
	  addr++;
	}
	else addr++;
	break;
      case 2:
	BIT81++;
	if (BIT81==60){
	  y=((BITADDRHI<<8)+BITADDRLO);
	  machine_pokeee(buffer,y,machine_p88kkk(buffer,y)^255);
	  BIT81=0;
	  y++;
	  BITADDRHI=y>>8; // hi/lo
	  BITADDRLO=y&255;
	}
	else addr++;
	break;
	case 3:
	  //	  seven rooms: divide cellspace into 7 - reduce wrap!!! constrain!!!
	  temp= addr%7;
	  for (u8 x=11;x<22;x++){//11-21
	    //	    settingsarray[x]=settingsarray[x]>>temp;
	  }// CONSTRAINT
	  addr++;
	  break;
	case 4:
	  y=((BITADDRHI<<8)+BITADDRLO);
	  machine_pokeee(buffer,y-1,machine_p88kkk(buffer,BIT81-1)^255);
	  machine_pokeee(buffer,y+1,machine_p88kkk(buffer,BIT81+1)^255);
	  addr++;
	  y++;
	  BITADDRHI=y>>8; // hi/lo
	  BITADDRLO=y&255;
	  break;
	case 5:
	  flag=randi()%4;
	  if (flag==0) 	  BIT81++;
	  if (flag==1) 	  BIT81--;
	  if (flag==2) 	  BIT81+=16;
	  if (flag==3) 	  BIT81-=16;
	  addr++;
	  break;
	case 6:
	  y=((BITADDRHI<<8)+BITADDRLO);
	  machine_pokeee(buffer,y+2,adc_buffer[(y>>8)%10]);
	  addr++;
	  break;
      }
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    

///////////////////////////////////////////////////////////////

    case 4:
      // plague: add in output???

      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);

      switch(instr%5){
      case 0:
	machine_pokeee(buffer,addr,255);
	machine_pokeee(buffer,addr+1,255);
	addr+=2;
	break;
      case 1:
	if (machine_p88kkk(buffer,addr)<128){
	  machine_pokeee(buffer,addr-1,machine_p88kkk(buffer,addr));
	  machine_pokeee(buffer,addr+1,machine_p88kkk(buffer,addr));
	}
	addr+=biotadir[BIT81%8];
	break;
      case 2:
	machine_pokeee(buffer,addr-1,0);
	machine_pokeee(buffer,addr+1,0);
	addr+=biotadir[BIT81%8];
	break;
      case 3:
	if ((machine_p88kkk(buffer,addr)%0x03)==1) BIT81+=4;
	else BIT81*=machine_p88kkk(buffer,addr)>>4;
	addr+=biotadir[BIT81%8];
	break;
      case 4:
	machine_pokeee(buffer,addr+1,adc_buffer[(BIT81>>8)%10]);
	addr+=biotadir[BIT81%8];
	  break;

      }
      if (machine_p88kkk(buffer,addr)==255) BIT81+=4;
	wormdir=BIT81%8;
	//      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    
	
///////////////////////////////////////////////////////////////

      case 5:
      // first from micro: add in output???
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      //      printf("instr %d ",instr);
      switch(instr%15){
      case 0:
	y=((BITADDRHI<<8)+BITADDRLO);
	y+=2;
	addr++;
	BITADDRHI=y>>8; // hi/lo
	BITADDRLO=y&255;
	break;
      case 1:
	y=((BITADDRHI<<8)+BITADDRLO);
	y-=2;
	addr++;
	BITADDRHI=y>>8; // hi/lo
	BITADDRLO=y&255;
	addr++;
	break;
      case 2:
	y=((BITADDRHI<<8)+BITADDRLO);
	machine_pokeee(buffer,y,machine_p88kkk(buffer,y)+1);
	addr++;
	break;
      case 3:	  
	y=((BITADDRHI<<8)+BITADDRLO);
	machine_pokeee(buffer,y,machine_p88kkk(buffer,y)-1);
	addr++;
	break;
      case 4:	  
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)+1);
	addr++;
	break;
      case 5:	  
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)-1);
	addr++;
	break;
      case 6:	  
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)>>1);
	addr++;
	break;
      case 7:	  
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)>>2);
	addr++;
	break;
      case 8:	  
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)>>3);
	addr++;
	break;
      case 9:	  
	y=((ADDRHI<<8)+ADDRLO);
	if (machine_peekkk(buffer,addr+1)==0) addr=y+(BITADDRHI<<8)+BITADDRLO;
	else	addr++;
	break;
      case 10:	  
	if (machine_p88kkk(buffer,addr+1)<128) addr+=machine_p88kkk(buffer,addr+1);
	else 	addr++;
	break;
      case 11:	  
	if (machine_p88kkk(buffer,addr-1)<128) machine_pokeee(buffer,addr+1,machine_p88kkk(buffer,addr));
	addr++;
	break;
      case 12:	  
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr+1));
	addr++;
	break;
      case 13:	  
	addr++;
	break;
      case 14:
	y=((BITADDRHI<<8)+BITADDRLO);
	machine_pokeee(buffer,addr,adc_buffer[(y>>8)%10]);
	break;
      }
      //      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    

///////////////////////////////////////////////////////////////

    case 6:
/* "real" corewars redcode SPL 

REF: http://vyznev.net/corewar/guide.html#start_instr

http://www.koth.org/info/akdewdney/images/Redcode.jpg

 */

      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);

      switch(instr%30){
      case 0:
	// MOV # to direct.
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),machine_p88kkk(buffer,addr+1));
	addr+=3;
	break;
      case 1:	  
	// MOV indirect to direct.
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),machine_p88kkk(buffer,machine_peekkk(buffer,addr+1)));
	addr+=3;
	break;
      case 2:
	// MOV # to indirect.
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)),machine_p88kkk(buffer,addr+1));
	addr+=3;
	break;
      case 3:
	//MOV indirect to indirect
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)),machine_p88kkk(buffer,machine_peekkk(buffer,addr+1)));
	addr+=3;
	break;
      case 4:
	//ADD # to direct
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),machine_p88kkk(buffer,addr+1)+machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2)));
	addr+=3;
	break;
	//HERE      
      case 5:
	// ADD indirect to direct.
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),machine_p88kkk(buffer,machine_peekkk(buffer,addr+1))+(machine_p88kkk(buffer,addr+(machine_p88kkk(buffer,addr+2)>>8))));
	addr+=3;
	break;
      case 6:
	// ADD # to indirect.
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)),machine_p88kkk(buffer,addr+1)+machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)));
	addr+=3;
	break;
      case 7:
	//ADD indirect to indirect
	machine_pokeee(buffer,machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)),machine_p88kkk(buffer,machine_peekkk(buffer,addr+1))+machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)));
	addr+=3;
	break;
      case 8:
	//SUB # to direct
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),machine_p88kkk(buffer,addr+1)-machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2)));
	addr+=3;
	break;
      case 9:
	// indirect to direct.
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),machine_p88kkk(buffer,machine_peekkk(buffer,addr+1)-machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))));
	addr+=3;
	break;
      case 10:
	// # to indirect.
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)),machine_p88kkk(buffer,addr+1)-machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)));
	addr+=3;
	break;
      case 11:
	// indirect to indirect
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)),machine_p88kkk(buffer,machine_peekkk(buffer,addr+1))-machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)));
	addr+=3;
	break;
      case 12:
	// jmp to direct
	addr+=machine_p88kkk(buffer,addr+1);
	break;
      case 13:
	// jmp to indirect
	y=(ADDRHI<<8)+ADDRLO;
	addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	break;
      case 14:
	// JMZdirect to direct
	if (machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))==0) 	addr+=machine_p88kkk(buffer,addr+1);
	else 	addr+=3;
	break;
      case 15:
	// JMZdirect to indirectXXX
	y=(ADDRHI<<8)+ADDRLO;
	if (machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))==0) 	addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	else 	addr+=3;
	break;
      case 16:	
	// JMZindirect to direct
	if (machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)))==0) 	addr+=machine_p88kkk(buffer,addr+1);
	else 	addr+=3;
	break;
      case 17:
	// JMZindirect to indirect
	if (machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)))==0){
	  y=(ADDRHI<<8)+ADDRLO;
	  addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	}
	else 	addr+=3;
	break;
      case 18:
	// JMGdirect to direct
	if (machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))>0) 	addr+=machine_p88kkk(buffer,addr+1);
	else 	addr+=3;
	break;
      case 19:
	// JMGdirect to indirect
	if (machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))>0){
	  y=(ADDRHI<<8)+ADDRLO;
	  addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	}	  
	else 	addr+=3;
	break;
      case 20:	
	// JMGindirect to direct
	if (machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)))>0) 	addr+=machine_p88kkk(buffer,addr+1);
	else 	addr+=3;
	break;
      case 21:
	// JMGindirect to indirect
	if (machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)))>0){
	  y=(ADDRHI<<8)+ADDRLO;
	  addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	}
	else 	addr+=3;
	break;
      case 22:
	// Dec, jump if zero... sub 1 from b. jump to a if b is zero
	// DJZ dir to dir
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2), machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))-1);
	if (machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))==0)	  addr+=machine_p88kkk(buffer,addr+1);
	else addr+=3;
	break;
      case 23:
	// DJZ dir to indir
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2), machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))-1);
	if (machine_p88kkk(buffer,addr+machine_p88kkk(buffer,addr+2))==0){
	y=(ADDRHI<<8)+ADDRLO;
	addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	}
	else addr+=3;
	break;
      case 24:
	// DJZ indir to dir
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)), machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)-1));
	if (machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)))==0) addr+=machine_p88kkk(buffer,addr+1);
	else addr+=3;
	break;
      case 25:
	// DJZ indir to indir
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)), machine_p88kkk(buffer,machine_peekkk(buffer,addr+2)-1));
	if (machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)))==0){
	y=(ADDRHI<<8)+ADDRLO;
	addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+1)));
	}
	else addr+=3;
	break;
      case 26:
	//- add new thread at address x SPL
	cpustackpushhh(buffer,machine_peekkk(buffer,addr+1),machine_peekkk(buffer,addr+2),6,0);
	addr+=3;
	break;
      case 27:
	addr+=3;
	break;
      case 28:
	// input to direct.
	machine_pokeee(buffer,addr+machine_p88kkk(buffer,addr+2),randi()%255);
	addr+=3;
	break;
      case 29:
	// to indirect.
	machine_pokeee(buffer,machine_peekkk(buffer,machine_peekkk(buffer,addr+2)),randi()%255);
	addr+=3;
	break;
      }
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    

///////////////////////////////////////////////////////////////

    case 7:
      // SIR: add in input and output??? untested for audio but...
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      //      printf("instr %d ",instr);
      switch(instr%5){
      case 0:
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
	if (machine_p88kkk(buffer,addr+1)<128) machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)+1);
	addr++;
	break;
      case 1:	  
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128))    if (randi()%10 < 4) cells[IP] = dead;     
	if (machine_p88kkk(buffer,addr+1)<128)  if (randi()%10 < 4) machine_pokeee(buffer,addr,255);     
	addr++;
	break;
      case 2:
	//  if (cells[(IP+1)] >= 128) cells[IP] = recovered;
	if (machine_p88kkk(buffer,addr+1)>=128)  machine_pokeee(buffer,addr,129);
	addr++;
	break;
      case 3:
	if (machine_p88kkk(buffer,addr+1)==0){
	  if ((machine_p88kkk(buffer,addr-1)>0 && machine_p88kkk(buffer,addr-1)<128) ||
	      (machine_p88kkk(buffer,addr+1)>0 && machine_p88kkk(buffer,addr+1)<128))
	    {
	if (randi()%10 < 4) machine_pokeee(buffer,addr,129);
	    }
	}
	addr++;
	break;
      case 4:
	machine_pokeee(buffer,addr,randi()%255);
	addr++;
	break;
      }
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    

///////////////////////////////////////////////////////////////

    case 8:
      // WOrmcode: but was with overlaps and also we need extra buffer to steer!
      //      instr=machine_peekkk(buffer,addr);
      //      printf("instr %d ",instr);

      wormdir=randi()%8;
      addr=((PCADDRHI<<8)+PCADDRLO);
      y=biotadir[wormdir];
      addr+=y;
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      switch(instr%13){
      case 0:
	break;
      case 1:
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)+1);
	break;
      case 2:
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)-1);
	break;
      case 3:
	addr+=machine_p88kkk(buffer,addr+y);
	break;
      case 4:
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)+machine_p88kkk(buffer,addr+y));
	break;
      case 5:
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)-machine_p88kkk(buffer,addr+y));
	break;
      case 6:
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)<<1);
	break;
      case 7:
	machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr)>>1);
	break;
      case 8:
	if (machine_p88kkk(buffer,addr+(BIT81*2))==0) addr+=machine_p88kkk(buffer,addr+y);
	break;
      case 9:
	flag=machine_p88kkk(buffer,addr);
	machine_pokeee(buffer,addr-y,flag);
	machine_pokeee(buffer,addr+y,flag);
	break;
      case 10:
	thread_pushhh(buffer,machine_p88kkk(buffer,addr+y),offset);
	break;
      case 11:
	machine_pokeee(buffer,addr+y,thread_poppp(buffer,offset));
	break;
      case 12:
	machine_pokeee(buffer,addr+y,adc_buffer[thread_poppp(buffer,offset)%10]);      
	break;
      }

      //      printf("%c",instr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
   break;    

///////////////////////////////////////////////////////////////

    case 9:
      // stack machine a la: http://www.ece.cmu.edu/~koopman/stack_computers/sec3_2.html#321
      // but sans return stack
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      //      printf("instr %d ",instr);
      switch(instr%16){
      case 0:
	flag=thread_poppp(buffer,offset);
	machine_pokeee(buffer,thread_poppp(buffer,offset),flag);
	addr++;
	break;
      case 1:
	thread_pushhh(buffer,thread_poppp(buffer,offset)+thread_poppp(buffer,offset),offset);
	addr++;
	break;
      case 2:
	thread_pushhh(buffer,thread_poppp(buffer,offset)-thread_poppp(buffer,offset),offset);
	addr++;
	break;
      case 3:
	thread_pushhh(buffer,machine_p88kkk(buffer,thread_poppp(buffer,offset)),offset);
	addr++;
	break;
      case 4:
	thread_pushhh(buffer,thread_poppp(buffer,offset)&thread_poppp(buffer,offset),offset);
	addr++;
	break;
      case 5:
	thread_poppp(buffer,offset);
	addr++;
	break;
      case 6:
	flag=thread_poppp(buffer,offset);
	thread_pushhh(buffer,flag,offset);
	thread_pushhh(buffer,flag,offset);
	addr++;
	break;
      case 7:
	thread_pushhh(buffer,thread_poppp(buffer,offset)|thread_poppp(buffer,offset),offset);
	addr++;
	break;
      case 8:
	//Push a copy of the second element on the stack, N1, onto the top of the stack
	/*	this->m_reg8bit1=thread_pop(this);
	this->m_reg8bit2=thread_pop(this);
	thread_push(this,this->m_reg8bit2);
	thread_push(this,this->m_reg8bit1);
	thread_push(this,this->m_reg8bit2);
	*/
	BIT81=thread_poppp(buffer,offset);
	BIT82=thread_poppp(buffer,offset);
	thread_pushhh(buffer,BIT82,offset);
	thread_pushhh(buffer,BIT81,offset);
	thread_pushhh(buffer,BIT82,offset);
	addr++;
	break;
      case 9:
	BIT81=thread_poppp(buffer,offset);
	BIT82=thread_poppp(buffer,offset);
	thread_pushhh(buffer,BIT81,offset);
	thread_pushhh(buffer,BIT82,offset);
	addr++;
	break;
      case 10:
	thread_pushhh(buffer,thread_poppp(buffer,offset)^thread_poppp(buffer,offset),offset);
	addr++;
	break;
      case 11:
	y=(ADDRHI<<8)+ADDRLO;
	if (thread_poppp(buffer,offset)==0) addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	else 	addr++;
	break;
      case 12:
	//sub call
	y=(ADDRHI<<8)+ADDRLO;
	thread_pushhh(buffer,addr,offset);
	addr=y+machine_p88kkk(buffer,machine_peekkk(buffer,addr+1));
	break;
      case 13:
	//sub return
	y=(ADDRHI<<8)+ADDRLO;
	addr=y+thread_poppp(buffer,offset);
	break;
      case 14:
	thread_pushhh(buffer,machine_p88kkk(buffer,addr+1),offset);
	addr++;
	break;
      case 15:
	machine_pokeee(buffer,machine_peekkk(buffer,addr+1),adc_buffer[thread_poppp(buffer,offset)%10]);      
	addr++;
	break;

      }
      //      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;    

///////////////////////////////////////////////////////////////

    case 10:
      // befunge: http://en.wikipedia.org/wiki/Befunge
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      //      printf("instr %d ",instr);
      switch(instr%CPU_TOTAL){
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
	thread_pushhh(buffer,instr%30,offset);
	break;
      case 10:
	thread_pushhh(buffer,thread_poppp(buffer,offset)-thread_poppp(buffer,offset),offset);
	break;
      case 11:
	flag=thread_poppp(buffer,offset);
	thread_pushhh(buffer,thread_poppp(buffer,offset)-flag,offset);
	break;
      case 12:
	thread_pushhh(buffer,thread_poppp(buffer,offset)*thread_poppp(buffer,offset),offset);
	break;
      case 13:
	flag=thread_poppp(buffer,offset);
	if (flag!=0) thread_pushhh(buffer,thread_poppp(buffer,offset)/flag,offset);
	break;
      case 14:
	flag=thread_poppp(buffer,offset);
	if (flag!=0) thread_pushhh(buffer,thread_poppp(buffer,offset)%flag,offset);
	break;
      case 15:
	flag=thread_poppp(buffer,offset);
	if (flag==0) thread_pushhh(buffer,1,offset);
	else thread_pushhh(buffer,0,offset);
	break;
      case 16:
	flag=thread_poppp(buffer,offset);
	if (thread_poppp(buffer,offset)>flag) thread_pushhh(buffer,1,offset);
	else thread_pushhh(buffer,0,offset);
	break;
      case 17: // right
	BIT81=2;
	break;
      case 18: // left
	BIT81=6;
	break;
      case 19: // up
	BIT81=0;
	break;
      case 20: // down
	BIT81=4;
	break;
      case 21:
	BIT81=(randi()%4)*2;
	break;
      case 22:
	if (thread_poppp(buffer,offset)==0)	BIT81=2;
	else 	BIT81=6;
	break;
      case 23:
	if (thread_poppp(buffer,offset)==0)	BIT81=4;
	else 	BIT81=0;
	break;
      case 24:
	flag=thread_poppp(buffer,offset);
	thread_pushhh(buffer,flag,offset);
	thread_pushhh(buffer,flag,offset);
	break;
      case 25:
	BIT81=thread_poppp(buffer,offset);
	BIT82=thread_poppp(buffer,offset);
	thread_pushhh(buffer,BIT81,offset);
	thread_pushhh(buffer,BIT82,offset);
	break;
      case 26:
	thread_poppp(buffer,offset);
	break;
      case 27:
	addr+=biotadir[BIT81%8];
	break;
      case 28:
	machine_pokeee(buffer,(thread_poppp(buffer,offset))*(thread_poppp(buffer,offset)),thread_poppp(buffer,offset));
	break;
      case 29:
	thread_pushhh(buffer,machine_p88kkk(buffer,thread_poppp(buffer,offset)*thread_poppp(buffer,offset)),offset);
	break;
      case 30:
	machine_pokeee(buffer,(thread_poppp(buffer,offset))*(thread_poppp(buffer,offset)),adc_buffer[thread_poppp(buffer,offset)%10]);      
	break;

      }
      wormdir=BIT81%8;
      addr+=biotadir[wormdir];
      //      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////

    case 11:
      // kind of langton's ant as CPU - each thread has direction and
      // changes state of cell according to stack?
      // turn right/left/flip and move on
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
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
	  thread_pushhh(buffer,instr,offset);
	  break;
	case 12:
	  BIT81+=2;
	  machine_pokeee(buffer,addr,instr^thread_poppp(buffer,offset));
	  break;
	case 13:
	  BIT81-=2;
	  machine_pokeee(buffer,addr,instr^thread_poppp(buffer,offset));
	  break;
	}
      wormdir=BIT81%8;
      addr+=biotadir[wormdir];
      //      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////

    case 12:
      // turmites code: turmite has state,direction,position...
      // reg8bit1 is state. reg8bit2 is direction index.
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
   
      machine_pokeee(buffer,addr,instr+BIT81);
      //delta = dmove[(instr - buffer->reg8bit1) & 0xf];
      flag=instr - BIT81;
      //tm->dir = (tm->dir + delta) & 3;
      BIT82=(BIT82+flag)%8;
      //do move and wrap
      wormdir=BIT82;
      addr+=biotadir[wormdir]; // %8 is above
      // finally
      BIT81 += deltastate[instr%16];
      //      printf("%c",addr);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////

    case 13:
      // linear CA/life code. copies to 128 steps ahead new state and keeps going(?)

      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);

      other=(machine_p88kkk(buffer,addr-1)&1)+(machine_p88kkk(buffer,addr+1)&1)+(machine_p88kkk(buffer,addr-32)&1)+(machine_p88kkk(buffer,addr+32)&1)+(machine_p88kkk(buffer,addr-31)&1)+(machine_p88kkk(buffer,addr-33)&1)+(machine_p88kkk(buffer,addr+31)&1)+(machine_p88kkk(buffer,addr+33)&1);

    if ((instr&1)==1 && other<2) flag=0;
    else if ((instr&1)==1 && other>3) flag=0;
    else if ((instr&1)==0 && other==3) flag=instr+1;
    else flag=instr;

    machine_pokeee(buffer,addr+128,flag);
    addr++; 
    PCADDRHI=addr>>8; // hi/lo
    PCADDRLO=addr&255;
      //      printf("%c",instr);
      break;

    case 14:
      // ants revisited... how to do multiple ants using stack?
      // read cell//process rule string//change cell//move ant
      // rule string is from cell[0]
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      //      machine_pokeee(buffer,addr,instr+1);
      machine_pokeee(buffer,addr,instr+biotadir[BIT81%8]);
      BIT81=antrule(BIT81,instr%8,machine_p88kkk(buffer,0));//last is rule
      addr+=biotadir[BIT81%8];
      //      printf("%c",addr);
    PCADDRHI=addr>>8; // hi/lo
    PCADDRLO=addr&255;
      break;

    case 15:
      // second CA from CA.c
      flag = 0;
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      if (machine_p88kkk(buffer,addr+1)>128)	flag |= 0x4;
      if (instr>128) flag |= 0x2;
      if (machine_p88kkk(buffer,addr-1)>128)	flag |= 0x1;
                     
      if ((machine_p88kkk(buffer,0) >> flag) & 1)	machine_pokeee(buffer,addr+128,instr+129);
      else machine_pokeee(buffer,addr+128,instr-129); // or we stay with 255 and 0 as pokeeed

      addr++; 
      if (addr==0) addr=1;
    PCADDRHI=addr>>8; // hi/lo
    PCADDRLO=addr&255;
      //      printf("%c",instr);
      break;

    case 16:
      // port of hodge -// larger 256*128 (32768) cellspace in two halves
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      flag=0;
      temprr=machine_p88kkk(buffer,addr)+machine_p88kkk(buffer,addr-1)+machine_p88kkk(buffer,addr+1)+machine_p88kkk(buffer,addr-256)+machine_p88kkk(buffer,addr+256)+machine_p88kkk(buffer,addr-255)+machine_p88kkk(buffer,addr-257)+machine_p88kkk(buffer,addr+255)+machine_p88kkk(buffer,addr+257);

      if (machine_p88kkk(buffer,addr-1)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr+1)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr-256)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr+256)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr-255)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr-257)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr+255)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;
      if (machine_p88kkk(buffer,addr+257)==machine_p88kkk(buffer,0)-1) flag++; else if (machine_p88kkk(buffer,addr-1)>0) other++;

      y=addr+32768;
      //      if (y<4) y=4;

      if(machine_p88kkk(buffer,addr) == 0)// && (machine_p88kkk(buffer,1)+1)!=0) 
	machine_pokeee(buffer,addr,floorf(other / ((machine_p88kkk(buffer,1)>>4)+1)) + floorf(flag/((machine_p88kkk(buffer,2)>>4)+1)));
      else if(machine_p88kkk(buffer,addr) < machine_p88kkk(buffer,0)-1){
	machine_pokeee(buffer,y,floorf(temprr / (other + 1)) + machine_p88kkk(buffer,3));
	//	printf("hell: %d %d %d %d %d\n", y,temprr, other, temprr / (other + 1), machine_p88kkk(buffer,3));

      }
  else
  machine_pokeee(buffer,y,0);

  if(machine_p88kkk(buffer,addr) > machine_p88kkk(buffer,0)-1)
    machine_pokeee(buffer,y,machine_p88kkk(buffer,0)-1);

  addr++; 
  //  if (addr<4) addr=4;
  PCADDRHI=addr>>8; // hi/lo
  PCADDRLO=addr&255;
  break;

  ////////////////////////////

    case 17:
      // start generic - add (add/sub/zero/copy/invert/swap)
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      machine_pokeee(buffer,addr,instr+1);
      addr++; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
    case 18:
      // generic - sub (add/sub/zero/copy/invert/swap)
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      machine_pokeee(buffer,addr,instr-1);
      addr++; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
    case 19:
      // generic - zero (add/sub/zero/copy/invert/swap)
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      machine_pokeee(buffer,addr,0);
      addr++; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
    case 20:
      // generic - copy (add/sub/zero/copy/invert/swap)
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr+1));
      addr++; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
    case 21:
      // generic - copy (add/sub/zero/copy/invert/swap)
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr^255));
      addr++; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
    case 22:
      // generic - swap (add/sub/zero/copy/invert/swap)
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr+1);
      machine_pokeee(buffer,addr+1,machine_p88kkk(buffer,addr));
      machine_pokeee(buffer,addr,instr);
      addr+=2; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
    case 23:
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      machine_pokeee(buffer,machine_peekkk(buffer,addr),adc_buffer[machine_p88kkk(buffer,addr)%10]);     addr++;
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 24:
      // from wormcode.c
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      switch(instr%15)
	{
	case 0:
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 1:
	  //inc
	  machine_pokeee(buffer,addr,instr+1);
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 2:
	  machine_pokeee(buffer,addr,instr-1);
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 3:
	  y=(ADDRHI<<8)+ADDRLO;
	  addr=y+machine_p88kkk(buffer,addr);
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 4:
	  machine_pokeee(buffer,addr,randi()%255);
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 5:
	  machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr+biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 6:
	  machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr+biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 7:
	  machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr-biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 8:
	  machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr<<biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 9:
	  machine_pokeee(buffer,addr,machine_p88kkk(buffer,addr>>biotadir[randi()%8]));
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 10:
	  if (machine_p88kkk(buffer,addr+(biotadir[randi()%8]*2))==0){
	    y=biotadir[randi()%8];
	    addr+=y;
	  }
	  break;
	case 11:
	  machine_pokeee(buffer,(addr-biotadir[randi()%8]),instr);
	  machine_pokeee(buffer,(addr+biotadir[randi()%8]),instr);
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 12:
	  thread_pushhh(buffer, machine_p88kkk(buffer,addr+biotadir[randi()%8]),offset);
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
	case 13:
	  machine_pokeee(buffer,(addr+=biotadir[randi()%8]),thread_poppp(buffer,offset));
	  y=biotadir[randi()%8];
	  addr+=y;
	  break;
      case 14:
	machine_pokeee(buffer,(addr+=biotadir[randi()%8]),adc_buffer[thread_poppp(buffer,offset)%10]);      
	break;
	}
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 25:
      // 16 bit increment
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      y=(instr<<8)+machine_p88kkk(buffer,addr+1)+1;
      machine_pokeee(buffer,addr,y>>8);      
      machine_pokeee(buffer,addr+1,y&255);      
      addr+=2;
      //      printf("%d\n",y);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 26:
      // 16 bit decrement
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      y=(instr<<8)+machine_p88kkk(buffer,addr+1)-1;
      machine_pokeee(buffer,addr,y>>8);      
      machine_pokeee(buffer,addr+1,y&255);      
      addr+=2;
      //      printf("%d\n",y);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 27:
      // 16 bit left
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      y=(instr<<9)+(machine_p88kkk(buffer,addr+1)<<1);
      machine_pokeee(buffer,addr,y>>8);      
      machine_pokeee(buffer,addr+1,y&255);      
      addr+=2;
      //      printf("%d\n",y);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 28:
      // 16 bit right
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      instr=machine_p88kkk(buffer,addr);
      y=(instr<<7)+(machine_p88kkk(buffer,addr+1)>>1);
      machine_pokeee(buffer,addr,y>>8);      
      machine_pokeee(buffer,addr+1,y&255);      
      addr+=2;
      //      printf("%d\n",y);
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 29:
      // pure leakage - push instr onto stack. when stack is full pull off...
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      if (thread_stack_counttt(buffer,STACK_SIZEE,offset)) machine_pokeee(buffer,addr,thread_poppp(buffer,offset));
      else thread_pushhh(buffer,machine_p88kkk(buffer,addr),offset);
      addr++;
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
      break;
///////////////////////////////////////////////////////////////
    case 30:
      // convolution
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      temp=(machine_p88kkk(buffer,addr-1)*machine_p88kkk(buffer,0))+(machine_p88kkk(buffer,addr)*machine_p88kkk(buffer,1))+(machine_p88kkk(buffer,addr+1)*machine_p88kkk(buffer,2));
      y=addr+32768;
      //      if (y<3) y=3;
      machine_pokeee(buffer,y,temp);
      addr++; 
      //      if (addr<3) addr=3;
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
///////////////////////////////////////////////////////////////
    case 31:
      // null
      addr=((PCADDRHI<<8)+PCADDRLO);
      if (addr>((WRAPADDRHI<<8)+WRAPADDRLO)) addr=((ADDRHI<<8)+ADDRLO);
      addr++; 
      PCADDRHI=addr>>8; // hi/lo
      PCADDRLO=addr&255;
    }
}
}

inline u16 machine_peekkk(u8* buffer, uint16_t addr) {
  //	return buffer->m_heap[addr%HEAP_SIZE];
  u16 addr2=addr+1;
  return (buffer[addr]<<8)+buffer[addr2];
}

inline u8 machine_p88kkk(u8* buffer, uint16_t addr) {
  //	return buffer->m_heap[addr%HEAP_SIZE];
  return (buffer[addr]);
}


inline void machine_pokeee(u8* buffer, uint16_t addr, u8 data) {
  //	buffer->m_heap[addr%HEAP_SIZE]=data;
  buffer[addr]=data; // protecting 0 as number of threads???
  //  printf("%c",data);
}



///////////////////////////////////////////////////////////////


#ifdef PCSIM
/*
int main(void)
{
  u16 x; u16 addr,offset=0;
  u8 buffer[65536];// u16 *testi; u8 *testo;
  srandom(time(0));
  for (x=0;x<65535;x++){
    buffer[x]=randi()%255;
  }

  //   u16 threads[MAX_FRED];// or this is also in the buffer! DONE

  // inc threadcount, array of offsets
  for (x=0;x<MAX_FRED/2;x++){
    addr=randi()%65536;
    cpustackpushhh(buffer,addr,addr+randi()%65536,randi()%31,1);
    //    cpustackpushhh(buffer,addr,addr+randi()%65536,3,1);//masque, 6=corewars
}

  x=0; 
    while(1) {
      machine_runnn(buffer);
          printf("%c",buffer[x++]);
            x++;
	  //	  addr=randi()%65536;
      //	   if ((rand()%2)==1)     cpustackpushhh(buffer,addr,addr+randi()%65536,randi()%31,1); 

      //   else  			cpustackpoppp(buffer); 
		 //  printf("stackposy: %d\n", stack_posy);


    }

}*/
#endif

