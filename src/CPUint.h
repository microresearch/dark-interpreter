#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#endif

typedef unsigned char u8;
typedef uint16_t u16;
    
static const u8 STACK_SIZE=16;
static const u8 MAX_THREADS=120;

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
      u8 m_CPU;
      u16 m_reg16bit1; u16 m_reg16bit2;
      u8 m_reg8bit1; u8 m_reg8bit2; u8 m_reg8bit3; u8 m_reg8bit4;
      uint16_t m_start;
      u16 m_pc;
      u16 m_wrap;
      u8 m_stack_pos;
      u8 m_stack[16];
    } thread;
    
    typedef struct {
      thread *m_threads;
      u8 *m_memory; 
      u8 m_threadcount;
      u8 m_leakiness;
    } machine;
    
//    u8 thread_peek(thread *t, machine *m, u8 addr);
//    void thread_poke(thread *t, machine *m, u8 addr, u8 data);
    void thread_set_start(thread *t, u16 s);
    void thread_set_pc(thread *t, u16 s);
    u8 thread_get_pc(thread *t);
    u8 thread_get_start(thread *t);
    void thread_run(thread *t, machine *m);
    const u8* thread_get_stack(thread *t);
    const int thread_get_stack_pos(thread *t);
    const u8 thread_stack_count(thread* this, u8 c);
    u8 thread_is_active(thread *t);
    void thread_set_active(thread *t, u8 s);
    void thread_push(thread *t, u8 data);
    u8 thread_pop(thread *t);
    u8 thread_top(thread *t);
    
/////////////////////////////////////////////////////////
    
void machine_create(machine *this, u8 leakiness, uint8_t *buffer);
u8 machine_peek(const machine *t, u16 addr);
void machine_poke(machine *t, u16 addr, u8 data);	
void machine_run(machine *t);
    
void write_mem(machine *m, int *a, u16 len);

const char *byte_to_binary(int x);
