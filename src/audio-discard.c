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

