
/* TODO:

///older:::

test leave all hanging= GPIO_Mode_IN_FLOATING

- maybe do all hardware init/setup in one go - as maybe problem with so many
- structures try also with BSRR or with GPIO_WriteBit(GPIO_TypeDef*
- GPIOx, uint16_t GPIO_Pin, BitAction BitVal);

- lmpwm is ringing (1-1000)

- clean up for functions below and test all combinations:

  functions:
- justfilter
- justdistortion
- filterthendistortion
- distortionthenfilter
- distortioninfilter on/off (could be done fast also)
- setfiltfeedbackpath - digital/lm13700
- feedback 
- setfloating (enum list)
- setallfloating (or how to set diff ones but not just one)

- setlmpwm- setmaximpwm
- set40106power

////

- test leave all hanging= GPIO_Mode_IN_FLOATING

*/


/////////////////////////////

latest knob business:


      if (mirror<128 && tmphardware!=effects && tmphardware!=effects-1 && tmphardware!=effects+1){
	hardware=tmphardware; // handled all below 
	thardware=tmphardware;
	effects=200; // never near but set to avoid gap...
	    }
	      else if (mirror >128 && tmphardware!=thardware && tmphardware!=thardware-1 && tmphardware!=thardware+1 && tmphardware!=effects){
		effects=tmphardware; //7 bits
		thardware=200;
		if (mirror<160){
		  settingsarray[51]=effects<<9; // EFFECTREAD
		}
		else if (mirror<192){
		  settingsarray[52]=effects<<9; // EFFECTWRITE
		}
		else if (mirror<224){
		  settingsarray[53]=effects<<9; // EFFECTFILT
		}
		else if (mirror<240){
		  settingsarray[51]=settingsarray[52];settingsarray[53]=settingsarray[52];
		}
		else  {
		  settingsarray[51]=0;
		  settingsarray[52]=0;
		  settingsarray[53]=0;
		}
		}
#else
      // TODO: REDO THIS as above when have tested!
      // TESTY as might need to SMOOTH!
      //      tmphardware=tmphardware<<3; // 15 bits
      /*      if (mirror<128 && tmphardware!=effects && tmphardware!=effects-1 && tmphardware!=effects+1){
	SAMPLEWRAP=tmphardware<<3; // handled all below 
	hardware=tmphardware;
	thardware=tmphardware;
	effects=200; // never near but set to avoid gap...
	    }
	      else if (mirror >128 && tmphardware!=thardware && tmphardware!=thardware-1 && tmphardware!=hardware+1 && tmphardware!=effects){
		effects=tmphardware;
		thardware=200;
		if (mirror<160){
		  EFFECTREAD=effects;
		}
		else if (mirror<192){
		  EFFECTWRITE=effects;
		}
		else if (mirror<224){
		  EFFECTFILTER=effects;
		}
		else if (mirror<240){
		  EFFECTREAD=EFFECTWRITE;EFFECTFILTER=EFFECTWRITE;
		}
		else  {
		  //		  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;
		  EFFECTREAD=effects;EFFECTREAD=EFFECTWRITE;EFFECTFILTER=EFFECTREAD;
		}
		}*/
