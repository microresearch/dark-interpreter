/*
 * audio.c - just the callback 

#define BUFF_LEN 128 - but is 64 for each left/right

- audio hardware

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#include "audio.h"
#include "CPUint.h"

/* Stereo buffers */
#define STEREO_BUFSZ (BUFF_LEN/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ],
		mono_buffer[MONO_BUFSZ];

extern __IO uint16_t adc_buffer[10];
extern u8 wormdir; 

//extern u16 edger; // REPLACE with direct poti!

//#define edger (adc_buffer[1]) // 4096!*8///32768 ?? TODO: as walker

extern u8 digfilterflag;
//extern int16_t datagenbuffer[DATA_BUFSZ] __attribute__ ((section (".ccmdata")));;
extern u8 *datagenbuffer;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));;
int16_t *audio_ptr;

void Audio_Init(void)
{
	uint32_t i;
	
	/* clear the buffer */
	audio_ptr = audio_buffer;
	i = AUDIO_BUFSZ;
	while(i-- > 0)
		*audio_ptr++ = 0;
	
	/* init the pointer */
	audio_ptr = audio_buffer;
}

void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src++;
		//		*rdst++ = 0;
		sz--;
	}
}

void di_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst,u16 edge, u8 step)
{
  static u16 count; 
  //  edge=0;
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		count+=step;
		if (count>=AUDIO_BUFSZ) count=edge;
		*rdst++ = *src;
		audio_buffer[count] = *src++;
		//		*rdst++ = 0;
		sz--;
	}
}


void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = (*rsrc++);
		sz--;
	}
}



