// gcc simulation.c -osim -lm

// was datagen.c previously

/* testing of all simulation data generators */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#define true 1
#define false 0
#define MAX_SAM 65536
#define floor(x) ((int)(x))
#define HEX__(n) 0x##n##UL
#define ONESIXTH 0.1666666666666667
#define BET(A, B, C)  (((A>=B)&&(A<=C))?1:0)    /* a between [b,c] */

/* TODO:

- *TODO* increment counter for each simulation datagen through working
   buffer (second rossler is example here)

- each datagen should return a value/values multiple values in
  workingbuffer with first terms as settings (keep first n values
  clear unless we flag otherwise)

so somehow declares offset for settings, or we store this somewhere

but write across buffer sequentially as history

- add stepsizing/speed for each

- add re-init//trigger bit - also how to organize x bytes of
  workingbuffer reserved or... (offset question)

- does NaN cause problems or not?

//TODO: SIR using equations

see: http://homepages.warwick.ac.uk/~masfz/ModelingInfectiousDiseases/

*/

//////////////////////////////////////////////////////////

//4-SIMULATIONS

//what do we do with NaN, what numbers should be fed in as inits from
//workingbuffer?

//what happens if workingbuffer is 0? copying across of workingbuffer/buffers...

//////////////////////////////////////////////////////////

// IFS

typedef struct{ double x, y; } Point;

struct IFS {
	double prob[5];
	double coeff[4][6];
	Point p1,p2;
	};

