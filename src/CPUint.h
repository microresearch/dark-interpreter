#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#else
#define u8 unsigned char
#define u16 uint16_t
#endif

#define DATA_BUFSZ 65535

#define MAX_FRED 60
    
#define STACK_SIZEE 16
#define MAX_THREADS 64

#define CPU_TOTAL 31

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
#define INP 24

    typedef struct {
      u8 *m_memory;
      u8 m_CPU;
      u8 m_del,m_delc;
      u8 m_infection;
      u16 m_reg16bit1; u8 m_reg8bit1, m_reg8bit2;
      u16 m_start;
      u16 m_pc;
      u16 m_wrap;
      signed char m_stack_pos;
      u8 m_stack[STACK_SIZEE];
    } thread;
    
    typedef struct {
      thread *m_threads;
      u8 *m_memory; u8 m_infectprob, m_mutateprob;
      signed char m_threadcount;
      u8 m_leakiness;
    } machine;
    
//    u8 thread_peek(thread *t, machine *m, u8 addr);
//    void thread_poke(thread *t, machine *m, u8 addr, u8 data);
    void thread_set_start(thread *t, u16 s);
    void thread_set_pc(thread *t, u16 s);
    u8 thread_get_pc(thread *t);
    u8 thread_get_start(thread *t);
    void thread_run(thread *t, machine *m);
    u8* thread_get_stack(thread *t);
    u8 thread_get_stack_pos(thread *t);
    u8 thread_stack_count(thread* this, u8 c);
    u8 thread_is_active(thread *t);
    void thread_set_active(thread *t, u8 s);
    void thread_push(thread *t, u8 data);
    u8 thread_pop(thread *t);
    u8 thread_top(thread *t);
    
/////////////////////////////////////////////////////////
    
void machine_create(machine *this, u8 *buffer);
u16 machine_peek(machine *t, u16 addr); // changed for CPUintrev2.h
u8 machine_p88k(machine* this, uint16_t addr); // ditto added
void machine_poke(machine *t, u16 addr, u8 data);	
void machine_run(machine *t);
void infectcpu(machine *m, u8 probI, u8 probD, u8 infected);    
void mutate(machine *m, u8 which, u8 identifier);

void cpustackpush(machine *this, u8 *buffer,u16 address, u16 wrapaddress,u8 cputype, u8 delay);
void cpustackpop(machine *this);

void cpustackpushhh(u8 *buffer,u16 addr,u16 wrapaddr,u8 cpuuu, u8 delayyy);
void machine_runnn(u8* buffer);
void cpustackpoppp(u8 *buffer);

const char *byte_to_binary(int x);
