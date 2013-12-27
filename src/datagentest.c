// gcc datagentest.c -odatagen -lm
// was... ./datagen CPU PLAGUE

/* testing of all datagens */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#define true 1
#define false 0
#define MAX_SAM 65536
#define MAXDIV2 512
#define CELLLEN 128
#define floor(x) ((int)(x))
#define HEX__(n) 0x##n##UL
#define recovered 255
#define susceptible 0


/* TODO:

simulations, SIR, hodge

- cpu/instruction sets should be threaded and some link to
  grains... so some kind of list of threads... port from leaky.c

push and pop CPUs onto the thread stack. also some variable/array for
exchange of grains/cpu start-end positions

also any changes for diff cpu/other datagens

- each datagen should return a value/values multiple values in
  workingbuffer with first terms as settings (keep first n values
  clear unless we flag otherwise)

- add stepsizing/speed for each

- how to re-init//trigger bit - also how to organize x bytes of
  workingbuffer reserved or... (offset question)

- does NaN cause problems or not?

///-->

divide into:

1- CPU/instruction sets from microbd (inc. corewars?) REDO AS THREADED/stacks

+leaky stack machines and wormcode: see simulation dir

SEE leaky.c

DONE!

/////

2- cellular automata: 1d,2d, classic, flexible - re-do all (again states 8 bits)

see CA.c - port here

see also: /root/collect2011/blackdeath/code/simulations

////

3- worms, langton's ants,turmites - NEW

langton.c -> langtonrev.c (now in CPUs but could also be as datagens - depends how we divide up)

http://en.wikipedia.org/wiki/Langton%27s_ant

in leaky.c

///

4- simulations: 

rosslerTOFINISH, orbital(bit fullon), ifs,

fitzhughnagumo - see fitz.c, oreganotor, brusselator - NEW DONE(to
test/tweak)

see also:

latestresearch/sc3-plugins-src-2012-05-26/source/SLUGens/SLUGens.cpp

http://doc.sccode.org/Classes/FitzHughNagumo.html

5- SIR simple, SIR using equations, more ambitious models...

see: http://homepages.warwick.ac.uk/~masfz/ModelingInfectiousDiseases/

+ hodge - hodge.c - how we map the 3 states (rewrite hodge.c)

*/

//2-CA

//

//4-SIMULATIONS - started to port from supercollider.. 

// rossler also in: /MCLDUGens/MCLDChaosUGens.cpp

//what do we do with NaN, what numbers should be fed in as inits from workingbuffer

//what happens if workingbuffer is 0? copying across of workingbuffer/buffers...

// ROSSLER

struct Rossler{
  double h,a,b,c,lx0,ly0,lz0;
};

void rosslerinit(struct Rossler* unit) {
  unit->h = 0.01;
  unit->a = 0.2;
  unit->b = 0.2;
  unit->c = 5.7;
  unit->lx0 = 0.1;
  unit->ly0 = 0;
  unit->lz0 = 0;
}

void runrossler(uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Rossler* unit){
  double lx0,ly0,lz0,lx1,ly1,lz1;
  double h,a,b,c;
  int x;

  h = unit->h;
  a = unit->a;
  b = unit->b;
  c = unit->c;
  lx0 = unit->lx0;
  ly0 = unit->ly0;
  lz0 = unit->lz0;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  unit->lx0 = lx1;
  unit->ly0 = ly1;
  unit->lz0 = lz1;
}

// BRUSSELATOR

struct Brussel{
    float x,y; 
};

void brusselinit(struct Brussel* unit) {
  unit->x = 0.5f; 
  unit->y = 0.5f; 
}

void runbrussel(uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Brussel* unit){
    
  float delta = (float)workingbuffer[0]/65536.0;
  float mu = (float)workingbuffer[1]/65536.0;
  float gamma = (float)workingbuffer[2]/65536.0;
  float x= unit->x; 
  float y= unit->y;  
    
    float dx, dy; 
    
    float muplusone = 1.0f+mu; 
    int i;

    for (i=0; i<howmuch; ++i) {
		
        float temp = x*x*y; 
        
        dx = temp - (muplusone*x) + gamma;
        dy =  (mu*x)  - temp; 
        
        x += delta*dx; 
        y += delta*dy; 
        
	//	output1[i]= x; 
	//        output2[i]= y; 
		
	}
	
    printf("brussels: x %f y %f\n",x,y); 
	
	unit->x = x; 
	unit->y = y;
}

