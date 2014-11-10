#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#include "stdint.h"
#include "arm_const_structs.h"
#endif

#ifdef PCSIM
typedef unsigned char u8;
typedef uint16_t u16;
#define float32_t float
#endif

#define true 1
#define false 0
#define MAX_SAM 32768
#define floor(x) ((int)(x))
#define ONESIXTH 0.1666666666666667
#define BET(A, B, C)  (((A>=B)&&(A<=C))?1:0)    /* a between [b,c] */
#define MAX_GROUPS 16 /// leave as 16
#define STACK_SIZE 64
#define NUM_FUNCS 34

#define CONVY 0
#define SINEY 1
#define INCY 2
#define DECY 3
#define LEFTY 4
#define RIGHTY 5
#define SWAPPY 6
#define NEXTINCY 7
#define NEXTDECY 8
#define NEXTMULTY 9
#define NEXTDIVY 10
#define COPYY 11
#define ZEROY 12
#define FULLY 13
#define RANDY 14
#define KNOBY 15
#define SWAPAUDIOY 16
#define ORAUDIOY 17
#define SIMPLESIRY 18
#define SEIRY 19
#define SICRY 20
#define IFSY 21
#define ROSSLERY 22
#define SECONDROSSLERY 23
#define BRUSSELY 24
#define SPRUCEY 25
#define OREGONY 26
#define FITZY 27
#define FORMY 28
#define CHUNKY 29
#define DEREFCHUNKY 30
#define WALKERCHUNKY 31
#define SWAPCHUNKY 32
#define NUNNY 33

typedef struct{ float32_t x, y; } Point;

struct stackey{
  //  u16 (*functione) (u8 howmuch, u16* workingbuffer, u16 count, u16 start, u16 wrap);  
  //  u8 howmuch;
  u16* buffer;
  u16 count;
  };

struct stackeyyy{
  //   u16 (*functione) (u8 howmuch, u8 * cells, u16 count, u16 start, u16 wrap);  
  u8* buffer;
  u16 count;
  };


void inittable(u8 r, u8 k, int rule);

signed char func_pushn(struct stackey stack[STACK_SIZE], u16 typerr, u16* buffer, u8 stack_pos, u16 howmuch, u16 start, u16 wrap);
void func_runall(struct stackey stack[STACK_SIZE], u8 stack_pos);
signed char func_pop(u8 stack_pos);


void ca_runall(struct stackeyyy stack[STACK_SIZE], u8 stack_posy);
signed char ca_pushn(struct stackeyyy stack[STACK_SIZE], u16 typerr, u8* buffer, u8 stack_posy, u16 howmuch, u16 start, u16 wrap);
signed char ca_pop(u8 stack_posy);

u16 runnoney(u8 step, u16 x);
u16 runkrum(u8 step, u16 x);
u16 runhodge(u8 step, u16 x,u16 start,u16 wrap);
u16 runhodgenet(u8 step, u16 x);
u16 runlife(u8 step, u16 x);
u16 runcel(u8 step, u16 x);
u16 runcel1d(u8 step, u16 x);
u16 runfire(u8 step, u16 x);
u16 runwire(u8 step, u16 x);
u16 runSIR(u8 step, u16 x);
u16 runSIR16(u8 step, u16 x);

u16 runform(u8 step, u16 count, u16 start, u16 wrap);
u16 runconv(u8 step, u16 count, u16 start, u16 wrap);
u16 runsine(u8 step, u16 count, u16 start, u16 wrap);
u16 runchunk(u8 step, u16 count, u16 start, u16 wrap);
u16 runderefchunk(u8 step, u16 count, u16 start, u16 wrap);
u16 runwalkerchunk(u8 step, u16 count, u16 start, u16 wrap);
u16 runswapchunk(u8 step, u16 count, u16 start, u16 wrap);
u16 runinc(u8 step, u16 count, u16 start, u16 wrap);
u16 rundec(u8 step, u16 count, u16 start, u16 wrap);
u16 runleft(u8 step, u16 count, u16 start, u16 wrap);
u16 runright(u8 step, u16 count, u16 start, u16 wrap);
u16 runswap(u8 step, u16 count, u16 start, u16 wrap);
u16 runnextinc(u8 step, u16 count, u16 start, u16 wrap);
u16 runnextdec(u8 step, u16 count, u16 start, u16 wrap);
u16 runnextmult(u8 step, u16 count, u16 start, u16 wrap);
u16 runnextdiv(u8 step, u16 count, u16 start, u16 wrap);
u16 runcopy(u8 step, u16 count, u16 start, u16 wrap);
u16 runzero(u8 step, u16 count, u16 start, u16 wrap);
u16 runfull(u8 step, u16 count, u16 start, u16 wrap);
u16 runrand(u8 step, u16 count, u16 start, u16 wrap);
u16 runknob(u8 step, u16 count, u16 start, u16 wrap);
u16 runswapaudio(u8 step, u16 count, u16 start, u16 wrap);
u16 runORaudio(u8 step, u16 count, u16 start, u16 wrap);
u16 runsimplesir(u8 step, u16 count, u16 start, u16 wrap);
u16 runseir(u8 step, u16 count, u16 start, u16 wrap);
u16 runsicr(u8 step, u16 count, u16 start, u16 wrap);
u16 runifs(u8 step, u16 count, u16 start, u16 wrap);
u16 runrossler(u8 step, u16 count, u16 start, u16 wrap);
u16 runsecondrossler(u8 step, u16 count, u16 start, u16 wrap);
u16 runbrussel(u8 step, u16 count, u16 start, u16 wrap);
u16 runspruce(u8 step, u16 count, u16 start, u16 wrap);
u16 runoregon(u8 step, u16 count, u16 start, u16 wrap);
u16 runfitz(u8 step, u16 count, u16 start, u16 wrap);
u16 runnoise(u8 step, u16 count, u16 start, u16 wrap);

void machine_runnn(u16 start,u16 end);


void seirinit(u16 *workingbuffer);
void sicrinit(u16 *workingbuffer);
void ifsinit(u16 *workingbuffer);
