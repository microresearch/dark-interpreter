//17 june


      // +effects 
      // 1-settings=64
      // 2-stacker=64x4(howmuch.start.end.cpu)=256
      // 3-stackery=64x4=256
      // 4-cpu=64
      // 5-exestack=	 4
      // [5-foldback=xx?? = walker for datagen into xxxxx]
      // 6-villager=128
      //7	      village_effects=64 (64 total villagers)
      // 8datagen
      //+ directions
 
      // <-> datagen (as ongoing, or not with its own walker which is
      // the foldback setting), finger, knob(s), EEG

      // + processes across arrays...
      // + swop across arrays and within arrays
      // + dump into datagen
      // ??? navigation of overlaid grids - walkers???means?	      
      // toggle as down. bare finger as up...


//16 june mode/mirror/attach code:

      u8 xx;



      mastermode=adc_buffer[FIRST]>>7; // 32=5 bits

      eff[0]=adc_buffer[SECOND]>>5; // was 7 bits=128//jitter???
      eff[1]=adc_buffer[THIRD]>>5;
      eff[2]=adc_buffer[FOURTH]>>5;

      /* // TESTCODE for tweaking fingers
	xx=fingerdirupdown();
      if (xx==0){//THIS IS UP!!!!!
	  settingsarray[12]=65530;
	}
      if (xx==1){
	  settingsarray[12]=8;
	  }
      */
      
      //      mastermode=30; // TESTY!!
      // re-doing modes
 
	switch(mastermode){
      case 0: // GROUPS0=EFFMODE
	xx=fingerdir(); // 0-3 -change knob assign depends on fingers
	if (xx!=5){
#ifdef LACH
	EFFECTREAD=eff[xx%2];
	EFFECTWRITE=eff[(xx+1)%2];
	settingsarray[12]=adc_buffer[FOURTH]<<4;
#else
	EFFECTREAD=eff[xx%3];
	EFFECTWRITE=eff[(xx+1)%3];
	EFFECTFILTER=eff[(xx+2)%3];
#endif
	}
	  break;
      case 1: //GROUPS3: expand in this case (or contract?-fingers) R/W/F
	// R 52
	xx=fingerdirupdown();
	if (xx==0){ // expand
	  settingsarray[52]=adc_buffer[SECOND]<<4;

	// W 51
	  settingsarray[51]=adc_buffer[THIRD]<<4;	  

	// F 53
	  settingsarray[53]=adc_buffer[FOURTH]<<4;
	}

	else if (xx==1){ // wrap
	  settingsarray[13]=adc_buffer[SECOND]<<4;

	// W 51
	  settingsarray[12]=adc_buffer[THIRD]<<4;	  

	// F 53
	  settingsarray[15]=adc_buffer[FOURTH]<<4;
	}

	break;
      case 2:
	/// access stacks and CPUs- UP and DOWN! just CPUs now!
	  
	  //  SIM: STACKER[STACK_SIZE*4]:16*4=48 start,wrap,howmuch,CPU
//	    CA: STACKERY[STACK_SIZE*4]:16*4=48 start,wrap,howmuch,CPU
//	    CPUINT:
//	    cpur to max m->m_threadcount (max is 120=7 bits)
//	    m->m_threads[cpur].m_CPU=cpu%31;
	xx=fingerdirupdown();
	if (xx==0){
	//SIM/CA/CPU:
	  stacker[(((eff[0]>>1)%STACK_SIZE)*4)+3]+=1;
	  stackery[(((eff[1]>>1)%STACK_SIZE)*4)+3]+=1;
	  cpur=(eff[2])%(m->m_threadcount);
	  cpu--;
	  m->m_threads[cpur].m_CPU=cpu;
	}
      	else if (xx==1){
	  //SIM/CA/CPU:
	  //if ((rand()%2)==1){
	  stacker[(((eff[0]>>1)%STACK_SIZE)*4)+3]-=1;
	  stackery[(((eff[1]>>1)%STACK_SIZE)*4)+3]-=1;
	  cpur=(eff[2])%(m->m_threadcount);
	  cpu-=1;
	  m->m_threads[cpur].m_CPU=cpu%31;
	}

	break;
	////
      case 3: //GROUPS5	// X and Y as chunk start/wrap	// Z as knob
	// total 64
	if (fingerdirupdown()==1){
	  groupstart=eff[0]>>1; // 6bits
	groupwrap=eff[1]>>1;
	for (x=0;x<groupwrap;x++){
	  settingsarray[(groupstart+x)%64]=adc_buffer[FOURTH]<<4;
	}
	}
	break;
      case 4: //GROUPS6 // algo group1: X,Y,Z
	if (fingerdirupdown()==1){
	  groupstart=datagenbuffer[adc_buffer[SECOND]<<4]>>2;// 16 bits//6 bits
	  groupwrap=datagenbuffer[adc_buffer[THIRD]<<4]>>2;// 16 bits//6 bits
	for (x=0;x<groupwrap;x++){
	  settingsarray[(groupstart+x)%64]=adc_buffer[FOURTH]<<4;
	}
	}
	break;

      case 5: //FINGERS4 // into stacks=push and pop = EXE,CA,SIM,CPU,PURE,VILLAGER
	// HERE if we wanted to knobs could set START,WRAP and FUNC! TO TESTY!!!

	// START=7/WRAP=21/FUNC=49
	settingsarray[7]=adc_buffer[SECOND]<<4;
	settingsarray[21]=adc_buffer[THIRD]<<4;
	settingsarray[49]=eff[2]<<9;
	whichpushpop=fingervalright(whichpushpop,8);
	//	whichpushpop=7; // TESTY!
	xx=fingerdirupdown();
	if (xx==0){//THIS IS UP!!!!!
	  // push which
	  switch(whichpushpop){
	  case 0:
	    cpustackpush(m,datagenbuffer,STACKSTART,STACKWRAP,STACKFUNC%31,STACKMUCH);
	    break;
	  case 1:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,audio_buf,stack_posy,STACKMUCH,STACKSTART,STACKWRAP);	
	    break;
	  case 2:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,datagenbuffer,stack_posy,STACKMUCH,STACKSTART,STACKWRAP);	    
	    break;
	  case 3:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,buf16,stack_pos,STACKMUCH,STACKSTART,STACKWRAP);
	    break;
	  case 4:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,audio_buffer,stack_pos,STACKMUCH,STACKSTART,STACKWRAP);
	    break;
	  case 5:
	    villagestackpos=villagepush(villagestackpos,STACKSTART,STACKWRAP,STACKFUNC%16);//pos/start/wrap
	    break;
	  case 6:
	    cpustackpushhh(datagenbuffer,STACKSTART,STACKWRAP,STACKFUNC%31,STACKMUCH);
	    break;
	  case 7:
	    exenums=exestackpush(exenums,exestack,STACKFUNC%4); //exetype=0-3;
	  }
      }
	else if (xx==1){

	// pop which
	  switch(whichpushpop){
	  case 0:
	    //pop CPU
	    cpustackpop(m);
	    break;
	  case 1:
	  case 2:
	    //pop CA
	    stack_posy=ca_pop(stack_posy);
	    break;
	  case 3:
	  case 4:
	    //pop SIM
	    stack_pos=func_pop(stack_pos);
	    break;
	  case 5:
	    //pop village
	    villagestackpos=villagepop(villagestackpos);
	    break;
	  case 6:
	    // pop pureleak
	    cpustackpoppp(datagenbuffer);
	      break;
	  case 7:
	    exenums=exestackpop(exenums,exestack);
	  }
      }
	break;

	//13/14/15=bare into CODE, VILLAGERS, SETTINGS (navigate and CHANGE SETTING)
	//all as one

      case 6:
	codepos=fingervalright(codepos,216); // TODO is stacker fixed as so=216!!! TESTY!
	if (codepos<48){
	  stacker[codepos]=(adc_buffer[DOWN]<<3);
	}
	else if (codepos<96){
	  stackery[codepos-48]=(adc_buffer[DOWN]<<3);
	}
	  else {
	  cpur=(codepos-96)%(m->m_threadcount);
	  cpu=(adc_buffer[DOWN]>>7)%31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	break;

      case 14:
	villagepos=fingervalright(villagepos,VILLAGE_SIZE);
	villager[villagepos]=(adc_buffer[DOWN]<<3);
	// TEST on wrap which is 12
	//	settingsarray[12]=fingervalright(settingsarray[12],254);
	break;

      case 15:
	settingspos=fingervalright(settingspos,64);
	settingsarray[settingspos]=(adc_buffer[DOWN]<<4);
	break;


	///////////////////////////////////////////////////////////////////////////
	// MIRROR
	// 16-23
	// 8 mirror modes
	// 3 knobs=//region//mirror walkers??? CHECK discard
	// mirror process set/unset by fingers
	// modes: mirror into 1/settingsarray 2/villager 3/stacks/CPU 4/foldback
	// mirror operations: buf16 walker(set it), EEG, where are swaps(or swaps above)
	// knob1/2/3: mirror walkers/settings

	/// NOTE DOWN=TO SET!!!
	///////////////////////////////////////////////////////////////////////////

      case 16: 
	// set FOLD 0,1,2
	FOLDD[0]=eff[0]; // how much
	FOLDD[1]=adc_buffer[THIRD]<<4; // start
	FOLDD[2]=eff[2]; // wrap

	for (x=0;x<((FOLDD[0]>>1)+1);x++){ //was >>9
	  settingsarray[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>10)+1)))%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>1)+1)))%32768];
	  coo++;
	}
	// toggle flag with the finger
	xx=fingerdirupdown();
	if (xx==1) m1flag|=1; //sets
	else if (xx==0) m1flag&=~1;
	break;

      case 17: 
	// set FOLD 0,1,2
	FOLDD[0]=eff[0];
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){ //was >>9
	  settingsarray[(((FOLDD[1])>>1)+(x%((FOLDD[2]>>1)+1)))%64]=(randi()<<4);
	  coo++;
	}
	// toggle flag with the finger
	xx=fingerdirupdown();
	if (xx==1) m1flag|=2; //sets=DOWN
	else if (xx==0) m1flag&=~2; 
	break;

      case 18:
	// set FOLD 0,1,2
	FOLDD[0]=eff[0];
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  villager[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>1)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>1)+1)))%32768]>>1;
	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=4; //sets
	else if (xx==0) m1flag&=~4; 
	break;

      case 19:
	// set FOLD 0,1,2
	FOLDD[0]=eff[0];
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  villager[(((FOLDD[1])>>1)+(x%((FOLDD[2]>>1)+1)))% VILLAGE_SIZE]=(randi()<<3);
	    coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=8; //sets
	else if (xx==0) m1flag&=~8; 
	break;

      case 20: // STACKS and CPU
	// set FOLD 0,1,2
	FOLDD[0]=adc_buffer[SECOND]<<4;
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=adc_buffer[FOURTH]<<4;
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[1])>>8)+(x%((FOLDD[2]>>8)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stacker[tmper]%=32768;
	}
	else if (tmper<96){
	  stackery[tmper-48]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stackery[tmper-48]%=32768;
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=16; //sets
	else if (xx==0) m1flag&=~16; 
	break;

      case 21: // STACKS and CPU
	// set FOLD 0,1,2
	FOLDD[0]=adc_buffer[SECOND]<<4;
	FOLDD[1]=adc_buffer[THIRD]<<4;
	FOLDD[2]=adc_buffer[FOURTH]<<4;
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[1])>>8)+(x%((FOLDD[2]>>10)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=(randi()<<3);
	  stacker[tmper]%=32768;
	}
	else if (tmper<96){
	  stackery[tmper-48]=(randi()<<3);
	  stackery[tmper-48]%=32768;
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=randi()>>7;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=32; //sets
	else if (xx==0) m1flag&=~32; 
	break;

      case 22:
	// set FOLD 0,1,2 ---> FOLDBACK!
	FOLDD[0]=eff[0];
	FOLDD[1]=adc_buffer[THIRD];
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  FOLDD[(((FOLDD[1])>>10)+(x%((FOLDD[2]>>1)+1)))% FOLD_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>1)+1)))%32768]>>1;
	  	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=64; //sets
	else if (xx==0) m1flag&=~64; 
	break;

      case 23:
	// set FOLD 0,1,2 ---> FOLDBACK!
	FOLDD[0]=eff[0];
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];

	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  FOLDD[(((FOLDD[1])>>1)+(x%((FOLDD[2]>>1)+1)))% FOLD_SIZE]=randi()<<4;
	  	  coo++;
	}
	xx=fingerdirupdown();
	if (xx==1) m1flag|=128; //sets
	else if (xx==0) m1flag&=~128; 
	break;

	///////////////////////////////////////////////////////////////////////////
	// MIRROR_MIRROR//swop=mirror region to region of...???
	// 
	// 1/settingsarray 2/villager 3/stacksandCPU 4/foldback
	//24-1-1//25-2-2//26-3-3//27-4-4/28-2-3(start<->end only)//29-3->2
	//toggle swop or not?
	// knobs=start/wrap/offset to copy+
      case 24:
	// settingsarray to itself
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[0]>>1)+1);x++){
	  settingsarray[(((FOLDD[1])>>1)+x)%64]=settingsarray[((FOLDD[2]>>1)+FOLDD[1]+x)%64];
	}
	break;

      case 25:
	// villagerarray to itself
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[0]>>1)+1);x++){ //was >>9
	  //	  settingsarray[(((FOLDD[0])>>10)+x)%64]=settingsarray[FOLDD[2]+(((FOLDD[1])>>10)+x)%64];
	  villager[(((FOLDD[1])>>1)+x)%VILLAGE_SIZE]=villager[((FOLDD[2]>>1)+((FOLDD[1])>>1)+x)%VILLAGE_SIZE];
	}
	break;

      case 26:
	// stacks and CPUs - how to deal with crossover (also missmatch of values?)??? TODO!
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=adc_buffer[THIRD];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[0]>>4)+1);x++){
	}
	break;

      case 27:
	// foldback
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	for (x=0;x<((FOLDD[0]>>1)+1);x++){ 
	  FOLDD[(((FOLDD[1])>>1)+x)% FOLD_SIZE]=FOLDD[((FOLDD[2]>>1)+((FOLDD[1])>>1)+x)% FOLD_SIZE];
	}
	break;

      case 28:
	// starts and ends only of stacks (not CPU) -> villagers
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	//	settingsarray[12]=128; // TESTY!!!!
	for (x=0;x<((FOLDD[0]>>1)+1);x++){ 
	  tmper=((FOLDD[1])+x)%(STACK_SIZE*2); // so both stacks entry point
	  villagerdest=(((FOLDD[1])>>1)+x+(FOLDD[2]>>1))%(VILLAGE_SIZE/2); // village entry
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
	break;

      case 29:
	// starts and ends only of villagers -> stacks
	// starts and ends only of stacks (not CPU) -> villagers
	FOLDD[0]=eff[0]; // 7 bits
	FOLDD[1]=eff[1];
	FOLDD[2]=eff[2];
	//	settingsarray[12]=128; // TESTY!!!!
	for (x=0;x<((FOLDD[0]>>1)+1);x++){ 
	  tmper=((FOLDD[1])+x)%(STACK_SIZE*2); // so both stacks entry point
	  villagerdest=(((FOLDD[1])>>1)+x+(FOLDD[2]>>1))%(VILLAGE_SIZE/2); // village entry
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

	break;

	//30->into settingsarray
	// UPDOWNLEFTRIGHT=1-process-buf16int/2-fingerbare/3-knob/4-detach
	// knob1,2=group settings/extent
	// knob3=setting
	// TODO: to toggle

      case 30:
	groupstart=eff[0]>>1; // 6bits
	groupwrap=eff[1]>>1;

	for (x=0;x<groupwrap;x++){
	switch(fingerdir()){
	case 0:
	  settingsarray[(groupstart+x)%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  settingsarrayattached[(groupstart+x)%64]=1;
	  coo++;
	  break;
	case 1:
	  settingsarray[(groupstart+x)%64]=adc_buffer[DOWN]<<4;
	  settingsarrayattached[(groupstart+x)%64]=2;
	  break;
	case 2:
	  settingsarray[(groupstart+x)%64]=adc_buffer[FOURTH]<<4;
	  settingsarrayattached[(groupstart+x)%64]=3;
	  break;
	case 3: //left
	  // DEtach
	  settingsarrayattached[(groupstart+x)%64]=0;
	  break;
	}
	}
	break;

      case 31:
	///infection across buffer: knobs; speed,probability,buffer
	//set according to probability
	if (eff[0]==0){
	  for (x=0;x<64;x++){
	    if ((rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1; // infected
	  else settingsarrayinfected[x]=0;
	  }
	  // run infection at speed eff[0] 

	  for (x=0;x<64;x++){
	    // infection - how many infected (not dead) round each one?
	    if (++del==eff[0]){
	      if (settingsarrayinfected[x]==0 && ((settingsarrayinfected[(x-1)%64]>=1 && settingsarrayinfected[x-1]<128) || (settingsarrayinfected[(x+1)%64]>=1 && settingsarrayinfected[x+1]<128)) && (rand()%255) > (adc_buffer[THIRD]>>4)) settingsarrayinfected[x]=1;
	    // inc
	    if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128) settingsarrayinfected[x]++;
	    }

	  // overmap onto buffer eff[0]: 0=stay same/infect=reduce by days/dead=128=zero
	    //0/settingsarray 1/villager 2/3/4//stacksandCPU 5/foldback	    
	    switch(eff[2]>>4) // 8 cases
	      {
	      case 0:
	      default:
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	settingsarray[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) settingsarray[x]=0;
		break;
	      case 1:
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	stacker[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) stacker[x]=0;
		break;
	      case 2:
 		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	stackery[x]-=settingsarrayinfected[x];
		else if (settingsarrayinfected[x]>127) stackery[x]=0;
		break;
	      case 4:
 		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	{
		  cpur=x%(m->m_threadcount);
		  m->m_threads[cpur].m_CPU-=settingsarrayinfected[x];//cpu
		  m->m_threads[cpur].m_CPU%=31;
		}
		else if (settingsarrayinfected[x]>127) {
		  cpur=x%(m->m_threadcount);
		  m->m_threads[cpur].m_CPU=0;//cpu
		}
		break;
	      case 5:
		if (x<14){
		if (settingsarrayinfected[x]>0 && settingsarrayinfected[x]<128)	FOLDD[x]-=settingsarrayinfected[x]; // foldd max
		else if (settingsarrayinfected[x]>127) FOLDD[x]=0; // foldd max
		}
		break;
	      }
	    /////
	  }
	}
	break;
      }
      
      //// DEAL WITH toggled MIRRORING

      if (m1flag&1){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  settingsarray[(((FOLDD[3])>>10)+(x%((FOLDD[4]>>10)+1)))%64]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  coo++;
	}
      }

      if (m1flag&2){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  settingsarray[(((FOLDD[4])>>10)+(x%((FOLDD[5]>>10)+1)))%64]=(randi()<<4);
	  coo++;
	}
      }

      if (m1flag&4){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  villager[(((FOLDD[6])>>10)+(x%((FOLDD[7]>>10)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768]>>1;
	  coo++;
	}
      }

      if (m1flag&8){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  villager[(((FOLDD[8])>>10)+(x%((FOLDD[9]>>10)+1)))% VILLAGE_SIZE]=(randi()<<3);
	  coo++;
	}
      }

      if (m1flag&16){ 
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[10])>>8)+(x%((FOLDD[11]>>8)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stacker[tmper]%=32768;
	}
	else if (tmper<96){
	  stackery[tmper-48]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  stackery[tmper-48]%=32768;
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>8)+1)))%32768]>>1;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
      }

      if (m1flag&32){ 
	for (x=0;x<((FOLDD[0]>>8)+1);x++){ // TODO: goes high enough
	  tmper=(((FOLDD[10])>>8)+(x%((FOLDD[11]>>8)+1)))%216;
  	if (tmper<48){
	  stacker[tmper]=(randi()<<3);
	}
	else if (tmper<96){
	  stackery[tmper-48]=(randi()<<3);
	}
	  else {
	  cpur=(tmper-96)%(m->m_threadcount);
	  cpu=randi()>>7;
	  cpu%=31;
	  m->m_threads[cpur].m_CPU=cpu;
	  }
	    coo++;
	}
      }

      if (m1flag&64){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  FOLDD[(((FOLDD[11])>>10)+(x%((FOLDD[12]>>10)+1)))% FOLD_SIZE]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768]>>1;
	  	  coo++;
	}
      }

      if (m1flag&128){ 
	for (x=0;x<((FOLDD[0]>>10)+1);x++){
	  FOLDD[(((FOLDD[12])>>10)+(x%((FOLDD[13]>>10)+1)))% FOLD_SIZE]=randi()<<4;
	  	  coo++;
	}
      }

      ///////////////////////////////
      /// DEAL with settingsattach:

      for (x=0;x<64;x++){
	switch(settingsarrayattached[x]){
	case 1:
	  settingsarray[x]=buf16[((FOLDD[1]>>1)+(coo%((FOLDD[2]>>10)+1)))%32768];
	  break;
	case 2:
	  settingsarray[x]=adc_buffer[DOWN]<<4;
	  break;
	case 3:
	  settingsarray[x]=adc_buffer[FOURTH]<<4;
	  break;
	}
      }


