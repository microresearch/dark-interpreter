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
 	if (villager->m_stack_pos>=0 && villager->m_stack_pos<16)
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
	if (villager->m_stack_pos>=0 && villager->m_stack_pos<16)
	{
	  return villager->m_stack[villager->m_stack_pos];
	}
	return 0;
}

///////////////////////////////////////////////////////////////

inline u16 machine_peek(uint16_t addr) {
  //	return villager->m_heap[addr%HEAP_SIZE];
  u16 y;
  y=addr+1;
  return (datagenbuffer[addr]<<8)+datagenbuffer[y];
}

inline u8 machine_p88k(uint16_t addr) {
  //	return villager->m_heap[addr%HEAP_SIZE];
  return datagenbuffer[addr];
}


inline void machine_poke(uint16_t addr, u8 data) {
  //	villager->m_heap[addr%HEAP_SIZE]=data;
  datagenbuffer[addr]=data;
  //      printf("%d\n",&villager->m_memory[addr]);
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

////////////////////

void xxrun1(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;
  u8 instr=datagenbuffer[pos];
  for (u8 xx=0;xx<villager->howmany;xx++){
      switch(instr%5){
      case 0:
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
	if (machine_p88k(pos+1)<128) machine_poke(pos,machine_p88k(pos)+1);
	pos+=villager->speed;
	break;
      case 1:	  
	//  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128))    if (randi()%10 < 4) cells[IP] = dead;     
	if (machine_p88k(pos+1)<128)  if (randi()%10 < 4) machine_poke(pos,255);     
	pos+=villager->speed;
	break;
      case 2:
	//  if (cells[(IP+1)] >= 128) cells[IP] = recovered;
	if (machine_p88k(pos+1)>=128)  machine_poke(pos,129);
	pos+=villager->speed;
	break;
      case 3:
	if (machine_p88k(pos+1)==0){
	  if ((machine_p88k(pos-1)>0 && machine_p88k(pos-1)<128) ||
	      (machine_p88k(pos+1)>0 && machine_p88k(pos+1)<128))
	    {
	if (randi()%10 < 4) machine_poke(pos,129);
	    }
	}
	pos+=villager->speed;
	break;
      case 4:
	machine_poke(pos,randi()&255);
	//	pos+=villager->speed;
	break;
      }
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
      villager->position=pos;
}

void xxrunworm(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;//=datagenbuffer[pos];
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};
  u8 flag;// float other=0.0f;
  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
      wormdir=randi()&7;
      villager->m_reg16bit1=biotadir[wormdir];
      pos+=villager->m_reg16bit1;
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
      instr=machine_p88k(pos);
      switch(instr%13){
      case 0:
	break;
      case 1:
	machine_poke(pos,machine_p88k(pos)+1);
	break;
      case 2:
	machine_poke(pos,machine_p88k(pos)-1);
	break;
      case 3:
	pos+=machine_p88k(pos+villager->m_reg16bit1);
	break;
      case 4:
	machine_poke(pos,machine_p88k(pos)+machine_p88k(pos+villager->m_reg16bit1));
	break;
      case 5:
	machine_poke(pos,machine_p88k(pos)-machine_p88k(pos+villager->m_reg16bit1));
	break;
      case 6:
	machine_poke(pos,machine_p88k(pos)<<1);
	break;
      case 7:
	machine_poke(pos,machine_p88k(pos)>>1);
	break;
      case 8:
	if (machine_p88k(pos+(villager->m_reg8bit2*2))==0) pos+=machine_p88k(pos+villager->m_reg16bit1);
	break;
      case 9:
	flag=machine_p88k(pos);
	machine_poke(pos-villager->m_reg16bit1,flag);
	machine_poke(pos+villager->m_reg16bit1,flag);
	break;
      case 10:
	thread_push(villager,machine_p88k(pos+villager->m_reg16bit1));
	break;
      case 11:
	machine_poke(pos+villager->m_reg16bit1,thread_pop(villager));
	break;
      case 12:
	machine_poke(pos+villager->m_reg16bit1,adc_buffer[thread_pop(villager)%10]);      	break;
      }
  }
villager->position=pos;
}

