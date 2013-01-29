/*

- collection of threads. do they share memory space/overlap? all in same space

- each has IP, start and end memory address, stack allowance, stack pointer
- stack which can leak

- run the thread according to instruction set - can each thread have diff inst set?

- two dimensional block of instructions (where do we get these?). worm
  walks through (test with random walk)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STACK_SIZE 20
#define BLOCK_SIZE 16384 // was 16384 // 128*128
#define MAX_THREADS 20

typedef struct {
  int IP;
  unsigned int start;
  int dir;
  int m_stack_pos;
  unsigned char *m_stack;
    } thread;

typedef struct {
  thread *m_threads;
  unsigned char *m_heap;
} machine;

unsigned char machine_peek(const machine* this, unsigned int addr);

void machine_poke(machine* this, unsigned int addr, unsigned char data) {
	this->m_heap[addr%BLOCK_SIZE]=data;
}


void thread_create(thread *this, int nextone) {
    this->start=rand()%BLOCK_SIZE;
    this->IP=rand()%BLOCK_SIZE;
    this->m_stack_pos=-1;
    this->dir=1;
    this->m_stack=(unsigned char*)malloc(sizeof(unsigned char)*STACK_SIZE);
    
    for (int n=0; n<STACK_SIZE; n++)
      {
	this->m_stack[n]=0;
      }
}

unsigned char tnop(thread* this, machine *m){
}

unsigned char tout(thread* this, machine *m){
  unsigned char c=machine_peek(m,this->IP);
    printf("%c",c);
}

unsigned char tinc(thread* this, machine *m){
  unsigned char c=machine_peek(m,this->IP);
  machine_poke(m,this->IP,c+1);
}

unsigned char tdec(thread* this, machine *m){
  unsigned char c=machine_peek(m,this->IP);
  machine_poke(m,this->IP,c-1);
}

unsigned char tjump(thread* this, machine *m){
  unsigned int x=(this->IP+1)%BLOCK_SIZE;
  unsigned char c=machine_peek(m,x);
  this->IP+=c;
}

unsigned char tin(thread* this, machine *m){
  unsigned char c=getchar();
  machine_poke(m,this->IP,c);
}

// additionals->plus,minus,shiftl,shiftr,branch,infect,store,die

unsigned char tplus(thread* this, machine *m){
  unsigned int x=(this->IP+1)%BLOCK_SIZE;
  unsigned int c=(this->IP+2)%BLOCK_SIZE;
  machine_poke(m,this->IP,x+c);
}

unsigned char tminus(thread* this, machine *m){
  unsigned int x=(this->IP+1)%BLOCK_SIZE;
  unsigned int c=(this->IP+2)%BLOCK_SIZE;
  machine_poke(m,this->IP,x-c);
}

unsigned char tshiftr(thread* this, machine *m){
  unsigned int c=(this->IP+1)%BLOCK_SIZE;
  machine_poke(m,this->IP,c>>1);
}

unsigned char tshiftl(thread* this, machine *m){
  unsigned int c=(this->IP+1)%BLOCK_SIZE;
  machine_poke(m,this->IP,c<<1);
}

unsigned char tbranch(thread* this, machine *m){
  unsigned int x=(this->IP+1)%BLOCK_SIZE;
  unsigned int c=(this->IP+2)%BLOCK_SIZE;
  if (c==0) this->IP+=c;
}

unsigned char tinfect(thread* this, machine *m){
  unsigned char c=machine_peek(m,this->IP);
  machine_poke(m,(this->IP+1)%BLOCK_SIZE,c);
  machine_poke(m,(this->IP-1)%BLOCK_SIZE,c);
}

void thread_run(thread* this, machine *m) {
  static int x=0;

// additionals->plus,minus,shiftl,shiftr,branch,infect,store,die

  unsigned char (*instructionsettry[])(thread * this, machine *m) = {tnop, tout, tinc, tdec, tjump, tin, tplus,tminus,tshiftl,tshiftr, tbranch}; // 10  
  //    unsigned char c;
  // read stdin for direction
    unsigned char c=getchar();

    if (c>196) this->dir=1;
    else if (c>128) this->dir=-1;
    else if (c>64) this->dir=128;
    else this->dir=-128;

    //if (x==1024){
    /*     switch(c%4){ // do more on larger changes in c */
  /* case 0: */
  /*   this->dir=1; */
  /*   break; */
  /* case 1: */
  /*   this->dir=-1; */
  /*   break; */
  /* case 2: */
  /*   this->dir=128; */
  /*   break; */
  /* case 3: */
  /*   this->dir=-128; */
  /*   //  } */
  /*   //  x=0; */
  /*     } */
    //  x++;
  this->IP+=this->dir;
  if (this->IP<0) this->IP=BLOCK_SIZE-this->IP;
  this->IP%=BLOCK_SIZE;
  unsigned char instr=machine_peek(m,this->IP);
  printf("%c",instr);

  // process instructions according to instruction set

  (*instructionsettry[instr%10]) (this, m); 

}

const unsigned char* thread_get_stack(thread* this) { 
    return this->m_stack; 
}

const unsigned char thread_stack_count(thread* this, unsigned char c) { 
    return (c-1)<=this->m_stack_pos; 
}

const int thread_get_stack_pos(thread* this) { 
    return this->m_stack_pos; 
}

void thread_push(thread* this, unsigned char data) {
	if (this->m_stack_pos<STACK_SIZE-1)
	{
		this->m_stack[++this->m_stack_pos]=data;
	}
}

unsigned char thread_pop(thread* this) {
 	if (this->m_stack_pos>=0)
	{
		unsigned char ret=this->m_stack[this->m_stack_pos];
		this->m_stack_pos--;
		return ret;
	}
	//    printf("errorr\n");
	return 0;   
}

unsigned char thread_top(thread* this) {
	if (this->m_stack_pos>=0)
	{
		return this->m_stack[this->m_stack_pos];
	}
	return 0;
}

void machine_create(machine *this) {
  int count=0;
    this->m_heap = (unsigned char*)malloc(sizeof(unsigned char)*BLOCK_SIZE);
    this->m_threads = (thread*)malloc(sizeof(thread)*MAX_THREADS);

	for (unsigned int n=0; n<MAX_THREADS; n++)
	{
	  thread_create(&this->m_threads[n], count);
	  count+=255;
    }

	for (unsigned int n=0; n<BLOCK_SIZE; n++)
	{
		this->m_heap[n]=0;
	}

    // start 1 thread by default
}

unsigned char machine_peek(const machine* this, unsigned int addr) {
	return this->m_heap[addr%BLOCK_SIZE];
}

void machine_run(machine* this) {
	for (unsigned int n=0; n<MAX_THREADS; n++) {
		thread_run(&this->m_threads[n],this);
	}
}

void write_mem(machine *m, int *a, unsigned int len) {
    for (unsigned int i=0; i<len; i++) {
        machine_poke(m,i,a[i]);
    }
}

int main(void)
{
  int x;

  machine *m=(machine *)malloc(sizeof(machine));
  machine_create(m);
  srandom(time(0));

  for (x=0;x<BLOCK_SIZE;x++){
    machine_poke(m,x,getchar());
  }
  x=0;
  while(1) { 
    machine_run(m);
    //    x++;
    //    printf("%c",machine_peek(m,x));
    //    if (x>BLOCK_SIZE) x=0;
 }
}
