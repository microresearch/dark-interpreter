  u8 del=0,attache=0,machine_count=0,tmpacht=0,villagerdest,spd; 


// from main.c

// hardware bit which was moved to audio.c

#ifndef LACH

      tmphardware=adc_buffer[FIFTH];
      hwalksel=(tmphardware>>6)&1;// if we walk HW at all???
      HARDWARE=tmphardware>>7; // 5 bits now!

      /// HW as attached/settings or as walkers - AUG

      if (hwalksel){
	set40106pwm(F0106ERCONS);
	//set40106pwm(0); // TESTY!

      if (digfilterflag&4){
	setlmmmpwm(LMERCONS);
      }

      if (digfilterflag&8){
	setmaximpwm(MAXIMERCONS);
      }
      }//hwalksel

      else{

      if (++hwdel>=HWSPEED){
	hwdel=0;

	if (wormflag[0]) hwpos+=direction[wormdir];
	else hwpos+=(HWSTEP*direction[HWDIR]);
	tmphw=HWSTART+(hwpos%HWWRAP); //to cover all directions
	tmper=((adc_buffer[THIRD]>>4)%16)<<8; // 8 bits
      //      set40106pwm(F0106ERBASE+(buf16[(tmp+F0106EROFFSET)%32768]%F0106ERCONS)); // constrain all to base+constraint
	tmp=F0106ERCONS-F0106ERBASE-tmper;
	if (tmp==0) tmp=1;
	set40106pwm(F0106ERBASE+tmper+(buf16[(tmphw+F0106EROFFSET)%32768]%tmp)); // constrain all to base+constraint - what is range? now want 0->2048 // 15 bits to 11 bits - now in cons AUG AUG
	//	set40106pwm(0); // TESTY!

      tmp=LMERCONS-LMERBASE;
	if (tmp==0) tmp=1;
      if (digfilterflag&4){
	//	setlmpwm(LMERBASE+(buf16[(tmphw+LMEROFFSET)%32768]%tmp),LMERBASE+(buf16[(tmphw+LMEROFFSETTWO)%32768]%tmp)); 
	//		setlmpwm(adc_buffer[FOURTH],adc_buffer[THIRD]<<3);//TESTY!=14 bits

	setlmmmpwm(LMERBASE+(buf16[(tmphw+LMEROFFSET)%32768]%tmp)); // AUGUST!
      }
	  
      if (digfilterflag&8){
	tmp=MAXIMERCONS-MAXIMERBASE;
	if (tmp==0) tmp=1;
	setmaximpwm(MAXIMERBASE+(buf16[(tmphw+MAXIMEROFFSET)%32768]%tmp)); // constrain CONS rather AUG!!!
	//	setmaximpwm(adc_buffer[FIFTH]<<2);//TESTY!=14 bits
	//	setmaximpwm(255);//TESTY!=14 bits

      }
            }
      }/////
      
      if (digfilterflag&16){
	dohardwareswitch(HARDWARE,datagenbuffer[tmphw]%HDGENERCONS);
      }
      else
	{
	  dohardwareswitch(HARDWARE,0);
	}


#endif //notLACH



/////////
	
#ifdef LACH
      settingsarray[6]=adc_buffer[FIFTH]<<4; // 16 bits SAMPLEWRAP!!!
      settingsarrayattached[6]=0;
