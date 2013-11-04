/*
 * audio.c - just the callback 

#define BUFF_LEN 128 - but is 64 for each left/right

- audio hardware

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#include "audio.h"

/* Stereo buffers */
#define STEREO_BUFSZ (BUFF_LEN/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ],
		mono_buffer[MONO_BUFSZ];

//int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".ccmdata")));;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));;
int16_t writeloc[BUFF_LEN/2];
int16_t readloc[BUFF_LEN/2];
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
	int32_t i,wcount,rcount; int16_t x;
	
	audio_split_stereo(sz, src, left_buffer, right_buffer);

	/*
	// load right buffer into mainbuffer using writegrainlist
	// or is just list of sz positions
	for (x=0;x<sz/2;x++){
	  audio_buffer[wcount+x]=right_buffer[x];
	}

	// load mainbuffer into right buffer using readgrainlist
	// or is just list of sz positions
	for (x=0;x<sz/2;x++){
	  right_buffer[x]= audio_buffer[rcount+x];
	}
	
	rcount+=(sz/2); // now works for each knob/adc - TODO-test all
	wcount+=(sz/2);
	  if (wcount>48000) wcount=0;
	  if (rcount>48000) rcount=0;
	*/
		for (x=0;x<sz/2;x++){
		  //		  right_buffer[(sz/2)-x]=left_buffer[x]*2;
		  		  	  left_buffer[x]=0;
		  //		  right_buffer[x]=0;
	  }

	audio_comb_stereo(sz, dst, left_buffer, right_buffer);
	// but what we hear is right_buffer here, left is for filter feedback
	// figure this out - 
	//	audio_comb_stereo(sz, dst, right_buffer=filter, right_buffer=audio);


}