// spruceworm

struct Spruce{
    float x, y; 
};

void spruceinit(struct Spruce* unit ) {
	
  unit->x = 0.9f; 
  unit->y = 0.1f; 
}

void runspruce(uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Spruce* unit){

  float k1 = (float)workingbuffer[0]/65536.0;
  float k2 = (float)workingbuffer[1]/65536.0;
  float alpha = (float)workingbuffer[2]/65536.0;
  float beta = (float)workingbuffer[3]/65536.0;
  float mu = (float)workingbuffer[4]/65536.0;
  float rho = (float)workingbuffer[5]/65536.0;
  float delta = (float)workingbuffer[6]/65536.0;

    
    float x= unit->x; 
    float y= unit->y;  
    
    float dx, dy; 
    int i;

	for (i=0; i<howmuch; ++i) {
		
        float temp = y*y; 
        float temp2 = beta*x;
        
        dx = (k1* x* (1.0-x)) - (mu*y);
        dy = (k2*y*(1.0- (y/(alpha*x))))  - (rho*(temp/(temp2*temp2 +  temp))); 
        
        
        x += delta*dx; 
        y += delta*dy; 
        
	//	output1[i]= x; 
	//        output2[i]= y; 
		
	}
	
	printf("spruce: x %f y %f z %f\n",x,y); 
	
	unit->x = x; 
	unit->y = y;
}


// OREGONATOR

struct Oregon
{
  float x, y, z; 
};

void oregoninit(struct Oregon* unit) {
    unit->x = 0.5f; 
    unit->y = 0.5f; 
    unit->z = 0.5f; 
}

void runoregon(uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Oregon* unit){
    
  float delta = (float)workingbuffer[0]/65536.0;
  float epsilon = (float)workingbuffer[1]/65536.0;
  float mu = (float)workingbuffer[2]/65536.0;
  float q = (float)workingbuffer[3]/65536.0;
    
  float x= unit->x; 
  float y= unit->y; 
  float z= unit->z; 
    
  float dx, dy, dz; 
        
  int i;
	for (i=0; i<howmuch; ++i) {
		
        dx = epsilon*((q*y) -(x*y) + (x*(1-x))); 
	dy = mu* (-(q*y) -(x*y) + z); 
        dz = x-y; 
        
        x += delta*dx; 
        y += delta*dy; 
        z += delta*dz; 
        
	//	output1[i]= x; 
	//        output2[i]= y; 
	//        output3[i]= z; 
		
	}
	
	printf("Oregonator: x %f y %f z %f\n",x,y,z); 
	
	unit->x = x; 
	unit->y = y;
	unit->z = z;
}

// FITZHUGH - writes into buffer 3xhowmuch, how to store local floats?

struct Fitz
{
	float u,w;
};

void fitzinit(struct Fitz* unit) {
	unit->u=0.0;
	unit->w=0.0;
}

void runfitz(uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Fitz* unit){

  /* SETTINGS */

  float urate= 0.7;
  float wrate= 1.7;
  float b0= 1.4;
  float b1= 1.1;
  float u,w;

  u=unit->u;
  w=unit->w;

  int x;

  for (x=0;x<howmuch;x++){

    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(b0+b1*u-w);
	  
    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
    if ((u>1.0) || (u<-1.0)) u=fabs(fmod((u-1.0),4.0)-2.0)-1.0;

    int z=((float)(u)*1500);
    //    int zz=((float)(w)*1500);
    workingbuffer[x]=z;//workingbuffer[x+2]=zz;
  }

  for (x=0;x<howmuch;x++){

    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(b0+b1*u-w);

    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
    if ((u>1.0) || (u<-1.0)) u=fabs(fmod((u-1.0),4.0)-2.0)-1.0;

    int z=((float)(u)*700);
    int zz=((float)(w)*700);
    //		workingbuffer[x+howmuch]=z;//deltay[x]=zz;
  }

  for (x=0;x<howmuch;x++){

    float dudt= urate*(u-(0.33333*u*u*u)-w);
    float dwdt= wrate*(b0+b1*u-w);

    u+=dudt;
    w+=dwdt;
    //assumes fmod works correctly for negative values
    if ((u>1.0) || (u<-1.0)) u=fabs(fmod((u-1.0),4.0)-2.0)-1.0;

    int z=((float)(u)*3600);
    //		workingbuffer[x+(howmuch*2)]=z;
    workingbuffer[x]=z;
  }
  unit->u=u;
  unit->w=w;
}


