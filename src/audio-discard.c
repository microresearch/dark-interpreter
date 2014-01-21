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

