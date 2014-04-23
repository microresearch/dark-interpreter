
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
