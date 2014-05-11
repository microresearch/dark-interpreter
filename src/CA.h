#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#else
#define u8 unsigned char
#define u16 uint16_t
#endif

#define HODGEY 0
#define HODGENETY 1
#define LIFEY 2
#define CELY 3
#define CEL1DY 4
#define FIREY 5
#define WIREY 6
#define SIRY 7
#define SIR16Y 8
#define NUM_CA 9

struct hodge{
  u8 *cells;
  u8 q,k1,k2,g,celllen;
  u16 x;
  u8 del;
};

struct CA{
  u8 *cells;
  u8 celllen,rule;
  u8 del;
};

struct fire{
  u8 *cells;
  u8 probB,probI,celllen;
  u8 del;
};

struct SIR{
  u8 *cells;
  u8 probI,probD,celllen;
  u8 del;
};

struct SIR16{
  u8 *cells;
  u8 del;
  u8 probM,probR,probC,probV;
};