#endif

      ////settingssss
      
      for (x=0;x<SETSIZE;x++){
	switch(settingsarrayattached[x]){
	case 0:
	  break;
	case 1:
	  settingsarray[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  printf("x %d where %d setted %d\n",x,((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768,settingsarray[x]);
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  settingsarray[x]=adc_buffer[9]<<4;
#else
	  settingsarray[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  settingsarray[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  settingsarray[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

     
      for (x=0;x<(STACKPOS*4);x++){  // AUG. fixed before was not *4
	switch(stackerattached[x]){
	case 0:
	  break;
	case 1:
	  stacker[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  coo++;
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG! 
	  break;
	case 2:
#ifdef TENE
	  stacker[x]=adc_buffer[9]<<4;
#else
	  stacker[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stacker[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stacker[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<(STACKPOSY*4);x++){ // AUG. fixed before was not *4
	switch(stackeryattached[x]){
	case 0:
	  break;
	case 1:
	  stackery[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  coo++;
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  stackery[x]=adc_buffer[9]<<4;
#else
	  stackery[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  stackery[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  stackery[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<VILLAGESTACKPOS;x++){
	switch(villagerattached[x]){
	case 0:
	  break;
	case 1:
	  villager[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768];
	  //	  coo++; 
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  villager[x]=adc_buffer[9]<<4;
#else
	  villager[x]=adc_buffer[DOWN]<<4;
#endif
	  break;
	case 3:
	  villager[x]=adc_buffer[THIRD]<<4;
	  break;
	case 4:
	  villager[x]=adc_buffer[SECOND]<<4; // where?
	  break;
	}
      }

      for (x=0;x<VILLAGESTACKPOS/2;x++){
	switch(villagereffattached[x]){
	case 0:
	  break;
	case 1:
	  village_effects[x]=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768]>>12;//4 bits
	  //	  coo++; 
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  village_effects[x]=adc_buffer[9]>>8;
#else
	  village_effects[x]=adc_buffer[DOWN]>>8;
#endif
	  break;
	case 3:
	  village_effects[x]=adc_buffer[THIRD]>>8;
	  break;
	case 4:
	  village_effects[x]=adc_buffer[SECOND]>>8; // where?
	  break;
	}
      }

      for (x=0;x<THREADCOUNT;x++){
	switch(cpuattached[x]){
	case 0:
	  break;
	case 1:
	  m->m_threads[x].m_CPU=buf16[((FOLDOFFSET>>1)+(coo%((FOLDTOP>>1)+1)))%32768]>>11;
	  //	  coo++;
	  if (attache++ > ATTACHSPEED) { coo++; attache=0;} // AUG!
	  break;
	case 2:
#ifdef TENE
	  m->m_threads[x].m_CPU=adc_buffer[9]>>7;//cpu - 5 bits
#else
	  m->m_threads[x].m_CPU=adc_buffer[DOWN]>>7;//cpu - 5 bits
#endif
	  break;
	case 3:
	  m->m_threads[x].m_CPU=adc_buffer[THIRD]>>7;
	  break;
	case 4: // never used...
	  m->m_threads[x].m_CPU=adc_buffer[SECOND]>>7; // where?
	  break;
	}
      }
      
      //MODECODE      /////////////////////////////////////

      mainmode=adc_buffer[FIRST]>>8; // 4 bits=16
      //      mainmode=0; // TESTY!
      //////
      switch(mainmode){
#ifdef LACH 
      case 0:
	xx=fingerdir(&spd);
	if (xx!=5) {
	EFFECTWRITE=adc_buffer[FOURTH]>>6;
	EFFECTREAD=adc_buffer[SECOND]>>6;
	// what spd could be? mod? (max 64=6bits<<10) 3 mods:
	settingsarray[20+xx]=spd<<10;
	settingsarrayattached[20+xx]=0;
	}
	break;
#else
      case 0: // up/down/left/right as INPUT
	xx=fingerdir(&spd);
	//	xx=5;// TESTY!
	if (xx!=5) {
	  inp=xx;
	  EFFECTWRITE=adc_buffer[FOURTH]>>6;
	  EFFECTREAD=adc_buffer[SECOND]>>6;
	  EFFECTFILTER=adc_buffer[THIRD]>>6;
	  
	// what spd could be? mod? (max 64=6bits<<10) 3 mods:
	settingsarray[46+xx]=spd<<10;
	settingsarrayattached[46+xx]=0;
	}
	break;
#endif	
	  ///////////////////////////////////
      case 1: // directions - and speed and step!
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5) {
	dirpos=adc_buffer[SECOND]>>9; // 3 bits
	dirpos=dirpos%6;
	settingsarray[12+dirpos]=adc_buffer[FOURTH]<<4;//steps
	settingsarrayattached[12+dirpos]=0;

	if (xx==0) {
	  wormflag[dirpos]=1; // up
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<8;
	    settingsarrayattached[18]=0;
	  }
	}
	else if (xx==1) { 
	  settingsarray[25+dirpos]=(1<<15); wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  } 
	}//right
	else if (xx==3) {//left 
	  settingsarray[25+dirpos]=0; wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	  if (dirpos==2) {
	    settingsarray[19]=spd<<9;
	    settingsarrayattached[19]=0;
	  }
	  else if (dirpos==3) {
	    settingsarray[18]=spd<<9;
	    settingsarrayattached[18]=0;
	  }
	} // left=0
	else { //down
	  settingsarray[25+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;
	  settingsarrayattached[25+dirpos]=0;
	    if (dirpos==2) {
	      settingsarray[19]=spd<<9;
	      settingsarrayattached[19]=0;
	    }
	    else if (dirpos==3) {
	      settingsarray[18]=spd<<9;
	      settingsarrayattached[18]=0;
	    }
	}
	}
#else
	if (xx!=5) {
	dirpos=adc_buffer[SECOND]>>8; // 4 bits=16
	dirpos=dirpos%10;
	//	dirpos=0;//TESTY!
	settingsarray[32+dirpos]=adc_buffer[FOURTH]<<4;
	settingsarrayattached[32+dirpos]=0;
	if (xx==0) {
	  wormflag[dirpos]=1; // up
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	}
	else if (xx==1) { //right
	  settingsarray[52+dirpos]=(1<<15); wormflag[dirpos]=0;
	  settingsarrayattached[52+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	} //right
	else if (xx==3) { //left
	  settingsarray[52+dirpos]=0; wormflag[dirpos]=0;
	  settingsarrayattached[52+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	} // left=0
	else { //down
	  settingsarray[52+dirpos]=adc_buffer[DOWN]<<4; wormflag[dirpos]=0;
	  settingsarrayattached[52+dirpos]=0;
	  if (dirpos==0) {
	    settingsarray[42]=spd<<9;
	    settingsarrayattached[42]=0;
	  }
	  else if (dirpos==4) {
	    settingsarray[44]=spd<<9;
	    settingsarrayattached[44]=0;
	  }
	  else if (dirpos==5) {
	    settingsarray[43]=spd<<9;
	    settingsarrayattached[43]=0;
	  }
	  else if (dirpos==6) {
	    settingsarray[45]=spd<<9;
	    settingsarrayattached[45]=0;
	  }
	}
	}
#endif
	break; 

      case 2:
	// set start and wrap for r/w/village_r/w according to fingers
	// speed is step
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5){
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){ // UP=read= start/wrap/step
	  settingsarray[1]=tmp;
	  settingsarray[7]=tmper;
	  settingsarray[14]=spd<<8;
	  settingsarrayattached[1]=0;
	  settingsarrayattached[7]=0;
	  settingsarrayattached[14]=0;
	}
	else if (xx==2){ // DOWN=write= start/speed/step
	  settingsarray[0]=tmp;
	  settingsarray[18]=tmper;  // changed as is already set // now is speed
	  settingsarray[15]=spd<<8; // step!!!
	  settingsarrayattached[0]=0;
	  settingsarrayattached[18]=0;
	  settingsarrayattached[15]=0;
	}
	else if (xx==3){ // LEFT=village_r= start/wrap/step - AUG - anyread
	  settingsarray[5]=tmp;
	  settingsarray[11]=tmper;
	  settingsarray[16]=spd<<10;

	  settingsarray[3]=tmp;
	  settingsarray[8]=tmper;
	  settingsarray[12]=spd<<10;

	  settingsarrayattached[3]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[12]=0;

	  settingsarrayattached[5]=0;
	  settingsarrayattached[11]=0;
	  settingsarrayattached[16]=0;
	}
	else { // RIGHT=village_w= start/wrap/step - AUG - anywrite
	  settingsarray[4]=tmp;
	  settingsarray[10]=tmper;
	  settingsarray[17]=spd<<10;

	  settingsarray[2]=tmp;
	  settingsarray[9]=tmper;
	  settingsarray[13]=spd<<10;

	  settingsarrayattached[2]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[13]=0;

	  settingsarrayattached[4]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[17]=0;
	}
	}
#else
	if (xx!=5){
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){ // UP=read= start/wrap/step
	  settingsarray[2]=tmp;
	  settingsarray[16]=tmper;
	  settingsarray[36]=spd<<8;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[16]=0;
	  settingsarrayattached[36]=0;
	}
	else if (xx==2){ // DOWN=write= start/wrap/step/
	  settingsarray[1]=tmp;
	  settingsarray[15]=tmper;
	  settingsarray[37]=spd<<8;
	  settingsarrayattached[1]=0;
	  settingsarrayattached[15]=0;
	  settingsarrayattached[37]=0;
	}

	else if (xx==3){ // LEFT=village_r= start/wrap/step// AUG also anystepread
	  settingsarray[8]=tmp;
	  settingsarray[22]=tmper;
	  settingsarray[39]=spd<<10;

	  settingsarray[5]=tmp;
	  settingsarray[18]=tmper;
	  settingsarray[33]=spd<<10;

	  settingsarrayattached[5]=0;
	  settingsarrayattached[18]=0;
	  settingsarrayattached[33]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[22]=0;
	  settingsarrayattached[39]=0;
	}
	else { // RIGHT=village_w= start/wrap/step // AUG also anystepwrite
	  settingsarray[7]=tmp; //7,21,40
	  settingsarray[21]=tmper;
	  settingsarray[40]=spd<<10; //16 bits

	  settingsarray[4]=tmp; //7,21,40
	  settingsarray[19]=tmper;
	  settingsarray[34]=spd<<10; //16 bits

	  settingsarrayattached[4]=0;
	  settingsarrayattached[19]=0;
	  settingsarrayattached[34]=0;
	  settingsarrayattached[7]=0;
	  settingsarrayattached[21]=0;
	  settingsarrayattached[40]=0;
	}
	}
#endif	
	break;
      case 3:
	// HW=start,wrap,offsets
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5){
	// 4 sets of settings:
	// ANYSTART/ANYWRAP/ANYSTEP/VILLAGESTEP *2
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){
	  settingsarray[2]=tmp; // anystart and anystartread - AUG villages starts
	  settingsarray[3]=tmper;
	  settingsarray[4]=tmp; // anystart and anystartread - AUG villages starts
	  settingsarray[5]=tmper;

	  settingsarrayattached[2]=0;
	  settingsarrayattached[3]=0;
	  settingsarrayattached[4]=0;
	  settingsarrayattached[5]=0;
	}
	else if (xx==1){
	  settingsarray[8]=tmp; // anywrap and anywrapread - AUG villages wraps
	  settingsarray[9]=tmper;
	  settingsarray[10]=tmp; // anywrap and anywrapread - AUG villages wraps
	  settingsarray[11]=tmper;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[11]=0;
	}
	else if (xx==2){ //AUG villagesteps
	  settingsarray[12]=tmp;
	  settingsarray[13]=tmper;
	  settingsarray[16]=tmp;
	  settingsarray[17]=tmper;

	  settingsarrayattached[12]=0;// anystep and anystepread
	  settingsarrayattached[13]=0;
	  settingsarrayattached[16]=0;// anystep and anystepread
	  settingsarrayattached[17]=0;

	}
	else {
	  settingsarray[23]=tmp; // villager but this should be another set of settings *2-FOLD=AUG
	  settingsarray[24]=tmper;

	  settingsarrayattached[23]=0;
	  settingsarrayattached[24]=0;
	}
	}
#else
	//HW start and wrap
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx!=5){
	if (xx==0){ // UP=generic HW start/wrap/hdgenercons/speed
	  settingsarray[0]=tmp;
	  settingsarray[14]=tmper;
	  settingsarray[24]=adc_buffer[THIRD]<<4;
	  settingsarray[42]=spd<<10;
	  settingsarrayattached[0]=0;
	  settingsarrayattached[14]=0;
	  settingsarrayattached[24]=0;
	  settingsarrayattached[42]=0;
	}
	else 	if (xx==2){ //DOWN---offset/cons/base 
	  settingsarray[28]=tmp;
	  settingsarray[29]=adc_buffer[THIRD]<<4;
	  settingsarray[25]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[28]=0;
	  settingsarrayattached[29]=0;
	  settingsarrayattached[25]=0;
	  //	  settingsarrayattached[10]=0;
	}
	else 	if (xx==3){ //LEFT=1016er-offset/cons/base
	  settingsarray[30]=tmp;
	  settingsarray[12]=adc_buffer[THIRD]<<4;
	  settingsarray[26]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[30]=0;
	  settingsarrayattached[26]=0;
	  settingsarrayattached[12]=0;
	}
	else { //RIGHT=maximer
	  settingsarray[31]=tmp; //offset
	  settingsarray[27]=adc_buffer[THIRD]<<4; //cons
	  settingsarray[13]=tmper; //base
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[31]=0;
	  settingsarrayattached[27]=0;
	  settingsarrayattached[13]=0;
	}
	}
#endif
	break;

      case 4:
	//	  select stackmax (left/right) (knob and set)
	xx=fingerdir(&spd);
	if (xx!=5) {
	  if (xx==0) {//UP
	    if (THREADCOUNT>0){

	    m->m_threads[(adc_buffer[THIRD]>>6)%THREADCOUNT].m_CPU=spd%31; // AUG - re-arranged so set first
	    }
	    else m->m_threads[0].m_CPU=spd%31;

	  settingsarray[THREADERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[THREADERR]=0;
	  settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[VILLAGERR]=0;
	}
	  else if (xx==1) //RIGHT=CA
	  {
	    if (STACKPOSY>0){
	    stackery[(((adc_buffer[THIRD]>>6)%STACKPOSY)*4)+3]=(spd%11)<<12; //type AUG <<12 must be there
	    }
	    else stackery[0]=(spd%11)<<12; //type AUG <<12 must be there
	    settingsarray[POSYERR]=adc_buffer[SECOND]<<4;
	    settingsarrayattached[POSYERR]=0;
	    settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	    settingsarrayattached[VILLAGERR]=0;
	  }
	  else if (xx==2) {//DOWN
	    if (STACKPOS>0){
	    stacker[(((adc_buffer[THIRD]>>6)%STACKPOS)*4)+3]=(spd%34)<<10;  //type AUG <<10 must be there
	    }
	    else stacker[0]=(spd%11)<<12; //type AUG <<12 must be there
	  settingsarray[POSERR]=adc_buffer[SECOND]<<4;
	  settingsarrayattached[POSERR]=0;
	  settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[VILLAGERR]=0;
	}
	  else//LEFT
	  {
	    exestack[adc_buffer[SECOND]>>10]=spd%4; // 2 bits 0,1,2,3=spd%4 or spd&3
	    settingsarray[VILLAGERR]=adc_buffer[FOURTH]<<4;
	    settingsarrayattached[VILLAGERR]=0;
	    }
	}
	break;

      case 5:
	// anydata stuff
	//for LACH is simply anystart*2,anywrap*2,anystep*2
	//for TENE and SUSP is *3 (inc filt)
	xx=fingerdir(&spd);
#ifdef LACH
	if (xx!=5){
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx==0){ // UP
	  settingsarray[3]=tmp;//anystartread
	  settingsarray[8]=tmper;//wrap
	  settingsarray[12]=spd<<10;//step

	  settingsarray[5]=tmp;//anystartread AUG DOUBLE VILLAGERS
	  settingsarray[11]=tmper;//wrap
	  settingsarray[16]=spd<<10;//step

	  settingsarrayattached[5]=0;
	  settingsarrayattached[11]=0;
	  settingsarrayattached[16]=0;

	  settingsarrayattached[3]=0;
	  settingsarrayattached[8]=0;
	  settingsarrayattached[12]=0;
	}
	else 	if (xx==2){ //DOWN
	  settingsarray[2]=tmp;//write
	  settingsarray[9]=tmper;
	  settingsarray[13]=spd<<10;

	  settingsarray[4]=tmp;//write AUG DOUBLE VILLAGERS
	  settingsarray[10]=tmper;
	  settingsarray[17]=spd<<10;

	  settingsarrayattached[4]=0;
	  settingsarrayattached[10]=0;
	  settingsarrayattached[17]=0;
	  settingsarrayattached[2]=0;
	  settingsarrayattached[9]=0;
	  settingsarrayattached[13]=0;
	}
	//#define FOLDTOP (settingsarray[23]) 
	//#define FOLDOFFSET (settingsarray[24]) 
	else { 	  //left and right
	  settingsarray[23]=tmp; //AUG
	  settingsarray[24]=tmper;
	  settingsarray[31]=spd<<10;
	  settingsarrayattached[23]=0;
	  settingsarrayattached[24]=0;
	  settingsarrayattached[31]=0;
	}
	}
#else
	//HW start and wrap
	tmp=adc_buffer[SECOND]<<4;
	tmper=adc_buffer[FOURTH]<<4;
	if (xx!=5){
	if (xx==0){ // UP=generic HW start/wrap/hdgenercons/speed
	  settingsarray[0]=tmp;
	  settingsarray[14]=tmper;
	  settingsarray[24]=adc_buffer[THIRD]<<4;
	  settingsarray[42]=spd<<10;
	  settingsarrayattached[0]=0;
	  settingsarrayattached[14]=0;
	  settingsarrayattached[24]=0;
	  settingsarrayattached[42]=0;
	}
	else 	if (xx==2){ //DOWN---offset/cons/base 
	  settingsarray[28]=tmp;
	  settingsarray[29]=adc_buffer[THIRD]<<4;
	  settingsarray[25]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[28]=0;
	  settingsarrayattached[29]=0;
	  settingsarrayattached[25]=0;
	  //	  settingsarrayattached[10]=0;
	}
	else 	if (xx==3){ //LEFT=1016er-offset/cons/base
	  settingsarray[30]=tmp;
	  settingsarray[12]=adc_buffer[THIRD]<<4;
	  settingsarray[26]=tmper;
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[30]=0;
	  settingsarrayattached[26]=0;
	  settingsarrayattached[12]=0;
	}
	else { //RIGHT=maximer
	  settingsarray[31]=tmp; //offset
	  settingsarray[27]=adc_buffer[THIRD]<<4; //cons
	  settingsarray[13]=tmper; //base
	  settingsarray[42]=spd<<12; 
	  settingsarrayattached[42]=0;
	  settingsarrayattached[31]=0;
	  settingsarrayattached[27]=0;
	  settingsarrayattached[13]=0;
	}
	}
#endif
	break;
	/////////////////////////////////////////////////////////////////////////////////////////

      case 6: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>SHIFTY; // 6bits=64 or7 for LACH
	groupstart=adc_buffer[FOURTH]>>SHIFTY;
	for (x=0;x<groupwrap;x++){
	  settingsarrayattached[(groupstart+x)%SETSIZE]=groupsel;
	}
	}
	break;

      case 7: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>5; // 7bits=128
	groupstart=adc_buffer[FOURTH]>>5;
	for (x=0;x<groupwrap;x++){
	  villagerattached[(groupstart+x)%VILLAGE_SIZE]=groupsel;
	  villagereffattached[((groupstart+x)/2)%64]=groupsel;
	}
	}
	break;

      case 8:
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>4; // 8bits
	groupstart=adc_buffer[FOURTH]>>4;
	for (x=0;x<groupwrap;x++){
	  stackerattached[(groupstart+x)%256]=groupsel;
	}
	}
	break;

      case 9: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>4; // 8bits
	groupstart=adc_buffer[FOURTH]>>4;
	for (x=0;x<groupwrap;x++){
	  stackeryattached[(groupstart+x)%256]=groupsel;
	}
	}
	break;

      case 10: //SETTED! 	//expand for 4stacker/stackery//5CPU//6villager/7village_effects
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	groupwrap=adc_buffer[SECOND]>>6; // 6bits
	groupstart=adc_buffer[FOURTH]>>6;
	for (x=0;x<groupwrap;x++){
	  cpuattached[(groupstart+x)%64]=groupsel;
	}
	}
	break;

	///////////////////////////////
	//algo-attach into all:
      case 11:
	groupsel=fingerdir(&spd);
	if (groupsel!=5){
	foldy=adc_buffer[SECOND]>>7; // how many from knob2 =max 32
	for (x=0;x<foldy;x++){
	  settingsposl=adc_buffer[FOURTH]<<3; // 15 bits
	  tmper=buf16[(settingsposl+x)%32768];
#ifdef LACH
	tmper=(tmper>>6)%804;
	if (tmper<36) settingsarrayattached[tmper]=groupsel;
	else 	if (tmper<292) stackerattached[tmper-36]=groupsel; // now 256
	else 	if (tmper<548) stackeryattached[tmper-292]=groupsel; // now 256
	else 	if (tmper<676) villagerattached[tmper-548]=groupsel; // 128
	else 	if (tmper<740) villagereffattached[tmper-676]=groupsel; // 64
	else 	cpuattached[tmper-740]=groupsel; //64
#else
	tmper=(tmper>>6)%834;
	if (tmper<66) settingsarrayattached[tmper]=groupsel;
	else 	if (tmper<322) stackerattached[tmper-66]=groupsel;
	else 	if (tmper<578) stackeryattached[tmper-322]=groupsel;
	else 	if (tmper<706) villagerattached[tmper-578]=groupsel;
	else 	if (tmper<770) villagereffattached[tmper-706]=groupsel;
	else 	cpuattached[tmper-770]=groupsel;
#endif
	}
	}
	break;

      case 12:
	groupsel=fingerdir(&spd);
	if (groupsel==0){//UP
	  foldy=adc_buffer[SECOND]>>SHIFTY; // howmuch-64
	foldposy=adc_buffer[THIRD]>>SHIFTY; // offset
	foldpos=adc_buffer[FOURTH]>>SHIFTY;
	for (x=0;x<(foldy);x++){
	  settingsarray[(foldpos+x)%SETSIZE]=settingsarray[(foldpos+foldposy+x)%SETSIZE];
	  }
	}
	else if (groupsel==1){// RIGHT
	foldy=adc_buffer[SECOND]>>5; // howmuch=128
	foldposy=adc_buffer[THIRD]>>5; // offset
	foldpos=adc_buffer[FOURTH]>>5;//start

	for (x=0;x<foldy;x++){
	  villager[(foldpos+x)%VILLAGE_SIZE]=villager[(foldpos+foldposy+x)%VILLAGE_SIZE];
	}
	}

	else if (groupsel==2){ //DOWN// various stack and villager exchanges - 
	// starts and ends only of stacks (not CPU) -> villagers
	  foldy=adc_buffer[SECOND]>>6; // howmuch-64
	  foldposy=adc_buffer[THIRD]>>6; // offset-64
	  foldpos=adc_buffer[FOURTH]>>5;//128
	///////
	for (x=0;x<foldy;x++){ // 64 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point 
	  villagerdest=((foldpos>>1)+x+foldposy)%(STACK_SIZE); // village entry
	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    villager[villagerdest*2]=stacker[tmper*4];  
	    villager[(villagerdest*2)+1]=stacker[(tmper*4)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    villager[villagerdest*2]=stackery[tmper*4];  
	    villager[(villagerdest*2)+1]=stackery[(tmper*4)+1];  
	  }
	}
	}
	else if (groupsel==3) {//LEFT // various stack and villager exchanges -  // bugfix AUG
	// other way round
	
	foldy=adc_buffer[SECOND]>>6; // howmuch-64
	foldposy=adc_buffer[THIRD]>>6; // offset
	foldpos=adc_buffer[FOURTH]>>5;

	for (x=0;x<(foldy);x++){ 
	  tmper=(foldpos+x)%(STACK_SIZE*2); // so both stacks entry point // 7 bits=128
	  villagerdest=((foldpos>>1)+x+foldposy)%(STACK_SIZE); // village entry

	  if (tmper<STACK_SIZE){
	    // deal with stacker
	    stacker[tmper*4]=villager[villagerdest*2];  
	    stacker[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	  else {
	    tmper-=STACK_SIZE;
	    // deal with stackery
	    stackery[tmper*4]=villager[villagerdest*2];  
	    stackery[(tmper*4)+1]=villager[(villagerdest*2)+1];  
	  }
	}
	}
	break;

      case 13: // dump (all) to datagen//back
	xx=fingerdirupdown();
	if (xx==1) { //down
	foldy=adc_buffer[SECOND]>>2; // howmuch=10 bits=1024
	foldpos=adc_buffer[FOURTH]<<4; // 16 bits

	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%740; // full house//10 bits=1024
	  if (tmper<36) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<292) buf16[(foldpos+x)%32768]=stacker[tmper-36];
	  else if (tmper<548) buf16[(foldpos+x)%32768]=stackery[tmper-292];
	  else if (tmper<612) buf16[(foldpos+x)%32768]=m->m_threads[tmper-548].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-612];
#else
	  tmper=((foldpos>>6)+x)%770; // full house//10 bits=1024
	  if (tmper<66) buf16[(foldpos+x)%32768]=settingsarray[tmper];
	  else if (tmper<322) buf16[(foldpos+x)%32768]=stacker[tmper-66];
	  else if (tmper<578) buf16[(foldpos+x)%32768]=stackery[tmper-322];
	  else if (tmper<642) buf16[(foldpos+x)%32768]=m->m_threads[tmper-578].m_CPU<<11;
	    else buf16[(foldpos+x)%32768]=villager[tmper-642];
#endif
	}
	}
	else if (xx==0) {
	foldy=adc_buffer[SECOND]>>2; // howmuch=10 bits=1024
	foldpos=adc_buffer[FOURTH]<<4; // 16 bits
	for (x=0;x<foldy;x++){ // 10 bits
#ifdef LACH
	  tmper=((foldpos>>6)+x)%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<292) stacker[tmper-32]=buf16[(foldpos+x)%32768];
	  else if (tmper<548) stackery[tmper-288]=buf16[(foldpos+x)%32768];
	  else if (tmper<612) m->m_threads[tmper-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-612]=buf16[(foldpos+x)%32768];
#else
	  tmper=((foldpos>>6)+x)%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=buf16[(foldpos+x)%32768];
	  else if (tmper<322) stacker[tmper-66]=buf16[(foldpos+x)%32768];
	  else if (tmper<578) stackery[tmper-322]=buf16[(foldpos+x)%32768];
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=buf16[(foldpos+x)%32768]>>11;
	    else villager[tmper-642]=buf16[(foldpos+x)%32768];
#endif
	}
	}
	break;
	////////////////
      case 14: // infection
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability

      xx=fingerdir(&spd);

      if (xx!=5) {
	tmp=adc_buffer[SECOND]>>2; // 10 bits//offset
	tmper=adc_buffer[FOURTH]>>2;//amount-10 bits

	if ((adc_buffer[FOURTH]>>5)==0){
	  for (x=0;x<(tmper%INFECTSIZE);x++){
	    if ((rand()%255) > (spd)) settingsarrayinfected[(tmp+x)%INFECTSIZE][which]=1; // infected 3
	  else settingsarrayinfected[(tmp+x)%INFECTSIZE][which]=0;
	  } // reset!
	}
	  // run infection at speed third knob
	  else {

	    if (++del>=(adc_buffer[THIRD]>>7)){ // speed
	      del=0;
	  for (i=0;i<(tmper%INFECTSIZE);i++){
      	    // infection - how many infected (not dead) round each one?
	    x=(i+tmp)%INFECTSIZE;
	      tmpacht=(x-1)%INFECTSIZE;

	      if (settingsarrayinfected[x][which]==0 && settingsarrayinfected[tmpacht][which]>=1 && settingsarrayinfected[tmpacht][which]<128 && settingsarrayinfected[(x+1)%INFECTSIZE][which]>=1 && settingsarrayinfected[(x+1)%INFECTSIZE][which]<128 && ((rand()%255) > (spd))) {
		settingsarrayinfected[x][other]=1;
		//			  printf("infecte\n");
	      }
	    // inc
	      else if (settingsarrayinfected[x][which]>0 && settingsarrayinfected[x][which]<128) {
		  settingsarrayinfected[x][other]++;
		  //		  	  printf("not\n");
	      }
#ifdef LACH
	    if (x<36) {
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	settingsarray[x]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) settingsarray[x]+=settingsarrayinfected[x][other];
	    }
	    else if (x<292) {
	      //	      stacker[x-32]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stacker[x-36]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stacker[x-36]+=settingsarrayinfected[x][other];;

	    }
	    else if (x<548) {
	      //	      stackery[x-288]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stackery[x-292]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stackery[x-292]+=settingsarrayinfected[x][other];;
	    }
	    else if (x<612) {
	      //	      m->m_threads[x-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	m->m_threads[x-548].m_CPU-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) m->m_threads[x-548].m_CPU+=settingsarrayinfected[x][other];;
	    }
	    else {
	      //	      villager[x-608]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)  villager[x-612]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) villager[x-612]+=settingsarrayinfected[x][other];;
	    }
#else
	    if (x<66) {
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	settingsarray[x]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) settingsarray[x]+=settingsarrayinfected[x][other];;
	    }
	    else if (x<322) {
	      //	      stacker[x-32]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stacker[x-66]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stacker[x-66]+=settingsarrayinfected[x][other];;

	    }
	    else if (x<578) {
	      //	      stackery[x-288]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	stackery[x-322]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) stackery[x-322]+=settingsarrayinfected[x][other];;
	    }
	    else if (x<642) {
	      //	      m->m_threads[x-544].m_CPU=buf16[(foldpos+x)%32768]>>11;
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)	m->m_threads[x-578].m_CPU-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) m->m_threads[x-578].m_CPU+=settingsarrayinfected[x][other];;
	    }
	    else {
	      //	      villager[x-608]=buf16[(foldpos+x)%32768];
	      if (settingsarrayinfected[x][other]>0 && settingsarrayinfected[x][other]<128)  villager[x-642]-=settingsarrayinfected[x][other];
	      else if (settingsarrayinfected[x][other]>127) villager[x-642]+=settingsarrayinfected[x][other];
	    }
#endif
	  }
	    }//del!
	  which^=1;
	  other^=1;
	  //	  printf("wich %d uther %d\n",which,other);
	  }
      }
	break;
    case 15: // fingers in the code... navigate and insert code - no knobs(?)
      // left-right move in datagen
      // down-up into all code values
      xx=fingerdir(&spd);
	    // xx=2;
      if (xx==1){ //right
	fingerposl+=spd;
	buf16[fingerposl%32768]=adc_buffer[RIGHT]<<4;
      }
      else if (xx==3){ //left
	fingerposl-=spd;
	buf16[fingerposl%32768]=adc_buffer[LEFT]<<4;
      }
      else if (xx==2){ //down
	fingerposl+=(spd>>1);
#ifdef LACH
	tmper=fingerposl%740; // full house//AUG was >>6
	  if (tmper<36) settingsarray[tmper]=adc_buffer[DOWN]<<4;
	  else if (tmper<292) stacker[tmper-36]=adc_buffer[DOWN]<<4;
	  else if (tmper<548) stackery[tmper-292]=adc_buffer[DOWN]<<4;
	  else if (tmper<612) m->m_threads[tmper-548].m_CPU=adc_buffer[DOWN]>>7;
	  else villager[tmper-612]=adc_buffer[DOWN]<<4;
#else
	  tmper=fingerposl%770; // full house//AUG was >>6
	  if (tmper<66) settingsarray[tmper]=adc_buffer[DOWN]<<4;
	  else if (tmper<322) stacker[tmper-66]=adc_buffer[DOWN]<<4;
	  else if (tmper<578) stackery[tmper-322]=adc_buffer[DOWN]<<4;
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=adc_buffer[DOWN]>>7;
	    else villager[tmper-642]=adc_buffer[DOWN]<<4;
#endif
      }
      else {//UP!
	fingerposl-=(spd>>1);
#ifdef LACH
		  tmper=fingerposl%740; // full house//10 bits=1024
	  if (tmper<36) settingsarray[tmper]=adc_buffer[UP]<<4;
	  else if (tmper<292) stacker[tmper-36]=adc_buffer[UP]<<4;
	  else if (tmper<548) stackery[tmper-292]=adc_buffer[UP]<<4;
	  else if (tmper<612) m->m_threads[tmper-548].m_CPU=adc_buffer[UP]>>7;
	  else villager[tmper-612]=adc_buffer[UP]<<4;
#else
	  tmper=fingerposl%770; // full house//10 bits=1024
	  if (tmper<66) settingsarray[tmper]=adc_buffer[UP]<<4;
	  else if (tmper<322) stacker[tmper-66]=adc_buffer[UP]<<4;
	  else if (tmper<578) stackery[tmper-322]=adc_buffer[UP]<<4;
	  else if (tmper<642) m->m_threads[tmper-578].m_CPU=adc_buffer[UP]>>7;
	    else villager[tmper-642]=adc_buffer[UP]<<4;
#endif
      }
      break;
      }

      //END MODECODE      /////////////////////////////////////