void xxrunstack(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr,flag;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
    instr=datagenbuffer[pos];
      switch(instr&15){
      case 0:
	flag=thread_pop(villager);
	machine_poke(thread_pop(villager),flag);
	pos+=villager->speed;
	break;
      case 1:
	thread_push(villager,thread_pop(villager)+thread_pop(villager));
	pos+=villager->speed;
	break;
      case 2:
	thread_push(villager,thread_pop(villager)-thread_pop(villager));
	pos+=villager->speed;
	break;
      case 3:
	thread_push(villager,machine_p88k(thread_pop(villager)));
	pos+=villager->speed;
	break;
      case 4:
	thread_push(villager,thread_pop(villager)&thread_pop(villager));
	pos+=villager->speed;
	break;
      case 5:
	thread_pop(villager);
	pos+=villager->speed;
	break;
      case 6:
	flag=thread_pop(villager);
	thread_push(villager,flag);
	thread_push(villager,flag);
	pos+=villager->speed;
	break;
      case 7:
	thread_push(villager,thread_pop(villager)|thread_pop(villager));
	pos+=villager->speed;
	break;
      case 8:
	//Push a copy of the second element on the stack, N1, onto the top of the stack
	villager->m_reg8bit1=thread_pop(villager);
	villager->m_reg8bit2=thread_pop(villager);
	thread_push(villager,villager->m_reg8bit2);
	thread_push(villager,villager->m_reg8bit1);
	thread_push(villager,villager->m_reg8bit2);
	pos+=villager->speed;
	break;
      case 9:
	villager->m_reg8bit1=thread_pop(villager);
	villager->m_reg8bit2=thread_pop(villager);
	thread_push(villager,villager->m_reg8bit1);
	thread_push(villager,villager->m_reg8bit2);
	pos+=villager->speed;
	break;
      case 10:
	thread_push(villager,thread_pop(villager)^thread_pop(villager));
	pos+=villager->speed;
	break;
      case 11:
	if (thread_pop(villager)==0) pos=villager->start+machine_p88k(machine_peek(pos+1));
	else 	pos+=villager->speed;
	break;
      case 12:
	//sub call
	thread_push(villager,pos);
	pos=villager->start+machine_p88k(machine_peek(pos+1));
	break;
      case 13:
	//sub return
	pos=villager->start+thread_pop(villager);
	break;
      case 14:
	thread_push(villager,machine_p88k(pos+1));
	pos+=villager->speed;
	break;
      case 15:
	machine_poke(machine_peek(pos+1),adc_buffer[thread_pop(villager)%10]);      
	pos+=villager->speed;
	break;
      }
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunbefunge(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr,flag;

  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};
  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
  instr=datagenbuffer[pos];
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
	villager->m_reg8bit2=(randi()&3)*2;
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
	pos+=biotadir[villager->m_reg8bit2&7];
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
      wormdir=villager->m_reg8bit2&7;
      pos+=biotadir[wormdir];
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunlang(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
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
	  machine_poke(pos,instr^thread_pop(villager));
	  break;
	case 13:
	  villager->m_reg8bit2-=2;
	  machine_poke(pos,instr^thread_pop(villager));
	  break;
	}
      wormdir=villager->m_reg8bit2&7;
      pos+=biotadir[wormdir];
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunturm(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr,flag;
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};
  u8 deltastate[16] = {1, 4, 2, 7, 3, 13, 4, 7, 8, 9, 3, 12,
			6, 11, 5, 13};

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
  instr=datagenbuffer[pos];
      machine_poke(pos,instr+villager->m_reg8bit1);
      //delta = dmove[(instr - villager->reg8bit1) & 0xf];
      flag=instr - villager->m_reg8bit1;
      //tm->dir = (tm->dir + delta) & 3;
      villager->m_reg8bit2=(villager->m_reg8bit2+flag)&8;
      //do move and wrap
      wormdir=(villager->m_reg8bit2)&7;
      pos+=biotadir[wormdir];
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
      // finally
      villager->m_reg8bit1 += deltastate[instr&15];
      //      printf("%c",pos);
}
villager->position=pos;
}

