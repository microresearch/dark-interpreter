#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#endif

//#include "simulation.h"

typedef unsigned char u8;
typedef uint16_t u16;


#define STACK_SIZE 16

struct stackey{
  u16 (*functione) (uint16_t count, u8 delay, u8 *workingbuffer, uint8_t howmuch, void * unity);
  u8 delay,howmuch;
  void* unit;
  };

struct hodge{
  u8 q,k1,k2,g,celllen;
  u16 x;
  u8 del;
};

struct CA{
  u8 celllen,rule;
  u8 del;
};

struct fire{
  u8 probB,probI,celllen;
  u8 del;
};

struct SIR{
  u8 probI,probD,celllen;
  u8 del;
};

struct SIR16{
  u8 del;
  u8 probM,probR,probC,probV;
};

signed char ca_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u8* buffer, u8 stack_posy,u8 delay, u8 howmuch);
void ca_runall(struct stackey stack[STACK_SIZE], u8* buffer, u8 stack_posy);
signed char ca_pop(struct stackey stack[STACK_SIZE], u8 stack_posy);
