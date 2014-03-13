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
	u16 tmp,any,counter,edge=0;
	u8 sampledir,samplestep,complexity;
	u8 anydir, anyspeed, anystep; 
	u16 anywrap,anystart;
	static u8 inproc,anydel;
	static u16 samplepos,anypos=0; u8 x;

	sampledir=2;samplestep=1;

#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);
	/*
	int16_t *buf16 = (int16_t*) datagenbuffer;
	
	for (x=0;x<sz/2;x++){

	  right_buffer[x]=buf16[(x+counter)%32768];
	  //    	  right_buffer[x]=(int16_t)datagenbuffer[(x+counter)%32768];
	    //	  right_buffer[x]=(counter+x)*128;
	  }
	
	  counter+=x;

	*/
	audio_comb_stereo(sz, dst, left_buffer, right_buffer);

#else

	u16 *buf16 = (u16*) datagenbuffer;

	// TODO- processing here:
	// TODO or walker for each except 0;;;

	di_split_stereo(sz, src, left_buffer, right_buffer,0, 1); // edger, step

	// do any effects on each sample here?
	complexity=0;
	switch(complexity){
	case 0:
	for (x=0;x<sz/2;x++){
	  samplepos+=1;
	  if (samplepos>=32768) samplepos=0;
	  mono_buffer[x]=audio_buffer[samplepos];
	}
	break;
	/////////
	case 1:
	for (x=0;x<sz/2;x++){
	  tmp=samplestep*direction[sampledir];
	  samplepos+=tmp;
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	break;
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
    	    any=anystep*direction[anydir];
	    if ((anypos+any)>=anywrap) anypos=(anypos+any)%(anywrap);
	    else anypos+=any;
	    any=(anystart+anypos)%32768;
	    }
	  tmp=samplestep*direction[datagenbuffer[any]%8];
	  samplepos+=tmp;
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	  break;
	case 4:
	  //4/walk datagen dir as grains-TODO!!!!
 	for (x=0;x<sz/2;x++){
	  //walk datagen 
	  if (inproc!=0){ // get next datagen
	    if (++anydel==anyspeed){
	      any=anystep*direction[anydir];
	      if ((anypos+any)>=anywrap) anypos=(anypos+any)%(anywrap);
	      else anypos+=any;
	      any=(anystart+anypos)%32768;
	      start=buf16[any];
	      any=anystep*direction[anydir];
	      if ((anypos+any)>=anywrap) anypos=(anypos+any)%(anywrap);
	      else anypos+=any;
	      any=(anystart+anypos)%32768;
	      end=buf16[any];
	      anydel=0;
	      inproc=0;
	    }
	    // walk sample until we reach end - then set inproc=1, pos=0
	    
	    
	  } // inproc	 
	  
	    

	    //	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}

	case 5:
	  //5/walk datagen dir as samples
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==anyspeed){ //do we need speed?
    	    any=anystep*direction[anydir];
	    if ((anypos+any)>=anywrap) anypos=(anypos+any)%(anywrap);
	    else anypos+=any;
	    any=(anystart+anypos)%32768;
	    }
	  mono_buffer[x]=buf16[any];
	}
	  
	  break;

	} // end case

	// complexity->0/straight,1/straight walk,2/wormcode walk,3/datagenasdirwalk,4/walk datagen dir as grains
	/// 5/walk datagen dir as samples, 6/walk datagen with wormdir as grains
	//// 7/walk datagen with wormdir as samples ///more?

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
