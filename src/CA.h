#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#endif

typedef unsigned char u8;
typedef uint16_t u16;

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
