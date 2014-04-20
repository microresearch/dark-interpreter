
void audio_morphy(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,
		  float32_t morph, u8 what)
{
	float32_t morph_inv = 1.0 - morph, f_sum;
	int32_t sum;
	
	while(sz--)
	{
	  if (what&1) f_sum = (float32_t)*asrc++ * morph_inv + (float32_t)*bsrc++ * morph;
	  else if (what&2) f_sum = (float32_t)*asrc++ * (float32_t)*bsrc++ * morph;
	  else f_sum = (float32_t)*asrc++ + (float32_t)*bsrc++ * morph; //same as aboive
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


///from last cull for read/write:
/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	cons=(adc_buffer[0]>>5)&15; // TEST granularity of cons
	
	/*	int16_t *buf16 = (int16_t*) datagenbuffer;
	
	for (x=0;x<sz/2;x++){
	  right_buffer[x]=buf16[(x+counter)%32768];
	  }
	  counter+=x;*/


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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	    /* 	 func_runall(stack,buf16,stack_pos); */
	    /* 	    if (++lmer->del==lmer->speed){ */
	    /* //lmer - set lmpwm */
	    /* tmp=lmer->step*direction[lmer->dir]; */
	    /* if ((lmer->pos+tmp)>=lmer->wrap) lmer->pos=(lmer->pos+tmp)%(lmer->wrap+1); */
	    /* else lmer->pos+=tmp; */
	    /* x=(lmer->start+lmer->pos)%32768; */
	    /* //	    tmp=(lmer->start+lmer->pos+1)%32768; */
	    /* //	    	    setlmpwm(buf16[x],buf16[tmp]);  */
	    /* //	    printf("where: %d value: %d\n",x,buf16[x]);  */
	    /* lmer->del=0; */
	    /* } */

		    if (handdown>oldhanddown) sstt++;
		    if (sstt>8) {
		      sstt=0;
		      settingsarray[settingsindex]--;
		      }*/



	    tmp=(adc_buffer[0]<<4); //adjust if 2 slow
	    if (speedwrapper>=tmp){
	      speedwrapper=0;


	      // memory TEST push/pop
	      // sims/ca/machine/pureleak

	      /*	      if (rand()%2==1) {
		stack_pos=func_pop(stackyy,stack_pos);
  		stack_posy=ca_pop(stackyyy,stack_posy);
		cpustackpop(m);
		cpustackpoppp(datagenbuffer);
						
	      }
	      else {
		addr=randi()<<4;
		cpustackpush(m,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
		addr=randi()<<4;
		cpustackpushhh(datagenbuffer,addr,addr+(randi()<<4),1,1);
		stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,1,10);
		stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,1,10); // delay,howmany);
		}*/


    /*
    //2-unhang all [where to re-hang-use a flag]+1 extra option: clocks hang/clocks unhang here
    //question is if really makes sense to unhang _all_
    hangflag=1;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ;
      GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
      GPIO_Init(GPIOB, &GPIO_InitStructure);

  // and what to hang on c=8,10,11,13
    //  GPIO_Init(GPIOC, &GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13;
      GPIO_InitStructure.GPIO_Mode = 0x04; // defined as IN_FLOATING?
      GPIO_Init(GPIOC, &GPIO_InitStructure);
    */

	 /*
	 	 //simulationforstack:	
	 for (x=0;x<STACK_SIZE;x++){
	   stack_pos=func_pushn(stackyy,randi()%NUM_FUNCS,buf16,stack_pos,1,10);
	   //	   stack_pos=func_pushn(stackyy,2,buf16,stack_pos,1,10);
	 	   }

	 // CPUintrev2:
	 	 for (x=0; x<100; x++)
		  {
		    addr=randi()<<4;
		    cpustackpush(m,addr,addr+(randi()<<4),randi()%31,1);//randi()%255);
		    }
	 // pureleak:
		 for (x=0;x<MAX_FRED;x++){//define MAX_FRED 60
		   //			  addr=randi()<<4;
		   // addr=x*1000;
		   addr=randi()<<4;
		   //	  	  cpustackpushhh(datagenbuffer,addr,addr+100,x%31,1);
		   cpustackpushhh(datagenbuffer,addr,addr+(randi()<<4),randi()%31,1);
		   }
	 // CA:
	 for (x=0;x<STACK_SIZE;x++){
	   stack_posy=ca_pushn(stackyyy,randi()%NUM_CA,datagenbuffer,stack_posy,1,10); // delay,howmany);
	   //	   stack_posy=ca_pushn(stackyyy,4,datagenbuffer,stack_posy,1,10); // delay,howmany);
	   }*/

////////////////////
	    //(top down= 2,0,3,4,1):
	    // just to test
	    //	   hardware=adc_buffer[2]>>5;
	    //	   if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	    //	   oldhardware=hardware;
	    //	   ca_runall(stackyyy,datagenbuffer,stack_posy); // some crash
	    //	   testing ADC9/ad620
	    //	   	   int16_t *buf16 = (int16_t*) datagenbuffer;
	    //	   x++;
	    //	   buf16[x%32768]=(adc_buffer[9]<<4)-32768;
	    //	   speedwrapper++;
	    //		    tmp=(adc_buffer[0]<<2);
		    //		    if (speedwrapper>=tmp){
	    //	      speedwrapper=0;
	      //	     func_runall(stackyy,buf16,stack_pos); // simulations
	      //	     ca_runall(stackyyy,datagenbuffer,stack_posy); // CA
	    //		    machine_run(m); //cpu - WRAP own speedTODO
		      //	     	     machine_runnn(datagenbuffer); // pureleak WRAP own speedTODO

			    }

	  //	     dohardwareswitch(adc_buffer[3]>>5,0);


	int16_t *buf16 = (int16_t*) datagenbuffer;

	
	/*
		for (x=0;x<sz/2;x++){

	  right_buffer[x]=buf16[(x+counter)%32768];
	  //    	  right_buffer[x]=(int16_t)datagenbuffer[(x+counter)%32768];
	    //	  right_buffer[x]=(counter+x)*128;
	  }
	*/
	  counter+=x;


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
		  //		  left_buffer[(sz/2)-x]=right_buffer[x];
		  left_buffer[x]=left_buffer[x]*16;
		  //		  right_buffer[x]=0;
	  }

	audio_comb_stereo(sz, dst, left_buffer, right_buffer);
	// but what we hear is right_buffer here, left is for filter feedback
	// figure this out - 
	//	audio_comb_stereo(sz, dst, right_buffer=filter, right_buffer=audio);
	// and left is filter in