// FROM audio.c



#ifdef LACH

	//	VILLAGEREAD=(EFFECTREAD&3);	
	VILLAGEREAD=(EFFECTREAD&48)>>4;	
	//VILLAGEREAD=2; // TESTY!

	if (VILLAGEREAD==2){ // moved out of loop AUG
	    tmpp=village_effects[vilr/2]%16;
	  }
	  else tmpp=EFFECTREAD&15;

      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	  src++;
	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 1:
	  src++;
	  //	  buf16[sampleposread%32768]=(*src)+32768;
	  fsum=(float32_t)*(src++) * morph_inv;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  //	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 2:
	  src++;
	  tmp16=buf16[sampleposread%32768]-32768;
	  buf16[sampleposread%32768]=*(src++)+32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 3:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  src++;
	  //	  tmp32=(*(src++))+audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  src++;
	  //	  tmp32=(*(src++))+audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  src++;
	  tmp16=(*(src++))-(buf16[sampleposread%32768]-32678);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  src++;
	  tmp16=(buf16[sampleposread%32768]-32768)-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  src++;
	  tmp16=(buf16[sampleposread%32768]-32768) | (*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  src++;
	  tmp16=(*(src++))-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 12:
	  src++;
	  tmp16=audio_buffer[sampleposread%32768]-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 13:
	  src++;
	  tmp16=audio_buffer[sampleposread%32768] | (*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 14:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 15:
	  src++;
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  }
	  /////

	  if (++delread>=SAMPLESPEEDREAD){
	    if (wormflag[2]) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
	    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;//+SAMPLEREXPAND;
		    if (dirry>0) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		    if (wormflag[0]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    if (wormflag[4]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=dirryy;
		    //		    vill=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2

		    tmp=VILLAGERSTART;tmper=VILLAGERWRAP;
		    vilr=(tmp+(villagerpos%tmper))*2;
		    vilr=vilr%VILLAGESTACKPOS;

		    startread=villager[vilr]>>1;
		    wrapread=((villager[vilr+1]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		    tmpp=village_effects[vilr/2]%16;
		    if (wrapread==0) wrapread=1;
		    if (dirry>0) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		    if (wormflag[0]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		  if (wormflag[0]) dirryy=newdirection[wormdir]; 
		  else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	}
	/////////////////////////////NO____LACH!!!!!!!!!
#else // end LACH

	// AUG NOTE: cases 8+ use leftbuffer!

	if (digfilterflag&1){

	  //	  VILLAGEREAD=EFFECTREAD&3
	  VILLAGEREAD=(EFFECTREAD&48)>>4;	
	  //	  VILLAGEREAD=0; // TESTY!
	
	  if (VILLAGEREAD==2){// moved AUG
	    tmpp=village_effects[vilr/2];
	  }
	  else tmpp=EFFECTREAD&15;

	  //	  tmpp=0; // TESTY!!!

	  for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  audio_buffer[sampleposread%32768]=*(src++);
	  //	  audio_buffer[sampleposread%32768]=0; // TESTY!
	  break;
	  case 1:
	    audio_buffer[sampleposread%32768]=*src; // LEFT
	  *(ldst++) = *(src++);
	  *(rdst++) = *(src++); 
	  break;
	  // Effects with/without clipping *, +, -, 
	  case 2:
	    *(ldst++) = *(src++);
	    *(rdst++) = *src; 
	    fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD; // shift = 32768 
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 3:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	  *(ldst++) = *(src++);
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)buf16[sampleposread%32768]-32768) * FMOD;
	  *(rdst++) = *src; 
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  tmp16=(*(src++))-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 7:
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  tmp16=audio_buffer[sampleposread%32768]-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;

	  case 8:
	  tmp16=*src;
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  tmp32= *(src++) * tmp16; //right * left
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;

	  case 9:
	  *(ldst++) = *src;
	  tmp16=*(src++); // left
	  *(rdst++) = *src; 
	  fsum=(float32_t)tmp16 * morph_inv * (float32_t)*(src++) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 10:
	  *(ldst++) = *src;
	  //tmp32=audio_buffer[samplepos%32768]* *(src++);
	  //	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  *(ldst++) = *src;
	  //	  tmp32=audio_buffer[samplepos%32768]* *(src++);
	  fsum=(float32_t)*(src++) * morph_inv * ((float32_t)audio_buffer[sampleposread%32768]+32768) * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;

	  case 12:
	  *(ldst++) = *src;
	  //	  tmp32=*(src++) +audio_buffer[sampleposread%32768];
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768]+32768 * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break; // was missing +AUG

	  case 13:
	  *(ldst++) = *src;
	  fsum=(float32_t)(buf16[sampleposread%32768]-32768) * morph_inv * (float32_t)*(src++) * FMOD;
	  tmp32=fsum;
	  *(rdst++) = *(src++); 
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	    tmper= *src;
	    *(ldst++) = *(src++);
	    tmp16=*src | tmper;
	    *(rdst++) = *(src++); 
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
 	  case 15:
	  tmp16=*src;
	  *(ldst++) = *(src++);
	  *(rdst++) = *src; 
	  fsum=(float32_t)tmp16 * morph_inv + (float32_t)*(src++) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  }
	  	  if (++delread>=SAMPLESPEEDREAD){
		    if (wormflag[4]) dirry=newdirection[wormdir]; 
		    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
		    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		      {
			sampleposread+=dirry;//)%32768;
		      }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;//+SAMPLEREXPAND;
		    if (dirry>0) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    if (wormflag[7]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=dirryy;
		    //		    vill=(VILLAGERSTART+(villagerpos%VILLAGERWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2
		    tmp=VILLAGERSTART;tmper=VILLAGERWRAP;
		    vilr=(tmp+(villagerpos%tmper))*2;
		    vilr=vilr%VILLAGESTACKPOS;
		    startread=villager[vilr]>>1;
		    wrapread=((villager[vilr+1]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		    tmpp=village_effects[vilr/2];
		    if (wrapread==0) wrapread=1;
		    if (dirry>0) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  if (wrapread==0) wrapread=1;
		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
		  }
	  }
	  /// INSERT writeDONE - STILL DIGFILTER
	  // 8-11 use left

	ldst=left_buffer;
	morph_inv = 1.0f - (float32_t)FMODW;
	//	VILLAGEWRITE=EFFECTWRITE&3;
	VILLAGEWRITE=(EFFECTWRITE&48)>>4;	
	//	VILLAGEWRITE=0; // TESTY!


	
	if (VILLAGEWRITE==2){// moved AUG
	    tmpp=village_effects[vilw/2];
	  }
	  else tmpp=EFFECTWRITE&15;

	//	tmpp=11; // TESTY!

      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	    ldst++; //added in AUG in case of village/change during this
	    mono_buffer[x]=audio_buffer[samplepos%32768];
	    //	    mono_buffer[x]=buf16[samplepos%32768]; ;// TESTY! 
	  break;
	  case 1:
	    ldst++; //added in AUG in case of village/change during this
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 2:
	    //	  tmp32=buf16[samplepos%32768] * audio_buffer[samplepos%32768];
	    ldst++; //added in AUG in case of village/change during this
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    ldst++; //added in AUG in case of village/change during this
	    //	  tmp32=audio_buffer[samplepos%32768]+buf16[samplepos%32768];
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    ldst++; //added in AUG in case of village/change during this
	    //	  tmp32=audio_buffer[samplepos%32768]+buf16[samplepos%32768];
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    ldst++; //added in AUG in case of village/change during this
	    tmp16=audio_buffer[samplepos%32768]-(buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 6:
	    ldst++; //added in AUG in case of village/change during this
	    tmp16=(buf16[samplepos%32768]-32768)-audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    ldst++; //added in AUG in case of village/change during this
	    fsum=(float32_t)*(ldst++) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 8:
	    //	  tmp32=audio_buffer[samplepos%32768]* *(ldst++);
	    fsum=(float32_t)*(ldst++) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 9:
	    tmp16=audio_buffer[samplepos%32768] - *(ldst++);
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    //	  tmp32=*(ldst++)+buf16[samplepos%32768];
	    fsum=(float32_t)*(ldst++) * morph_inv + (float32_t)(buf16[samplepos%32768]-32768) * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 11:
	    //	  tmp32=*(ldst++)+buf16[samplepos%32768];
	    fsum=(float32_t)*(ldst++) * morph_inv + (float32_t)(buf16[samplepos%32768]-32768) * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    ldst++; //added in AUG in case of village/change during this
	    mono_buffer[x]=((adc_buffer[9]<<3)-32768) * audio_buffer[samplepos%32768];
	  break;
	  case 13:
	    ldst++; //added in AUG in case of village/change during this
	    //	    tmp32=audio_buffer[samplepos%32768]+adc_buffer[9]<<3;
	    fsum=(float32_t)((adc_buffer[9]<<3)-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 14:
	    ldst++; //added in AUG in case of village/change during this
	    //	    tmp32=audio_buffer[samplepos%32768]*adc_buffer[9]<<3;
	    fsum=(float32_t)((adc_buffer[9]<<3)-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
 	  case 15:
	    ldst++; //added in AUG in case of village/change during this
	    w0=(float32_t) (buf16[0]-32768)/16384.0f;w1=(float32_t) (buf16[1]-32768)/16384.0f;w2=(float32_t) buf16[2]/16384.0f;
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpw]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  } 
	  ////////////////////////--->>>>

	  
	if (++del>=SAMPLESPEED){
	  if (wormflag[5]) dirry=newdirection[wormdir]; 
	  else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);
	    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;//+SAMPLEEXPAND;
		    if (dirry>0) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }
		  else if (VILLAGEWRITE==1) {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);//+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    if (wormflag[8]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=dirryy;
		    //		    vill=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2
		    tmp=VILLAGEWSTART;tmper=VILLAGEWWRAP;
		    vilw=(tmp+(villagewpos%tmper))*2;
		    vilw=vilw%VILLAGESTACKPOS;
		    tmpp=village_effects[vilw/2];
		    start=villager[vilw]>>1;
		    wrap=((villager[vilw+1]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (dirry>0) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		  if (wormflag[2]) dirryy=newdirection[wormdir]; 
		  else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (dirry>0) samplepos=start;
		  else samplepos=start+wrap;
		  }
		  }
	  del=0;
	} // end del
	}
	}
	else  // READIN NO DIG FILTER
	  {
	    //	    VILLAGEREAD=EFFECTREAD&3;
	    VILLAGEREAD=(EFFECTREAD&48)>>4;	

	    //	    VILLAGEREAD=2; // TESTY!	    
	    if (VILLAGEREAD==2){//moved AUG
	    tmpp=village_effects[vilr/2];
	  }
	  else tmpp=EFFECTREAD&15; 

	    for (x=0;x<sz/2;x++){
	  switch(tmpp){
	  case 0:
	  default:
	  src++;
	  audio_buffer[sampleposread%32768]=*(src++);
	  break;
	  case 1:
	    src++;
	    //	    buf16[sampleposread%32768]=(*src)+32768;
	    //	    audio_buffer[sampleposread%32768]=*(src++);
	    fsum=(float32_t)*(src++) * morph_inv;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;	    
	  case 2:
	  src++;
	  tmp16=buf16[sampleposread%32768]-32768;
	  buf16[sampleposread%32768]=*(src++)+32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 3:
	    (src++);
	    fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 4:
	    (src++);
	    fsum=(float32_t)*(src++) * morph_inv * (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 5:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 6:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)(buf16[sampleposread%32768]-32768) * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 7:
	  (src++);
	  tmp16=(*(src++))-(buf16[sampleposread%32768]-32768);
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 8:
	  (src++);
	  tmp16=buf16[sampleposread%32768]-(*(src++))-32768;
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 9:
	  (src++);
	  tmp16=(buf16[sampleposread%32768]-32768)|(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 10:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv * (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 11:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 12:
	  (src++);
	  fsum=(float32_t)*(src++) * morph_inv + (float32_t)audio_buffer[sampleposread%32768] * FMOD;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 13:
	  (src++);
	  tmp32=(*(src++))-audio_buffer[sampleposread%32768];
	  audio_buffer[sampleposread%32768]=tmp32;
	  break;
	  case 14:
	  (src++);
	  tmp16=audio_buffer[sampleposread%32768]-(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  break;
	  case 15:
	  (src++);
	  tmp16=audio_buffer[sampleposread%32768]|(*(src++));
	  audio_buffer[sampleposread%32768]=tmp16;
	  }

	  if (++delread>=SAMPLESPEEDREAD){
	    if (wormflag[4]) dirry=newdirection[wormdir];  
	    else dirry=direction[SAMPLEDIRR]*SAMPLESTEPREAD;	    
	    count=((sampleposread-startread)+dirry);
		    if (count<wrapread && count>0)
		  {
		    sampleposread+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEREAD==0) {
		    startread=SAMPLESTARTREAD;wrapread=SAMPLEWRAPREAD;//+SAMPLEREXPAND;
		    if (dirry>0) sampleposread=startread; //forwards
		    else sampleposread=startread+wrapread;
		  }

		  else if (VILLAGEREAD==1) {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  tmp=(ANYSTARTREAD+(anyposread%ANYWRAPREAD))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;
		  sampleposread=SAMPLESTARTREAD+(tmper%SAMPLEWRAPREAD);//+SAMPLEREXPAND;
		  wrapread=0;startread=0;
		  }
		  else if (VILLAGEREAD==2) {
		    if (wormflag[7]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGERSTEP*direction[VILLAGERDIR];
		    villagerpos+=dirryy; 
		    //		    vill=((VILLAGERSTART+(villagerpos%VILLAGERWRAP))*2)%VILLAGESTACKPOS; //to cover all directions
		    tmp=VILLAGERSTART;tmper=VILLAGERWRAP;
		    vilr=(tmp+(villagerpos%tmper))*2;
		    vilr=vilr%VILLAGESTACKPOS;
		    tmpp=village_effects[vilr/2];
		    startread=villager[vilr]>>1;
		    wrapread=((villager[vilr+1]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;;
		    if (wrapread==0) wrapread=1;
		    if (dirry>0) sampleposread=startread;
		    else sampleposread=startread+wrapread;
		  }
		  else {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD; 
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  startread=buf16[tmp]>>1;
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPREAD*direction[DATADIRR];
		  anyposread+=dirryy;
		  wrapper=ANYWRAPREAD;
		  tmp=(ANYSTARTREAD+(anyposread%wrapper))%32768; //to cover all directions
		  wrapread=((buf16[tmp]>>1)%SAMPLEWRAPREAD);//+SAMPLEREXPAND;;
		  if (dirry>0) sampleposread=startread;
		  else sampleposread=startread+wrapread;
		  }
		}
	  delread=0;
	  }
	    }

/// STRAIGHT SANS FILTEROPSSS!!!
	    VILLAGEWRITE=(EFFECTWRITE&48)>>4; // top 2 bits was just & 3

	    morph_inv = 1.0f - (float32_t)FMODW;
	    
	    //	    VILLAGEWRITE=2;//testy!
	    
	    //settingsarray[15]=1024;//testy! SAMPLEWRAP	  
	    if (VILLAGEWRITE==2){// moved AUG
	    tmpp=village_effects[vilw/2];
	  }
	  else tmpp=EFFECTWRITE&15;

	    //tmpp=0; //testy!

	  for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	      mono_buffer[x]=audio_buffer[samplepos%32768];
	    //	    	    mono_buffer[x]=buf16[samplepos%32768]; // TESTY!!!!
	    //	    	    mono_buffer[x]=0; // TESTY!!!!
	    break;
	  case 1:
	    tmp16=(buf16[samplepos%32768]-32768);
	    if (audio_buffer[samplepos%32768]<tmp16) tmp16=audio_buffer[samplepos%32768];
	    mono_buffer[x]=tmp16;
	  break;
	  case 2:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	    tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	    tmp16=audio_buffer[samplepos%32768]-(buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    tmp16=(buf16[samplepos%32768]-32768)-audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	    tmp16=(buf16[samplepos%32768]-32768) | audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	    tmp16=audio_buffer[samplepos%32768] & (buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    mono_buffer[x]=((adc_buffer[9]<<3)-32768)*audio_buffer[samplepos%32768];
	  break;
	  case 11:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv + (float32_t)((adc_buffer[9]<<3)-32768) * FMODW;
 	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 12:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)((adc_buffer[9]<<3)-32768) * FMODW;
	  tmp32=fsum;
	    mono_buffer[x]=tmp32;
	  break;
	  case 13:
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpw]*FMOD)+((float)audio_buffer[samplepos%32768]*FMODF)+((float)audio_buffer[(samplepos+1)%32768]*FMODW);
	    break;
	  case 14:
	    w0=(float32_t) (buf16[0]-32768)/16384.0f;w1=(float32_t) (buf16[1]-32768)/16384.0f;w2=(float32_t) (buf16[2]-32768)/16384.0f;
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpw]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  case 15:
	    w0=(float32_t) audio_buffer[0]/16384.0f;w1=(float32_t)audio_buffer[1]/16384.0f;w2=(float32_t)audio_buffer[2]/16384.0f;
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    mono_buffer[x]=((float)(buf16[tmpw]-32768)*w0)+((float)(buf16[samplepos%32768]-32768)*w1)+((float)(buf16[(samplepos+1)%32768]-32768)*w2);
	    //	    mono_buffer[x]=((float)(buf16[tmpp])*w0)+((float)(buf16[samplepos%32768])*w1)+((float)(buf16[(samplepos+1)%32768])*w2);
	    break;
	  }
 
	  if (++del>=SAMPLESPEED){ 
	    if (wormflag[5]) dirry=newdirection[wormdir]; 
	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);
	    //	    VILLAGEWRITE=0; // TESTY!!!!
		    if (count<wrap && count>0)
		  {
		    samplepos+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;//+SAMPLEEXPAND;
		    if (dirry>0) samplepos=start; //forwards
		    else samplepos=start+wrap;
		  }
		  else if (VILLAGEWRITE==1) {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);//+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    if (wormflag[8]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=dirryy;
		    tmp=VILLAGEWSTART;tmper=VILLAGEWWRAP;
		    vilw=(tmp+(villagewpos%tmper))*2;
		    vilw=vilw%VILLAGESTACKPOS;
		    start=villager[vilw]>>1;
		    wrap=((villager[vilw+1]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		    tmpp=village_effects[vilw/2];
		    //		    printf("vill %d stackpos %d\n",vill,VILLAGESTACKPOS);
		    if (wrap==0) wrap=1;
		    if (dirry>0) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		    if (wormflag[2]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (dirry>0) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  }
	  }
	  }
#endif

#ifdef LACH

	morph_inv = 1.0f - (float32_t)FMODW;

	//	VILLAGEWRITE=EFFECTWRITE&3;
	VILLAGEWRITE=(EFFECTWRITE&48)>>4;	

	//	VILLAGEWRITE=2; // TESTY!
	if (VILLAGEWRITE==2){// moved AUG
	    tmpp=village_effects[vilw/2];
	  }
	  else tmpp=EFFECTWRITE&15;
	//	tmpp=0; // TESTY!

      	for (x=0;x<sz/2;x++){
	  switch(tmpp){ 
	  case 0:
	  default:
	         mono_buffer[x]=audio_buffer[samplepos%32768];
	    //	    mono_buffer[x]=buf16[samplepos%32768]-32768; // TESTY!!!!
	  break;
	  case 1:
	    tmp16=(buf16[samplepos%32768]-32768);
	    if (audio_buffer[samplepos%32768]<tmp16) tmp16=audio_buffer[samplepos%32768];
	    mono_buffer[x]=tmp16;
	  break;
	  // effects with/without clipping *, +, -, 
	  case 2:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 3:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 4:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
	  mono_buffer[x]=tmp32;
	  break;
	  case 5:
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv + (float32_t)audio_buffer[samplepos%32768] * FMODW;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  mono_buffer[x]=tmp32;
	  break;
	  case 6:
	    tmp16=audio_buffer[samplepos%32768]-(buf16[samplepos%32768]-32768);
	  mono_buffer[x]=tmp16;
	  break;
	  case 7:
	    tmp16=(buf16[samplepos%32768]-32768)-audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 8:
	    tmp16=(buf16[samplepos%32768]-32768)|audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 9:
	    tmp16=(buf16[samplepos%32768]-32768)&audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 10:
	    tmp16=(buf16[samplepos%32768]-32768)^audio_buffer[samplepos%32768];
	  mono_buffer[x]=tmp16;
	  break;
	  case 11:
	    tmp16=(buf16[samplepos%32768]-32768)<<(audio_buffer[samplepos%32768]&7);
	  mono_buffer[x]=tmp16;
	  break;
	  case 12:
	    tmp16=(buf16[samplepos%32768]-32768)>>(audio_buffer[samplepos%32768]&7);
	  break;
	  case 13:
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpw]*FMOD)+((float)audio_buffer[samplepos%32768]*FMODF)+((float)audio_buffer[(samplepos+1)%32768]*FMODW);
	    break;
	  case 14:
	    w0=(float32_t) (buf16[0]-32768)/16384.0f;w1=(float32_t) (buf16[1]-32768)/16384.0f;w2=(float32_t) (buf16[2]-32768)/16384.0f;
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    mono_buffer[x]=((float)audio_buffer[tmpw]*w0)+((float)audio_buffer[samplepos%32768]*w1)+((float)audio_buffer[(samplepos+1)%32768]*w2);
	    break;
	  case 15:
	    w0=(float32_t) audio_buffer[0]/16384.0f;w1=(float32_t)audio_buffer[1]/16384.0f;w2=(float32_t)audio_buffer[2]/16384.0f;
	    tmpw=samplepos-1;
	    tmpw=tmpw%32768;
	    //	    mono_buffer[x]=((float)(buf16[tmpp]-32768)*w0)+((float)(buf16[samplepos%32768]-32768)*w1)+((float)(buf16[(samplepos+1)%32768]-32768)*w2);
	    mono_buffer[x]=((float)(buf16[tmpw])*w0)+((float)(buf16[samplepos%32768])*w1)+((float)(buf16[(samplepos+1)%32768])*w2);

	    break;
	  }

	  	  if (++del>=SAMPLESPEED){
		    if (wormflag[3]) dirry=newdirection[wormdir]; 
	  	    else dirry=direction[SAMPLEDIRW]*SAMPLESTEP;	    
	    count=((samplepos-start)+dirry);// samplepos is start or start+wrap++
		    if (count<wrap && count>0)
	      {
		    samplepos+=dirry;
		  }
		else {
		  if (VILLAGEWRITE==0) {
		    start=SAMPLESTART;wrap=SAMPLEWRAP;//+SAMPLEEXPAND;
		     if (dirry>0) samplepos=start; //forwards
		    else samplepos=start+wrap;
		    count=samplepos;
		  }
		  else if (VILLAGEWRITE==1) {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  tmp=(ANYSTART+(anypos%ANYWRAP))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  samplepos=SAMPLESTART+(tmper%SAMPLEWRAP);//+SAMPLEEXPAND;
		  wrap=0;start=0;
		  }
		  else if (VILLAGEWRITE==2) {
		    if (wormflag[5]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagewpos+=dirryy;
		    //		    vill=(VILLAGEWSTART+(villagewpos%VILLAGEWWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2

		    tmp=VILLAGEWSTART;tmper=VILLAGEWWRAP;
		    vilw=(tmp+(villagewpos%tmper))*2;
		    vilw=vilw%VILLAGESTACKPOS;
		    tmpp=village_effects[vilw/2];
		    start=villager[vilw]>>1;
		    wrap=((villager[vilw+1]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		    if (wrap==0) wrap=1;
		    if (dirry>0) samplepos=start;
		    else samplepos=start+wrap;
		  }
		  else {
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP; 
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  start=buf16[tmp]>>1;
		    if (wormflag[1]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEP*direction[DATADIRR];
		  anypos+=dirryy;
		  wrapper=ANYWRAP;
		  tmp=(ANYSTART+(anypos%wrapper))%32768; //to cover all directions
		  wrap=((buf16[tmp]>>1)%SAMPLEWRAP);//+SAMPLEEXPAND;
		  if (wrap==0) wrap=1;
		  if (dirry>0) samplepos=start;
		  else samplepos=start+wrap;
		  }
		}
	  del=0;
	  	  }
	}
	/////////////////////////////NO____LACH!!!!!!!!!
#else // end LACH

	///	///	///	/// FILTER process

if (digfilterflag&1){ 

	ldst=left_buffer;
	rdst=right_buffer;
	morph_inv = 1.0f - (float32_t)FMODF;

	  ////////////////////////////////////LDST effects also...
	//	EFFECTFILT=(EFFECTWRITE+EFFFOFFSET)%64;
	//	VILLAGEFILT=EFFECTFILTER&3;
	VILLAGEFILT=(EFFECTFILTER&48)>>4;	
	//	VILLAGEFILT=0; // TESTY!

	if (VILLAGEFILT==2){// move out of loop AUG
	    tmpp=village_effects[vilf/2];
	  }
	  else tmpp=EFFECTFILTER&15;

	///	tmpp=0;// TESTY!

      	for (x=0;x<sz/2;x++){ 
 	  switch(tmpp){ 
	  case 0:
	  default:
	  *(ldst++)=audio_buffer[sampleposfilt%32768];
	  rdst++;
	  break;
	  case 1:
	  *(ldst++)=*(rdst++);
	  break;
	  case 2:
	    *(ldst++) =0;
	  rdst++;
	    break;
	  case 3:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 4:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 5:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 6:
	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  case 7:
	    fsum=(float32_t)*ldst * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 8:
	    fsum=(float32_t)(*ldst) * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  ////////////////////
	  case 9:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 10:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*ldst) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*ldst) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  rdst++;
	  break;
	  case 11:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
	  *(ldst++)=tmp32;
	  break;
	  case 12:
	    //	    fsum=(float32_t)audio_buffer[samplepos%32768] * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	    fsum=(float32_t)(buf16[samplepos%32768]-32768) * morph_inv * (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  case 13:
	    //	    fsum=(*ldst) & *(rdst++);
	    //	    tmp32=fsum;
	    *(ldst++)=(*ldst) & *(rdst++);
	    //	    *(ldst++)=(*ldst);
	  break;
	  case 14:
	    fsum=(float32_t)(*ldst) * morph_inv + (float32_t)(*(rdst++)) * FMODF;
	  tmp32=fsum;
#ifndef PCSIM
	  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
#endif
	  *(ldst++)=tmp32;
	  break;
	  case 15:
	    ldst++; 	    // AUG -ldst left as it is
	    //	  tmp32=audio_buffer[sampleposfilt%32768]| *ldst;
	    //	  *(ldst++)=tmp32;
	    rdst++;
	  break;
	  }
	  ///	  HERE////////////////////////--->>>>

	  	  if (++delf==SAMPLESPEEDFILT){
		    if (wormflag[6]) dirry=newdirection[wormdir]; 
		    else dirry=direction[SAMPLEDIRF]*SAMPLESTEPFILT;	    
	    count=((sampleposfilt-startfilt)+dirry);
		    if (count<wrapfilt && count>0)
		  {
		    sampleposfilt+=dirry;//)%32768;
		  }
		else {
		  if (VILLAGEFILT==0) {
		    startfilt=SAMPLESTARTFILT;wrapfilt=SAMPLEWRAPFILT;//+SAMPLEFEXPAND;
		    if (dirry>0) sampleposfilt=startfilt; //forwards
		    else sampleposfilt=startfilt+wrapfilt;
		  }

		  else if (VILLAGEFILT==1) {
		    if (wormflag[3]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPFILT*direction[DATADIRR];
		  anyposfilt+=dirryy;
		  tmp=(ANYSTARTFILT+(anyposfilt%ANYWRAPFILT))%32768; //to cover all directions
		  tmper=buf16[tmp]>>1;	
		  sampleposfilt=SAMPLESTARTFILT+(tmper%SAMPLEWRAPFILT);//+SAMPLEFEXPAND;
		  wrapfilt=0;startfilt=0;
		  }
		  else if (VILLAGEFILT==2) {
		    if (wormflag[9]) dirryy=newdirection[wormdir]; 
		    else dirryy=VILLAGEWSTEP*direction[VILLAGEWDIR];
		    villagefpos+=dirryy;
		    //		    vill=(VILLAGEFSTART+(villagefpos%VILLAGEFWRAP)*2)%VILLAGESTACKPOS; //VILLAGESTACKPOS always +-2

		    tmp=VILLAGEFSTART;tmper=VILLAGEFWRAP;
		    vilf=(tmp+(villagefpos%tmper))*2;
		    vilf=vilf%VILLAGESTACKPOS;
		    tmpp=village_effects[vilf/2];
		    startfilt=villager[vilf]>>1;
		    wrapfilt=((villager[vilf+1]>>1)%SAMPLEWRAPFILT);//+SAMPLEFEXPAND;
		    if (wrapfilt==0) wrapfilt=1;
		    if (dirry>0) sampleposfilt=startfilt;
		    else sampleposfilt=startfilt+wrapfilt;
		  }
		  else {
		    if (wormflag[3]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPFILT*direction[DATADIRF];

		  anyposfilt+=dirryy;
		  wrapper=ANYWRAPFILT; 
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  startfilt=buf16[tmp]>>1;
		    if (wormflag[3]) dirryy=newdirection[wormdir]; 
		    else dirryy=ANYSTEPFILT*direction[DATADIRF];

		  anyposfilt+=dirryy;
		  wrapper=ANYWRAPFILT;
		  tmp=(ANYSTARTFILT+(anyposfilt%wrapper))%32768; //to cover all directions
		  wrapfilt=((buf16[tmp]>>1)%SAMPLEWRAPFILT);//+SAMPLEFEXPAND;
		  if (wrapfilt==0) wrapfilt=1;
		  if (dirry>0) sampleposfilt=startfilt;
		  else sampleposfilt=startfilt+wrapfilt;
		  }
		}
	  delf=0;
		  }
	}
 }

#endif // for LACH