void buffer_put(int16_t in)
{
	/* put data in */
	*audio_ptr++ = in;
	
	/* wrap pointer */
	if(audio_ptr-audio_buffer == AUDIO_BUFSZ)
		audio_ptr = audio_buffer;
}

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz, uint16_t ht)
{
	float32_t f_p0, f_p1, tb_l, tb_h, f_i, m;
	u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768
	u16 tmp,edge=0;
	u8 sampledir=3,samplestep=1,complexity;
	u8 anydir=3, anyspeed=1, anystep=1; 
	u16 anywrap=32768,anystart=0;
	static u8 inproc=1,anydel;
	static u16 counter=0,start=0,wrap=32768,samplepos=0,anypos=0;
	u16 samplestart=0,samplewrap=32768;
	u8 x; 

	sampledir=2;samplestep=1;

#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);
	
	/*	int16_t *buf16 = (int16_t*) datagenbuffer;
	
	for (x=0;x<sz/2;x++){
	  right_buffer[x]=buf16[(x+counter)%32768];
	  }
	  counter+=x;*/
	audio_comb_stereo(sz, dst, left_buffer, right_buffer);

#else

	u16 *buf16 = (u16*) datagenbuffer;

	di_split_stereo(sz, src, left_buffer, right_buffer,0, 1); // edger, step

	// TODO: any effects on each sample here?
	//
	/* settings: 
	   NORMAL: samplestep,sampledir,direction,samplewrap,samplestart
	   DATAGENWALK: anyspeed, anydir, anystep, anywrap,anystart 
	*/
	
	complexity=0;//TEST
	samplestep=((adc_buffer[2]>>7)+1); // 5 bits=32 and still jitter///average???
	//	sampledir=2;
	switch(complexity){
	case 0:
	for (x=0;x<sz/2;x++){
	  samplepos+=samplestep;// TODO samplestep as also fractional! - only in this case or?
	  // or if we throw in speed will fraction it? TEST
	  if (samplepos>=samplewrap) samplepos=0;
	  mono_buffer[x]=audio_buffer[(samplestart+samplepos)%32768];
	}
	break;
	/////////
	case 1:
	for (x=0;x<sz/2;x++){
	  tmp=samplestep*direction[sampledir];
	  samplepos+=tmp;
	  mono_buffer[x]=audio_buffer[samplepos%32768]; // TODO: whether also start/wrap here?
	}
	break;
	/////////
	case 2:
 	for (x=0;x<sz/2;x++){
	  tmp=samplestep*direction[wormdir];
	  samplepos+=tmp;
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	break;
	case 3:
	  //3/walkdatagenasdirwalk 
 	for (x=0;x<sz/2;x++){
	  //walk any 
	    if (++anydel==anyspeed){
    	    tmp=anystep*direction[anydir];
	    if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap+1);
	    else anypos+=tmp;
	    tmp=(anystart+anypos)%32768;
	    }
	  tmp=samplestep*direction[datagenbuffer[tmp]%8];
	  samplepos+=tmp;
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	  break;
	case 4:
	  //4/walk datagen dir as grains
 	for (x=0;x<sz/2;x++){
	  if (inproc!=0){ // get next datagen
	    if (++anydel==anyspeed){
	      tmp=anystep*direction[anydir];
	      if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap);
	      else anypos+=tmp;
	      tmp=(anystart+anypos);
	      start=buf16[tmp];
	      tmp=anystep*direction[anydir];
	      if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap);
	      else anypos+=tmp;
	      tmp=(anystart+anypos);
	      wrap=buf16[tmp];
	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir?
	      else wrap=start-wrap;
	      if (wrap==0) wrap=1;
	      start=start%32768;wrap=wrap%1024;  //constrain sample wrap size//TODO complex/speed?
	      anydel=0;
	    }
	  }// inproc
	    // walk sample until we reach end - then set inproc=1, pos=0
		inproc=0;
		tmp=samplestep*direction[sampledir]; // and if goes backwards in dir? wrap?
		// that wrap could just be generic len???
		if ((samplepos+tmp)<=wrap)
		  {
		    samplepos=(samplepos+tmp)%32768;
		    mono_buffer[x]=audio_buffer[(start+samplepos)%32768];
		  }
		else {
		  inproc=1;
		  samplepos=0;
		}
	}
	  break;
	case 5:
	  //5/walk datagen dir as samples
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==anyspeed){ //do we need speed?
    	    tmp=anystep*direction[anydir];
	    if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap+1);
	    else anypos+=tmp;
	    tmp=(anystart+anypos)%32768;
	    anydel=0;
	    }
	  mono_buffer[x]=buf16[tmp];
	}
	break;
	/////////
	case 6:
	  //walk datagen with wormdir as grains1
 	for (x=0;x<sz/2;x++){
	  if (inproc!=0){ // get next datagen
	    if (++anydel==anyspeed){
	      tmp=anystep*direction[wormdir];
	      if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap);
	      else anypos+=tmp;
	      tmp=(anystart+anypos);
	      start=buf16[tmp];
	      tmp=anystep*direction[anydir];
	      if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap);
	      else anypos+=tmp;
	      tmp=(anystart+anypos);
	      wrap=buf16[tmp];
	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir?
	      else wrap=start-wrap;
	      if (wrap==0) wrap=1;
	      start=start%32768;wrap=wrap%1024;  //constrain sample wrap size//TODO complex/speed?
	      anydel=0;
	    }
	  }// inproc
	    // walk sample until we reach end - then set inproc=1, pos=0
		inproc=0;
		tmp=samplestep*direction[sampledir]; // and if goes backwards in dir? wrap?
		// that wrap could just be generic len???
		if ((samplepos+tmp)<=wrap)
		  {
		    samplepos=(samplepos+tmp)%32768;
		    mono_buffer[x]=audio_buffer[(start+samplepos)%32768];
		  }
		else {
		  inproc=1;
		  samplepos=0;
		}
	}
	  break;
	case 7:
	  //walk datagen with wormdir as grains2 (sampledir as wormdir)
 	for (x=0;x<sz/2;x++){
	  if (inproc!=0){ // get next datagen
	    if (++anydel==anyspeed){
	      tmp=anystep*direction[wormdir];
	      if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap);
	      else anypos+=tmp;
	      tmp=(anystart+anypos);
	      start=buf16[tmp];
	      tmp=anystep*direction[anydir];
	      if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap);
	      else anypos+=tmp;
	      tmp=(anystart+anypos);
	      wrap=buf16[tmp];
	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir?
	      else wrap=start-wrap;
	      if (wrap==0) wrap=1;
	      start=start%32768;wrap=wrap%1024;  //constrain sample wrap size//TODO complex/speed?
	      anydel=0;
	    }
	  }// inproc
	    // walk sample until we reach end - then set inproc=1, pos=0
		inproc=0;
		tmp=samplestep*direction[wormdir]; // and if goes backwards in dir? wrap?
		// that wrap could just be generic len???
		if ((samplepos+tmp)<=wrap)
		  {
		    samplepos=(samplepos+tmp)%32768;
		    mono_buffer[x]=audio_buffer[(start+samplepos)%32768];
		  }
		else {
		  inproc=1;
		  samplepos=0;
		}
	}
	  break;
	  /////
	case 8:
	  //5/walk datagen dir as samples-wormdir
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==anyspeed){ //do we need speed?
    	    tmp=anystep*direction[wormdir];
	    if ((anypos+tmp)>=anywrap) anypos=(anypos+tmp)%(anywrap+1);
	    else anypos+=tmp;
	    tmp=(anystart+anypos)%32768;
	    anydel=0;
	    }
	  mono_buffer[x]=buf16[tmp];
	}
	break;

	} // end case

	// TODO:re-directions???

#ifndef LACH	
	if (digfilterflag&1){
	  // 3- any processing of left buffer
	  // set via walker for effects//complexity????
	  // left as datagen/as process of right/as process of left/as new buffer/as mix of these
	  //

	for (x=0;x<sz/2;x++){
	  
	  left_buffer[x]=0;
	  //    	  right_buffer[x]=(int16_t)datagenbuffer[(x+counter)%32768];
	  //	  right_buffer[x]=(counter+x)*128;
	    }

	}
#endif

	// 4-out
	//	audio_comb_stereo(sz, dst, left_buffer, right_buffer);
		audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#endif

}