// 11 june


	/*
      machine_count++;
      if (machine_count>=MACHINESPEED){

      for (x=0;x<4;x++){
	switch(exeperms[((EXESPOT%22)*4)+x]){
	case 0:
	  func_runall(stackyy,stack_pos); // simulations
	  break;
	case 1:
	  ca_runall(stackyyy,stack_posy); // CA
	  break;
	case 2:
	  machine_run(m);
	    m->m_leakiness=leakiness;
	    m->m_infectprob=infection;
	    machine_count=0;
	  break;
	case 3:
	  leak_count++;
	  if (leak_count>=LEAKSPEED){
	    machine_runnn(datagenbuffer);
	    leak_count=0;
	  }
	}
	}
      } // end of machine count
	*/


  //  u8 exeperms[88]={0,1,2,3, 0,1,3,2, 0,2,3,1 ,0,2,1,3, 0,3,1,2, 0,3,2,1, 1,0,2,3, 1,0,3,2, 1,2,3,0, 1,2,0,3, 1,3,2,0, 1,3,0,2, 2,1,0,3, 2,1,3,0, 2,3,1,0, 2,3,0,1, 3,0,1,2, 3,0,2,1, 3,1,0,2, 3,1,2,0, 3,2,0,1, 3,2,1,0}; 


	//	if (fingerdirupdown()==0){
	if ((rand()%6)==1){
	//SIM/CA/CPU:
	  /*	  stacker[(adc_buffer[SECOND]>>4)%(STACK_SIZE*4)]++;
	  stackery[(adc_buffer[THIRD]>>4)%(STACK_SIZE*4)]++;
	  cpur=(adc_buffer[FOURTH]>>5)%(m->m_threadcount);
	  */
	  //stacker[(rand()%256)%(STACK_SIZE*4)]++;
	  stackery[(rand()%256)%(STACK_SIZE*4)]++;
	  cpur=((rand()%256))%(m->m_threadcount);

	  cpu++;
	  m->m_threads[cpur].m_CPU=cpu%31;
	}
      //	else if (fingerdirupdown()==1){
	  //SIM/CA/CPU:
	if ((rand()%2)==1){
	  /*	  stacker[(adc_buffer[SECOND]>>4)%(STACK_SIZE*4)]--;
	  stackery[(adc_buffer[THIRD]>>4)%(STACK_SIZE*4)]--;
	  cpur=(adc_buffer[FOURTH]>>5)%(m->m_threadcount);*/
	  //stacker[(rand()%256)%(STACK_SIZE*4)]--;
	  stackery[(rand()%256)%(STACK_SIZE*4)]--;
	  cpur=((rand()%256))%(m->m_threadcount);

	  cpu--;
	  m->m_threads[cpur].m_CPU=cpu%31;


// 10 JUNE:

//RETHINK


      /* FIRST as modes - 8 modes/8 fingermods

	 0- EFFMODE
	 1,2,3- GROUPMODE1,2,3
	 5,6,7,8- MIRRORMODE5,6,7,8 // or so - to figure OUT!

	 fingers:
	 0-fingers in the code0stacker,1stackery,2CPUs
	 3-in settings
	 4-in stack
	 5-in datagens
	 6,7-contract/expand of groups eg. mirror and wraps

       */

      switch(mainmode){
      case 0: // EFFMODE
      default:
	// if enter this case then must be a change (from new mode knobs) to register...
	if (changeflagone==1){	  //new case - check for change
	  if (effr!=oldeffr && effr!=oldeffr-1 && effr!=oldeffr+1) {
	  //	  if (effr!=oldeffr) {
	    EFFECTREAD=effr;
	  changeflagone=0;
	}
	}
	  else {
	    EFFECTREAD=effr;
	  }

	if (changeflagtwo==1){	  //new case - check for change
	  if (effw!=oldeffw && effw!=oldeffw-1 && effw!=oldeffw+1) {
	  //	  if (effw!=oldeffw) {
	    EFFECTWRITE=effw;
	  changeflagtwo=0;
	}
	}
	  else {
	    EFFECTWRITE=effw;
	  }

	// TODO deal with LACH here!

	if (changeflagthree==1){	  //new case - check for change
	  //	  if (efff!=oldefff) {
	  if (efff!=oldefff && efff!=oldefff-1 && efff!=oldefff+1) {
	  changeflagthree=0;
	  EFFECTFILTER=efff;
	}
	}
	  else {
	    EFFECTFILTER=efff;
	  }
	  break;
      case 1:

	// knobs change groups X,Y,Z- preformed:
	// X-START Y-WRAP Z-SPEED
	// if enter this case then must be a change (from new mode knobs) to register...
	
	if (changeflagone==1){	  //new case - check for change
	  // STEP NOW!
	  //	  if (effr!=oldeffr){
	  if (effr!=oldeffr && effr!=oldeffr-1 && effr!=oldeffr+1) {
	    for (x=25;x<35;x++){
	      settingsarray[x]=adc_buffer[SECOND]<<4; // 16 bit value
	    }
	  changeflagone=0;
	  }
	}
	  else {
	    for (x=25;x<35;x++){
	      settingsarray[x]=adc_buffer[SECOND]<<4; // 16 bit value
	    }
	  }

	if (changeflagtwo==1){	  //new case - check for change
	  // WRAP!
	  //	  if (effw!=oldeffw){
	  if (effw!=oldeffw && effw!=oldeffw-1 && effw!=oldeffw+1) {
	    for (x=11;x<25;x++){
	      	      settingsarray[x]=adc_buffer[THIRD]<<4; // 16 bit value
	    }
	  changeflagtwo=0;
	}
	}
	  else {
	    for (x=11;x<25;x++){
	      	      settingsarray[x]=adc_buffer[THIRD]<<4; // 16 bit value
	    }
	  }

	if (changeflagthree==1){	  //new case - check for change
	  // SPEED!
	  //	  if (efff!=oldefff){
	  if (efff!=oldefff && efff!=oldefff-1 && efff!=oldefff+1) {
	    for (x=35;x<41;x++){
	      	      settingsarray[x]=adc_buffer[FOURTH]<<4; // 16 bit value
	    }	   
	  changeflagthree=0;
	}
	}
	  else {
	    for (x=35;x<41;x++){
	         settingsarray[x]=adc_buffer[FOURTH]<<4; // 16 bit value
	    }	   
	  }
	    break;
      case 2:
	// knobs change groups X,Y,Z
	// X and Y as chunk start/wrap
	// Z as knob
	break;
      case 3:
	// knobs change groups X,Y,Z
	// algo walker???? TODO???
	break;
      case 4:
	// knobs change mirror settings - how to toggle?
	break;
      case 5:
	// knobs change mirror settings
	break;
      case 6:
	// knobs change mirror settings
	break;
      case 7:
	// knobs change mirror settings
	break;
	    }
     
      oldeffr=effr;
      oldeffw=effw;
      oldefff=efff;

      if (oldmainmode!=mainmode) {
	changeflagone=1;changeflagtwo=1;changeflagthree=1;
      }

      oldmainmode=mainmode;

////

      //#define SAMPLEWRAP ((settingsarray[12]>>1)+1)

      settingsarray[12]=adc_buffer[SECOND]<<1; // wrap

      settingsarray[51]=adc_buffer[THIRD]<<4; // expand TODO


 for (x=0;x<STACK_SIZE*4;x+=4){
   stacker[x+3]=adc_buffer[FIRST]>>7; // CPU!!!
   //   stacker[x+3]=25;
   //   stackery[x+3]=(adc_buffer[FIRST]>>8)%9;
   //   stackery[x+3]=8;
 }

/// 9 JUNE

      /////////////////////////////
      // KKNOBBBSSS

      // TODO: should these be before or after mirroring-TEST!

#ifdef LACH
	  //SAMPLEWRAP (out-play) TO TEST!!

	  for (x=0;x<14;x++){
	    settingsarray[11+x]=adc_buffer[FIRST]<<4; // 16 bit value
	  }

      effects=adc_buffer[SECOND]>>5;  // 7 bits

      /*      if (effectmod&1 || effectmod&4) settingsarray[51]=effects<<9;
	      if (effectmod&2) settingsarray[52]=effects<<9; // WRITE=PLAY*/

      // TESTY - note that always sets effects to be the SAME!! TODO!

      if (effectmod&1 || effectmod&4) EFFECTREAD=effects;//<<9; FIXED!!!
      if (effectmod&2) EFFECTWRITE=effects;//<<9; // WRITE=PLAY*/

#else
      tmphardware=0;
      for (x=0;x<256;x++){ // was 256
	tmphardware+=adc_buffer[FIRST]>>5; // 7 bits
      }
            HARDWARE=tmphardware>>8; //was >>8 to divide average
      //      hardware=rand()%127; // TESTY!
            effects=adc_buffer[SECOND]>>5;  // 7 bits

      // TESTY - note that always sets effects to be the SAME!! TODO!

	    if (digfilterflag&1){
      if (effectmod&1) EFFECTREAD=effects;
      if (effectmod&2) EFFECTWRITE=effects; // WRITE=PLAY
      if (effectmod&4) EFFECTFILTER=effects; 
	    }
	    else
	      {
      if (effectmod&1 || effectmod&4) EFFECTREAD=effects;
      if (effectmod&2) EFFECTWRITE=effects; // WRITE=PLAY
      }


#endif

      // KNOB FIVE = knobby //TODO: underused
      knobby=adc_buffer[FIFTH];
      // KNOB THREE = modifiers and fingers
      fingermod=adc_buffer[THIRD]>>7;// 5 bits=32 
      fingerfing=fingermod&1; //finger open0 or as dir
      fingermod=fingermod>>1; //4 bits=16
      //      fingermod=13; // TESTY!!!
      //      fingermod=0;
      switch(fingermod){
      case 0:
	//effectmod
	effectmod=fingervaleff(effectmod); // return 1-7
	break;
      case 1:
	// stack
	whichstack=fingervalright(whichstack,5);
	//	whichstack=3; // TESTY!!
	//	whichstack=rand()%5; //TESTY
	if (fingerdirupdown()==1) {
	  switch (whichstack){ // which stack to push=0-4
	  case 0:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,audio_buf,stack_posy,stackmuch,STACKSTART,STACKWRAP);	    
	    break;
	  case 1:
	    stack_posy=ca_pushn(stackyyy,STACKFUNC%NUM_CA,datagenbuffer,stack_posy,stackmuch,STACKSTART,STACKWRAP);	    
	    break;
	  case 2:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,audio_buffer,stack_pos,stackmuch,STACKSTART,STACKWRAP);
	    break;
	  case 3:
	    stack_pos=func_pushn(stackyy,STACKFUNC%NUM_FUNCS,buf16,stack_pos,stackmuch,STACKSTART,STACKWRAP);
	    break;
	  case 4:
	    villagestackpos=villagepush(villagestackpos,STACKSTART,STACKWRAP);//pos/start/wrap
			  }
	}
	else if (fingerdirupdown()==0){
	  ///	  whichstack=1; // TESTY!
	  switch (whichstack%3){ // which stack to pop=0-3
	  case 0:
	    stack_posy=ca_pop(stack_posy);
	    break;
	  case 1:
	    stack_pos=func_pop(stack_pos);
	    break;
	  case 2:
	    villagestackpos=villagepop(villagestackpos);
	  }
	}
	break;
      case 2:
	//stackmuch 
	if (fingerfing&1) stackmuch=fingervalright(stackmuch,100);
	else stackmuch=adc_buffer[UP]>>5;
      case 3:
	// direction
	whichdir=fingervalright(whichdir,10);//
	whichdiritis=(fingerdirupdown()); // was UP
	settingsarray[54+whichdir]=whichdiritis<<15;  //<<1 bit to 16
	break;
      case 4:
	// speed
	whichspeed=fingervalright(whichspeed,6);
	if (fingerfing&1) speed=fingervalup(speed);
	else speed=adc_buffer[UP]>>4;
	settingsarray[35+whichspeed]=speed<<8; // 8 bits to 16
	break;
      case 5:
	// step
	whichstep=fingervalright(whichstep,10);
	if (fingerfing&1) step=fingervalup(step);
	else step=adc_buffer[UP]>>8;
	settingsarray[25+whichstep]=step<<8; // 8 bits to 16
	break;
      case 6:
	// start 16 bits
	started=fingervalright(started,11);
	if (fingerfing&1) start=fingervalup16bits(start,32);
	else start=adc_buffer[UP]<<4;
	settingsarray[started]=start; // 16 bit value
	break;
      case 7:
	// start all
	for (x=0;x<14;x++){
	if (fingerfing&1) start=fingervalup16bits(settingsarray[x],32);
	else start=adc_buffer[UP]<<4;
	  settingsarray[x]=start; // 16 bit value
      }
	break;
      case 8:
	// wrap 16 bits
	constrained=fingervalright(constrained,15);
	if (fingerfing&1) constrain=fingervalup16bits(constrain,32);
	else constrain=adc_buffer[UP]<<4;
	settingsarray[11+constrained]=constrain; // 16 bit value
	break;
      case 9:
	// wrap all
	for (x=11;x<25;x++){
	constrain=fingervalup16bits(settingsarray[x],32);
	  settingsarray[x]=constrain; // 16 bit value
      }
	break;
      case 10:
	// exespot
	exespot=fingervalright(exespot,32);
	settingsarray[50]=exespot<<11; 
	break;
      case 11:
	cpur=fingervalright(cpur,m->m_threadcount);
	if (fingerfing&1) cpu=fingervalup(cpu);
	else cpu=adc_buffer[UP]>>4;
	m->m_threads[cpur].m_CPU=cpu%31;
	break;
      case 12:
	startrr=fingervalright(startrr,m->m_threadcount);
	if (fingerfing&1) startr=fingervalup16bits(startr,32);
	else startr=adc_buffer[UP]<<4;
	m->m_threads[startrr].m_start=startr;
	break;
      case 13:
	wraprr=fingervalright(wraprr,m->m_threadcount);
	if (fingerfing&1) wrapr=fingervalup16bits(wrapr,32);
	else wrapr=adc_buffer[UP]<<4;
	m->m_threads[wraprr].m_wrap=m->m_threads[wraprr].m_start+wrapr;
	break;
      case 14:
	// folder?
	foldback=fingervalright(foldback,44);
	if (fingerfing&1) foldbackset=fingervalup16bits(foldbackset,32);
	else foldbackset=adc_buffer[UP]<<4;
	FOLDD[foldback]=foldbackset;
	break;
      case 15:
	// fold all?
	for (x=0;x<44;x++){
	if (fingerfing&1) foldbackset=fingervalup16bits(FOLDD[x],32);
	else foldbackset=adc_buffer[UP]<<4;
	FOLDD[x]=foldbackset;
	}
      } ///end of fingermod

      // KNOB FOUR = mirrors ---> TODO: last or first??
      mirror=adc_buffer[FOURTH]>>6; // 6 bits
      // toggle repeats is top bit but how we set this 
      mirrortoggle=mirror>>5; // top bit
      mirror=mirror&31; // so now 32... fixed as kept mirrortoggle bit...
      FOLDD[44]=knobby<<4; // knob five as length of mirror
      FOLDD[1]=knobby<<4; // knob five as length of mirror???TODO


      //////////////////////////////////////////////// INTO THE MIRROR

      switch(mirror){
      case 0: // do nothing
	break;
     case 1:
       for (x=0;x<((FOLDD[1]>>10)+1);x++){ //was >>9
	 settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768];
	  coo++;
	}

       if (mirrortoggle&1) m1flag^=1;
	break;
      case 2:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=(randi()<<4);
	}
	if (mirrortoggle&1) m1flag^=2;
	break;
      case 3:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=adc_buffer[UP]<<4;
	}
	if (mirrortoggle&1) m1flag^=4;
	break;
      case 4:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=knobby<<4;
	}
	if (mirrortoggle&1) m1flag^=8;
	break;
      case 5:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]+=8;
	}
	if (mirrortoggle&1) m1flag^=16;
	break;
      case 6:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]-=8; //TODO TUNING this +-1!!!
	}
	if (mirrortoggle&1) m1flag^=32;
	break;
      case 7:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%64]=settingsarray[(((FOLDD[2])>>10)+8+(x%((FOLDD[3]>>10)+1)))%64];
	}
	if (mirrortoggle&1) m1flag^=64;
	break;
	////////////////////////////////VILLAGER MIRROR
      case 8: // do nothing
	break;
      case 9:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  	  coo++;
	}
	if (mirrortoggle&1) m1flag^=128;
	break;
      case 10:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(randi()<<3);
	}
	if (mirrortoggle&1) m1flag^=256;
	break;
      case 11:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=adc_buffer[UP]<<3;
	}
	if (mirrortoggle&1) m1flag^=512;
	break;
      case 12:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=knobby<<3;
	}
	if (mirrortoggle&1) m1flag^=1024;
	break;
      case 13:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]+1)%32768; // TUNING +-1 TODO
	}
	if (mirrortoggle&1) m1flag^=2048;
	break;
      case 14:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]-1)%32768;
	}
	if (mirrortoggle&1) m1flag^=4096;
	break;
      case 15:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  // take start and ends from stacks TODO!	  
	  tmper=((((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))*3)%96; // div by 3 96/3=32
	  //extract stackerstart and stackerwrap
	  // start is 0 wrap is +2;
	  if (tmper<48) {
	    stackerstart=stackery[tmper];
	    stackerwrap=stackery[(tmper+2)%48];
	  }
	  else {
	    tmper=tmper-48;
	    stackerstart=stacker[tmper];
	    stackerwrap=stacker[(tmper+2)%48];
	  }
	  villager[((((FOLDD[2])>>10)*2)+(x%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerstart;
	  villager[((((FOLDD[2])>>10)*2)+((x+1)%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerwrap;
	}
	if (mirrortoggle&1) m1flag^=8192;
	break;
	////////////////////////////////STACKER MIRROR
      case 16:// do nothing
	break;
      case 17:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
		}
	  coo++;
	}
	if (mirrortoggle&1) m1flag^=16384;
      break;
      case 18:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(randi()<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(randi()<<3);
		}
	}
	if (mirrortoggle&1) m1flag^=32768;
      break;
      case 19:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=adc_buffer[UP]<<3;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=adc_buffer[UP]<<3;
		}
	}
	if (mirrortoggle&1) m2flag^=1;
      break;

      case 20:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(knobby<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(knobby<<3);
		}
	}
	if (mirrortoggle&1) m2flag^=2;
      break;
      case 21:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]+1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]+1)%32768; // TODO TUNING +-1
		}
	}
	if (mirrortoggle&1) m2flag^=4;
      break;
      case 22:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]-1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]-1)%32768; // TODO TUNING +-1
		}
	}
	if (mirrortoggle&1) m2flag^=8;
      break;
      case 23:
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=stackery[(tmper+3)%48];
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=stacker[(tmper+3)%48];
		}
	}
	if (mirrortoggle&1) m2flag^=16;
      break;

      ////////////////////////////////MIRROR | MIRROR
      // but can't short circuit itself
      case 24:
	break; // do nothing
      case 25:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[44]>>10)+1)))%32768];
	  coo++;
		}
	//	if (mirrortoggle&1) mirrorflag^=2097152;
	break;
      case 26:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=(randi()<<4);
		}
	//	if (mirrortoggle&1) mirrorflag^=4194304;
	break;
      case 27:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=adc_buffer[UP]<<4;
		}
	//	if (mirrortoggle&1) mirrorflag^=8388608;
	break;
      case 28:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=knobby<<4;
		}
	//	if (mirrortoggle&1) mirrorflag^=16777216;
	break;
      case 29:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]+=1; // TODO tuning! and below
		}
	//	if (mirrortoggle&1) mirrorflag^=33554432;
	break;
      case 30:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]-=1;
		}
	//	if (mirrortoggle&1) mirrorflag^=67108864;
	break;
      case 31:
	for (x=0;x<((FOLDD[44]>>10)+1);x++){
	  //	  tmper=(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))%44;
	  tmper=((FOLDD[2]>>10)+x)%44;
	  FOLDD[tmper]=FOLDD[((tmper+1)%44)];
		}
      } // end of mirror switch
      //	if (mirrortoggle&1) mirrorflag^=134217700;

      ////////////////////////////////////////////////MIRROR ACTIONZ
	//// leave as 2 and 3 and inc here!

      //           /* for mirror

      //      m1flag=8; m2flag=1; // TESTY
            
      if (m1flag&1){ //skip 0
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  	  settingsarray[(((FOLDD[4])>>10)+(x%((FOLDD[5]>>10)+1)))%64]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768];
	  //	  settingsarray[(((FOLDD[4])>>10)+(x%((FOLDD[5]>>10)+1)))%64]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))];
	  coo++;
	  //	  if (((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))>32768)  printf("coo %d\n",((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1))));
	}
      }

      if (m1flag&2){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[6])>>10)+(x%((FOLDD[7]>>10)+1)))%64]=(randi()<<4);
	}
    }

      if (m1flag&4){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[8])>>10)+(x%((FOLDD[9]>>10)+1)))%64]=adc_buffer[UP]<<4;
	}
}

      if (m1flag&8){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[10])>>10)+(x%((FOLDD[11]>>10)+1)))%64]=knobby<<4;
	}
}

      if (m1flag&16){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[12])>>10)+(x%((FOLDD[13]>>10)+1)))%64]+=8;
	}
}

      if (m1flag&32){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[14])>>10)+(x%((FOLDD[15]>>10)+1)))%64]-=8; //TODO TUNING this +-1!!!
	}
}

      if (m1flag&64){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  settingsarray[(((FOLDD[16])>>10)+(x%((FOLDD[17]>>10)+1)))%64]=settingsarray[(((FOLDD[2])>>10)+8+(x%((FOLDD[3]>>10)+1)))%64];
	}
}

      if (m1flag&128){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[18])>>10)+(x%((FOLDD[19]>>10)+1)))% VILLAGE_SIZE]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  	  coo++;
	}
}

      if (m1flag&256){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[20])>>10)+(x%((FOLDD[21]>>10)+1)))% VILLAGE_SIZE]=(randi()<<3);
	}
}

      if (m1flag&512){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[22])>>10)+(x%((FOLDD[23]>>10)+1)))% VILLAGE_SIZE]=adc_buffer[UP]<<3;
	}
}

      if (m1flag&1024){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[24])>>10)+(x%((FOLDD[25]>>10)+1)))% VILLAGE_SIZE]=knobby<<3;
	}
}

      if (m1flag&2048){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[26])>>10)+(x%((FOLDD[27]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]-1)%32768;
	}
}

      if (m1flag&4096){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]=(villager[(((FOLDD[2])>>10)+(x%((FOLDD[3]>>10)+1)))% VILLAGE_SIZE]-1)%32768;
	}
}

      if (m1flag&8192){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  // take start and ends from stacks TODO!	  
	  tmper=((((FOLDD[28])>>10)+(x%((FOLDD[29]>>10)+1)))*3)%96; // div by 3 96/3=32
	  //extract stackerstart and stackerwrap
	  // start is 0 wrap is +2;
	  if (tmper<48) {
	    stackerstart=stackery[tmper];
	    stackerwrap=stackery[(tmper+2)%48];
	  }
	  else {
	    tmper=tmper-48;
	    stackerstart=stacker[tmper];
	    stackerwrap=stacker[(tmper+2)%48];
	  }
	  villager[((((FOLDD[2])>>10)*2)+(x%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerstart;
	  villager[((((FOLDD[2])>>10)*2)+((x+1)%(((FOLDD[3]>>10)+1)*2)))% VILLAGE_SIZE]=stackerwrap;
	}
}

      //skip 0
      if (m1flag&16384){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[30])>>10)+(x%((FOLDD[31]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=buf16[((FOLDD[0]>>1)+(coo%((FOLDD[1]>>10)+1)))%32768]>>1;
		}
	  	  coo++;
	}
}

      if (m1flag&32768){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[32])>>10)+(x%((FOLDD[33]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(randi()<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(randi()<<3);
		}
	}
}

      if (m2flag&1){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[34])>>10)+(x%((FOLDD[35]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=adc_buffer[UP]<<3;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=adc_buffer[UP]<<3;
		}
	}
}

      if (m2flag&2){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[36])>>10)+(x%((FOLDD[37]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(knobby<<3);
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(knobby<<3);
		}
	}
}

      if (m2flag&4){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[38])>>10)+(x%((FOLDD[39]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]+1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]+1)%32768; // TODO TUNING +-1
		}
	}
}

      if (m2flag&8){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[40])>>10)+(x%((FOLDD[41]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=(stackery[tmper]-1)%32768;
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=(stacker[tmper]-1)%32768; // TODO TUNING +-1
		}
	}
}

      if (m2flag&16){
	for (x=0;x<((FOLDD[1]>>10)+1);x++){
	  tmper=(((FOLDD[42])>>10)+(x%((FOLDD[43]>>10)+1)))%96;
	  if (tmper<48) stackery[tmper]=stackery[(tmper+3)%48];
	  else {
	    tmper=tmper-48;
	    stacker[tmper]=stacker[(tmper+3)%48];
		}
	}
}
	/// mirror miror or not??? no as can short circuit?
	
            	
      ////////////////////////////////////////////////END OF MIRROR