#endif      	
      // HERE!
      // SETTINGSARRAY
      hardware=adc_buffer[FIRST]>>5; //TESTY!!
      tmpsettings=adc_buffer[THIRD]>>6; // 0-64 ???
      tmper=64;settings=36; // TESTY!!
      setted=fingervalup(settingsarray[settings],tmper);
      settingsarray[settings]=setted; 
      //tmper=64;settings=12; // TESTY!!
      //      setted=fingervalup(settingsarray[settings],tmper);
      //      settingsarray[settings]=setted; 

      //      EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;//TESTER!
      if (mirror<128 && tmpsettings!=settingsops && tmpsettings!=settingsops-1 && tmpsettings!=settingsops+1){
	settings=tmpsettings; 
	settingsops=200;
	// TODO: if is 0 we use fingers left/right!
	// but then we also need to set 0 settings - [settings-1]
	//	if (settings<25) tmper=8; else tmper=2;//?????

	if (settings<46){ // TESTY was 54
	  //	  	  tmper=64;settings=36; // TESTY!!
	  //TODO: wrap here means that we get 16 bits coming in
	  //	  setted=fingervalup(settingsarray[settings],tmper);
	  //	  settingsarray[settings]=setted; 
	  //	  settingsarray[settings]=adc_buffer[FIFTH];
	}
	else
	  {
	// TODO: what of directions???
	  }
      }
      else if (mirror>128 && tmpsettings!=settings && tmpsettings!=settings-1 && tmpsettings!=settings+1){
	settingsops=tmpsettings; // 0-64???
	settings=200;
      // operations which are set and act continuously elsewhere
      // operations which just take place here: contract, expand, shift a region, the region
    }

      // BLACK STACKS AND EXTRA KNOB

      tmpstack=adc_buffer[FOURTH]>>6; // 0-64???

      if (mirror<128 && tmpstack!=stackops && tmpstack!=stackops-1 && tmpstack!=stackops+1){
	stack=tmpstack;
	stackops=200;

	///KEY!!!TODO!!!
	// finger up/down to choose stack and value
	// tmpstack changes value - or vice versa

	// STACKSTART, STACKWRAP, STACKMUCH
	// ((stack_pos+stack_posy)*3) = maximum 32x3=96
	// +villager (max 64)

	// push/pop (push<x pop>x) and set which CPU/grain is pushed or
	// popped [and with what buffer and settings?]
	// previous code was based on mirror to choose which (setting we can inherit from settings.h)
	//- simulation: stack[max=stack_pos]: start, wrap, howmuch
	//- CAforstack: stack[max=stack_posy]: start, wrap, howmuch
	//- villager: start, wrap [][]

	    }
      else if (mirror>128 && tmpstack!=stack && tmpstack!=stack-1 && tmpstack!=stack+1){
	stackops=tmpstack; //0-64???
	stack=200;
      // operations which are set and act continuously elsewhere
      // operations which just take place here: contract, expand, shift a region, the region
    }

      // FIFTH KNOB is spare - use instead of finger up/down = override if changes
      // on mirror we use as mirror/foldback settings with finger up/down - see below
      // with mirror we also have free finger left/right to use 


      /////////// TODO: maintain those operations flagged above
      // and where we have these settings from = finger and fifth knob as above

      //       // ops on region of settingsarray:    
      // none
      // mirror from stack(region), from datagen to region
      // infect, randi across (if TENE)

      // srcstart,srcwrap, deststart,destwrap, speed, buffer
      // none
      //      // ops on region of stacks:    
      // mirror from settings(region), from datagen to region
      // infect,  randi across (if TENE)

      // srcstart,srcwrap, deststart,destwrap, speed, buffer




/////////////////////////////

      //      u8 hh; u8 hard[64];u16 thardware;
      // moving average- write into circular buffer say 64
      /*hard[hh%64]=adc_buffer[0]>>5;
	hh++;
	for (x=0;x<64;x++){thardware+=hard[x];}
	tmphardware=thardware>>8; // divide by 64
	thardware=0;*/
      //      hardware=tmphardware;
       // this mirroring kind of works!

      // finger tests
      //      hardware=fingerval(hardware);//>>9; // 16 bits ideally to 7
      //      hardware=adc_buffer[2]>>5;


