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
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];

extern __IO uint16_t adc_buffer[10];
//extern u8 wormdir;
extern u8 villagestackpos;
extern u16 settingsarray[71];
extern u16 villager[129];
extern int16_t newdir[2];
extern int16_t direction[2];
extern int16_t villagedirection[2];
extern int16_t villagedirectionf[2];
extern int16_t villagedirectionw[2];
extern int16_t newdirf[2];
extern int16_t directionf[2];
extern int16_t newdirread[2];
extern int16_t directionread[2];
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
  //	float32_t f_p0, f_p1, tb_l, tb_h, f_i, m;
	u16 tmp=0,tmper,count;
	int16_t tmp16;
	int32_t tmp32;
	u8 x;
	static u16 start=0,startfilt,wrapfilt,wrap,samplepos=0,villagefpos=0,villagewpos=0,villagerpos=0,sampleposfilt=0,anyposfilt=0,anypos=0;
	static u8 del=0,delf=0;
	u8 VILLAGEREAD=0,VILLAGEWRITE=0,VILLAGEFILT=0;
	int16_t dirry=1;
	float temp;

	static u16 anyposread=0,sampleposread=0,wrapread=0,startread=0;
	static u8 delread=0;
	u16 wrapper; 

#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);
	audio_comb_stereo(sz, dst, left_buffer, right_buffer);
#else

	u16 *buf16 = (u16*) datagenbuffer;
	int16_t *buf16int =(int16_t*) datagenbuffer;
	int16_t *firstbuf, *secondbuf;

	///	///	///	///

	// readin villager processing of left into left and right into audio_buffer

	int16_t * ldst=left_buffer;
	int16_t * rdst=right_buffer;