void xxrunca(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr,other,flag;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
      other=(machine_p88k(pos-1)&1)+(machine_p88k(pos+1)&1)+(machine_p88k(pos-32)&1)+(machine_p88k(pos+32)&1)+(machine_p88k(pos-31)&1)+(machine_p88k(pos-33)&1)+(machine_p88k(pos+31)&1)+(machine_p88k(pos+33)&1);

    if ((instr&1)==1 && other<2) flag=0;
    else if ((instr&1)==1 && other>3) flag=0;
    else if ((instr&1)==0 && other==3) flag=instr+1;
    else flag=instr;

    machine_poke(pos+128,flag);
    pos+=villager->speed; 
    if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunant(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr; 
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

  for (u8 xx=0;xx<villager->howmany;xx++){
  instr=datagenbuffer[pos];
  machine_poke(pos,instr+biotadir[villager->m_reg8bit1&7]);
  villager->m_reg8bit1=antrulee(villager->m_reg8bit1,instr&7,machine_p88k(0));//last is rule
  pos+=biotadir[villager->m_reg8bit1&7];
  if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
  villager->position=pos;
}

void xxrunca2(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;
  u8 flag;//,temp,other=0;// float other=0.0f;
  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
    instr=datagenbuffer[pos];
    flag=0;
      if (machine_p88k(pos+1)>128)	flag |= 0x4;
      if (instr>128) flag |= 0x2;
      if (machine_p88k(pos-1)>128)	flag |= 0x1;
                     
      if ((machine_p88k(0) >> flag) & 1)	machine_poke(pos+128,instr+129);
      else machine_poke(pos+128,instr-129); // or we stay with 255 and 0 as poked
      pos+=villager->speed; 
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunhodge(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;
  u8 instr,flag,other,temp;
  u16 y;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
  instr=datagenbuffer[pos]; flag=0; other=0;
      temp=machine_p88k(pos)+machine_p88k(pos-1)+machine_p88k(pos+1)+machine_p88k(pos-256)+machine_p88k(pos+256)+machine_p88k(pos-255)+machine_p88k(pos-257)+machine_p88k(pos+255)+machine_p88k(pos+257);

      if (machine_p88k(pos-1)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos-1)>0) other+=1;
      if (machine_p88k(pos+1)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos+1)>0) other+=1;
      if (machine_p88k(pos-256)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos-256)>0) other+=1;
      if (machine_p88k(pos+256)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos+256)>0) other+=1;
      if (machine_p88k(pos-255)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos-255)>0) other+=1;
      if (machine_p88k(pos-257)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos-257)>0) other+=1;
      if (machine_p88k(pos+255)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos+255)>0) other+=1;
      if (machine_p88k(pos+257)==machine_p88k(0)-1) flag++; else if (machine_p88k(pos+257)>0) other+=1;

      y=pos+32768;
      if (y<4) y=4;

      //      y=floor((double)other / (double)(machine_p88k(1)>>4));
      //      printf("floor: %d other: %d peek: %d\n",y,other,machine_p88k(2));

      //      printf("peek: %d\n",machine_p88k(2));


      if(machine_p88k(pos) == 0){
	u8 zx=floorf(other / ((machine_p88k(1)>>4)+1)) + floorf(flag/((machine_p88k(2)>>4)+1));
	    machine_poke(pos,zx);
      }
      else if(machine_p88k(pos) < machine_p88k(0)-1){
	u8 zx=floorf(temp / (other + 1)) + machine_p88k(3);
	machine_poke(pos,zx);
      }
  else
    machine_poke(y,0);

  if(machine_p88k(pos) > machine_p88k(0)-1)
    machine_poke(y,machine_p88k(0)-1);
  pos+=villager->speed; 
  if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunworm2(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u16 y;
  u8 instr;
  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
      switch(instr%15)
	{
	case 0:
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 1:
	  //inc
	  machine_poke(pos,instr+1);
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 2:
	  machine_poke(pos,instr-1);
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 3:
	  pos=villager->start+machine_p88k(pos);
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 4:
	  machine_poke(pos,randi()&255);
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 5:
	  machine_poke(pos,machine_p88k(pos+biotadir[randi()&7]));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 6:
	  machine_poke(pos,machine_p88k(pos+biotadir[randi()&7]));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 7:
	  machine_poke(pos,machine_p88k(pos-biotadir[randi()&7]));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 8:
	  machine_poke(pos,machine_p88k(pos<<biotadir[randi()&7]));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 9:
	  machine_poke(pos,machine_p88k(pos>>biotadir[randi()&7]));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 10:
	  if (machine_p88k(pos+(biotadir[randi()&7]*2))==0){
	    y=biotadir[randi()&7];
	    pos+=y;
	  }
	  else {  y=biotadir[randi()&7];
	  pos+=y;
	  }
	  break;
	case 11:
	  machine_poke((pos-biotadir[randi()&7]),instr);
	  machine_poke((pos+biotadir[randi()&7]),instr);
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 12:
	  thread_push(villager, machine_p88k(pos+biotadir[randi()&7]));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
	case 13:
	  machine_poke((pos+=biotadir[randi()&7]),thread_pop(villager));
	  y=biotadir[randi()&7];
	  pos+=y;
	  break;
      case 14:
	machine_poke((pos+=biotadir[randi()&7]),adc_buffer[thread_pop(villager)%10]);      
	break;
	}
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunleaky(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
  instr=datagenbuffer[pos];
      if (thread_stack_count(villager,16)) machine_poke(pos,thread_pop(villager));
      else thread_push(villager,machine_p88k(pos));
      pos+=villager->speed;
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunconvy(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr,temp;
  u16 y;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
    instr=datagenbuffer[pos];
      temp=(machine_p88k(pos-1)*machine_p88k(0))+(machine_p88k(pos)*machine_p88k(1))+(machine_p88k(pos+1)*machine_p88k(2));
      y=pos+32768;
      //      if (y<3) y=3;
      machine_poke(y,temp);
      pos+=villager->speed; 
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunplague(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;

  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};
  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
      switch(instr%5){
      case 0:
	machine_poke(pos,255);
	machine_poke(pos+1,255);
	//	pos+=2;
	pos+=biotadir[villager->m_reg8bit2&7];
	break;
      case 1:
	if (machine_p88k(pos)<128){
	  machine_poke(pos-1,machine_p88k(pos));
	  machine_poke(pos+1,machine_p88k(pos));
	}
	pos+=biotadir[villager->m_reg8bit2&7];
	break;
      case 2:
	machine_poke(pos-1,0);
	machine_poke(pos+1,0);
	pos+=biotadir[villager->m_reg8bit2&7];
	break;
      case 3:
	if ((machine_p88k(pos)%0x03)==1) villager->m_reg8bit2+=4;
	else villager->m_reg8bit2*=machine_p88k(pos)>>4;
	pos+=biotadir[villager->m_reg8bit2&7];
	break;
      case 4:
	machine_poke(pos+1,adc_buffer[(villager->m_reg8bit1>>8)%10]);
	pos+=biotadir[villager->m_reg8bit2&7];
	  break;
      }
      if (pos>(villager->start+villager->wrap)) pos=villager->start;

      if (machine_p88k(pos)==255) villager->m_reg8bit2+=4;
      	wormdir=villager->m_reg8bit2&7;
	//      printf("%c",pos);
  }
villager->position=pos;
}

void xxrunmicro(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
  instr=datagenbuffer[pos];
      switch(instr%15){
      case 0:
	villager->m_reg16bit1+=2;
	pos+=villager->speed;
	break;
      case 1:
	villager->m_reg16bit1-=2;
	pos+=villager->speed;
	break;
      case 2:
	machine_poke(villager->m_reg16bit1,machine_p88k(villager->m_reg16bit1)+1);
	pos+=villager->speed;
	break;
      case 3:	  
	machine_poke(villager->m_reg16bit1,machine_p88k(villager->m_reg16bit1)-1);
	pos+=villager->speed;
	break;
      case 4:	  
	machine_poke(pos,machine_p88k(pos)+1);
	pos+=villager->speed;
	break;
      case 5:	  
	machine_poke(pos,machine_p88k(pos)-1);
	pos+=villager->speed;
	break;
      case 6:	  
	machine_poke(pos,machine_p88k(pos)>>1);
	pos+=villager->speed;
	break;
      case 7:	  
	machine_poke(pos,machine_p88k(pos)>>2);
	pos+=villager->speed;
	break;
      case 8:	  
	machine_poke(pos,machine_p88k(pos)>>3);
	pos+=villager->speed;
	break;
      case 9:	  
	if (machine_peek(pos+1)==0) {
	    pos=villager->start+villager->m_reg16bit1;
	}
	else pos+=villager->speed;
	break;
      case 10:	  
	if (machine_p88k(pos+1)<128 && machine_p88k(pos+1)>0) pos+=machine_p88k(pos+1);
	else 	pos+=villager->speed;
	break;
      case 11:	  
	if (machine_p88k(pos-1)<128) machine_poke(pos,machine_p88k(pos));
	pos+=villager->speed;
	break;
      case 12:	  
	machine_poke(pos,machine_p88k(pos+1));
	pos+=villager->speed;
	break;
      case 13:	  
	pos+=villager->speed;
	break;
      case 14:
	machine_poke(pos,adc_buffer[((villager->m_reg8bit1)>>8)%10]);
	pos+=villager->speed;
	break;
      }
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxruncw(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
      switch(instr%30){
      case 0:
	// MOV # to direct.
	machine_poke(pos+machine_p88k(pos+2),machine_p88k(pos+1));
	pos+=3;
	break;
      case 1:	  
	// MOV indirect to direct.
	machine_poke(pos+machine_p88k(pos+2),machine_p88k(machine_peek(pos+1)));
	pos+=3;
	break;
      case 2:
	// MOV # to indirect.
	machine_poke(machine_peek(machine_peek(pos+2)),machine_p88k(pos+1));
	pos+=3;
	break;
      case 3:
	//MOV indirect to indirect
	machine_poke(machine_peek(machine_peek(pos+2)),machine_p88k(machine_peek(pos+1)));
	pos+=3;
	break;
      case 4:
	//ADD # to direct
	machine_poke(pos+machine_p88k(pos+2),machine_p88k(pos+1)+machine_p88k(pos+machine_p88k(pos+2)));
	pos+=3;
	break;
	//HERE
      case 5:
	// ADD indirect to direct.
	machine_poke(pos+machine_p88k(pos+2),machine_p88k(machine_peek(pos+1))+(machine_p88k(pos+(machine_p88k(pos+2)>>8))));
	pos+=3;
	break;
      case 6:
	// ADD # to indirect.
	machine_poke(machine_peek(machine_peek(pos+2)),machine_p88k(pos+1)+machine_p88k(machine_peek(pos+2)));
	pos+=3;
	break;
      case 7:
	//ADD indirect to indirect
	machine_poke(machine_p88k(machine_peek(pos+2)),machine_p88k(machine_peek(pos+1))+machine_p88k(machine_peek(pos+2)));
	pos+=3;
	break;
      case 8:
	//SUB # to direct
	machine_poke(pos+machine_p88k(pos+2),machine_p88k(pos+1)-machine_p88k(pos+machine_p88k(pos+2)));
	pos+=3;
	break;
      case 9:
	// indirect to direct.
	machine_poke(pos+machine_p88k(pos+2),machine_p88k(machine_peek(pos+1)-machine_p88k(pos+machine_p88k(pos+2))));
	pos+=3;
	break;
      case 10:
	// # to indirect.
	machine_poke(machine_peek(machine_peek(pos+2)),machine_p88k(pos+1)-machine_p88k(machine_peek(pos+2)));
	pos+=3;
	break;
      case 11:
	// indirect to indirect
	machine_poke(machine_peek(machine_peek(pos+2)),machine_p88k(machine_peek(pos+1))-machine_p88k(machine_peek(pos+2)));
	pos+=3;
	break;
      case 12:
	// jmp to direct
	pos+=machine_p88k(pos+1);
	break;
      case 13:
	// jmp to indirect
	pos=villager->start+machine_p88k(machine_peek(pos+1));
	break;
      case 14:
	// JMZdirect to direct
	if (machine_p88k(pos+machine_p88k(pos+2))==0) 	pos+=machine_p88k(pos+1);
	else 	pos+=3;
	break;
      case 15:
	// JMZdirect to indirect
	if (machine_p88k(pos+machine_p88k(pos+2))==0) 	pos=villager->start+machine_p88k(machine_peek(pos+1));
	else 	pos+=3;
	break;
      case 16:	
	// JMZindirect to direct
	if (machine_p88k(machine_peek(machine_peek(pos+2)))==0) 	pos+=machine_p88k(pos+1);
	else 	pos+=3;
	break;
      case 17:
	// JMZindirect to indirect
	if (machine_p88k(machine_peek(machine_peek(pos+2)))==0) 	pos=villager->start+machine_p88k(machine_peek(pos+1));
	else 	pos+=3;
	break;
      case 18:
	// JMGdirect to direct
	if (machine_p88k(pos+machine_p88k(pos+2))>0) 	pos+=machine_p88k(pos+1);
	else 	pos+=3;
	break;
      case 19:
	// JMGdirect to indirect
	if (machine_p88k(pos+machine_p88k(pos+2))>0) 	pos=villager->start+machine_p88k(machine_peek(pos+1));
	else 	pos+=3;
	break;
      case 20:	
	// JMGindirect to direct
	if (machine_p88k(machine_peek(machine_peek(pos+2)))>0) 	pos+=machine_p88k(pos+1);
	else 	pos+=3;
	break;
      case 21:
	// JMGindirect to indirect
	if (machine_p88k(machine_peek(machine_peek(pos+2)))>0) 	pos=villager->start+machine_p88k(machine_peek(pos+1));
	else 	pos+=3;
	break;
      case 22:
	// Dec, jump if zero... sub 1 from b. jump to a if b is zero
	// DJZ dir to dir
	machine_poke(pos+machine_p88k(pos+2), machine_p88k(pos+machine_p88k(pos+2))-1);
	if (machine_p88k(pos+machine_p88k(pos+2))==0)	  pos+=machine_p88k(pos+1);
	else pos+=3;
	break;
      case 23:
	// DJZ dir to indir
	machine_poke(pos+machine_p88k(pos+2), machine_p88k(pos+machine_p88k(pos+2))-1);
	if (machine_p88k(pos+machine_p88k(pos+2))==0)	  pos=villager->start+machine_p88k(machine_peek(pos+1));
	else pos+=3;
	break;
      case 24:
	// DJZ indir to dir
	machine_poke(machine_peek(machine_peek(pos+2)), machine_p88k(machine_peek(pos+2)-1));
	if (machine_p88k(machine_peek(machine_peek(pos+2)))==0) pos+=machine_p88k(pos+1);
	else pos+=3;
	break;
      case 25:
	// DJZ indir to indir
	machine_poke(machine_peek(machine_peek(pos+2)), machine_p88k(machine_peek(pos+2)-1));
	if (machine_p88k(machine_peek(machine_peek(pos+2)))==0) pos=villager->start+machine_p88k(machine_peek(machine_peek(pos+1)));
	else pos+=3;
	break;
      case 26:
	// SPL
	//- add new thread at address x TESTY TODO!
	//	cpustackpush(machine_peek(pos+1),machine_peek(pos+2),6,villager->m_del);
	//	printf("adde\n");
	pos+=3;
	break;
      case 27:
	pos+=3;
	break;
      case 28:
	// input to direct.
	machine_poke(pos+machine_p88k(pos+2),randi()&255);
	pos+=3;
	break;
      case 29:
	// to indirect.
	machine_poke(machine_peek(machine_peek(pos+2)),randi()&255);
	pos+=3;
	break;
      }
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunmasque(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;
  u8 instr,temp,flag;

  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
      switch(instr%7){
      case 0:
	if (villager->m_reg8bit2==12){
	  machine_poke(pos+1,machine_p88k(pos));
	  if (machine_p88k(pos)==255) villager->m_reg8bit2=13;
	  pos+=villager->speed;
	}
	else pos+=villager->speed;
	break;
      case 1:
	if (villager->m_reg8bit2==13){
	  villager->m_reg16bit1++;
	  machine_poke(villager->m_reg16bit1,audio_buffer[pos&32767]); //READ IN
	  pos+=villager->speed;
	}
	else pos+=villager->speed;
	break;
      case 2:
	villager->m_reg8bit2++;
	if (villager->m_reg8bit2==60){
	  villager->m_reg8bit2++;
	  machine_poke(villager->m_reg16bit1,machine_p88k(villager->m_reg16bit1)^255);
	  villager->m_reg8bit2=0;
	}
	else pos+=villager->speed;
	break;
	case 3:
	  //	  seven rooms: 
	  temp= (pos)%7;
	  for (u8 x=11;x<22;x++){//11-21
	    //	    settingsarray[x]=settingsarray[x]>>temp;
	  } // CONSTRAINT
	  pos+=villager->speed;
	  break;
	case 4:
	  machine_poke(villager->m_reg16bit1-1,machine_p88k(villager->m_reg8bit1-1)^255);
	  machine_poke(villager->m_reg16bit1+1,machine_p88k(villager->m_reg8bit1+1)^255);
	  pos+=villager->speed;
	  break;
	case 5:
	  flag=randi()&3;
	  if (flag==0) 	  villager->m_reg8bit2++;
	  if (flag==1) 	  villager->m_reg8bit2--;
	  if (flag==2) 	  villager->m_reg8bit2+=16;
	  if (flag==3) 	  villager->m_reg8bit2-=16;
	  pos+=villager->speed;
	  break;
	case 6:
	  machine_poke(villager->m_reg16bit1+2,adc_buffer[((villager->m_reg16bit1)>>8)%10]);
	  pos+=villager->speed;
	  break;
      }
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunbf(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  u8 instr;
  //  pos+=villager->speed;
  for (u8 xx=0;xx<villager->howmany;xx++){
    instr=datagenbuffer[pos];
      switch(instr%7)
	{
	case 0:
	  villager->m_reg16bit1+=1;
	  pos+=villager->speed;
	  break;
	case 1:
	  villager->m_reg16bit1-=1;
	  pos+=villager->speed;
	  break;
	case 2:
	  //	    cells[omem]=cells[omem]++; 
	  machine_poke(villager->m_reg16bit1,machine_peek(villager->m_reg16bit1)+1);
	  pos+=villager->speed;
	  break;
	case 3:
	  machine_poke(villager->m_reg16bit1,machine_peek(villager->m_reg16bit1)-1);
	  pos+=villager->speed;
	  break;
	case 4:
	  villager->m_reg8bit2+=2;
	  if (villager->m_reg8bit2>=14) villager->m_reg8bit2=0;
	  villager->m_stack[villager->m_reg8bit2&15]= pos>>8;
	  villager->m_stack[(villager->m_reg8bit2+1)&15]= pos&255;
	  pos+=villager->speed;
	  break;
	case 5:
	  if (villager->m_reg8bit2>=14) villager->m_reg8bit2=0;
	  if (machine_p88k(villager->m_reg16bit1)!=0) pos=((villager->m_stack[villager->m_reg8bit2&15])<<8)+((villager->m_stack[(villager->m_reg8bit2+1)&15]));
	  villager->m_reg8bit2-=2;
	  if (villager->m_reg8bit2==0) villager->m_reg8bit2=14;
	  break;
	case 6:
	  //  cells[omem] = adcread(3); 
	  machine_poke(villager->m_reg16bit1,adc_buffer[villager->m_reg16bit1%10]);
	  pos+=villager->speed;
	  break;
	}
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunbiota(villager_generic *villager){ // proto->filled
  u16 pos=villager->position;

  //  pos+=villager->speed;

  u16 biotadir[8]={65279,65280,1,257,256,255,65534,65278};
  u8 instr,flag,temp;

  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];
      flag=0;
      //      printf("instr %d ",instr);
      switch(instr&7)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  villager->m_reg16bit1+=biotadir[villager->m_reg8bit1&7];
	  if (machine_p88k(villager->m_reg16bit1)==0) flag=1;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+4)&7];
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
	  temp=villager->m_reg8bit1&7;
	  villager->m_reg16bit1+=biotadir[temp];
	  if (machine_p88k(villager->m_reg16bit1)==0) {
	    villager->m_reg16bit1-=biotadir[temp]; // go back
	    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+1)&7]; // right 45
	    if (machine_p88k(villager->m_reg16bit1)==0) {
	      villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1+1)&7]; // go back
	      villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1-1)&7]; // left 45
	      if (machine_p88k(villager->m_reg16bit1)==0) {
		villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-1)&7]; // go back
		villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+2)&7]; // right 90
		if (machine_p88k(villager->m_reg16bit1)==0) {
		  villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1+2)&7]; // go back
		  villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1-2)&7]; // left 90
		  if (machine_p88k(villager->m_reg16bit1)==0) {
		    villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-2)&7]; // go back
		    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+3)&7]; // right 135
		    if (machine_p88k(villager->m_reg16bit1)==0) {
		    villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-3)&7]; // go back
		    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1-3)&7]; // left 135
		    if (machine_p88k(villager->m_reg16bit1)==0) {
		    villager->m_reg16bit1-=biotadir[(villager->m_reg8bit1-3)&7]; // go back
		    villager->m_reg16bit1+=biotadir[(villager->m_reg8bit1+4)&7]; // back
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

	  if (machine_p88k(villager->m_reg16bit1)==0 || (machine_p88k(villager->m_reg16bit1+(villager->m_reg16bit1+((villager->m_reg8bit1-4)&7))))!=0) flag=1;
	  else machine_poke(villager->m_reg16bit1+(villager->m_reg16bit1+((villager->m_reg8bit1-4)&7)),machine_p88k(villager->m_reg16bit1));
	  break;
	case 7:
	  //    * . -- no-op, a non-empty do nothing. 
	  break;
	}

      // - pos turns (where?) when it finds an empty location or a failing instruction
      // m_reg8bit3 is direction
      wormdir=villager->m_reg8bit2&7;
      if (machine_p88k(pos)==0 || flag==1){
	villager->m_reg8bit2-=1;
      }
      else pos+=biotadir[wormdir];
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}