///former hardware
      /////////////////////////////////////
      // 4-hardware operations
      // do hardware datagen walk into hdgen (8 bit) if flagged
      if (digfilterflag&16){ // if we use hdgen at all
	if (HDGENERSPEED==0) HDGENERSPEED=1;
	if (HDGENERSTEP==0) HDGENERSTEP=1;
	if (++hdgenerdel==HDGENERSPEED){
	  hdgenerpos+=(HDGENERSTEP*hddir[HDGENERDIR]);
	  wrapper=HDGENERWRAP; // can go 65536
	  if (wrapper==0) wrapper=1;
	  tmp=(HDGENERSTART+(hdgenerpos%wrapper))%32768; //to cover all directions
	  dohardwareswitch(hardware,HDGENERBASE+(datagenbuffer[tmp]%HDGENERCONS));
	  hdgenerdel=0;
	}

      }
      else
	{
	  if (hardware!=oldhardware) dohardwareswitch(hardware,0);
	  oldhardware=hardware;
	}
	     		   
      // 3 datagenclocks->40106/lm/maxim - filterflag as bits as we also need signal which clocks we		     		     
      if (digfilterflag&2){
	if (F0106ERSPEED==0) F0106ERSPEED=1;
	if (F0106ERSTEP==0) F0106ERSTEP=1;
	if (++f0106erdel==F0106ERSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  f0106erpos+=(F0106ERSTEP*dir40106[F0106ERDIR]);
	  wrapper=F0106ERWRAP;
	  if (wrapper==0) wrapper=1;
	  tmp=(F0106ERSTART+(f0106erpos%wrapper))%32768; //to cover all directions
	  set40106pwm(F0106ERBASE+(buf16[tmp]%F0106ERCONS)); // constrain all to base+constraint

	  f0106erdel=0;
	}
      }
	  
      if (digfilterflag&4){
	if (LMERSPEED==0) LMERSPEED=1;
	if (LMERSTEP==0) LMERSTEP=1;
	if (++lmerdel==LMERSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  lmerpos+=(LMERSTEP*lmdir[LMERDIR]);
	  wrapper=LMERWRAP;
	  if (wrapper==0) wrapper=1;
	  x=(LMERSTART+(lmerpos%wrapper))%32768; //to cover all directions

	  lmerpos+=(LMERSTEP*lmdir[LMERDIR]);
	  wrapper=LMERWRAP;
	  if (wrapper==0) wrapper=1;
	  tmp=(LMERSTART+(lmerpos%wrapper))%32768; //to cover all directions
	  setlmpwm(LMERBASE+(buf16[x]%LMERCONS),LMERBASE+(buf16[tmp]%LMERCONS)); 
	  lmerdel=0;
	}

      }
	  
      if (digfilterflag&8){
	if (MAXIMERSPEED==0) MAXIMERSPEED=1;
	if (MAXIMERSTEP==0) MAXIMERSTEP=1;
	if (++maximerdel==MAXIMERSPEED){
	  // when wrapper changes we need to redo direction array!!!
	  maximerpos+=(MAXIMERSTEP*mxdir[MAXIMERDIR]);
	  wrapper=MAXIMERWRAP;
	  if (wrapper==0) wrapper=1;
	  x=(MAXIMERSTART+(maximerpos%wrapper))%32768; //to cover all directions
	  maximerdel=0;
	  setmaximpwm(MAXIMERBASE+(buf16[x]%MAXIMERCONS));
	}
      }


///////


      // 0-4 top down
      // 0=mirror left///right selector and ops across all knobbed settings/feedbacks
      // 1=hardware///1=filterops/effects
      // 2=push///2=pull-datagen ops and actions _or_ 2=settings Y
      // 3=speed///3=micro-macro
      // 4=settingsX///fingers on 0/dir//4=ops on settings array/foldbacks/feedbacks