//6-SIR section

// SIR: we we could resolve edges? redo completely for speed, also all
// inits from workingbuffer

uint16_t SIR(uint16_t delay, uint16_t speed, uint16_t *workingbuffer){ // sudden death!
  static uint8_t flag=0;
  uint16_t cell,cell1,cell2,cell3,sum=0;
  uint16_t *newcells, *cells;
  int16_t x,y,k,p;

  if (flag==0) {
    cells=workingbuffer; newcells=&workingbuffer[MAXDIV2];
  }
  else {
    cells=&workingbuffer[MAXDIV2]; newcells=workingbuffer;
  }      

  /* cell = cells[MAXDIV2]>>8; */
  /* newcells[0]=cell<<8; */
  /* if (cell >= k) newcells[0] = recovered<<8;                                                  */
  /* else if ((cell>32 && cell<k)) newcells[0]+=255; */
  /* else if (cell == susceptible) { */
  /*   cell = cells[MAXDIV2]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[CELLLEN]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[1]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[MAXDIV2-CELLLEN]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /* 	} */

  /////

  for (x=1;x<MAXDIV2;x++){
        cell = cells[x]>>8;
	newcells[x]=cells[x];
	if (cell >= k) newcells[x] = recovered<<8;                                                 
	else if (cell>8){
	  newcells[x]+=255; // aging                                                       
	}
	else { // susceptible
	  sum++;
	  y=x-1;
	  cell = cells[y]>>8;
	  y=(x+CELLLEN)%MAXDIV2;
	  cell1 = cells[y]>>8;
	  y=(x+1)%MAXDIV2;
	  cell2 = cells[y]>>8;
	  if ((x-CELLLEN)<0) y=MAXDIV2-(x+CELLLEN);
	  else y=x-CELLLEN;
	  cell3 = cells[y]>>8;

      if ( (cell>0 && cell<k) ||
	   (cell1>0 && cell1<k) ||
	   (cell2>0 && cell2<k) ||
	   (cell3>0 && cell3<k))
	{
	if (rand()%10 < p) newcells[x] = 9<<8;       
	}
	}}

  //SPEED UP: can also pull out loop for 0->celllen and for last celllen

  flag^=0x01;
  return sum;
}

//void main(int argc, char **argv)
void main(void)
{
  //  int cuu=atoi(argv[1]), pll=atoi(argv[2]);
  int x;
  uint16_t xxx[MAX_SAM+12],result;

  srand(time(NULL));

  for (x=0;x<MAX_SAM;x++){
    xxx[x]=rand()%65536;
  }

  // for Fitz? de-alloc?
  //  struct Fitz *unit=malloc(sizeof(struct Fitz));
  //  struct Oregon *unit=malloc(sizeof(struct Oregon));
  //  struct Spruce *unit=malloc(sizeof(struct Spruce));
  //  struct Brussel *unit=malloc(sizeof(struct Brussel));

  //  fitzinit(unit);
  //  oregoninit(unit);
  //  spruceinit(unit); 
  //  brusselinit(unit); 

  //  printf("%f",(float)xxx[0]/65536.0);
        while(1){ 
	  //  runfitz(10,10,xxx,40,unit);
	  // runoregon(10,10,xxx,10,unit);
	  // runspruce(10,10,xxx,10,unit);
	  //	  runbrussel(10,10,xxx,10,unit);


	  /*	  	  for (x=0;x<8000;x++){
	    printf("%c",xxx[x]>>8);
	    }*/
	    
    }
}
