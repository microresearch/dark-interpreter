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
extern u8 settingsarray[64];

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

void di_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst,u16 edge, u8 step)
{// edge must always be under 32768
  static u16 count=0; 
  	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src;
		count+=step;
		//if (count>=AUDIO_BUFSZ) count=edge; //TODO; stops short/wrap???
		// RE_TEST! jitter can be good with [edge+count]=edge always changes!
		if (count>=AUDIO_BUFSZ) count=(edge+((count-AUDIO_BUFSZ)%(AUDIO_BUFSZ-edge))); // tyring to fix

		//	count=count%(AUDIO_BUFSZ-edge); // THIS WAS ALT:
		//audio_buffer[edge+count] = *src++;
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
	u16 direction[8]={32512,32513,1,257,256,255,32767,32511}; //for 16 bits 32768
	u16 tmp=0,cons;
	u8 complexity=0;
	static u8 anydel=0,del=0,edgedel=0;
	static u16 edger=0,counter=0,start=0,wrap=32768,samplepos=0,anypos=0,edgepos=0;
	u8 x,res; 
	static signed char dir; int16_t dirry=1;

	INSTEP=1; //TESTER!

#ifdef TEST_STRAIGHT
	audio_split_stereo(sz, src, left_buffer, right_buffer);
	
	/*	int16_t *buf16 = (int16_t*) datagenbuffer;
	
	for (x=0;x<sz/2;x++){
	  right_buffer[x]=buf16[(x+counter)%32768];
	  }
	  counter+=x;*/
	audio_comb_stereo(sz, dst, left_buffer, right_buffer);

#else
	//	cons=(adc_buffer[0]>>5)&15; // TEST granularity of cons
	cons=(adc_buffer[0])>>2; // TESTY! granularity of cons
	u16 *buf16 = (u16*) datagenbuffer;
	complexity=adc_buffer[3]>>5; // 7 bits=128 >>2 to 32 

	if (INSTEP==0) INSTEP=1;
	if (SAMPLESTEP==0) SAMPLESTEP=1;
	if (SAMPLESPEED==0) SAMPLESPEED=1;
	if (ANYSTEP==0) ANYSTEP=1;
       	if (ANYSPEED==0) ANYSPEED=1;
	if (EDGESTEP==0) EDGESTEP=1;
	if (EDGESPEED==0) EDGESPEED=1;
	
#ifdef LACH
	edger=adc_buffer[0]<<3;
#else

	INSTEP=1;EDGESTEP=1;EDGESPEED=1;

	res=complexity&3;
	res=0;
	switch(res){
	case 0:
	  edger=0;
	  break;
	case 1:
	  // edger is specific setting from settingarray-TODO
	  edger=EDGERASSETTING; // but should never be higher than 32768 
		  // edger=16000;//TESTER!
	  break;
	case 2:
	  // edger is walker thru datagen using SAMPLEDIR

	  if (++edgedel==EDGESPEED){ 
	    //    	    tmp=EDGESTEP*direction[SAMPLEDIR];
    	    tmp=EDGESTEP*direction[2]; // TESTER!
	    edgepos+=tmp;
	    tmp=edgepos%32768;
	    edger=buf16[tmp]%32768;
	    edgedel=0;
	    }
	  break;
	case 3:
	  // edger is walker thru datagen using wormdir
	  if (++edgedel==EDGESPEED){ 
    	    tmp=EDGESTEP*direction[wormdir];
	    edgepos+=tmp;
	    tmp=edgepos%32768;
	    edger=buf16[tmp]%32768;
	    edgedel=0;
	    }
	  break;
	}

#endif

		di_split_stereo(sz, src, left_buffer, right_buffer,edger, INSTEP);

	// COMPLEXITY TOTAL is: 21 so far- should be 32 with bitwise for ??? - effects
	// COMPLEXITY &3 for edger
	// so for effects????
	
		complexity=complexity>>2;
		ANYSPEED=1;ANYSTEP=1;SAMPLESTEP=1; SAMPLESPEED=1;//TESTER!
		settingsarray[41]=2; // SAMPLEDIR TESTER!	
		settingsarray[42]=2; // ANYDIR TESTER!	
		complexity=4; 

		u16 wrappie;
	switch(complexity){// 32 options
	case 0: // straight out
	for (x=0;x<sz/2;x++){
	  if (++del==(SAMPLESPEED%8)){
	  samplepos+=SAMPLESTEP;
	  wrappie=SAMPLEWRAP;
	  if ((SAMPLESTART+wrappie)>=AUDIO_BUFSZ) wrappie=AUDIO_BUFSZ-SAMPLESTART;
	  if (samplepos>=wrappie) samplepos=samplepos-wrappie; 
	  // if (samplepos>=adc_buffer[0]<<3) samplepos=0; // TESTER!
	  // if (samplepos>=32768) samplepos=0;//TESTER!
	  del=0;
	  }
	  //tmp=samplepos%32768;
	  mono_buffer[x]=audio_buffer[SAMPLESTART+samplepos];
	  //mono_buffer[x]=audio_buffer[tmp]; // TESTER!
	}
	break;
	/////////
	case 1: // direction and no start/wrap - should be or no: TODO?
	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    tmp=SAMPLESTEP*direction[SAMPLEDIR];
	  samplepos+=tmp;
	  del=0;
	  }
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	break;
	/////////
	case 2: // AGAIN TODO any wrap - wormdir
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	  tmp=SAMPLESTEP*direction[wormdir];
	  samplepos+=tmp;
	  del=0;
	  }
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	break;
	case 3:	  //3/walkdatagenasdirwalk (???)
 	for (x=0;x<sz/2;x++){
	  //walk any 
	    if (++anydel==ANYSPEED){
    	    tmp=ANYSTEP*direction[ANYDIR];
	    anypos+=tmp;
	    anydel=0;
	    }
	    if (++del==SAMPLESPEED){
	      tmp=SAMPLESTEP*direction[datagenbuffer[tmp]%8];
	      samplepos+=tmp;
	      del=0;
	    }
	  mono_buffer[x]=audio_buffer[samplepos%32768];
	}
	  break;
	case 4:
	  //4/walk datagen dir as grains
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    if (SAMPLEDIR&1) dir=1;
	    else dir=-1;
	    dirry=(int16_t)dir*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>start)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  tmp=ANYSTEP*direction[ANYDIR];
		  anypos+=tmp;
		  tmp=anypos%32768;
		  start=buf16[tmp]>>1;
		  tmp=ANYSTEP*direction[ANYDIR];
		  anypos+=tmp;
		  tmp=anypos%32768;
		  wrap=buf16[tmp]>>1;

	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
	      else wrap=start-wrap;
	      wrap=wrap%cons;
	      if ((start+wrap)>=AUDIO_BUFSZ) wrap=AUDIO_BUFSZ-start; 
	      if (SAMPLEDIR&1) samplepos=start;
	      else samplepos=start+wrap;
		}
	  del=0;
	  }
	  //	  mono_buffer[x]=audio_buffer[(start+samplepos)%32768]; INCORRECT as need go back to start????
	  mono_buffer[x]=audio_buffer[samplepos];
	}
	  break;
	/////////
	case 5:	  //5/walk datagen dir as grains with direction: TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    dirry=(int16_t)dir*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      start=buf16[tmp]>>1;
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) {
		wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
		dir=1;
	      }
	      else {
		wrap=start-wrap;
		dir=-1;
	      }
	      //	      start=start%32768;wrap=wrap>>cons; 
	      wrap=wrap>>cons; 
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=audio_buffer[(start+samplepos)%32768];
	}
	/////////
	case 6://5/walk datagen dir as grains with wormdir: TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    if (wormdir&1) dirry=1;
	    else dirry=-1;
	    dirry=(int16_t)dirry*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      start=buf16[tmp]>>1;
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
	      else wrap=start-wrap;
	      //	      start=start%32768;wrap=wrap>>cons;
	      wrap=wrap>>cons; 
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=audio_buffer[(start+samplepos)%32768];
	}
	break;	  /////
	case 7: //5/walk datagen dir as grains with wormdir and wormdir: TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    if (wormdir&1) dirry=1;
	    else dirry=-1;
	    dirry=(int16_t)dirry*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[wormdir];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      start=buf16[tmp]>>1;
	      tmp=ANYSTEP*direction[wormdir];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
	      else wrap=start-wrap;
	      //	      start=start%32768;wrap=wrap>>cons;
	      wrap=wrap>>cons; 
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=audio_buffer[(start+samplepos)%32768];
	}
	break;
	  //////
	case 8:
	  //5/walk datagen dir as samples-wormdir
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==ANYSPEED){ 
    	    tmp=ANYSTEP*direction[wormdir];
	    anypos+=tmp;
	    tmp=anypos%32768;
	    anydel=0;
	    }
	  mono_buffer[x]=audio_buffer[buf16[tmp]%32768];
	}
	break;
	case 9:
	  //5/walk datagen dir as samples
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==ANYSPEED){ 
	    tmp=ANYSTEP*direction[ANYDIR];
	  //	  tmp=direction[3];
	    anypos+=tmp;
	    tmp=anypos%32768;
	    anydel=0;
	    	  }
	  mono_buffer[x]=audio_buffer[buf16[tmp]%32768];
	}
	break;
	case 10:
	  //5/walk datagen dir as samples
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==ANYSPEED){ 
    	    tmp=ANYSTEP*direction[ANYDIR];
	    anypos+=tmp;
	    tmp=anypos%32768;
	    anydel=0;
	  }
	  mono_buffer[x]=audio_buffer[(buf16[buf16[tmp]%32768]%32768)];
	}
	break;
	/////11+ is datagen
	case 11:
	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	  samplepos+=SAMPLESTEP;
	  //    if (samplepos>=SAMPLEWRAP) samplepos=0;
	  	  if (samplepos>=32000) samplepos=0;	  //TESTER!
	  del=0;
	  }
	  //	  mono_buffer[x]=buf16[(SAMPLESTART+samplepos)%32768];
	  	  mono_buffer[x]=buf16[(samplepos)%32768]; 	  //TESTER!

	}
	break;
	/////////
	case 12:
	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	  tmp=SAMPLESTEP*direction[SAMPLEDIR];
	  samplepos+=tmp;
	  del=0;
	  }
	  mono_buffer[x]=buf16[samplepos%32768];
	}
	break;
	/////////
	case 13:
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	  tmp=SAMPLESTEP*direction[wormdir];
	  samplepos+=tmp;
	  del=0;
	  }
	  mono_buffer[x]=buf16[samplepos%32768];
	}
	break;
	case 14:
	  //3/walkdatagenasdirwalk 
 	for (x=0;x<sz/2;x++){
	  //walk any 
	    if (++anydel==ANYSPEED){
    	    tmp=ANYSTEP*direction[ANYDIR];
	    anypos+=tmp;
	    anydel=0;
	    }
	    if (++del==SAMPLESPEED){
	      tmp=SAMPLESTEP*direction[datagenbuffer[tmp]%8];
	      samplepos+=tmp;
	      del=0;
	    }
	  mono_buffer[x]=buf16[samplepos%32768];
	}
	  break;
	case 15:
	  //15/walk datagen dir as grains -- TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    if (SAMPLEDIR&1) dirry=1;
	    else dirry=-1;
	    dirry=(int16_t)dirry*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      start=buf16[tmp]>>1;
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
	      else wrap=start-wrap;
	      //	      start=start%32768;wrap=wrap>>cons; 
	      wrap=wrap>>cons;  //constrain sample wrap size//TODO complex/speed?
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=buf16[(start+samplepos)%32768];
	}
	  break;
	/////////
	case 16:
	  //16/walk datagen dir as grains -- TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    dirry=(int16_t)dir*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      start=buf16[tmp]>>1;
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) {
		wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
		dir=1;
	      }
	      else {
		wrap=start-wrap;
		dir=-1;
	      }
	      //	      start=start%32768;wrap=wrap>>cons;  //constrain sample wrap size//TODO complex/speed?
	      wrap=wrap>>cons; 
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=buf16[(start+samplepos)%32768];
	}
	/////////
	case 17:
	  //17/walk datagen dir as grains -- TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    if (wormdir&1) dirry=1;
	    else dirry=-1;
	    dirry=(int16_t)dirry*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768>>1;
	      start=buf16[tmp];
	      tmp=ANYSTEP*direction[ANYDIR];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
	      else wrap=start-wrap;
	      //	      start=start%32768;wrap=wrap>>cons;  //constrain sample wrap size//TODO complex/speed?
	      wrap=wrap>>cons; 
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=buf16[(start+samplepos)%32768];
	}
	break;	  /////
	case 18:
	  //18/walk datagen dir as grains -- TODO: alter audio section as above once tested!!!!
 	for (x=0;x<sz/2;x++){
	  if (++del==SAMPLESPEED){
	    if (wormdir&1) dirry=1;
	    else dirry=-1;
	    dirry=(int16_t)dirry*SAMPLESTEP;
	    if ((samplepos+dirry)<wrap && (samplepos+dirry)>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
	      tmp=ANYSTEP*direction[wormdir];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      start=buf16[tmp]>>1;
	      tmp=ANYSTEP*direction[wormdir];
	      anypos+=tmp;
	      tmp=anypos%32768;
	      wrap=buf16[tmp]>>1;

	      if (wrap>start) wrap=wrap-start; //or grain is backwards - alter dir/AS OPTION!
	      else wrap=start-wrap;
	      //	      start=start%32768;wrap=wrap>>cons;  //constrain sample wrap size//TODO complex/speed?
	      wrap=wrap>>cons; 
	      if (wrap<1) wrap=2;
	      if (SAMPLEDIR&1) samplepos=0;
	      else samplepos=wrap;
		}
	  del=0;
	  }
	  mono_buffer[x]=buf16[(start+samplepos)%32768];
	}
	break;
	  //////
	case 19:
	  //5/walk datagen dir as samples-wormdir
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==ANYSPEED){ 
    	    tmp=ANYSTEP*direction[wormdir];
	    anypos+=tmp;
	    tmp=anypos%32768;
	    anydel=0;
	    }
	  mono_buffer[x]=buf16[buf16[tmp]%32768];
	}
	break;
	case 20:
	  //5/walk datagen dir as samples
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==ANYSPEED){ 
    	    tmp=ANYSTEP*direction[ANYDIR];
	    anypos+=tmp;
	    tmp=anypos%32768;
	    anydel=0;
	  }
	  mono_buffer[x]=buf16[buf16[tmp]%32768];
	}
	break;
	case 21:
	  //5/walk datagen dir as samples
 	for (x=0;x<sz/2;x++){
	  //walk any 
	  if (++anydel==ANYSPEED){ 
    	    tmp=ANYSTEP*direction[ANYDIR];
	    anypos+=tmp;
	    tmp=anypos%32768;
	    anydel=0;
	  }
	  mono_buffer[x]=buf16[(buf16[buf16[tmp]%32768]%32768)];
	}
	break;

	} // end case

#ifndef LACH	



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