#ifdef LACH

	// firstbuf, secondbuf--->TODO in loop below???

	  if (EFFECTREAD&2) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	  VILLAGEREAD=(EFFECTREAD&12)>>2;

      	for (x=0;x<sz/2;x++){
	  switch(EFFECTREAD>>4){ //TODO make 16
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
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *src++;
	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *src++;
	  tmp32=(*src++)-secondbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *src++;
	  tmp32=secondbuf[sampleposread%32768]-(*src++);
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
 	  case 7:
	  case 6:
	  *src++;
	  tmp32=secondbuf[sampleposread%32768]^(*src++);
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;

	  }
	  /////
	  if (++delread>=SAMPLESPEEDREAD){
	    dirry=newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
	    count=((sampleposread-startread)+dirry);
	    if (count<wrapread && (sampleposread+dirry)>startread)
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

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){

	  ////////////////////////////////////LDST effects also...

	// TODO: put this in loop below or????
	  if (EFFECTREAD&2) firstbuf=buf16int;// TODO could also add in secondbuf here
	else firstbuf=audio_buffer;
	VILLAGEREAD=(EFFECTREAD&12)>>2;
      	for (x=0;x<sz/2;x++){
	  switch(EFFECTREAD>>4){
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
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 3:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=(*src++)+firstbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=(*src++)-firstbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *ldst++ = *src++;
	  *rdst++ = *src; 
	  tmp32=firstbuf[sampleposread%32768]-(*src++);
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  // start of *(src-1)
	  case 6:
	  *ldst++ = *src;
	  tmp32= *(src++) * (*src++);
	  *rdst++ = *src; 
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  *ldst++ = *src;
	  tmp32=firstbuf[samplepos%32768]* *src++;
	  *rdst++ = *src++; 
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *src++;
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 8:
	  *ldst++ = *src;
	  tmp32=*src++ - *src++;
	  *rdst++ = *src; 
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 9:
	  *ldst++ = *src;
	  tmp32=firstbuf[sampleposread%32768]-*src++;
	  *rdst++ = *src; 
	  *src++;
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *ldst++ = *src;
	  tmp32= *src++ -firstbuf[sampleposread%32768];
	  *rdst++ = *src++; 
	  *src++;
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
 	  case 12:
	  *ldst++ = *src;
	  tmp32=*src++ + *src++;
	  *rdst++ = *src; 
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 13:
	  *ldst++ = *src;
	  tmp32=*src++ +firstbuf[sampleposread%32768];
	  *rdst++ = *src; 
	  *src++;
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));	  firstbuf[sampleposread%32768]=tmp32;
	  case 14:
	  *ldst++ = *src;
	  tmp32=*src++ * *src++;
	  *rdst++ = *src; 
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 15:
	  *ldst++ = *src;
	  tmp32=*src++ ^ *src++;
	  *rdst++ = *src; 
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;

	  }
	  	  if (++delread>=SAMPLESPEEDREAD){
		    dirry=newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
	    count=((sampleposread-startread)+dirry);
	    if (count<wrapread && (sampleposread+dirry)>startread)
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
	else  // READIN NO DIG FILTER
	  {
	// TODO: put this in loop below or????

	    if (EFFECTREAD&2) {firstbuf=buf16int;secondbuf=audio_buffer;}
	    else  {secondbuf=buf16int;firstbuf=audio_buffer;}
	    VILLAGEREAD=(EFFECTREAD&12)>>2;
	//	EFFECTREAD=0;
      	for (x=0;x<sz/2;x++){
	  switch(EFFECTREAD>>4){ //TODO inc to 16
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
	  ////////
	  // Effects with/without clipping *, +, -, 
	  case 3:
	  *src++;
	  tmp32=(*src++)*secondbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *src++;
	  tmp32=(*src++)+secondbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *src++;
	  tmp32=(*src++)-secondbuf[sampleposread%32768];
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *src++;
	  tmp32=secondbuf[sampleposread%32768]-(*src++);
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  *src++;
	  tmp32=secondbuf[sampleposread%32768]^(*src++);
	  if (EFFECTREAD&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  firstbuf[sampleposread%32768]=tmp32;
	  }
	  if (++delread>=SAMPLESPEEDREAD){
	    dirry=newdirread[SAMPLEDIRR]*SAMPLESTEPREAD;
	    count=((sampleposread-startread)+dirry);
	    if (count<wrapread && (sampleposread+dirry)>startread)
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
		    villagerpos+=(tmp*2);
		    tmp=(VILLAGERSTART+(villagerpos%VILLAGERWRAP))%villagestackpos; //to cover all directions
		    startread=villager[tmp];
		    wrapread=villager[tmp+1];
		    if (wrapread==0) wrapread=1;
		    if (SAMPLEDIRR==1) samplepos=start;
		    else samplepos=startread+wrapread;
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
	  if (EFFECTWRITE&2) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	VILLAGEWRITE=(EFFECTWRITE&12)>>2;

      	for (x=0;x<sz/2;x++){

	  switch(EFFECTWRITE>>4){ //TODO: extend to 16???
	  case 0:
	  default:
	    mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  case 1:
	  mono_buffer[x]=secondbuf[samplepos%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp32=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	  tmp32=secondbuf[samplepos%32768]^firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 7:
	  tmp32=secondbuf[samplepos%32768]&firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  }
	  /////

	  	  if (++del>=SAMPLESPEED){
	    dirry=newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    if (count<wrap && (samplepos+dirry)>start)
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
		    villagewpos+=(tmp*2);
		    tmp=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP))%villagestackpos; //to cover all directions
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
	}

	/////////////////////////////NO____LACH!!!!!!!!!
#else

	if (digfilterflag&32 || digfilterflag&1){
	int16_t * ldst=left_buffer;

	  ////////////////////////////////////LDST effects also...

	// TODO: put this in loop below or????
	  if (EFFECTWRITE&2) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	VILLAGEWRITE=(EFFECTWRITE&12)>>2;

	//	EFFECTWRITE=0;
      	for (x=0;x<sz/2;x++){
	  	  switch(EFFECTWRITE>>4){ //TODO: extend to 16
	  case 0:
	  default:
	    mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 1:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 2:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	  tmp32=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	  tmp32=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp32=firstbuf[samplepos%32768]* *ldst++;
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	    tmp32=firstbuf[samplepos%32768] - *ldst++;
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 7:
	  tmp32=*(ldst++)+secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  }
	  ////////////////////////--->>>>


	  	  if (++del>=SAMPLESPEED){
	    dirry=newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    if (count<wrap && (samplepos+dirry)>start)
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
		    villagewpos+=(tmp*2);
		    tmp=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP))%villagestackpos; //to cover all directions
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
	}

	}
	else
	  { /// STRAIGHT SANS FILTEROPSSS!!!
	// TODO: put this in loop below or????

	  if (EFFECTWRITE&2) {firstbuf=buf16int;secondbuf=audio_buffer;}
	  else  {secondbuf=buf16int;firstbuf=audio_buffer;}

	  VILLAGEWRITE=(EFFECTWRITE&12)>>2;
		
	//	EFFECTWRITE=0;
      	for (x=0;x<sz/2;x++){
	  switch(EFFECTWRITE>>4){ // make 0-15 TODO!
	  case 0:
	  default:
	    mono_buffer[x]=firstbuf[samplepos%32768];
	  break;
	  case 1:
	    mono_buffer[x]=secondbuf[samplepos%32768];
	  break;

	  // effects with/without clipping *, +, -, 
	  case 2:
	  tmp32=secondbuf[samplepos%32768] * firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	  tmp32=firstbuf[samplepos%32768]+secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[samplepos%32768]-secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	  tmp32=secondbuf[samplepos%32768]-firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	  tmp32=secondbuf[samplepos%32768] ^ firstbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  break;
	  case 7:
	  tmp32=firstbuf[samplepos%32768] & secondbuf[samplepos%32768];
	  if (EFFECTWRITE&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  mono_buffer[x]=tmp32;
	  }

	  //	  VILLAGEWRITE=2; // TESTER!!!!
 
	  if (++del>=SAMPLESPEED){
	    dirry=newdir[SAMPLEDIRW]*SAMPLESTEP;
	    count=((samplepos-start)+dirry);
	    if (count<wrap && (samplepos+dirry)>start)
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
		    villagewpos+=(tmp*2);
		    tmp=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP))%villagestackpos; //to cover all directions
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
	}
	}
#endif
      
	///!!!!!!////////////////////////////////END OF WRITEOUTSSS

	///	///	///	///

#ifndef LACH // as we have no filter!

if (digfilterflag&1){ 

	int16_t * ldst=left_buffer;
	int16_t * rdst=right_buffer;

	  ////////////////////////////////////LDST effects also...
	// TODO: put this in loop below or????
	if (EFFECTFILTER&2) firstbuf=buf16int; // TODO: could also add in secondbuf
	else firstbuf=audio_buffer;
	VILLAGEWRITE=(EFFECTFILTER&12)>>2;

      	for (x=0;x<sz/2;x++){ 
	  
	  switch(EFFECTFILTER>>4){ 
	  case 0:
	  default:
	  *ldst++=firstbuf[sampleposfilt%32768];
	  break;
	  // effects with/without clipping *, +, -, 
	  case 1:
	  *ldst++=*rdst++;
	  break;
	  case 2:
	    *ldst++ =0;
	    break;
	  case 3:
	  tmp32=firstbuf[sampleposfilt%32768]* *ldst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 4:
	  tmp32=firstbuf[sampleposfilt%32768]* *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 5:
	  tmp32=*ldst++ * *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 6:
	  tmp32=firstbuf[sampleposfilt%32768]+ *ldst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 7:
	  tmp32=firstbuf[sampleposfilt%32768]+ *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 8:
	  tmp32=*ldst++ + *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 9:
	  tmp32=firstbuf[sampleposfilt%32768]^ *ldst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 10:
	  tmp32=firstbuf[sampleposfilt%32768]^ *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 11:
	  tmp32=*ldst++ ^ *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 12:
	  tmp32=firstbuf[sampleposfilt%32768]| *ldst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 13:
	  tmp32=firstbuf[sampleposfilt%32768]| *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;
	  case 14:
	  tmp32=*ldst++ | *rdst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  break;

	  ////&

	  case 15:
	  tmp32=firstbuf[sampleposfilt%32768]& *ldst++;
	  if (EFFECTFILTER&1) asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
	  *ldst=tmp32;
	  }
	  ///	  HERE////////////////////////--->>>>

	  	  if (++delf==SAMPLESPEEDFILT){
	    dirry=newdirf[SAMPLEDIRF]*SAMPLESTEPFILT;
	    count=((sampleposfilt-startfilt)+dirry);
	    if (count<wrapfilt && (sampleposfilt+dirry)>startfilt)
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
		    villagefpos+=(tmp*2);
		    tmp=(VILLAGEFSTART+(villagefpos%VILLAGEFWRAP))%villagestackpos; //to cover all directions
		    startfilt=villager[tmp];
		    wrapfilt=villager[tmp+1];
		    if (wrapfilt==0) wrapfilt=1;
		    if (SAMPLEDIRW==1) sampleposfilt=startfilt;
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
#endif // for straight

}
