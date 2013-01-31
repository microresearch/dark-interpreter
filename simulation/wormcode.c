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

#define NOP 0
#define ORG 1
#define EQU 2
#define JMP 3 
#define JMPZ 4
#define PSHL 5
#define PSH 6 
#define PSHI 7
#define POP 8
#define POPI 9
#define ADD 10
#define SUB 11
#define INC 12
#define DEC 13
#define AND 14
#define OR 15
#define XOR 16
#define NOT 17
#define ROR 18
#define ROL 19
#define PIP 20
#define PDP 21
#define DUP 22
#define SAY 23

#define STACK_SIZE 20
#define BLOCK_SIZE 65536 // 128*128=16384 256*256=65536
//#define MAX_THREADS 80

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
const unsigned char thread_stack_count(thread* this, unsigned char c);
unsigned char machine_peek(const machine* this, unsigned int addr);
unsigned char thread_pop(thread* this);
void thread_push(thread* this, unsigned char data);



unsigned char thread_top(thread* this) {
	if (this->m_stack_pos>=0)
	{
		return this->m_stack[this->m_stack_pos];
	}
	return 0;
}


void machine_poke(machine* this, unsigned int addr, unsigned char data) {
	this->m_heap[addr%BLOCK_SIZE]=data;
}


unsigned char thread_peek(thread* this, machine *m, unsigned char addr) {
	return machine_peek(m,this->start+addr);
}

void thread_poke(thread* this, machine *m, unsigned char addr, unsigned char data) {
	machine_poke(m,this->start+addr,data);
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
  unsigned int x=(this->IP+this->dir)%BLOCK_SIZE;
  unsigned char c=machine_peek(m,x);
  this->IP+=c;
}

unsigned char tin(thread* this, machine *m){
  unsigned char c=getchar();
  machine_poke(m,this->IP,c);
}

// additionals->plus,minus,shiftl,shiftr,branch,infect,store,die

unsigned char tplus(thread* this, machine *m){
  unsigned int x=(this->IP);
  unsigned int c=(this->IP+this->dir)%BLOCK_SIZE;
  machine_poke(m,this->IP,x+c);
}

unsigned char tminus(thread* this, machine *m){
  unsigned int x=(this->IP);
  unsigned int c=(this->IP+this->dir)%BLOCK_SIZE;
  machine_poke(m,this->IP,x-c);
}

unsigned char tshiftr(thread* this, machine *m){
  unsigned int c=(this->IP);
  machine_poke(m,this->IP,c>>1);
}

unsigned char tshiftl(thread* this, machine *m){
  unsigned int c=(this->IP);
  machine_poke(m,this->IP,c<<1);
}

unsigned char tbranch(thread* this, machine *m){
  unsigned int x=(this->IP+this->dir)%BLOCK_SIZE;
  unsigned int c=(this->IP+(this->dir*2))%BLOCK_SIZE;
  if (c==0) this->IP+=x;
}

unsigned char tinfect(thread* this, machine *m){
  unsigned char c=machine_peek(m,this->IP);
  machine_poke(m,(this->IP-this->dir)%BLOCK_SIZE,c);
  machine_poke(m,(this->IP+=this->dir)%BLOCK_SIZE,c);
}

unsigned char tpush(thread* this, machine *m){
  unsigned char c=machine_peek(m,(this->IP+this->dir)%BLOCK_SIZE);
  thread_push(this,c);
}

unsigned char tpop(thread* this, machine *m){
  unsigned char c=thread_pop(this);
  machine_poke(m,(this->IP+=this->dir)%BLOCK_SIZE,c);
}

