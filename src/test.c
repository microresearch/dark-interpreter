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
      int samplepos;
      int dirry;
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

double
hamming (int i, int nn)
{
  return ( 0.54 - 0.46 * cos (2.0*M_PI*(double)i/(double)(nn-1)) );
}

double hanning (int i, int nn)
{
  return ( 0.5 * (1.0 - cos (2.0*M_PI*(double)i/(double)(nn-1))) );
}

float intun_to_float(unsigned int inbuffer){
    return ((float)(inbuffer)/32768.0f)-1.0f;
}

#define MAX(a,b) (((a)>(b))?(a):(b))

void convolve1D(float* in, float* out, int dataSize, float* kernel, int kernelSize)
{
    int i, j, k;

    // start convolution from out[kernelSize-1] to out[dataSize-1] (last)
    for(i = kernelSize-1; i < dataSize; ++i)
    {
        out[i] = 0;                             // init to 0 before accumulate

        for(j = i, k = 0; k < kernelSize; --j, ++k){
            out[i] += in[j] * kernel[k];
	printf("i %d j %d\n",i,j);
    }
    }
    // convolution from out[0] to out[kernelSize-2]
    for(i = 0; i < kernelSize - 1; ++i)
    {
      //        out[i] = 0;                             // init to 0 before sum

      for(j = i, k = 0; j >= 0; --j, ++k)      {
      out[i] += in[j] * kernel[k];
      printf("i %d j %d\n",i,j);
      }
    }
}

void doenvelopefollower(int* envbuffer, u8 envsize, int* inbuffer, u8 insize, int* outbuffer){ // stick to 32 samples=48k/32 ms???
  // but env is dependent on that size
  static u8 xx=0;
  float envout; static int env=0;
  /* for (int x=0;x<envsize;x++){
   if (abs(env)<envbuffer[x]) env=envbuffer[x];
   }*/
 for (int x=0;x<insize;x++){
   xx++;
   if (xx%envsize) env=0;
   if (abs(env)<envbuffer[x]) env=envbuffer[x];
   envout=(float)env/32768.0;
   outbuffer[x]=(float)inbuffer[x]*envout;
}
} 

void runnoney(int x){

  printf("rrr %d\n",x);

}