#ifdef TENE

	      // TENE: 2,0,3,4,1 
	      //0-mirror settings
	    // smoothing
	      mirror=adc_buffer[2]>>4; // 8 bits or less?
	      // how we divide up mirror?
	      // what are mirror ops and how they make sense?
	      // and that these could be in each setting eg. for effects:

	      //1-hardware
	      // set hardware for below...
	      hardware=adc_buffer[0]>>5; //7 bits but what of jitter?counter???
		
	      if (mirror<128 && tmphardware!=effects){
		//		hardware=tmphardware; // handled all below 
		//TODO: but is change when move mirror just down to jitter?
		hardware=tmphardware;
	    }
	      else if (tmphardware!=hardware && tmphardware!=effects){
		effects=tmphardware;
	      //1-filterops/effects
		// EFFECTREAD, EFFECTWRITE, EFFECTFILTER upto 128 each?
		if (mirror<160){
		  EFFECTREAD=effects;
		}
		else if (mirror<192){
		  EFFECTWRITE=effects;
		}
		else if (mirror<224){
		  EFFECTFILTER=effects;
		}
		else if (mirror<240){
		  EFFECTREAD=EFFECTWRITE;EFFECTFILTER=EFFECTWRITE;
		}
		else  {
		  EFFECTREAD=0;EFFECTWRITE=0;EFFECTFILTER=0;
		}
	      }
	      }
   
	      ////////////
	      //2-push/pop
	      //knob as 0 to push, 255 to pop with settings divided inbetween
	      //type(which,func,exetype)//howmuch//start//wrap(check>???) 	   	   
	      //also which buffer to attack as bitwise option
	      
		 /*	      tmppushpull=adc_buffer[3]>>5; //7 bits but what of jitter?counter???
	      if (mirror<128){// && tmppushpull!=Ysettings && pushpull!=tmppushpull){
		pushpull=tmppushpull;
		which=mirror>>3; // 4 bits as less than 128
		func=randi(); // TESTY!
		//		start=randi();
		//		wrap=randi();
		if (pushpull<16) {
		//cpustackpush=31,hhh=31,numca=9,numfuncs=33,exetype=4 (totals)

		// first bit of which is buffer indication
		  //		  which=which^1;
		  switch(which>>1){ // 3 bits
		case 0:
		if (which&1) buff=audio_buf;
		else buff=datagenbuffer;
		cpustackpush(m,buff,start,(start+wrap)%32768,func%31,howmany); // in this case delay is howmany
		break;
		case 1:
		if (which&1) buff=audio_buf;
		else buff=datagenbuffer;
		cpustackpushhh(buff,start,start+wrap,func%31,howmany); // in this case delay is howmany
		break;
		case 2:
		if (which&1) buff=audio_buf;
		else buff=datagenbuffer;
		stack_posy=ca_pushn(stackyyy,func%NUM_CA,buff,stack_posy,howmany,start,start+wrap);
		break;
		case 3:
		if (which&1) buf=audio_buffer;
		else buf=buf16;
		stack_pos=func_pushn(stackyy,func%NUM_FUNCS,buf,stack_pos,howmany,start,start+wrap);
		break;
		case 4:
		exenums=exestackpush(exenums,exestack,0); //exetype=0-3;
		break;
		case 5:
		exenums=exestackpush(exenums,exestack,1); //exetype=0-3;
		break;
		case 6:
		exenums=exestackpush(exenums,exestack,2); //exetype=0-3;
		break;
		case 7:
		exenums=exestackpush(exenums,exestack,3); //exetype=0-3;
		}
		}
		else if (pushpull>112) {
		which=mirror>>3; // 4 bits as less than 128
		switch(which>>1){//3 bits
		case 0:
		cpustackpop(m);
		break;
		case 1:
		if (which&1) buff=(u8*)audio_buffer;
		else buff=datagenbuffer;
		cpustackpoppp(buff);
		break;
		case 2:
		stack_posy=ca_pop(stackyyy,stack_posy);
		break;
		case 3:
		stack_pos=func_pop(stackyy,stack_pos);
		break;
		  case 4:
		  default:
		exenums=exestackpop(exenums,exestack);
	       
		}
		}
				else{
		  		pushsetted=(pushpull>>5);
		  handup=adc_buffer[6]>>8; //4 bits //adc6???
		    if (handup>oldhandup) sstt++;
		    else sstt=0;
		    if (sstt>2){
		      sstt=0;
		      tempsetting++;
		    }
		    oldhandup=handup;

		    // down- as long as [8] > lastdownsetting decrement value
		    handdown=adc_buffer[8]>>8; //4 bits
		    if (handdown>oldhanddown) ttss++;
		    else ttss=0;
		    if (ttss>2){
		      ttss=0;
		      tempsetting--;
		    }
		    oldhanddown=handdown;
		    pushsetting[pushsetted]=tempsetting;
		    }

				
	// inbetween we have settings by finger?!
		// for:   u8 which, func;u16 start,wrap,howmany;
	      }
	      //2=settings Y
	      else if (tmppushpull!=Ysettings && tmppushpull!=pushpull){
		Ysettings=tmppushpull;
		//		settingsarray[settings]=Ysettings;
		}*/
	      //3-speed -global action on all speed settings
	      tmpspeed=adc_buffer[4]>>4; //8 bits but what of jitter?counter???
	      if (mirror<128 && tmpspeed!=micromacro){
		/*		speed=tmpspeed; // all 8 bit
		if (speed==0) speed=1;
	      LMERSPEED=LMERSPEED%speed;
	      MAXIMERSPEED=MAXIMERSPEED%speed;
	      F0106ERSPEED=F0106ERSPEED%speed;
	      HDGENERSPEED=HDGENERSPEED%speed;
	      SAMPLESPEED=SAMPLESPEED%speed;
	      SAMPLESPEEDREAD=SAMPLESPEEDREAD%speed;
	      SAMPLESPEEDFILT=SAMPLESPEEDFILT%speed;
	      LEAKSPEED=LEAKSPEED%speed;
	      MACHINESPEED=MACHINESPEED%speed;*/
		// TODO: mirror = all speeds // what of expanding speeds
		}
	      else if (tmpspeed!=speed && tmpspeed!=micromacro){
		micromacro=tmpspeed;
		//3-micro-macro - constraints and expansion actions

	      // here also villageread, villagewrite, villagefilt=0-2 = bits alongside?
	      // constraints

	      /*
		# define HDGENERCONS (settingsarray[49]) // less than 255
		# define LMERCONS (settingsarray[51])
		# define F0106ERCONS (settingsarray[53])
		# define MAXIMERCONS (settingsarray[55])

		and constrain/expand the wraps:
		#define LMERWRAP (settingsarray[10])//when wrapper changes we need to redo direction array!!!
		#define MAXIMERWRAP (settingsarray[11])
		#define F0106ERWRAP (settingsarray[12])
		#define HDGENERWRAP (settingsarray[13])
		#define SAMPLEWRAP (settingsarray[14])
		#define SAMPLEWRAPREAD (settingsarray[15])
		#define SAMPLEWRAPFILT (settingsarray[16])
		#define ANYWRAPREAD (settingsarray[17])
		#define ANYWRAP (settingsarray[18])
		#define ANYWRAPFILT (settingsarray[19])
	      */
		// TODO: re-eval dirr arrays
		// TODO: mirror = all wraps

		if (mirror<192){
		  //expand <<

		}
		else
		  {
		    //constrain %

		  }
	      }
	      //4-settings-all finger at 0
	      tmpsettings=adc_buffer[1]>>6; //6bits ->64settings but what of jitter?counter???

	      if (mirror<128 && tmpsettings!=foldback){
		//		settings=tmpsettings;
	      }
	      else if (tmpsettings!=settings){
	      //4-foldback
		foldback=tmpsettings;
		// walker for foldback
		// also adc_buffer[9] across foldback
		// flatten all
		// where to place re-eval for dirs
	      }
#endif

#ifdef SUSP
	      // SUSP (and LACH): 3,0,2,4,1
	      // as above but different knob alignments
#endif

#ifdef LACH
	      // SUSP (and LACH): 3,0,2,4,1
	      // there is no hardware!

	      //1- 1-
	      // depending on quarter mirror as 2x start.edge read.write
	      /*
		SAMPLESTARTREAD=
		SAMPLEWRAPREAD=
		SAMPLEWRAP=
		SAMPLESTART=
	       */

#endif