void thread_run(thread* this, machine *m) {
  static int x=0;

// additionals->plus,minus,shiftl,shiftr,branch,infect,store,die

  unsigned char (*instructionsettry[])(thread * this, machine *m) = {tnop, tout, tinc, tdec, tjump, tin, tplus,tminus,tshiftl,tshiftr, tbranch, tpush,tpop,tinfect}; // 13 - but/as infect has issues
  //    unsigned char c;
  // read stdin for direction
    unsigned char c=getchar();

    if (c>196) this->dir=1;
    else if (c>128) this->dir=-1;
    else if (c>64) this->dir=256;
    else this->dir=-256;

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

    (*instructionsettry[instr%13]) (this, m); 

    /* 	switch(instr%24)
	{
    case NOP: break;
    case ORG: this->start=this->start+this->IP-1; this->IP=1; break;
    case EQU: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)==thread_pop(this)); break;
    case JMP: this->IP=thread_peek(this,m,this->IP++); break;
    case JMPZ: if (thread_stack_count(this,1) && thread_pop(this)==0) this->IP=thread_peek(this,m,this->IP); else this->IP++; break;
    case PSHL: thread_push(this,thread_peek(this,m,this->IP++)); break;
    case PSH: thread_push(this,thread_peek(this,m,thread_peek(this,m,this->IP++))); break;
    case PSHI: thread_push(this,thread_peek(this,m,thread_peek(this,m,thread_peek(this,m,this->IP++)))); break;
    case POP: if (thread_stack_count(this,1)) thread_poke(this,m,thread_peek(this,m,this->IP++),thread_pop(this)); break;
    case POPI: if (thread_stack_count(this,1)) thread_poke(this,m,thread_peek(this,m,thread_peek(this,m,this->IP++)),thread_pop(this)); break;
    case ADD: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)+thread_pop(this)); break;
    case SUB: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)-thread_pop(this)); break;
    case INC: if (thread_stack_count(this,1)) thread_push(this,thread_pop(this)+1); break;
    case DEC: if (thread_stack_count(this,1)) thread_push(this,thread_pop(this)-1); break;
    case AND: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)&thread_pop(this)); break;
    case OR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)|thread_pop(this)); break;
    case XOR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)^thread_pop(this)); break;
    case NOT: if (thread_stack_count(this,1)) thread_push(this,~thread_pop(this)); break;
    case ROR: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)>>(thread_peek(this,m,this->IP++)%8)); break;
    case ROL: if (thread_stack_count(this,2)) thread_push(this,thread_pop(this)<<(thread_peek(this,m,this->IP++)%8)); break;
    case PIP: 
    {
        unsigned char d=thread_peek(this,m,this->IP++); 
        thread_poke(this,m,d,thread_peek(this,m,d)+1); 
    } break;
	case PDP: 
    {
        unsigned char d=thread_peek(this,m,this->IP++); 
        thread_poke(this,m,d,thread_peek(this,m,d)-1); 
    } break;
	case DUP: 
	  if (thread_stack_count(this,1)) thread_push(this,thread_top(this)); 
	  break;
	case SAY: 
	  	  c=machine_peek(m,this->IP);
	  //c=thread_pop(this);
	  printf("%c",c);
	  break;
    default : break;
	}
    */

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


void machine_create(machine *this, int thr) {
  int count=0;
    this->m_heap = (unsigned char*)malloc(sizeof(unsigned char)*BLOCK_SIZE);
    this->m_threads = (thread*)malloc(sizeof(thread)*thr);

	for (unsigned int n=0; n<thr; n++)
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

void machine_run(machine* this, int thr) {
	for (unsigned int n=0; n<thr; n++) {
		thread_run(&this->m_threads[n],this);
	}
}

void write_mem(machine *m, int *a, unsigned int len) {
    for (unsigned int i=0; i<len; i++) {
        machine_poke(m,i,a[i]);
    }
}

void main(int argc, char **argv)
{
  int x;
  int maxthreads=atoi(argv[1]);
  machine *m=(machine *)malloc(sizeof(machine));
  machine_create(m,maxthreads);
  srandom(time(0));

  for (x=0;x<BLOCK_SIZE;x++){
    machine_poke(m,x,getchar());
  }
  x=0;
  while(1) { 
    machine_run(m,maxthreads);
    //    x++;
    //    printf("%c",machine_peek(m,x));
    //    if (x>BLOCK_SIZE) x=0;
 }
}