void main(void)
{

  //  int i; float xx,xa,xb,xc; int xxx;
  int i,xx,tmp,inpos,inwrap,tmpinlong;
  int inbuffer[255],modbuffer[255],outbuffer[255];
  float finbuffer[255],fmodbuffer[255],foutbuffer[255],tmpp;

  /*  while(1){

    inpos=rand()%32768;
    inwrap=rand()%32768;

    if (inpos>=inwrap) {
      inpos=0;
    }
    if ((inpos+32)<=inwrap) tmpinlong=32;
    else tmpinlong=inwrap-inpos; 
    if (tmpinlong>32)    printf("inpos %d inwarp %d tmpinblog %d\n",inpos, inwrap, tmpinlong);
    
    }*/




    for (xx=0;xx<32;xx++){
      inbuffer[xx]=rand()%32768;
      finbuffer[xx]=(float)(inbuffer[xx])/32768.0f;//REDO! why/how?
    }

//    x=modifier&15;
    for (xx=0;xx<16;xx++){
       modbuffer[xx]=rand()%32768;
      fmodbuffer[xx]=(float)(modbuffer[xx])/32768.0f;//REDO! why/how?
    }
    //void convolve1D(float* in, float* out, int dataSize, float* kernel, int kernelSize)
    //    convolve1D(finbuffer, foutbuffer, 0, fmodbuffer, 1);

    float freq = (float)(255)/2550.0f; // mod is now 8 bits
    float fb= 0.8f + 0.8f/(1.0f - freq),hp,bp;
    float buf0=0,buf1=0;

    //    xx=bandpassmod(xxx,0.9f,10.0f,1.0f); // q freq gain

    for (xx=0;xx<32;xx++){
      tmpp=(float)(rand()%32768)/32768.0f;//REDO! why/how?
      //      tmpp=dobandpass(tmpp, freq,fb); // from OWL code - statevariable
      hp=tmpp-buf0;
      bp = buf0 - buf1; 
      buf0 = buf0 + freq * (hp + fb * bp); 
      buf1 = buf1 + freq * (buf0 - buf1);
      
	// out here
      tmp = (int)(bp * 32768.0f);
      tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
      printf("out: %d\n",tmp);
      //      audio_buffer[(vill_eff->outstart+vill_eff->outpos)&32767]=(int16_t)tmp;
      //      vill_eff->outpos+=vill_eff->step;
      //      if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
      }


    /*    for (xx=0;xx<32;xx++){
    tmp = (int)(foutbuffer[xx] * 32768.0f);
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    printf("out: %d\n",tmp);
    //    audio_buffer[(outstart+outpos)&32767]=(int16_t)tmp;
    //    outpos+=step;
    //    if (outpos>outwrap) outpos=0;
    }
    */
    float f = 2.0*M_PI*10.0f/32000.0f;

    printf("f=%f",f);

    i=32;
    if (i&16) printf("hhh\n");

    //  for (i=0;i<32;i++){
    //    printf("out: %d\n",outbuffer[i]);
    //  }


  //  int tmp;
  //  //  u8 x,xx,tmpinlong,tmpmodlong,longest; // never longer than 32!
  //  u16 inpos=0,modpos=0,oldmodwrap,oldinwrap,modwrap=50,inwrap=100;
  //  u16 n1=32,n2=32;

  /*
    float k[5] = {3, 4, 5, 0, 0};
    float out[5];
    float in[2] = {2,1};

    convolve1D(in, out, 5, k, 2);

    printf("INPUT\n");
    for(i=0; i < 5; ++i)
    {
        printf("%5.2f, ", in[i]);
    }
    printf("\n\n");

    printf("KERNEL\n");
    for(i=0; i < 2; ++i)
    {
        printf("%5.2f, ", k[i]);
    }
    printf("\n\n");

    printf("OUTPUT\n");
    for(i=0; i < 5; ++i)
    {
        printf("%5.2f, ", out[i]);
    }
    printf("\n");
  */

    //    int inpos=0;
    //      while(1){
    //	inpos+=16;
    //	printf("inpos %d\n",inpos&32767);
    //      }
	/*
      //        xx++;

    if (inpos>=inwrap) {
    inwrap=(rand()%100);
      inpos=0;
    }

    if (modpos>=modwrap) {
    modwrap=(rand()%100);
      modpos=0;
    }

    if ((inpos+32)<=inwrap) tmpinlong=32;
    else tmpinlong=inwrap-inpos; 
    
    // same for mod...
    if ((modpos+32)<=modwrap) tmpmodlong=32;
    else tmpmodlong=modwrap-modpos;
    
    //    now copy with length as longest
    if (tmpinlong<=tmpmodlong) longest=tmpinlong;
    else longest=tmpmodlong;

    for (x=0;x<longest;x++){
      inpos++;
      //      modbuffer[xx]=audio_buffer[(modstart+modpos++)%32768];
    printf("shortest %d inlong %d modlong %d inpos %d modpos %d\n inwrap %d\n",longest,tmpinlong,tmpmodlong,inpos,modpos,inwrap);
    }

    //longest=32;tmpinlong=32;tmpmodlong=32;// TESTY!

    // and update vill_eff
    //    modpos+=tmpmodlong;
    //    inpos+=tmpinlong;

    }*/

  //  xx=intun_to_float(65536);
  //  printf ("intun %f\n",xx);

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
  //  float bw[5]={0.11428571428571, 0.13333333333333, 0.041666666666667, 0.044859813084112, 0.040677966101695};
  /*  for (i=0;i<5;i++){
  xx=sqrtf(powf(2,bw[i]));
  xa=xx/(powf(2,bw[i])-1);

  printf ("%f,",xa);
  }*/

    //  float freq;int mod=200;

  //  freq = 2.0*M_PI*((float)(mod/255.0f)); // mod is now 8 bits
  //  mod=freq*(float)mod;
  //  printf("freq*mod: %d\n",mod);

  u8 whichdatagenwalkervillager=0,step=10;
  //  u16 countdatagenwalker=0,knoboffset=100,samplepos=0,length=120,dataoffset=0,tmpp, start=0, wrap=400;
  int dirry=-1;

  //    while(1){
      /*  samplepos+=dirry;//)&32767;
  printf("samplepos %d\n",samplepos);
  if (samplepos>=start+wrap || samplepos<=start){
    //    running==0;
    dirry=-1;
    if (dirry>0) samplepos=start;
    else samplepos=start+wrap;
    }*/
  //    }


  /*  while(1){
    //  x=whichdatagenwalkervillager%howmanydatagenwalkervill;
  countdatagenwalker+=step;
  tmp=knoboffset; // as is =32768 for datagenwalker
  //  if (tmp==32768) tmp=32767;  // as knoboffset never gets so high!
  tmpp=tmp+(128)%(32768-tmp);
  //  tmp=buf16[(dataoffset+samplepos)%32768];

  samplepos+=dirry;
  if (samplepos>=length) samplepos=0;
  else if (samplepos<0) samplepos=length;

  if (countdatagenwalker>=length){
    countdatagenwalker=0;
    whichdatagenwalkervillager++; //u8
  }

  printf("which %d samplepos %d tmpp %d\n",whichdatagenwalkervillager,samplepos,tmpp);
  }*/


  /*    for (i = 0; i < 32; i ++)
    {
      xx=hamming(i,32);
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

  //    u8 tmpinlong=inwrap-inpos; // test that works for u8

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
