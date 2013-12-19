//gcc -DLINUX -std=gnu99 leaky.c -o leaky

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "leaky.h"

/* to port for D.I:

- for each thread heap points into buffer - DONE
- stack of diff cpus/threads which can be added/subbed to by knob - TODO(each thread has m_CPU)
- re-init (per thread?)
*/

//int HEAP_SIZE=20400; // see above (80*255)

void thread_create(thread *this, int start, uint8_t which) {
    this->m_CPU=which;
    this->m_start=start;
    this->m_pc=0;
    this->m_stack_pos=-1;
    this->m_stack=(u8*)malloc(sizeof(u8)*STACK_SIZE);

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

void thread_set_start(thread* this, u8 s) { 
    this->m_start=s; 
}

void thread_set_pc(thread* this, u8 s) { 
    this->m_pc=s; 
}

u8 thread_get_pc(thread* this) { 
    return this->m_pc+this->m_start; 
}

u8 thread_get_start(thread* this) { 
    return this->m_start; 
}

void thread_run(thread* this, machine *m) {
  u8 instr;	
  u8 flag=0;
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
      printf("%c",thread_pop(this));
      //      thread_poke(this,m,thread_peek(this,m,this->m_pc++),rand()%255);      
        break;
	case INP:
	  thread_poke(this,m,thread_peek(this,m,this->m_pc++),rand()%255);      
	  break;

    default : break;
	}
      break;
    case 1:// BIOTA see: http://c2.com/cgi/wiki?BiotaLanguage
      /* biota:
	 - needs PC and DC, and each has direction/45 degrees
	 - program counter turns when it finds an empty location or a failing instruction

	 inst:

Other instructons write to the memory at or near the data counter

    * c -- clear character at DC. Fails if already empty.
    * d -- duplicate the current data into the cell left of the DC. Fails if source cell is empty or target cell is non-empty.

One instruction exists only to connect instructions into loops that the program counter follows

    * . -- no-op, a non-empty do nothing. 
       */
      instr=thread_peek(this,m,this->m_pc);
      switch(instr%25)
	{
	case 0:
	  //s -- straight: move DC in the current direction. Fail if that cell is empty.
	  if (this->m_reg8bit1==0) this->m_reg8bit2+=1;
	  else if (this->m_reg8bit1==1) this->m_reg8bit2-=1;
	  else if (this->m_reg8bit1==2) this->m_reg8bit2+=16;
	  else if (this->m_reg8bit1==3) this->m_reg8bit2-=16;
	  else if (this->m_reg8bit1==4) this->m_reg8bit2-=15;
	  else if (this->m_reg8bit1==5) this->m_reg8bit2+=17;
	  else if (this->m_reg8bit1==6) this->m_reg8bit2+=15;
	  else if (this->m_reg8bit1==7) this->m_reg8bit2-=17;
	  if (thread_peek(this,m,this->m_reg8bit2)==0) flag=1;
	  break;
	case 1:
	  //* b -- backup: move DC opposite the current direction. Fail if that cell is empty.
	  if (this->m_reg8bit1==0) this->m_reg8bit1-=1;
	  else if (this->m_reg8bit1==1) this->m_reg8bit2+=1;
	  else if (this->m_reg8bit1==2) this->m_reg8bit2-=16;
	  else if (this->m_reg8bit1==3) this->m_reg8bit2+=16;
	  else if (this->m_reg8bit1==4) this->m_reg8bit2+=15;
	  else if (this->m_reg8bit1==5) this->m_reg8bit2-=17;
	  else if (this->m_reg8bit1==6) this->m_reg8bit2-=15;
	  else if (this->m_reg8bit1==7) this->m_reg8bit2+=17;
	  if (thread_peek(this,m,this->m_reg8bit2)==0) flag=1;
	  break;
	case 2:
	  //* t -- turn DC right 45 degrees
	  this->m_reg8bit1+=1;
	  this->m_reg8bit1=this->m_reg8bit1%8;
	  break;
	case 3:
	  //* u -- unturn: turn DC left 45 degrees
	  this->m_reg8bit1-=1;
	  this->m_reg8bit1=this->m_reg8bit1%8;
	  break;
	case 4:
	  //    * g -- go to a non-empty character ahead (tries to move DC straight ahead, then right and left 45 degrees, then 90, then 135, then back).
	  /*QUESTIONMARK:	  if (this->m_reg8bit1==0) {
	    this->m_reg8bit2+=1;
	    if (thread_peek(this,m,this->m_reg8bit2)==0) {
	      // try right 45 deg
	      this->m_reg8bit2+=16;
	      if (thread_peek(this,m,this->m_reg8bit2)==0) {
		// left 45
		this->m_reg8bit2-=31;
		if (thread_peek(this,m,this->m_reg8bit2)==0) {
		  // 90
		  this->m_reg8bit2+=47;
		  if (thread_peek(this,m,this->m_reg8bit2)==0) {
		    this->m_reg8bit2+=47;
		    
		    if (thread_peek(this,m,this->m_reg8bit2)==0) {
				  
	  

	  else if (this->m_reg8bit1==1) this->m_reg8bit2-=1;
	  else if (this->m_reg8bit1==2) this->m_reg8bit2+=16;
	  else if (this->m_reg8bit1==3) this->m_reg8bit2-=16;
	  else if (this->m_reg8bit1==4) this->m_reg8bit2-=15;
	  else if (this->m_reg8bit1==5) this->m_reg8bit2+=17;
	  else if (this->m_reg8bit1==6) this->m_reg8bit2+=15;
	  else if (this->m_reg8bit1==7) this->m_reg8bit2-=17;
	  if (thread_peek(this,m,this->m_reg8bit2)==0) flag=1;*/
	  break;

	}
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
    this->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS);

	for (unsigned char n=0; n<MAX_THREADS; n++)
	{
	  thread_create(&this->m_threads[n], count, 0);// last is CPU
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

const char *byte_to_binary(int x) {
    static char b[9];
    b[0] = '\0';
    int z;
    for (z = 128; z > 0; z >>= 1) {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    return b;
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
  x=rand()%MAX_THREADS;
  y=rand()%MAX_THREADS;
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
