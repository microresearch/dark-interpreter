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
};

struct CA{
  u8 celllen,rule;
};

struct fire{
  u8 probB,probI,celllen;
};

struct SIR{
  u8 probI,probD,celllen;
};