void xxrunleakystack(villager_generic *villager){
  u16 pos=villager->position;

  u8 instr;
  for (u8 xx=0;xx<villager->howmany;xx++){
instr=datagenbuffer[pos];

//  pos+=villager->speed;
      switch(instr%25)
	{
	case NOP: break;
	case ORG: villager->start=pos-1; pos=villager->start+1; break;
	case EQU: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)==thread_pop(villager)); break;
	case JMP: pos=machine_peek(pos++); break;
	case JMPZ: if (thread_stack_count(villager,1) && thread_pop(villager)==0) pos=machine_peek(pos); else pos+=villager->speed; break;
    case PSHL: thread_push(villager,machine_peek(pos++)); break;
	case PSH: thread_push(villager,machine_p88k(machine_peek(pos++))); break;
	case PSHI: thread_push(villager,machine_p88k(machine_peek(machine_peek(pos++)))); break;
	case POP: if (thread_stack_count(villager,1)) machine_poke(machine_peek(pos++),thread_pop(villager)); break;
	case POPI: if (thread_stack_count(villager,1)) machine_poke(machine_peek(machine_peek(pos++)),thread_pop(villager)); break;

	case ADD: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)+thread_pop(villager));pos++; break;
    case SUB: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)-thread_pop(villager));pos++; break;
    case INC: if (thread_stack_count(villager,1)) thread_push(villager,thread_pop(villager)+1);pos++; break;
    case DEC: if (thread_stack_count(villager,1)) thread_push(villager,thread_pop(villager)-1);pos++; break;
    case AND: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)&thread_pop(villager));pos++; break;
    case OR: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)|thread_pop(villager));pos++; break;
    case XOR: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)^thread_pop(villager));pos++; break;
    case NOT: if (thread_stack_count(villager,1)) thread_push(villager,~thread_pop(villager));pos++; break;
    case ROR: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)>>(machine_p88k(pos++)&7)); break;
    case ROL: if (thread_stack_count(villager,2)) thread_push(villager,thread_pop(villager)<<(machine_p88k(pos++)&7)); break;

    case PIP: 
    {
        u16 d=machine_peek(pos++); 
        machine_poke(d,machine_peek(d)+1); 
    } break;
    case PDP: 
    {
        u16 d=machine_peek(pos++); 
        machine_poke(d,machine_peek(d)-1); 
    } break;
    case DUP: if (thread_stack_count(villager,1)) thread_push(villager,thread_top(villager)); pos++; break;
    case SAY: 
      pos++;
      //      printf("%c",thread_pop(villager));
      //      machine_poke(machine_peek(pos++),randi()%255);      
        break;
	case INP:
	  machine_poke(machine_peek(pos++),adc_buffer[thread_pop(villager)%10]);      
	  pos+=villager->speed;
	  break;

	}
      if (pos>(villager->start+villager->wrap)) pos=villager->start;
  }
villager->position=pos;
}


