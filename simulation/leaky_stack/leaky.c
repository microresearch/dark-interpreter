//gcc -DLINUX -std=gnu99 leaky.c -o leaky

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "leaky.h"
#include "program.h"

void thread_create(thread *this, int start) {
    this->m_active=1;
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

void thread_run(thread* this, machine *m, u32 clock) {
	if (!this->m_active) return;
	u8 instr=thread_peek(this,m,this->m_pc);
		printf("%c",instr);

			this->m_pc++;
	
 	switch(instr%24)
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
      //	printf("%c",thread_pop(this));
        break;
    default : break;
	};   
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

u8 thread_is_active(thread* this) { 
    return this->m_active; 
}

void thread_set_active(thread* this, u8 s) { 
    this->m_active=s; 
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

void machine_create(machine *this) {
  int count=0;
    this->m_heap = (u8*)malloc(sizeof(u8)*HEAP_SIZE);
    this->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS);
    this->m_clock=0;

	for (u32 n=0; n<MAX_THREADS; n++)
	{
	  thread_create(&this->m_threads[n], count);
	  count+=255;
    }

	for (u32 n=0; n<HEAP_SIZE; n++)
	{
		this->m_heap[n]=0;
	}

    // start 1 thread by default
    thread_set_active(&this->m_threads[0],1);
}

u8 machine_peek(const machine* this, u32 addr) {
	return this->m_heap[addr%HEAP_SIZE];
}

void machine_poke(machine* this, u32 addr, u8 data) {
	this->m_heap[addr%HEAP_SIZE]=data;
}

void machine_run(machine* this) {
	for (u32 n=0; n<MAX_THREADS; n++) {
		thread_run(&this->m_threads[n],this,this->m_clock);
	}
    this->m_clock++;
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

void write_mem(machine *m, int *a, u32 len) {
    for (u32 i=0; i<len; i++) {
        machine_poke(m,i,a[i]);
    }
}

void leak(machine *m){
  // leak bottom of stack x into top y
  int x=0, y=1;
  while (x!=y){ 
  x=rand()%MAX_THREADS;
  y=rand()%MAX_THREADS;
  }


}

int main(void)
{
  int x;
  int program[2040];
  machine *m=(machine *)malloc(sizeof(machine));
  machine_create(m);
  srandom(time(0));

  for (x=0;x<2040;x++){
    program[x]=rand()%255;
    //    printf("%c",program[x]);
  }

      write_mem(m,program,2040);

          thread *t=&m->m_threads[0];
      //    thread_set_active(t,1);
    
    u32 count=0;
    while(1) { 
      machine_run(m);

	}
}
