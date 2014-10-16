#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#include <audio.h>
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

const char *byte_to_binary(int x);
