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
//extern u16 edger; // REPLACE with direct poti! **TODO

//#define edger (adc_buffer[3]<<3) // 32768

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
	static u16 counter=0; int16_t x;

#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);

	// datagenbuffer test (note that is an INT though +-32768)

		for (x=0;x<sz/2;x++){
	  right_buffer[x]=(int16_t)datagenbuffer[(x+counter)%32768];
	  //	  right_buffer[x]=(counter+x)*128;
	}

		counter+=x;

	audio_comb_stereo(sz, dst, left_buffer, right_buffer);

#else

	// TODO- processing here:
	// 1- right buffer goes into audio_buffer according to edge (counter to return to)
	//	di_split_stereo(sz, src, left_buffer, right_buffer, edger);

	// 2- databuffer or wormdir or complexities/combination of these
	// databuffer[x] as index into audiobuf & 32767 

	//	di_process_buffer(sz,mono_buffer,right_buffer,complexity, dir, step)
	// mono is result... right_buffer there just for possible processing

	// or as grains with grainsize???
	//   granular style (start->end%maxgrainsize) - as option
	// - reads back samples into right_buffer with any processing
	
	if (digfilterflag&1){
	// 3- any processing of left buffer

	}


	// 4-out
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#endif

}
