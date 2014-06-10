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
#define KRUMMY 9
#define NUM_CA 10
