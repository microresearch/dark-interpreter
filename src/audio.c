/*
 * audio.c - just the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#include "audio.h"
#include "CPUint.h"
#include "settings.h"

#define STEREO_BUFSZ (BUFF_LEN/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)

int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], temp_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];

extern __IO uint16_t adc_buffer[10];
//extern u8 wormdir;
extern u8 villagestackpos;
extern u16 settingsarray[71];
extern u16 villager[130];
extern signed char newdir[2];
extern signed char direction[2];
extern signed char villagedirection[2];
extern signed char villagedirectionf[2];
extern signed char villagedirectionw[2];
extern signed char newdirf[2];
extern signed char directionf[2];
extern signed char newdirread[2];
extern signed char directionread[2];
extern u8 digfilterflag;
extern u8 *datagenbuffer;

int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data")));;
int16_t *audio_ptr;

void runconvforaudio(u8 sz, int16_t *src, int16_t *dst, float c0, float c1, float c2){
  u8 i=0,tmp=0,tmpp;
  for (i=0; i<sz; i++) {
    tmp++;tmpp=(tmp-1)%sz;
  *dst++ =((float)src[tmpp]*c0)+((float)src[tmp]*c1)+((float)src[(tmp+1)%sz]*c2);
  }
}

extern const u16 SAMPLE_FREQUENCY;
extern const float Pi;
extern const float PI_2;
extern u8 ww[3],freqy[3];

void runformforaudio(u8 sz, int16_t *src, int16_t *dst){

  float buff[64]; float x; 

  for (u8 f = 0; f < 3; f++ ) {
  u8 ff = freqy[f]; // the three freqs

  float freq = (float)ff*(50.0f/SAMPLE_FREQUENCY);

  float buf1Res = 0, buf2Res = 0;
  float q = 1.0f - (float)ww[f] * (Pi * 10.0f / SAMPLE_FREQUENCY);
  float xp = 0;
  
  for (u8 s = 0; s < sz; s++ ) {
    // x is our float sample
    // Apply formant filter
    x=(float)(src[s])/32768.0f;
    x = x + 2.0f * cosf ( PI_2 * freq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;

    buff[s]+=x; // as float
    if (f==2){
    *dst++=(float)buff[s]*32768.0f;
    }
  }
  }
}

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

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz, uint16_t ht)
{
  u16 tmp,tmper;
  int16_t tmp16,count;
  int32_t tmp32;
  u8 x,tmpp;
  static u16 start=0,startfilt=0,wrapfilt=1,wrap=1,samplepos=0,sampleposfilt=0,anyposfilt=0,anypos=0;
  static u8 villagerpos=0,villagefpos=0,villagewpos=0,del=0,delf=0,delread=0;
  u8 VILLAGEREAD,VILLAGEWRITE,VILLAGEFILT;
  int16_t dirry;
  float w0,w1,w2;

  static u16 anyposread=0,sampleposread=0,wrapread=1,startread=0;
  u16 wrapper; 

#ifdef TEST_STRAIGHT
  audio_split_stereo(sz, src, left_buffer, right_buffer);
  audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *buf16int =(int16_t*) datagenbuffer;
	int16_t *firstbuf, *secondbuf;

#ifdef TEST_EEG
	// write buf16 into mono
	      	for (x=0;x<sz/2;x++){
	  mono_buffer[x]=buf16[samplepos%32768];
	  samplepos++;
		}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else

	///	///	///	///

	// readin villager processing of left into left and right into audio_buffer

	int16_t *ldst=left_buffer;
	int16_t *rdst=right_buffer;

#ifdef LACH

	if (EFFECTREAD&64) {firstbuf=buf16int;secondbuf=audio_buffer;} // top bit now is buffer
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	VILLAGEREAD=(EFFECTREAD&3);
	
	tmpp=(EFFECTREAD&63)>>2;
      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	  *src++;
	  firstbuf[sampleposread%32768]=*src++;
	  break;
	  case 1:
	  *src++;
	  secondbuf[sampleposread%32768]=*src++;
	  break;
	  case 2:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768];
	  secondbuf[sampleposread%32768]=firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	  *src++;
	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *src++;
	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *src++;
	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *src++;
	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  *src++;
	  tmp16=(*src++)-secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768]^(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  *src++;
	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *src++;
	  tmp16=(*src++)-firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 12:
	  *src++;
	  tmp16=firstbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 13:
	  *src++;
	  tmp16=firstbuf[sampleposread%32768]^(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 14:
	  *src++;
	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 15:
	  *src++;
	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  }
	  /////
	  if (++delread>=SAMPLESPEEDREAD){
	    dirry=newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
	    ///
	    count=((sampleposread-startread)+dirry);
	    //	    if (count<wrapread && (sampleposread+dirry)>startread)
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
	    ////
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*directionread[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    // advance to next in array based on new start and wrap

		    tmp=VILLAGERSTEP*villagedirection[VILLAGERDIR];
		    villagerpos+=tmp;
		    tmp=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startread=villager[tmp];
		    wrapread=villager[tmp+1];
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
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
		  wrapread=(buf16[tmp]>>1)%SAMPLEWRAPREAD;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){

	  ////////////////////////////////////LDST effects also...

	  if (EFFECTREAD&64) firstbuf=buf16int;
	else firstbuf=audio_buffer;
	VILLAGEREAD=EFFECTREAD&3;
	tmpp=(EFFECTREAD&63)>>2;
      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=*src++;
	  break;
	  case 1:
	  firstbuf[sampleposread%32768]=*src;
	  *ldst++ = *src++;
	  *rdst++ = *src++; 
	  break;
	  // Effects with/without clipping *, +, -, 
	  case 2:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 3:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp16=(*src++)-firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 7:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp16=firstbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  // start of *(src-1)
	  case 8:
	  *ldst++ = *src;
	  tmp32= *(src++) * (*src++);
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 9:
	  *ldst++ = *src;
	  tmp32= *(src++) * (*src++);
	  *rdst++ = *src; 
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 10:
	  *ldst++ = *src;
	  tmp32=firstbuf[samplepos%32768]* *src++;
	  *rdst++ = *src++; 
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *ldst++ = *src;
	  tmp32=firstbuf[samplepos%32768]* *src++;
	  *rdst++ = *src++; 
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  *ldst++ = *src;
	  tmp32=*src++ +firstbuf[sampleposread%32768];
	  *rdst++ = *src; 
	  *src++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  case 13:
	  *ldst++ = *src;
	  tmp32=*src++ * *src++;
	  *rdst++ = *src; 
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  *ldst++ = *src;
	  tmp16=*src++ ^ *src++;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
 	  case 15:
	  *ldst++ = *src;
	  tmp32=*src++ + *src++;
	  *rdst++ = *src; 
	  firstbuf[sampleposread%32768]=tmp32;
	  }
	  	  if (++delread>=SAMPLESPEEDREAD){
		    dirry=newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
		    count=((sampleposread-startread)+dirry);
		    //if (count<wrapread && (sampleposread+dirry)>startread)
		    if (count<wrapread && count>0)
		      {
			sampleposread+=dirry;//)%32768;
		      }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*directionread[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGERSTEP*villagedirection[VILLAGERDIR];
		    villagerpos+=tmp;
		    tmp=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startread=villager[tmp];
		    wrapread=villager[tmp+1];
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
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
		  wrapread=(buf16[tmp]>>1)%SAMPLEWRAPREAD;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	}
	else  // READIN NO DIG FILTER
	  {
	    if (EFFECTREAD&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	    else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	    VILLAGEREAD=EFFECTREAD&3;
	    tmpp=(EFFECTREAD&63)>>2;
      	for (x=0;x<sz/2;x++){
	  switch(tmpp){
	  case 0:
	  default:
	  *src++;
	  firstbuf[sampleposread%32768]=*src++;
	  break;
	  case 1:
	  *src++;
	  secondbuf[sampleposread%32768]=*src++;
	  break;	    
	  case 2:
	    //*src++;
	  tmp16=secondbuf[sampleposread%32768];
	  secondbuf[sampleposread%32768]=firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  ////////
	  // Effects with/without clipping *, +, -, 
	  case 3:
	  *src++;
	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *src++;
	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *src++;
	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *src++;
	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  *src++;
	  tmp16=(*src++)-secondbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  *src++;
	  tmp16=secondbuf[sampleposread%32768]^(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  *src++;
	  tmp32=(*src++)*firstbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *src++;
	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  *src++;
	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 13:
	  *src++;
	  tmp32=(*src++)-firstbuf[sampleposread%32768];
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  *src++;
	  tmp16=firstbuf[sampleposread%32768]-(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  break;
	  case 15:
	  *src++;
	  tmp16=firstbuf[sampleposread%32768]^(*src++);
	  firstbuf[sampleposread%32768]=tmp16;
	  }
	  //	  	  VILLAGEREAD=0; // TESTY!
	  if (++delread>=SAMPLESPEEDREAD){
	    dirry=newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
	    count=((sampleposread-startread)+dirry);
	    //	    if (count<wrapread && (sampleposread+dirry)>startread)
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;
		    if (SAMPLEDIRR==1) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		  tmp=ANYSTEPREAD*directionread[DATADIRR];
		  anyposread+=tmp;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    tmp=VILLAGERSTEP*villagedirection[VILLAGERDIR];
		    //tmp=1; // TSTER!!
		    villagerpos+=(tmp*2);
		    tmp=(VILLAGERSTART+(villagerpos%VILLAGERWRAP))%villagestackpos; //to cover all directions
		    startread=villager[tmp];
		    wrapread=villager[tmp+1];
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) sampleposread=startread;
		    else sampleposread=startread+wrapread;
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
		  wrapread=(buf16[tmp]>>1)%SAMPLEWRAPREAD;
		  if (wrapread==0) wrapread=1;
		  if (SAMPLEDIRR==1) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	  }
#endif
	///!!!!!////////////////////////////////END OF READINSSS
	///!!!!!////////////////////////////////END OF READINSSS
	///!!!!!////////////////////////////////END OF READINSSS

#ifdef LACH
	// firstbuf, secondbuf
	  if (EFFECTWRITE&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	VILLAGEWRITE=EFFECTWRITE&3;
	tmpp=(EFFECTWRITE&63)>>2;
      	for (x=0;x<sz/2;x++){

	  switch(tmpp){ 
	  case 0:
	  mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  case 1:
	  mono_buffer[x]=secondbuf[samplepos%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	  tmp16=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	  tmp16=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	  tmp16=secondbuf[samplepos%32768]^firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	  tmp16=secondbuf[samplepos%32768]&firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	  tmp16=secondbuf[samplepos%32768]|firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 11:
	    tmp16=secondbuf[samplepos%32768]<<(firstbuf[samplepos%32768])&7;
	  mono_buffer[x]=tmp16;
	  break;
	  case 12:
	  break;
	    tmp16=secondbuf[samplepos%32768]>>(firstbuf[samplepos%32768])&7;
 	  default:
	    temp_buffer[x]=firstbuf[samplepos%32768];
	  }

	  	  if (++del>=SAMPLESPEED){
	    dirry=newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);// samplepos is start or start+wrap++
		    if (count<wrapread && count>0)
	      {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		    count=samplepos;
		  }

		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*villagedirectionw[VILLAGEWDIR];
		    villagewpos+=tmp;
		    tmp=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[tmp];
		    wrap=villager[tmp+1];
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
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
		  wrap=(buf16[tmp]>>1)%SAMPLEWRAP;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	  // process mono_buffer for extra effects 13/14/15
	  if (tmpp>12){
	    switch(tmpp){
	    case 13:
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,0.5,0.5,0.5);
	    break;
	    case 14:
	      // 3 floats!
	      w0=buf16[0]/65536.0;w1=buf16[1]/65536.0;w2=buf16[2]/65536.0;
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,w0,w1,w2);
	      break;
	    case 15:
	      runformforaudio(sz/2,temp_buffer,mono_buffer);
	      break;
	  }
	  } // end of tmpp>12
	}

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){
	int16_t * ldst=left_buffer;

	  ////////////////////////////////////LDST effects also...

	  if (EFFECTWRITE&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	VILLAGEWRITE=EFFECTWRITE&3;
	tmpp=(EFFECTWRITE&63)>>2;
      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	    mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 1:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 2:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp16=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 6:
	  tmp16=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	  tmp32=firstbuf[samplepos%32768]* *ldst++;
	  mono_buffer[x]=tmp32;
	  break;
	  case 8:
	  tmp32=firstbuf[samplepos%32768]* *ldst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 9:
	    tmp16=firstbuf[samplepos%32768] - *ldst++;
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	  tmp32=*(ldst++)+secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 11:
	  tmp32=*(ldst++)+secondbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 12:
	  mono_buffer[x]=adc_buffer[9]<<3;
	  break;
	  case 13:
	    tmp32=firstbuf[samplepos%32768]+adc_buffer[9]<<3;
	    mono_buffer[x]=tmp32;
	  break;
	  case 14:
	    tmp32=firstbuf[samplepos%32768]*adc_buffer[9]<<3;
	    mono_buffer[x]=tmp32;
	  break;
 	  default:
	    temp_buffer[x]=firstbuf[samplepos%32768];
	  }
	  ////////////////////////--->>>>


	  	  if (++del>=SAMPLESPEED){
	    dirry=newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    //	    if (count<wrap && (samplepos+dirry)>start)
	    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }

		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*villagedirectionw[VILLAGEWDIR];
		    villagewpos+=tmp;
		    tmp=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[tmp];
		    wrap=villager[tmp+1];
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
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
		  wrap=(buf16[tmp]>>1)%SAMPLEWRAP;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	  // process mono_buffer for extra effects 13/14/15
	  if (tmpp==15){
	      // 3 floats!
	      w0=buf16[0]/65536.0;w1=buf16[1]/65536.0;w2=buf16[2]/65536.0;
	      runconvforaudio(sz/2,temp_buffer,mono_buffer,w0,w1,w2);
	  } // end of tmpp==15
	}
	}
	else
	  { /// STRAIGHT SANS FILTEROPSSS!!!
	    	  if (EFFECTWRITE&64) {firstbuf=buf16int;secondbuf=audio_buffer;}
	    	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	    //	    firstbuf=buf16int; //TESTY!
	  VILLAGEWRITE=EFFECTWRITE&3;
	  tmpp=(EFFECTWRITE&63)>>2;
	  //	  tmpp=0; // TESTYYYY!!!
      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	    mono_buffer[x]=firstbuf[samplepos%32768];
	    break;
	  case 1:
	    mono_buffer[x]=secondbuf[samplepos%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	  tmp16=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	  tmp16=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	  tmp16=secondbuf[samplepos%32768] ^ firstbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	  tmp16=firstbuf[samplepos%32768] & secondbuf[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	  mono_buffer[x]=adc_buffer[9]<<3;
	  break;
	  case 11:
	    tmp32=firstbuf[samplepos%32768]+adc_buffer[9]<<3;
	    mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    tmp32=firstbuf[samplepos%32768]*adc_buffer[9]<<3;
	    mono_buffer[x]=tmp32;
	  break;
 	  default:
	    temp_buffer[x]=firstbuf[samplepos%32768];
	 	  }

	  //	  VILLAGEWRITE=0; // TESTY!!!!
 
	  if (++del>=SAMPLESPEED){
	    dirry=newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    //	    if (count<wrap && (samplepos+dirry)>start)
		    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;
		    if (SAMPLEDIRW==1) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }
		  else if (VILLAGEWRITE==1) {
		  tmp=ANYSTEP*direction[DATADIRW];
		  anypos+=tmp;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEWSTEP*villagedirectionw[VILLAGEWDIR];
		    villagewpos+=tmp*2;
		    tmp=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    start=villager[tmp];
		    wrap=villager[tmp+1];
		    if (wrap==0) wrap=1;
		    if (SAMPLEDIRW==1) samplepos=start;
		    else samplepos=start+wrap;
		  }
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
		  wrap=(buf16[tmp]>>1)%SAMPLEWRAP;
		  if (wrap==0) wrap=1;
		  if (SAMPLEDIRW==1) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	  // process mono_buffer for extra effects 13/14/15
	  if (tmpp>12){

	    switch(tmpp){
	    case 13:
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,0.5,0.5,0.5);
	    break;
	    case 14:
	      // 3 floats!
	      w0=buf16[0]/65536.0;w1=buf16[1]/65536.0;w2=buf16[2]/65536.0;
	    runconvforaudio(sz/2,temp_buffer,mono_buffer,w0,w1,w2);
	      break;
	    case 15:
	      runformforaudio(sz/2,temp_buffer,mono_buffer);
	      break;
	  }
	  } // end of tmpp>12
	}
	  }
      	///!!!!!!////////////////////////////////END OF WRITEOUTSSS

	///	///	///	///

if (digfilterflag&1){ 

	int16_t * ldst=left_buffer;
	int16_t * rdst=right_buffer;

	  ////////////////////////////////////LDST effects also...
	if (EFFECTFILTER&64) firstbuf=buf16int; 
	else firstbuf=audio_buffer;
	VILLAGEFILT=EFFECTFILTER&3;
	tmpp=(EFFECTFILTER&63)>>2;
      	for (x=0;x<sz/2;x++){ 
	  switch(tmpp){ 
	  case 0:
	  default:
	  *ldst++=firstbuf[sampleposfilt%32768];
	  break;
	  case 1:
	  *ldst++=*rdst++;
	  break;
	  case 2:
	    *ldst++ =0;
	    break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	  tmp32=firstbuf[sampleposfilt%32768]* *ldst++;
	  *ldst=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[sampleposfilt%32768]* *ldst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 5:
	  tmp32=firstbuf[sampleposfilt%32768]* *rdst++;
	  *ldst++=tmp32;
	  break;
	  case 6:
	  tmp32=firstbuf[sampleposfilt%32768]* *rdst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst++=tmp32;
	  break;
	  case 7:
	  tmp32=*ldst++ * *rdst++;
	  *ldst=tmp32;
	  break;
	  case 8:
	  tmp32=*ldst++ * *rdst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 9:
	  tmp32=firstbuf[sampleposfilt%32768]+ *ldst++;
	  *ldst=tmp32;
	  break;
	  case 10:
	  tmp32=firstbuf[sampleposfilt%32768]+ *ldst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 11:
	  tmp32=firstbuf[sampleposfilt%32768]+ *rdst++;
	  *ldst++=tmp32;
	  break;
	  case 12:
	  tmp32=firstbuf[sampleposfilt%32768]+ *rdst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst++=tmp32;
	  break;
	  case 13:
	  tmp32=*ldst++ + *rdst++;
	  *ldst=tmp32;
	  break;
	  case 14:
	  tmp32=*ldst++ + *rdst++;
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 15:
	  tmp32=firstbuf[sampleposfilt%32768]^ *ldst++;
	  *ldst=tmp32;
	  break;
	  }
	  ///	  HERE////////////////////////--->>>>

	  	  if (++delf==SAMPLESPEEDFILT){
	    dirry=newdirf[SAMPLEDIRF]*SAMPLESTEPFILT;
	    count=((sampleposfilt-startfilt)+dirry);
	    //	    if (count<wrapfilt && (sampleposfilt+dirry)>startfilt)
		    if (count<wrapfilt && count>0)
		  {
		    sampleposfilt+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEFILT==0) {
		    startfilt=SAMPLESTARTFILT;wrapfilt=SAMPLEWRAPFILT;
		    if (SAMPLEDIRF==1) sampleposfilt=startfilt; //forwards
		    else sampleposfilt=startfilt+wrapfilt;
		  }

		  else if (VILLAGEFILT==1) {
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  tmp=(ANYSTARTFILT+(anyposfilt%ANYWRAPFILT))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposfilt=SAMPLESTARTFILT+(tmper%SAMPLEWRAPFILT);
		  wrapfilt=0;startfilt=0;
		  }
		  else if (VILLAGEFILT==2) {
		    // advance to next in array based on new start and wrap
		    tmp=VILLAGEFSTEP*villagedirectionf[VILLAGEFDIR];
		    villagefpos+=tmp;
		    tmp=(VILLAGEFSTART+(villagefpos%VILLAGEFWRAP)*2)%villagestackpos; //villagestackpos always +-2
		    startfilt=villager[tmp];
		    wrapfilt=villager[tmp+1];
		    if (wrapfilt==0) wrapfilt=1;
		    if (SAMPLEDIRF==1) sampleposfilt=startfilt;
		    else sampleposfilt=startfilt+wrapfilt;
		  }
		  else {
		  tmp=ANYSTEPFILT*direction[DATADIRF];
		  anyposfilt+=tmp;
		  wrapper=ANYWRAPFILT; 
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  startfilt=buf16[tmp]>>1;
		  tmp=ANYSTEPFILT*directionf[DATADIRF];
		  anyposfilt+=tmp;
		  wrapper=ANYWRAPFILT;
		  if (wrapper==0) wrapper=1;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  wrapfilt=(buf16[tmp]>>1)%SAMPLEWRAPFILT;
		  if (wrapfilt==0) wrapfilt=1;
		  if (SAMPLEDIRF==1) sampleposfilt=startfilt;
		  else sampleposfilt=startfilt+wrapfilt;
		  }
		}
	  delf=0;
	  }
	}

 }

#endif // for LACH

	// 4-out
	//audio_comb_stereo(sz, dst, left_buffer, right_buffer);
 audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#endif // for test eeg
#endif // for straight

}
