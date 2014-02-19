#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#endif

typedef unsigned char u8;
typedef uint16_t u16;

struct stackey{
  u16 (*functione) (uint16_t count, uint16_t delay, u8 *workingbuffer, uint8_t howmuch, void * unity);
  void (*inite) (void *unity, u8 *workingbuffer);
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