void ifsinit(struct IFS* unit){
  int i,iter;
  int column = 6, row = 4;
  unit->p1.x=0.1;
  unit->p1.y=0.1;         

  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){
      //      iter=rand()%row;
      //      i=rand()%column;
      unit->coeff[iter][i]=((double)rand()/(double)(RAND_MAX));
      if (((double)rand()/(double)(RAND_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
      unit->prob[iter]=((double)rand()/(double)(RAND_MAX));
    }
  }
}

uint16_t runifs(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct IFS* unit){

  double random_num;
  int iter,i,it,x;
  int column = 6, row = 4;


  /*  ifs->prob[0]=0.0;
  ifs->prob[1]=0.85; 
  ifs->prob[2]=0.92; 
  ifs->prob[3]=0.99; 
  ifs->prob[4]=1.0; 
  */

  unit->prob[0]=(float)workingbuffer[0]/65536.0;
  unit->prob[1]=(float)workingbuffer[1]/65536.0;
  unit->prob[2]=(float)workingbuffer[2]/65536.0;
  unit->prob[3]=(float)workingbuffer[3]/65536.0;
  unit->prob[4]=(float)workingbuffer[4]/65536.0;

  random_num = (double)rand()/(double)(RAND_MAX);

  for (x=0;x<howmuch;x++){
  for(i = 0; i < row; i++){
    if ( BET(random_num,unit->prob[i],unit->prob[i+1]) ){
      unit->p2.x = unit->coeff[i][0]*unit->p1.x + unit->coeff[i][1]*unit->p1.y + unit->coeff[i][4];
      unit->p2.y = unit->coeff[i][2]*unit->p1.x + unit->coeff[i][3]*unit->p1.y + unit->coeff[i][5];
      break;
    }
  }
  unit->p1=unit->p2;  
					
  //  if (unit->p2.x>0.0)
    //    unit->returnvalx=(int)((unit->p2.x)*1024);
  //  if (unit->p2.y>0.0)
    //   unit->returnvaly=(int)((unit->p2.y)*1024);
  workingbuffer[count+x+5]=unit->p2.x;
  printf("%c",unit->p2.x);
  /*    iter=rand()%row;
    i=rand()%column;
    unit->coeff[iter][i]=((double)rand()/(double)(RAND_MAX));
    if (((double)rand()/(double)(RAND_MAX))>0.5) unit->coeff[iter][i]= unit->coeff[iter][i]-1;
    unit->prob[iter]=((double)rand()/(double)(RAND_MAX));
    unit->p1.x=0.5;
    unit->p1.y=0.5;*/
}
  return x;
}

//////////////////////////////////////////////////////////

// ROSSLER

struct Rossler{
  double h,a,b,c,lx0,ly0,lz0;
};

void rosslerinit(struct Rossler* unit) {
  unit->h = 0.1;
  unit->a = 0.3;
  unit->b = 0.2;
  unit->c = 5.8;
  unit->lx0 = 0.1;
  unit->ly0 = 0;
  unit->lz0 = 0;
}

uint16_t runrossler(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Rossler* unit){
  double lx0,ly0,lz0,lx1,ly1,lz1;
  //  double h,a,b,c;
  int x;
  int i;

  /* which unit to vary according to workingbuffer */
  // leave as so!

  double  h = unit->h;
  //  a = unit->a;
  //  b = unit->b;
  double c = unit->c;

  //  double h = (float)workingbuffer[0]/120536.0;
  double a = (float)workingbuffer[1]/122536.0;
  double b = (float)workingbuffer[2]/100536.0;

  for (i=0; i<howmuch; i++) {

  lx0 = unit->lx0;
  ly0 = unit->ly0;
  lz0 = unit->lz0;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  unit->lx0 = lx1;
  unit->ly0 = ly1;
  unit->lz0 = lz1;
  //  printf("%d",lz1);
  workingbuffer[count+i+3]=lx1;
  //  workingbuffer[i+1]=ly1;
  //  workingbuffer[i+2]=lz1;

  }
  return i;
}

//////////////////////////////////////////////////////////

// 2nd rossler from: MCLDChaosUGens.cpp

struct secondRossler{
  double z0, zn, znm1;
  double x0, y0, xn, yn, xnm1, ynm1;
};

void secondrosslerinit(struct secondRossler* unit){

  }

uint16_t runsecondrossler(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct secondRossler* unit){

  
  double a = (float)workingbuffer[0]/65536.0;
  double b = (float)workingbuffer[1]/65536.0;
  double c = (float)workingbuffer[2]/65536.0;
  double h = (float)workingbuffer[3]/65536.0;
  double x0 = (float)workingbuffer[4]/65536.0;
  double y0 = (float)workingbuffer[5]/65536.0;
  double z0 = (float)workingbuffer[6]/65536.0;

  double xn = unit->xn;
  double yn = unit->yn;
  double zn = unit->zn;
  double xnm1 = unit->xnm1;
  double ynm1 = unit->ynm1;
  double znm1 = unit->znm1;
  int i;
	
	if((unit->x0 != x0) || (unit->y0 != y0) || (unit->z0 != z0)){
		xnm1 = xn;
		ynm1 = yn;
		znm1 = zn;
		unit->x0 = xn = x0;
		unit->y0 = yn = y0;
		unit->z0 = zn = z0;
	}

	double dx = xn - xnm1;
	double dy = yn - ynm1;
	double dz = zn - znm1;

	for (i=0; i<howmuch; ++i) {

			xnm1 = xn;
			ynm1 = yn;
			znm1 = zn;

			double k1x, k2x, k3x, k4x,
				k1y, k2y, k3y, k4y,
				k1z, k2z, k3z, k4z,
				kxHalf, kyHalf, kzHalf;

			// 4th order Runge-Kutta approximate solution for differential equations
			k1x = - (h * (ynm1 + znm1));
			k1y = h * (xnm1 + a * ynm1);
			k1z = h * (b + znm1 * (xnm1 - c));
			kxHalf = k1x * 0.5;
			kyHalf = k1y * 0.5;
			kzHalf = k1z * 0.5;

			k2x = - (h * (ynm1 + kyHalf + znm1 + kzHalf));
			k2y = h * (xnm1 + kxHalf + a * (ynm1 + kyHalf));
			k2z = h * (b + (znm1 + kzHalf) * (xnm1 + kxHalf - c));
			kxHalf = k2x * 0.5;
			kyHalf = k2y * 0.5;
			kzHalf = k2z * 0.5;

			k3x = - (h * (ynm1 + kyHalf + znm1 + kzHalf));
			k3y = h * (xnm1 + kxHalf + a * (ynm1 + kyHalf));
			k3z = h * (b + (znm1 + kzHalf) * (xnm1 + kxHalf - c));

			k4x = - (h * (ynm1 + k3y + znm1 + k3z));
			k4y = h * (xnm1 + k3x + a * (ynm1 + k3y));
			k4z = h * (b + (znm1 + k3z) * (xnm1 + k3x - c));

			xn = xn + (k1x + 2.0*(k2x + k3x) + k4x) * ONESIXTH;
			yn = yn + (k1y + 2.0*(k2y + k3y) + k4y) * ONESIXTH;
			zn = zn + (k1z + 2.0*(k2z + k3z) + k4z) * ONESIXTH;

			dx = xn - xnm1;
			dy = yn - ynm1;
			dz = zn - znm1;
	
	/*		ZXP(xout) = (xnm1 + dx) * 0.5f;
		ZXP(yout) = (ynm1 + dy) * 0.5f;
		ZXP(zout) = (znm1 + dz) * 1.0f;*/
			workingbuffer[count+i+7]=xnm1+dx;
			//			printf("%c",(xnm1+dx));
	}
	unit->xn = xn;
	unit->yn = yn;
	unit->zn = zn;
	unit->xnm1 = xnm1;
	unit->ynm1 = ynm1;
	unit->znm1 = znm1;
	return i;
}

//////////////////////////////////////////////////////////

// BRUSSELATOR

struct Brussel{
    float x,y; 
};

void brusselinit(struct Brussel* unit) {
  unit->x = 0.5f; 
  unit->y = 0.5f; 
}

void runbrussel(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Brussel* unit){
    
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

//////////////////////////////////////////////////////////

// spruceworm

struct Spruce{
    float x, y; 
};

void spruceinit(struct Spruce* unit ) {
	
  unit->x = 0.9f; 
  unit->y = 0.1f; 
}

void runspruce(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Spruce* unit){

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

//////////////////////////////////////////////////////////

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

void runoregon(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Oregon* unit){
    
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

//////////////////////////////////////////////////////////

struct Fitz
{
	float u,w;
};

void fitzinit(struct Fitz* unit) {
	unit->u=0.0;
	unit->w=0.0;
}

void runfitz(uint16_t count, uint16_t delay, uint16_t speed, uint16_t *workingbuffer, uint8_t howmuch, struct Fitz* unit){

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


//void main(int argc, char **argv)
void main(void)
{
  //  int cuu=atoi(argv[1]), pll=atoi(argv[2]);
  int x;
  uint16_t xxx[MAX_SAM+12],result;
  uint16_t count=0;
  srand(time(NULL));

  for (x=0;x<MAX_SAM;x++){
    xxx[x]=rand()%65536;
  }

  // for Fitz? de-alloc?
  //  struct Fitz *unit=malloc(sizeof(struct Fitz));
  //  struct Oregon *unit=malloc(sizeof(struct Oregon));
  //  struct Spruce *unit=malloc(sizeof(struct Spruce));
  //  struct Brussel *unit=malloc(sizeof(struct Brussel));
  //  struct Rossler *unit=malloc(sizeof(struct Rossler));
  //  struct secondRossler *unit=malloc(sizeof(struct secondRossler));
  struct IFS *unit=malloc(sizeof(struct IFS));

  //  fitzinit(unit);
  //  oregoninit(unit);
  //  spruceinit(unit); 
  //  brusselinit(unit); 
  // rosslerinit(unit)
  //  secondrosslerinit(unit);
  ifsinit(unit);

  //  printf("%f",(float)xxx[0]/65536.0);
        while(1){ 
	  //	  count+=runsecondrossler(count,10,10,xxx,10,unit);
	  count+=runifs(count,10,10,xxx,10,unit);

	  //	  printf("%d\n",count);
	  /*	  for (x=3;x<13;x++){
	    printf("%c",xxx[x]>>8);
	    }*/
	    
    }
}
