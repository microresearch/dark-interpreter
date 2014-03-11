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
//extern u16 edger; // REPLACE with direct poti!

#define edger (adc_buffer[1]<<3) // 32768

extern u8 digfilterflag;
//extern int16_t datagenbuffer[DATA_BUFSZ] __attribute__ ((section (".ccmdata")));;
extern u16 *datagenbuffer;
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

void di_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst,u16 edge)
{
  static u16 count;
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		count++;
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
	u16 tmp;
	u8 sampledir,samplestep;
	static u16 samplepos,counter=0; u8 x;

	sampledir=2;samplestep=1;

#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);

	// datagenbuffer test (note that is an INT though +-32768)

	int16_t *buf16 = (int16_t*) datagenbuffer;
	
	/*
		for (x=0;x<sz/2;x++){

	  right_buffer[x]=buf16[(x+counter)%32768];
			  //    	  right_buffer[x]=(int16_t)datagenbuffer[(x+counter)%32768];
	    //	  right_buffer[x]=(counter+x)*128;
	  }
	*/
	  counter+=x;

	audio_comb_stereo(sz, dst, left_buffer, right_buffer);

#else

	u16 *buf16 = (u16*) datagenbuffer;


	//	audio_split_stereo(sz, src, left_buffer, right_buffer); // TEST!!!

	//	audio_comb_stereo(sz, dst, left_buffer, right_buffer);

	// TODO- processing here:

	//[[[
	// 1- right buffer goes into audio_buffer according to edger (counter to return to)
	//edger can also be datagen walker variations!
	di_split_stereo(sz, src, left_buffer, right_buffer, edger); // last is edger

	//	di_process_buffer(sz,mono_buffer,right_buffer,complexity, dir, step)

	// set via walk-through for effects
	// mono is result... right_buffer there just for possible processing
	//   granular style (start->end%maxgrainsize) - as option
	// - reads back samples into right_buffer with any processing
	//]]]*

	// TRY-walk through (2 below)

	for (x=0;x<sz/2;x++){
	  tmp=samplestep*direction[sampledir];
	  samplepos+=tmp;
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}

	// complexity->1/straight,2/straight walk,2.5/wormcode walk,2.6/datagenasdirwalk,3/walk datagen dir as grains
	/// 4/walk datagen dir as samples, 5/walk datagen with wormdir as grains
	//// 6/walk datagen with wormdir as samples more?

	// complexity->effects???

	
	if (digfilterflag&1){
	  // 3- any processing of left buffer
	  // set via walker for effects//complexity?
	  // left as datagen/as process of right/as process of left/as new buffer/as mix of these
	  //

	for (x=0;x<sz/2;x++){
	  
	  left_buffer[x]=0;
	  //    	  right_buffer[x]=(int16_t)datagenbuffer[(x+counter)%32768];
	  //	  right_buffer[x]=(counter+x)*128;
	    }

	}


	// 4-out
		audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#endif

}
