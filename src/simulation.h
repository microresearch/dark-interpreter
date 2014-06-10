#ifndef PCSIM
#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#include "stdint.h"
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
#define STACK_SIZE 8
#define NUM_FUNCS 33

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

struct siney{
  u16 cc;
  u16* buffer;
};

struct CONV{
float c0, c1, c2;
  u16* buffer;
};

struct FORM{
  u8 freq[3];
  u8 w[3];

  u16* buffer;
};

struct generik{
  u16 cop;

  u16* buffer;
};

struct chunkey{
  u16 otherstart,otherwrap,othercount;
  u8 dirr;
  signed char newdir[2];
  u16* buffer;
};


struct simpleSIR{
  float32_t beta;//=520.0/365.0;
  float32_t gamm;//=1.0/7.0;
  float32_t S0;//=1.0-1e-6;
  float32_t I0;//=1e-6;
  float32_t step;
  float32_t S,I,R;
  float32_t dPop[3];
  u16* buffer;
};

struct SEIR {
  float32_t beta;
  float32_t step;
  float32_t gamm;
  int n;
  int m;
  float32_t mu;
  float32_t S0,I0;
  float32_t S,I[MAX_GROUPS]; // 4x16=64bytes
  float32_t dPop[MAX_GROUPS+1];//4x9=36bytes
  u16* buffer;
};

struct SICR {
  float32_t beta;
  float32_t epsilon;
  float32_t gamm;
  float32_t Gamm; 
  float32_t mu;
  float32_t q;
  float32_t S0;
  float32_t I0;
  float32_t C0;
  float32_t t,S,I,C,R;
  float32_t dPop[3];
  float32_t step;

  u16* buffer;
};

typedef struct{ float32_t x, y; } Point;

struct IFS {
  float32_t prob[5];
  float32_t coeff[4][6];
  Point p1,p2;

  u16* buffer;
};

struct Rossler{
  float32_t h,a,b,c,lx0,ly0,lz0;

  u16* buffer;
};

struct secondRossler{
  float32_t z0, zn, znm1;
  float32_t a,b,c,h;
  float32_t x0, y0, xn, yn, xnm1, ynm1;

  u16* buffer;
};

struct Brussel{
  float32_t x,y; 
  float32_t delta,mu,gamma;

  u16* buffer;
};

struct Spruce{
  float32_t x, y; 
  float32_t k1,k2,alpha,beta,mu,rho,delta;
  u16* buffer;
};

struct Oregon
{
  float32_t x, y, z; 
  float32_t delta,epsilon,mu,q;
  u16* buffer;
};

struct Fitz
{
  float32_t u,w,b0,b1;
  u16* buffer;
};

/*struct stackey{
  u16 (*functione) (u8 howmuch, void * unity, u16 count, u16 start, u16 wrap);  
  u8 howmuch;
  u16 count;
  void* unit;
  };
*/


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

signed char func_pushn(struct stackey stack[STACK_SIZE], u8 typerr, u16* buffer, u8 stack_pos, u8 howmuch, u16 start, u16 wrap);
void func_runall(struct stackey stack[STACK_SIZE], u8 stack_pos);
signed char func_pop(u8 stack_pos);


void ca_runall(struct stackeyyy stack[STACK_SIZE], u8 stack_posy);
signed char ca_pushn(struct stackeyyy stack[STACK_SIZE], u8 typerr, u8* buffer, u8 stack_posy, u8 howmuch, u16 start, u16 wrap);
signed char ca_pop(u8 stack_posy);
