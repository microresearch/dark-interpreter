/*
 * audio.c - just the callback 

#define BUFF_LEN 128 - but is 64 for each left/right

- audio hardware

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#include "audio.h"
#include "CPUint.h"
#include "settings.h"

#define STEREO_BUFSZ (BUFF_LEN/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ],
		mono_buffer[MONO_BUFSZ];

extern __IO uint16_t adc_buffer[10];
extern u8 wormdir;
extern u16 settingsarray[64];

extern u8 digfilterflag;
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

void di_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{ // TESTY!!!
  static u16 count=0; 
  	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src;
		count+=1;
 		if (count>=AUDIO_BUFSZ) count=0;
		audio_buffer[count] = *src++;
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

void audio_morphy(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,
		  float32_t morph, u8 what)
{
	float32_t morph_inv = 1.0 - morph, f_sum;
	int32_t sum;
	
	while(sz--)
	{
	  if (what&1) f_sum = (float32_t)*asrc++ * morph_inv + (float32_t)*bsrc++ * morph;
	  else if (what&2) f_sum = (float32_t)*asrc++ * (float32_t)*bsrc++ * morph;
	  else f_sum = (float32_t)*asrc++ + (float32_t)*bsrc++ * morph;
		sum = f_sum;
#if 0
		sum = sum > 32767 ? 32767 : sum;
		sum = sum < -32768 ? -32768 : sum;
#else
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (sum) : [src] "r" (sum));
#endif
		
		/* save to destination */
		*dst++ = sum;
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
	u16 tmp=0,tmper;
	u8 x;
	static u16 start=0,wrap,samplepos=0,anypos=0,count;
	static u8 del=0,villagewrite=1;

	int16_t dirry=1;
	float temp;

	static u16 anyposread=0,sampleposread=0,wrapread=0,startread=0;
	static u8 delread=0,villageread=1;
	u16 wrapper; 
	// TODO:find a place in settings for these
#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);
	audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;

	// set dirs
	static int16_t newdir[4]={-180,1,180,-1};
	static int16_t direction[4]={-180,1,180,-1};
	static int16_t newdirread[4]={-180,1,180,-1};
	static int16_t directionread[4]={-180,1,180,-1};

	///	///	///	///

	
	// readin villager processing of left into left and right into audio_buffer

	int16_t * ldst=left_buffer;
	int16_t * rdst=right_buffer;
	SAMPLEWRAPREAD=adc_buffer[0]; // TESTER!
	SAMPLEWRAP=adc_buffer[3]; // TESTER!
	
 	for (x=0;x<sz/2;x++){
	  *ldst++ = *src++;
	  *rdst++ = *src;
	  audio_buffer[sampleposread%32768]=*src++;

	  if (++delread==SAMPLESPEEDREAD){
	    dirry=(int16_t)newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
	    count=((sampleposread-startread)+dirry);
	    if (count<wrapread && (sampleposread+dirry)>startread)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (villageread==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;
		    newdirread[0]=-180;newdirread[2]=180;
		    if (SAMPLEDIRR==1 || SAMPLEDIRR==2) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (villageread==1) {
		  tmp=ANYSTEPREAD*directionread[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+tmper;
		  wrapread=0;startread=0;
		  }
		  else {
		  tmp=ANYSTEPREAD*directionread[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD; 
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  tmp=ANYSTEPREAD*directionread[DATADIRR];
		  anyposread+=tmp;
		  wrapper=ANYWRAPREAD;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=buf16[tmp]>>1;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1 || SAMPLEDIRR==2) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  temp=sqrtf((float)wrapread);newdirread[0]=-temp;newdirread[2]=temp;
		  }
		}
	  delread=0;
	  }
	}

	// writeout villager processing  into mono_buffer
	SAMPLESPEED=1;SAMPLESTEP=1;SAMPLEDIRW=1;
	SAMPLESTART=0;
	//	cons=32768;


 	for (x=0;x<sz/2;x++){
	  	    mono_buffer[x]=audio_buffer[samplepos%32768];
	  //  	    mono_buffer[x]=buf16[samplepos%32768];

	  if (++del==SAMPLESPEED){
	    dirry=(int16_t)newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    if (count<wrap && (samplepos+dirry)>start)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (villagewrite==0) {
		  start=SAMPLESTART;wrap=SAMPLEWRAP;
		  newdir[0]=-180;newdir[2]=180;
		  if (SAMPLEDIRW==1 || SAMPLEDIRW==2) samplepos=SAMPLESTART;
		  else samplepos=SAMPLESTART+SAMPLEWRAP;
		  }

		  /////////////////////////////////////
		  else if (villagewrite==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+tmper;
		  wrap=0;start=0;
		  }
		  /////////////////////////////////////
		  else {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  wrapper=ANYWRAP;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=buf16[tmp]>>1;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1 || SAMPLEDIRW==2) samplepos=start;
		  else samplepos=start+wrap;
		  // recalc direction array
		  temp=sqrtf((float)wrap);newdir[0]=-temp;newdir[2]=temp;
		  }
		}
	  del=0;
	  }
	}
	       

	///	///	///	///

#ifndef LACH // as we have no filter!

	if (digfilterflag&32){ // TODO/TESTCODE here....
	  // processing of left buffer mixed(or not)back into mono_buffer

	  //audio_morphy(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,float32_t morph, u8 what) what - what is 1 or 2 so far for options
	  //	  audio_morphy(sz/2, mono_buffer, mono_buffer, left_buffer,0.1f,2);
	}

	else if (digfilterflag&1){ // TODO
	  // 3- any processing of left buffer into left buffer
	  // left as datagen/as process of right/as process of left/as new buffer/as mix of these

	  /// but for filter/leftbuffer effect we have no buffer spare - should act
	  /// as mirror with list of ops: mix/multiply left with audio/datagen,
	  /// write left into audiobuffer or datagenbuffer, playback from place in
	  /// audiobuffer -> these ops perform at location array[XXX]

	  // use left, use audiobuffer, use datagenbuffer, read/write

	  //

	}


#endif

	// 4-out
	//audio_comb_stereo(sz, dst, left_buffer, right_buffer);
		audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#endif

}
