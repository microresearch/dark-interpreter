/// testing

//gcc test.c -otest -lm -std=gnu99 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#define randi() rand()

typedef int u16;
typedef unsigned char u8;

    typedef struct {
      u16 length;
      u16 dataoffset;
      u16 knoboffset;
      int16_t samplepos;
      int16_t dirry;
      u8 speed, step;
      u8 dir;
    } villager_hardwarehaha;

signed char direction[2]={-1,1};

double
blackman (int i, int nn)
{
  return ( 0.42 - 0.5 * cos (2.0*M_PI*(double)i/(double)(nn-1))
	  + 0.08 * cos (4.0*M_PI*(double)i/(double)(nn-1)) );
}

double hanning (int i, int nn)
{
  return ( 0.5 * (1.0 - cos (2.0*M_PI*(double)i/(double)(nn-1))) );
}

float intun_to_float(unsigned int inbuffer){
    return ((float)(inbuffer)/32768.0f)-1.0f;
}


void main(void)
{

  int i; float xx,xa,xb,xc; int xxx;

  xx=intun_to_float(65536);
  printf ("intun %f\n",xx);

  /*

  float Fc=2000.0f,Fs=48000.0f, Q=0.9f,peakGain=20;
  int aa0,aa1,aa2,bb1,bb2;
    float a0,a1,a2,b1,b2,norm;
    
    //    float V = pow(10, abs(peakGain) / 20);
    float K = tan(M_PI * Fc / Fs);
        
    norm = 1 / (1 + K / Q + K * K);
    a0 = K / Q * norm;
    a1 = 0;
    a2 = -a0;
    b1 = 2 * (K * K - 1) * norm;
    b2 = (1 - K / Q + K * K) * norm;

//    printf("%d,0,%d,%d,%d,%d\n\n",aa0,aa1,aa2,bb1,bb2);
//    printf("%f,%f,%f,%f,%f,",a0,a1,a2,-b1,-b2);

*/

  //  float bw[5]={0.1, 0.067307692307692, 0.048888888888889, 0.048979591836735, 0.047272727272727};
  float bw[5]={0.11428571428571, 0.13333333333333, 0.041666666666667, 0.044859813084112, 0.040677966101695};
  /*  for (i=0;i<5;i++){
  xx=sqrtf(powf(2,bw[i]));
  xa=xx/(powf(2,bw[i])-1);

  printf ("%f,",xa);
  }*/




  /*  for (i = 0; i < 32; i ++)
    {
      xx=hanning(i,32);
      xxx=xx*32768.0f;
      printf("%d,",xxx);
      }*/

  /*  int x, which40106villager=0,howmany40106vill=16,count40106=0;
  int buf16[32768];
  villager_hardwarehaha village_40106[17];

  for (int i=0;i<32768;i++){
    buf16[i]=rand()%65536;

  }

  // init village_40106[x].
  for (u8 whichvillager=0;whichvillager<16;whichvillager++){

    village_40106[whichvillager].length=rand()%4096; 
    village_40106[whichvillager].dataoffset=rand()%32768;

    village_40106[whichvillager].knoboffset=rand()%65536; // test knoboffset
    village_40106[whichvillager].knoboffset=32768; // test knoboffset

    village_40106[whichvillager].dir=rand()%2;
    village_40106[whichvillager].speed=(rand()%15)+1;
    village_40106[whichvillager].step=(rand()%15)+1; 

    //    if (village_40106[whichvillager].dir==2) village_40106[whichvillager].dirry=newdirection[wormdir];
    //    else if (village_40106[whichvillager].dir==3) village_40106[whichvillager].dirry=direction[adc_buffer[DOWN]&1]*village_40106[whichvillager].speed;
    village_40106[whichvillager].dirry=direction[village_40106[whichvillager].dir]*village_40106[whichvillager].speed;

    if (village_40106[whichvillager].dirry>0) village_40106[whichvillager].samplepos=0;
    else village_40106[whichvillager].samplepos=village_40106[whichvillager].length;

    }

  //    u8 tmpinlong=inwrap-vill_eff->inpos; // test that works for u8

  u8 tmpinlong,tmpmod=10; u16 inpos,inwrap; 

  inpos=32760; inwrap=32768;

    if ((inpos+32)<=inwrap) tmpinlong=32;
    else tmpinlong=inwrap-inpos; // test that works for u8

    if (tmpmod) printf("xxx");

    //    printf("tmpinlong= %d\n",tmpinlong);

    */
  
  /*   while(1){

  x=which40106villager%howmany40106vill;
  count40106+=village_40106[x].step;


  //	  set40106pwm(village_40106[x].knoboffset+(-buf16[(village_40106[x].dataoffset+village_40106[x].samplepos)%32768]);
  int tmp=(village_40106[x].knoboffset>>5)+1;
	  //	  set40106pwm(tmp+(buf16[(village_40106[x].dataoffset+village_40106[x].samplepos)%32768])%(2048-tmp));
	  printf("whichone is %d 40106val as %d\n",x,tmp+(buf16[(village_40106[x].dataoffset+village_40106[x].samplepos)%32768])%(2048-tmp));


  village_40106[x].samplepos+=village_40106[x].dirry;
  if (village_40106[x].samplepos>=village_40106[x].length) village_40106[x].samplepos=0;
  else if (village_40106[x].samplepos<0) village_40106[x].samplepos=village_40106[x].length;

  if (count40106>=village_40106[x].length){
    count40106=0;
    which40106villager++; //u8
	  }
	  }*/
}
