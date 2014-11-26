	      switch(ranger){
	      case 0:
		village_write[whichx]=village_write[whichy]; // restrict here or???
		break;
	      case 1:
		//		village_write[whichx]=village_read[whichy];// TODOcopy as now different
		break;
	      case 2:
		//	village_write[whichx]=village_filtout[whichy];
		break;
	      case 3:
		// copy appropriate datagen settings
		village_write[whichx].start=village_datagen[whichy].start;
		village_write[whichx].wrap=village_datagen[whichy].wrap;
		village_write[whichx].samplepos=village_datagen[whichy].position;
		village_write[whichx].dirry=village_datagen[whichy].dirry;
		//		village_write[whichx].dir=village_datagen[whichy].dir;
		village_write[whichx].step=village_datagen[whichy].step;
		village_write[whichx].speed=village_datagen[whichy].speed;
		break;
	      case 4:
		// datagen walker 
		village_write[whichx].start=village_datagenwalker[whichy].dataoffset;
		village_write[whichx].wrap=village_datagenwalker[whichy].length;
		village_write[whichx].samplepos=village_datagenwalker[whichy].samplepos;
		village_write[whichx].dirry=village_datagenwalker[whichy].dirry;
		village_write[whichx].dir=village_datagenwalker[whichy].dir;
		village_write[whichx].step=village_datagenwalker[whichy].step;
		village_write[whichx].speed=village_datagenwalker[whichy].speed;
		break;
	      case 5://TODO REDO!
		// with its mirror
		village_write[whichx].start=village_write[whichy].mstart;
		village_write[whichx].wrap=village_write[whichy].mwrap;
		break;
	      case 6:
		/// copy outmod for effects! outstart, outwrap
		village_write[whichx].start=village_effect[whichx].outstart;
		village_write[whichx].wrap=village_effect[whichx].outwrap;
		village_write[whichx].samplepos=village_effect[whichx].outpos;
		break;
		/// TODO: mult all above by village_read,village_datagen,village_datagenwalker,village mirrors,outmod=8*8=64

	      } // end of ranger
	      break;
	    //case 1: // next actionsssTODO!
	      //break;
 		
		// swop
		//tmpvillage=village_write[whichx];
//		village_write[whichx]=village_read[whichy];
//		village_read[whichy]=tmpvillage;
//		break;

//	    case 2:// infect
//		village_read[whichx]%=(village_write[whichy]+1);

//		case 3: fingers in!




	      case 0: // overlay=all,effect=straight
		buf16[lp]=tmp+32768;
		audio_buffer[lp]=tmp16;
	      break;
	      case 1://or
		buf16[lp]|=tmp+32768;
		audio_buffer[lp]|=tmp16;
	      break;
	      case 2:///+
		tmp32d=buf16[lp]+tmp;
		tmp32=audio_buffer[lp]+tmp16;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 3://last
		if (tmp>lasttmp) buf16[lp]=tmp+32768;
		if (tmp16>lasttmp16) audio_buffer[lp]=tmp16;
		lasttmp=tmp; lasttmp16=tmp16;
	      break;

	      // overlay(=,|,+,last)=4=2 bits - 16 bits
	      case 4: // // overlay=all,effect=&
		buf16[lp]&=tmp+32768;
		audio_buffer[lp]&=tmp16;
	      break;
	      case 5:// overlay or
		tmptmp=tmp&tmp16;
		buf16[lp]|=tmptmp+32768;
		audio_buffer[lp]|=tmptmp;
		break;
	      case 6: // overlay +
		tmptmp=tmp&tmp16;
		tmp32d=buf16[lp]+tmptmp;
		tmp32=audio_buffer[lp]+tmptmp;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 7: // overlay last
		tmptmp=tmp&tmp16;
		if (tmptmp>lasttmp) {
		  buf16[lp]=tmptmp+32768;
		  audio_buffer[lp]=tmptmp;
	      }
		lasttmp=tmptmp;
		break;

	      case 8: // // overlay=all,effect=+
		tmp32d=tmp+tmp16; 
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32d+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
	      break;
	      case 9:// overlay or
		tmp32d=tmp+tmp16; 
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32d+32768;
		audio_buffer[lp]|=tmp32d;
		break;
	      case 10: // overlay +
		tmp32d=tmp+tmp16; 
		tmp32=buf16[lp]+tmp32d;
		tmp32d+=audio_buffer[lp];
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
		break;
	      case 11: // overlay last
		tmp32d=tmp+tmp16; 
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32d>lasttmp) {
		  buf16[lp]=tmp32d+32768;
		audio_buffer[lp]=tmp32d;
		}
		lasttmp=tmp32d;
		break;

	      case 12: // // overlay=all,effect=*
		tmp32d=tmp*tmp16; 
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32d+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:// overlay or
		tmp32d=tmp*tmp16; 
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32d+32768;
		audio_buffer[lp]|=tmp32d;
		break;
	      case 14: // overlay +
		tmp32d=tmp*tmp16; 
		tmp32=buf16[lp]+tmp32d;
		tmp32d+=audio_buffer[lp];
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
		break;
	      case 15: // overlay last
		tmp32d=tmp*tmp16; 
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32d>lasttmp) {
		  buf16[lp]=tmp32d+32768;
		audio_buffer[lp]=tmp32d;
		}
		lasttmp=tmp32d;
		break;



	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;

	      case 9:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as + with fmod
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_read[x].effectinv;
		fsum=(float)tmp*village_read[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;


	      break;
	      case 6:
		break;
		//////////////////////
	      case 7:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32&=tmp32d;
		tmp32d&=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32>lasttmp16) buf16[lp]=tmp32+32768;
		if (tmp32d>lasttmp16) audio_buffer[lp]=tmp32d;
		lasttmp=tmp32; lasttmp16=tmp32d;
	      break;

	      case 8: // effect as + 
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
tmp32d=fsumd;//tmp
tmp32=fsum;//tmp16
tmptmp32=fsum; //tmp
tmp32+=tmp32d;//tmp16+tmp
tmp32d+=tmptmp32;//tmp16+tmp = same
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32+32768;
		audio_buffer[lp]|=tmp32d;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;

		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		tmp32d=buf16[lp]+tmp32;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 11:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32+=tmp32d;
		tmp32d+=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		if (tmp32>lasttmp16) buf16[lp]=tmp32+32768;
		if (tmp32d>lasttmp16) audio_buffer[lp]=tmp32d;
		lasttmp=tmp32; lasttmp16=tmp32d;
	      break;

	      case 12: // effect as * 
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;
		audio_buffer[lp]=tmp32d;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;
		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]|=tmp32+32768;
		audio_buffer[lp]|=tmp32d;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_read[x].effect;
		fsum=(float)tmp*village_read[x].effectinv;
		tmp32d=fsumd;
		tmp32=fsum;
		tmptmp32=fsum;

		tmp32*=tmp32d;
		tmp32d*=tmptmp32;
		tmp32d=buf16[lp]*tmp32;
		tmp32=audio_buffer[lp]*tmp32;
		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32d) : [src] "r" (tmp32d));
		}
		buf16[lp]=tmp32+32768;// TODO: add here or before???
		audio_buffer[lp]=tmp32d;
		break;
	      case 15:


		if (village_read[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp16) buf16[lp]=tmp32+32768;
		if (tmp32d>lasttmp16) audio_buffer[lp]=tmp32d;
		lasttmp=tmp32; lasttmp16=tmp32d;
	      break;


	  lasttmp=0;
	  for (x=0;x<howmanywritevill;x++){
	  if (delwrite==0) village_write[x].counterr+=dirryw;

	  tmpp=village_write[x].compress; if (tmpp==0) tmpp=1;

	  if (village_write[x].counterr>=(32768/tmpp)) {
	    village_write[x].counterr=0;
	    village_write[x].running=1;
	  }

	    if ((village_write[x].offset/tmpp)<=village_write[x].counterr && village_write[x].running==1){

	      lp=village_write[x].samplepos%32768;
	      tmp16=buf16[lp]-32768;
	      tmp=audio_buffer[lp];
	      switch(village_write[x].overlay&15){
	      case 0: // straight. no fmod
		mono_buffer[xx]=tmp;
		break;
	      case 1:
		mono_buffer[xx]|=tmp;
	      break;
	      case 2:
		tmp32=mono_buffer[xx]+tmp;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) mono_buffer[xx]=tmp;
		lasttmp=tmp;
	      break;

	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=mono_buffer[xx]+tmp32;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) mono_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=mono_buffer[xx]+tmp32;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) mono_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as * with fmod
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=mono_buffer[xx]+tmp32;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		mono_buffer[xx]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_write[x].effectinv;
		fsum=(float)tmp*village_write[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_write[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) mono_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;
	      ////////////////////////
	      } // end of overlay switch
	      //////
	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>=0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
	      }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	    }
	    }
	  }
}

	  /*
#ifndef LACH
	  // READFILTIN - effects are across LEFT and right

	  ldst=left_buffer;
	  rdst=right_buffer;

	  for (xx=0;xx<sz/2;xx++){

	    tmp=*(ldst++); // left
	    tmp16=*(rdst++); // right

	  delfiltin++;
	  if (delfiltin>=filtinspeed) {
	    delfiltin=0;
	      }
	  lasttmp=0;
	  for (x=0;x<howmanyfiltinvill;x++){

	    tmpp=village_filtin[x].compress; if (tmpp==0) tmpp=1;

	    if (delfiltin==0) village_filtin[x].counterr+=dirryf;
	    if (village_filtin[x].counterr>=(32768/tmpp)) {
	      village_filtin[x].counterr=0;
	      village_filtin[x].running=1;
	    }

	    if ((village_filtin[x].offset/tmpp)<=village_filtin[x].counterr && village_filtin[x].running==1){

	    lp=village_filtin[x].samplepos%32768;

	    // switcher:
	      switch(village_filtin[x].overlay&15){
	      case 0: // straight. no fmod
		audio_buffer[lp]=tmp;
	      break;
	      case 1:
		audio_buffer[lp]|=tmp;
	      break;
	      case 2:
		tmp32=audio_buffer[lp]+tmp;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) audio_buffer[lp]=tmp;
		lasttmp=tmp;
	      break;
	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as + with fmod
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=audio_buffer[lp]+tmp32;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		audio_buffer[lp]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_filtin[x].effectinv;
		fsum=(float)tmp*village_filtin[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtin[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) audio_buffer[lp]=tmp32;
		lasttmp=tmp32;
	      break;
	      /////

	    }// end of switcher

	      if (++village_filtin[x].del>=village_filtin[x].step){
	      countf=((village_filtin[x].samplepos-village_filtin[x].start)+village_filtin[x].dirry);
	      if (countf<village_filtin[x].wrap && countf>0)
	      {
		village_filtin[x].samplepos+=village_filtin[x].dirry;//)%32768;
		  }
	      else
		{
		  village_filtin[x].running==0;
		if (village_filtin[x].dir==2) village_filtin[x].dirry=newdirection[wormdir];
		else if (village_filtin[x].dir==3) village_filtin[x].dirry=direction[adc_buffer[DOWN]&1]*village_filtin[x].speed;
		else village_filtin[x].dirry=direction[village_filtin[x].dir]*village_filtin[x].speed;
		if (village_filtin[x].dirry>0) village_filtin[x].samplepos=village_filtin[x].start;
		  else village_filtin[x].samplepos=village_filtin[x].start+village_filtin[x].wrap;
		}
	    village_filtin[x].del=0;
	      }
	    }
	  }
	  }

	  // WRITEFILTOUT - effects are across LEFT in and audio_buffer???
	  // NOW AS: or buf16 and audio???

	  //	  ldst=left_buffer;
	  //	  rdst=right_buffer;

	  for (xx=0;xx<sz/2;xx++){
	    left_buffer[xx]=0;

	  delfiltout++;
	  if (delfiltout>=filtoutspeed) {
	    delfiltout=0;
	      }
	  lasttmp=0;

	  for (x=0;x<howmanyfiltoutvill;x++){

	    tmpp=village_filtout[x].compress; if (tmpp==0) tmpp=1;

	  if (delfiltout==0) village_filtout[x].counterr+=dirryff;
	  if (village_filtout[x].counterr>=(32768/tmpp)) {
	    village_filtout[x].counterr=0;
	    village_filtout[x].running=1;
	  }

	    if ((village_filtout[x].offset/tmpp)<=village_filtout[x].counterr && village_filtout[x].running==1){

	      lp=village_filtout[x].samplepos%32768;
	      tmp16=buf16[lp]-32768;
	      tmp=audio_buffer[lp];

	      switch(village_filtout[x].overlay&15){
	      case 0: // straight. no fmod
		left_buffer[xx]=tmp;
		break;
	      case 1:
		left_buffer[xx]|=tmp;
	      break;
	      case 2:
		tmp32=left_buffer[xx]+tmp;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 3:
		if (tmp>lasttmp) left_buffer[xx]=tmp;
		lasttmp=tmp;
	      break;

	      case 4: // effect as & with fmod
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
	      break;
	      case 5:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]|=tmp32;
	      break;
	      case 6:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		tmp32=left_buffer[xx]+tmp32;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 7:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32&=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) left_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 8: // effect as + with fmod
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
	      break;
	      case 9:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]|=tmp32;
	      break;
	      case 10:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		tmp32=left_buffer[xx]+tmp32;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 11:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32+=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) left_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;

	      case 12: // effect as * with fmod
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
	      break;
	      case 13:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]|=tmp32;
	      break;
	      case 14:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		tmp32=left_buffer[xx]+tmp32;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		left_buffer[xx]=tmp32;
		break;
	      case 15:
		fsumd=(float)tmp16*village_filtout[x].effectinv;
		fsum=(float)tmp*village_filtout[x].effect;
		tmp32d=fsumd;
		tmp32=fsum;
		tmp32*=tmp32d;
		if (village_filtout[x].overlay&16) {
		  asm("ssat %[dst], #16, %[src]" : [dst] "=r" (tmp32) : [src] "r" (tmp32));
		}
		if (tmp32>lasttmp) left_buffer[xx]=tmp32;
		lasttmp=tmp32;
	      break;
	      ////////////////////////
	      } // end of overlay switch
	      //////
	      if (++village_filtout[x].del>=village_filtout[x].step){
	      countff=((village_filtout[x].samplepos-village_filtout[x].start)+village_filtout[x].dirry);
	      if (countff<village_filtout[x].wrap && countff>=0)
	      {
		village_filtout[x].samplepos+=village_filtout[x].dirry;//)%32768;
		  }
	      else
		{
		  village_filtout[x].running=0;
		if (village_filtout[x].dir==2) village_filtout[x].dirry=newdirection[wormdir];
		if (village_filtout[x].dir==3) village_filtout[x].dirry=direction[adc_buffer[DOWN]&1]*village_filtout[x].speed;
		else village_filtout[x].dirry=direction[village_filtout[x].dir]*village_filtout[x].speed;

		if (village_filtout[x].dirry>0) village_filtout[x].samplepos=village_filtout[x].start;
		  else village_filtout[x].samplepos=village_filtout[x].start+village_filtout[x].wrap;
		}
	    village_filtout[x].del=0;
	    }
	    }
	  }
	  }
#endif
	  */


    //  float omega = (M_PI/2.0f - 0.01f)*(2000.0f/48000.0f) + 0.00001f; // Frequency
    /*  float omega = M_PI*(2000.0f/48000.0f); // Frequency
  float K = tanf(omega);
  float Q = 1.0f * 10.0f + 0.1f; // Resonance
  //  float gain = 1.0f;
  //  float V = abs(gain-0.5f)*60.0f + 1.0f; // Gain
  float norm;*/
  //  state = (float*)malloc(4*sizeof(float));
  float coeffs[5];

  //    case ZOELZER_BANDPASS_FILTER_MODE:

float omega=(float)(2*M_PI*2000/48000);
float sn=sinf(omega);
float cs=cosf(omega);
 float alpha=(float)(sn/(2*0.1));// 1.0 is Q
float inv=(float)(1.0/(1.0+alpha));
coeffs[0] =  (float)(1.0*inv*alpha);
coeffs[1] =  0.f;
coeffs[2] =  (float)(-1.0*inv*alpha);
coeffs[3] =  (float)(-1*cs*inv);
coeffs[4] =  (float)((1 - alpha)*inv);

      for (i=0;i<5;i++){
	printf("COEFF %d =%f\n",i,coeffs[i]);
}

      //    printf("float=%f\n",xxx);

  /*  for (i = 0; i <= 256; i++)
    {
      yi= 32767*sinf(phase); // was 2047???
      phase=phase+w;
      sign_samp=32767+yi;     // dc offset
      printf("samp=%d",sign_samp); // write value into array
      }*/



  //		double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);

  /*float omega=(float)(2*M_PI*200/48000);
float sn=sinf(omega);
float cs=cosf(omega);
 float alpha=(float)(sn/(2*0.1));//Q
float inv=(float)(1.0/(1.0+alpha));
 coeffs[0] =  (float)(1.0*inv*alpha);
 coeffs[1] =  0.f;
 coeffs[2] =  (float)(-1.0*inv*alpha);
 coeffs[3] =  (float)(-1*cs*inv)/2.0f;
 coeffs[4] =  (float)((1 - alpha)*inv);*/

  coeffs[0] = 0.05014382013893669/2.0f;//616991280577938;
  coeffs[1] = 0.01;
  coeffs[2] = -0.05014382013893669/2.0f;//616991280577938;
  coeffs[3] = -1.8850117538910771/2.0f;
  coeffs[4] = 0.8997123597221269/2.0f;


  state = (float*)malloc(4*sizeof(float));
  df1= (arm_biquad_casd_df1_inst_f32*)malloc(sizeof(arm_biquad_casd_df1_inst_f32));

  //  float coeffs[] = {1,   0.558697707784438,   0.257307012382698,  -0.292447944767337,   0.108452664934473};

  //float coeffs[]={1.000000000000000,  -1.957555609146701,   0.957555609146701, -1.562244197866329,   0.641280516968023};//{a0 a1 a2 -b1 -b2}

  arm_biquad_cascade_df1_init_f32(df1, 1, coeffs, state); // before coefficients???


  //    case ZOELZER_BANDPASS_FILTER_MODE:
  /*  norm = 1 / (1 + K / Q + K * K);
      coeffs[0] = K / Q * norm;
      coeffs[1] = 0;
      coeffs[2] = -coeffs[0];
      coeffs[3] = 2 * (K * K - 1) * norm;
      coeffs[4] = (1 - K / Q + K * K) * norm;*/


  //  omega = 2*PI*frequency/sample_rate
  float omega = (M_PI/2.0f - 0.01f)*(2000.0f/48000.0f) + 0.00001f; // Frequency
  //  float omega = M_PI*(2000.0f/48000.0f); // Frequency
  float K = tanf(omega);
  float Q = 1.0f * 10.0f + 0.1f; // Resonance
  //  float gain = 1.0f;
  //  float V = abs(gain-0.5f)*60.0f + 1.0f; // Gain
  float norm;

	      village_read[x].overlay=20; // TESTY!

	      if (village_read[x].overlay>>4){ // datagen business readin!
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]=tmp+32768; // leave buf16 as here only
	      // overlay on it!
	      switch(village_read[x].overlay&15){
	      case 0:
		audio_buffer[village_read[x].samplepos%32768]=tmp16; 
	      break;
	      case 1:
		audio_buffer[village_read[x].samplepos%32768]|=tmp16; 
	      break;
	      case 2:
		audio_buffer[village_read[x].samplepos%32768]^=tmp16; 
	      break;
	      case 3:
		audio_buffer[village_read[x].samplepos%32768]+=tmp16; 
	      break;
	      case 4:
		// last thing...
	      if (tmp16>last) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      last=tmp16;

	      // others if not 0= &, %, *(should be float), and, last value???
	      case 5:
		if (audio_buffer[village_read[x].samplepos%32768]!=0 && tmp16!=0)
		  audio_buffer[village_read[x].samplepos%32768]%=tmp16; 
		else audio_buffer[village_read[x].samplepos%32768]=tmp16; 
		break;
	      case 6:
		if (audio_buffer[village_read[x].samplepos%32768]!=0 && tmp16!=0)
		  audio_buffer[village_read[x].samplepos%32768]&=tmp16; 
		else audio_buffer[village_read[x].samplepos%32768]=tmp16; 
		break;
	      case 7:
		if (audio_buffer[village_read[x].samplepos%32768]!=0 && tmp16!=0)
		  audio_buffer[village_read[x].samplepos%32768]*=tmp16; 
		else audio_buffer[village_read[x].samplepos%32768]=tmp16; 
		break;
	      // FMODed parameters as floats - question of saturation/clipping?
	      case 8:
		fsum=(float32_t)tmp16 * village_read[x].effect;
		tmp32=fsum;
		audio_buffer[village_read[x].samplepos%32768]=tmp32;
		break;
	      case 9:
		fsum=(float32_t)tmp16 * village_read[x].effect;
		tmp32=fsum;
		audio_buffer[village_read[x].samplepos%32768]+=tmp32;
		break;
	      case 10:
		fsum=(float32_t)tmp16 * village_read[x].effect;
		tmp32=fsum;
		audio_buffer[village_read[x].samplepos%32768]^=tmp32;
		break;
	      case 11:
		fsum=(float32_t)tmp16 * village_read[x].effect;
		tmp32=fsum;
		audio_buffer[village_read[x].samplepos%32768]|=tmp32;
		break;
	      case 12:
		if (audio_buffer[village_read[x].samplepos%32768]!=0 && tmp16!=0)
		  //  audio_buffer[village_read[x].samplepos%32768]*=tmp16; 
		  {
		fsum=(float32_t)tmp16 * village_read[x].effect;
		tmp32=fsum;
		audio_buffer[village_read[x].samplepos%32768]*=tmp32;
		  }
		else audio_buffer[village_read[x].samplepos%32768]=tmp16; 
		break;
		//		13/14/15/16 buf16*tmp,+tmp, |tmp, &tmp

/////////////////

	    //	    writeoverlay=adc_buffer[FIRST]>>9; // 8 possibles 
	    databegin=loggy[adc_buffer[SECOND]]; //as logarithmic
	    dataend=loggy[adc_buffer[THIRD]]; //as logarithmic
	    //	    writeoffset=loggy[adc_buffer[FOURTH]];
	    dataspeed=spd&15; // check how many bits is spd? 8 as changed in main.c 
	    if (xx==0) dirryd=-(((spd&240)>>4)+1);
	    else if (xx==1) dirryd=((spd&240)>>4)+1;
	    else if (xx==2) dirryd=newdirection[wormdir];
	    else dirryd=direction[adc_buffer[DOWN]&1]*(((spd&240)>>4)+1);
	    if (dirryd>0) counterd=databegin;
	      else counterd=dataend+databegin;


//// FFT test

// first test dsplib

#include "arm_math.h"
#include "arm_const_structs.h"

#define TEST_LENGTH_SAMPLES 2048

//static float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES];
static float32_t testOutput[TEST_LENGTH_SAMPLES/2];

float32_t testInput_f32_10khz[2048] = 
{   
-0.865129623056441, 	0.000000000000000, 	-2.655020678073846, 	0.000000000000000, 	0.600664612949661, 	0.000000000000000, 	0.080378093886515, 	0.000000000000000, 	
-2.899160484012034, 	0.000000000000000, 	2.563004262857762, 	0.000000000000000, 	3.078328403304206, 	0.000000000000000, 	0.105906778385130, 	0.000000000000000, 	
0.048366940168201, 	0.000000000000000, 	-0.145696461188734, 	0.000000000000000, 	-0.023417155362879, 	0.000000000000000, 	2.127729174988954, 	0.000000000000000, 	
-1.176633086028377, 	0.000000000000000, 	3.690223557991855, 	0.000000000000000, 	-0.622791766173194, 	0.000000000000000, 	0.722837373872203, 	0.000000000000000, 	
2.739754205367484, 	0.000000000000000, 	-0.062610410524552, 	0.000000000000000, 	-0.891296810967338, 	0.000000000000000, 	-1.845872258871811, 	0.000000000000000, 	
1.195039415434387, 	0.000000000000000, 	-2.177388969045026, 	0.000000000000000, 	1.078649103637905, 	0.000000000000000, 	2.570976050490193, 	0.000000000000000, 	
-1.383551403404574, 	0.000000000000000, 	2.392141424058873, 	0.000000000000000, 	2.858002843205065, 	0.000000000000000, 	-3.682433899725536, 	0.000000000000000, 	
-3.488146646451150, 	0.000000000000000, 	1.323468578888120, 	0.000000000000000, 	-0.099771155430726, 	0.000000000000000, 	1.561168082500454, 	0.000000000000000, 	
1.025026795103179, 	0.000000000000000, 	0.928841900171200, 	0.000000000000000, 	2.930499509864950, 	0.000000000000000, 	2.013349089766430, 	0.000000000000000, 	
2.381676148486737, 	0.000000000000000, 	-3.081062307950236, 	0.000000000000000, 	-0.389579115537544, 	0.000000000000000, 	0.181540149166620, 	0.000000000000000, 	
-2.601953341353208, 	0.000000000000000, 	0.333435137783218, 	0.000000000000000, 	-2.812945856162965, 	0.000000000000000, 	2.649109640172910, 	0.000000000000000, 	
-1.003963025744654, 	0.000000000000000, 	1.552460768755035, 	0.000000000000000, 	0.088641345335247, 	0.000000000000000, 	-2.519951327113426, 	0.000000000000000, 	
-4.341348988610527, 	0.000000000000000, 	0.557772429359965, 	0.000000000000000, 	-1.671267412948494, 	0.000000000000000, 	0.733951350960387, 	0.000000000000000, 	
0.409263788034864, 	0.000000000000000, 	3.566033071952806, 	0.000000000000000, 	1.882565173848352, 	0.000000000000000, 	-1.106017073793287, 	0.000000000000000, 	
0.154456720778718, 	0.000000000000000, 	-2.513205795512153, 	0.000000000000000, 	0.310978660939421, 	0.000000000000000, 	0.579706500111723, 	0.000000000000000, 	
0.000086383683251, 	0.000000000000000, 	-1.311866980897721, 	0.000000000000000, 	1.840007477574986, 	0.000000000000000, 	-3.253005768451345, 	0.000000000000000, 	
1.462584328739432, 	0.000000000000000, 	1.610103610851738, 	0.000000000000000, 	0.761914676858907, 	0.000000000000000, 	0.974541361089834, 	0.000000000000000, 	
0.686845845885983, 	0.000000000000000, 	1.849153122025191, 	0.000000000000000, 	0.787800410401453, 	0.000000000000000, 	-1.187438909666279, 	0.000000000000000, 	
-0.754937911044720, 	0.000000000000000, 	0.084373858395232, 	0.000000000000000, 	-2.600269011710521, 	0.000000000000000, 	-0.962982842142644, 	0.000000000000000, 	
-0.369328108540868, 	0.000000000000000, 	0.810791418361879, 	0.000000000000000, 	3.587016488699641, 	0.000000000000000, 	-0.520776145083723, 	0.000000000000000, 	
0.640249919627884, 	0.000000000000000, 	1.103122489464969, 	0.000000000000000, 	2.231779881455556, 	0.000000000000000, 	-1.308035392685241, 	0.000000000000000, 	
0.424070304330106, 	0.000000000000000, 	-0.200383932651189, 	0.000000000000000, 	-2.365526783356541, 	0.000000000000000, 	-0.989114757436628, 	0.000000000000000, 	
2.770807688959777, 	0.000000000000000, 	-0.444172737462307, 	0.000000000000000, 	0.079760979374078, 	0.000000000000000, 	-0.005199118412183, 	0.000000000000000, 	
-0.664712668309527, 	0.000000000000000, 	-0.624171857561896, 	0.000000000000000, 	0.537306979007338, 	0.000000000000000, 	-2.575955675497642, 	0.000000000000000, 	
1.562363235756780, 	0.000000000000000, 	1.814069369848895, 	0.000000000000000, 	-1.293428583392509, 	0.000000000000000, 	-1.026188449495686, 	0.000000000000000, 	
-2.981771815588717, 	0.000000000000000, 	-4.223468103075124, 	0.000000000000000, 	2.672674782004045, 	0.000000000000000, 	-0.856096801117735, 	0.000000000000000, 	
0.048517345512563, 	0.000000000000000, 	-0.026860721136222, 	0.000000000000000, 	0.392932277758187, 	0.000000000000000, 	-1.331740855093099, 	0.000000000000000, 	
-1.894292129477081, 	0.000000000000000, 	-1.425006468460681, 	0.000000000000000, 	-2.721772427617057, 	0.000000000000000, 	-1.616831100216806, 	0.000000000000000, 	
3.551177651488947, 	0.000000000000000, 	-0.069685667896087, 	0.000000000000000, 	-3.134634907409102, 	0.000000000000000, 	-0.263627598944639, 	0.000000000000000, 	
-1.650469945991350, 	0.000000000000000, 	-2.203580339374399, 	0.000000000000000, 	-0.872203246123242, 	0.000000000000000, 	1.230782812607287, 	0.000000000000000, 	
0.257288860093291, 	0.000000000000000, 	1.989083106173137, 	0.000000000000000, 	-1.985638729453261, 	0.000000000000000, 	-1.416185105842892, 	0.000000000000000, 	
-1.131097688325772, 	0.000000000000000, 	-2.245130805416057, 	0.000000000000000, 	-1.938873996219074, 	0.000000000000000, 	2.043608361562645, 	0.000000000000000, 	
-0.583727989880841, 	0.000000000000000, 	-1.785266378212929, 	0.000000000000000, 	1.961457586224753, 	0.000000000000000, 	1.139400099963223, 	0.000000000000000, 	
-1.979519343363991, 	0.000000000000000, 	2.003023322818429, 	0.000000000000000, 	0.229004069076829, 	0.000000000000000, 	3.452808862193135, 	0.000000000000000, 	
2.882273808365857, 	0.000000000000000, 	-1.549450501844438, 	0.000000000000000, 	-3.283872089931876, 	0.000000000000000, 	-0.327025884099064, 	0.000000000000000, 	
-0.054979977136430, 	0.000000000000000, 	-1.192280531479012, 	0.000000000000000, 	0.645539328365578, 	0.000000000000000, 	2.300832863404618, 	0.000000000000000, 	
-1.092951789535240, 	0.000000000000000, 	-1.017368249363773, 	0.000000000000000, 	-0.142673056169787, 	0.000000000000000, 	0.831073544881250, 	0.000000000000000, 	
-2.314612531587064, 	0.000000000000000, 	-2.221456299106321, 	0.000000000000000, 	0.460261143885226, 	0.000000000000000, 	0.050585301888595, 	0.000000000000000, 	
0.364373329183988, 	0.000000000000000, 	-1.685956552069538, 	0.000000000000000, 	0.050664512351055, 	0.000000000000000, 	-0.193355783902718, 	0.000000000000000, 	
-0.158660446046828, 	0.000000000000000, 	2.394156453841953, 	0.000000000000000, 	-1.562965718554525, 	0.000000000000000, 	-2.199750600869900, 	0.000000000000000, 	
1.544984022381773, 	0.000000000000000, 	-1.988307216807315, 	0.000000000000000, 	-0.628240722541046, 	0.000000000000000, 	-1.436235771505429, 	0.000000000000000, 	
1.677013691147313, 	0.000000000000000, 	1.600741781678228, 	0.000000000000000, 	-0.757380959134706, 	0.000000000000000, 	-4.784797439515566, 	0.000000000000000, 	
0.265121462834569, 	0.000000000000000, 	3.862029485934378, 	0.000000000000000, 	2.386823577249430, 	0.000000000000000, 	-3.655779745436893, 	0.000000000000000, 	
-0.763541621368016, 	0.000000000000000, 	-1.182140388432962, 	0.000000000000000, 	-1.349106114858063, 	0.000000000000000, 	-2.287533624396759, 	0.000000000000000, 	
-0.028603745188423, 	0.000000000000000, 	-1.353580755934427, 	0.000000000000000, 	0.461602380352937, 	0.000000000000000, 	-0.059599055078928, 	0.000000000000000, 	
-0.929946734342228, 	0.000000000000000, 	0.065773089295561, 	0.000000000000000, 	1.106565863102982, 	0.000000000000000, 	4.719295086373593, 	0.000000000000000, 	
-2.108377703544395, 	0.000000000000000, 	-2.226393620240159, 	0.000000000000000, 	1.375668397437521, 	0.000000000000000, 	-0.960772428525443, 	0.000000000000000, 	
-2.156313465390571, 	0.000000000000000, 	1.126060012375311, 	0.000000000000000, 	2.756485137030720, 	0.000000000000000, 	0.739639690862600, 	0.000000000000000, 	
3.914769510295006, 	0.000000000000000, 	1.685232785586675, 	0.000000000000000, 	4.079058040970612, 	0.000000000000000, 	-1.174598301660513, 	0.000000000000000, 	
-2.885776587275580, 	0.000000000000000, 	-0.241073635188767, 	0.000000000000000, 	3.080489872502403, 	0.000000000000000, 	-2.051244183999421, 	0.000000000000000, 	
0.664330486845139, 	0.000000000000000, 	-1.697798999370016, 	0.000000000000000, 	1.452369423649782, 	0.000000000000000, 	-1.523532831019280, 	0.000000000000000, 	
0.171981186587481, 	0.000000000000000, 	-4.685274721583927, 	0.000000000000000, 	-1.336175835319380, 	0.000000000000000, 	1.419070770428945, 	0.000000000000000, 	
-0.035791601713475, 	0.000000000000000, 	2.291937971632081, 	0.000000000000000, 	-1.962559313450293, 	0.000000000000000, 	-4.831595589339301, 	0.000000000000000, 	
-1.857055284000925, 	0.000000000000000, 	2.606271522635512, 	0.000000000000000, 	-0.576447978738030, 	0.000000000000000, 	0.082299166967720, 	0.000000000000000, 	
1.888399453494614, 	0.000000000000000, 	-3.564705298046079, 	0.000000000000000, 	-0.939357831083889, 	0.000000000000000, 	-1.903578203697778, 	0.000000000000000, 	
-2.642492215447250, 	0.000000000000000, 	-0.182990405251017, 	0.000000000000000, 	3.742026478011174, 	0.000000000000000, 	0.104295803798333, 	0.000000000000000, 	
1.848678195370347, 	0.000000000000000, 	-1.887384346896369, 	0.000000000000000, 	0.365048973046045, 	0.000000000000000, 	-0.889638010354219, 	0.000000000000000, 	
1.173877118428863, 	0.000000000000000, 	-1.178562827540109, 	0.000000000000000, 	0.610271645685184, 	0.000000000000000, 	1.831284815697871, 	0.000000000000000, 	
0.449575390102283, 	0.000000000000000, 	1.597171905253443, 	0.000000000000000, 	3.918574971904773, 	0.000000000000000, 	0.868104027970404, 	0.000000000000000, 	
0.582643134746494, 	0.000000000000000, 	2.321256382353331, 	0.000000000000000, 	-0.238118642223180, 	0.000000000000000, 	-2.890287868054370, 	0.000000000000000, 	
0.970995414625622, 	0.000000000000000, 	0.666137930891283, 	0.000000000000000, 	-0.202435718709502, 	0.000000000000000, 	2.057930200518194, 	0.000000000000000, 	
3.120583443719949, 	0.000000000000000, 	-0.863945271701041, 	0.000000000000000, 	0.906848893874630, 	0.000000000000000, 	-1.434124930222570, 	0.000000000000000, 	
0.754659384848783, 	0.000000000000000, 	-5.224154442713778, 	0.000000000000000, 	2.330229744098967, 	0.000000000000000, 	1.113946320164698, 	0.000000000000000, 	
0.523324920322840, 	0.000000000000000, 	1.750740911548348, 	0.000000000000000, 	-0.899333972913577, 	0.000000000000000, 	0.228705845203506, 	0.000000000000000, 	
-1.934782624767648, 	0.000000000000000, 	-3.508386237231303, 	0.000000000000000, 	-2.107108523073510, 	0.000000000000000, 	0.380587645474815, 	0.000000000000000, 	
-0.476200877183279, 	0.000000000000000, 	-2.172086712642198, 	0.000000000000000, 	1.795372535780299, 	0.000000000000000, 	-2.100318983391055, 	0.000000000000000, 	
-0.022571122461405, 	0.000000000000000, 	0.674514020010955, 	0.000000000000000, 	-0.148872569390857, 	0.000000000000000, 	0.298175890592737, 	0.000000000000000, 	
-1.134244492493590, 	0.000000000000000, 	-3.146848422289455, 	0.000000000000000, 	-1.357950199087602, 	0.000000000000000, 	0.667362732020878, 	0.000000000000000, 	
-3.119397998316724, 	0.000000000000000, 	-1.189341126297637, 	0.000000000000000, 	-1.532744386856668, 	0.000000000000000, 	-1.672972484202534, 	0.000000000000000, 	
-2.042283373871558, 	0.000000000000000, 	-1.479481547595924, 	0.000000000000000, 	-0.002668662875396, 	0.000000000000000, 	0.262737760129546, 	0.000000000000000, 	
2.734456080621830, 	0.000000000000000, 	-0.671945925075102, 	0.000000000000000, 	-3.735078262179111, 	0.000000000000000, 	-0.161705013319883, 	0.000000000000000, 	
0.748963512361001, 	0.000000000000000, 	1.128046374367600, 	0.000000000000000, 	0.649651335592966, 	0.000000000000000, 	1.880020215025867, 	0.000000000000000, 	
-1.095632293842306, 	0.000000000000000, 	1.197764876160487, 	0.000000000000000, 	0.323646656252985, 	0.000000000000000, 	-1.655502751114502, 	0.000000000000000, 	
3.666399062961496, 	0.000000000000000, 	-0.334060899735197, 	0.000000000000000, 	-2.119056978738397, 	0.000000000000000, 	3.721375117275012, 	0.000000000000000, 	
0.044874186872307, 	0.000000000000000, 	-2.733053897593234, 	0.000000000000000, 	1.590700278891042, 	0.000000000000000, 	3.215711772781902, 	0.000000000000000, 	
-1.792085012843801, 	0.000000000000000, 	-0.405797188885475, 	0.000000000000000, 	-0.628080020080892, 	0.000000000000000, 	-1.831815840843960, 	0.000000000000000, 	
2.973656862522834, 	0.000000000000000, 	-0.212032655138417, 	0.000000000000000, 	0.372437389437234, 	0.000000000000000, 	-1.614030579023492, 	0.000000000000000, 	
-0.704900996358698, 	0.000000000000000, 	1.123700273452105, 	0.000000000000000, 	-0.136371848130819, 	0.000000000000000, 	3.020284357635585, 	0.000000000000000, 	
-0.550211350877649, 	0.000000000000000, 	5.101256236381711, 	0.000000000000000, 	3.367051512192333, 	0.000000000000000, 	-4.385131946669234, 	0.000000000000000, 	
-3.967303337694391, 	0.000000000000000, 	-0.965894936640022, 	0.000000000000000, 	0.328366945264681, 	0.000000000000000, 	0.199041562924914, 	0.000000000000000, 	
1.067681999025495, 	0.000000000000000, 	-1.939516091697170, 	0.000000000000000, 	-1.092980954328824, 	0.000000000000000, 	0.273786079368066, 	0.000000000000000, 	
-0.040928322190265, 	0.000000000000000, 	-0.118368078577437, 	0.000000000000000, 	1.766589628899997, 	0.000000000000000, 	1.738321311635393, 	0.000000000000000, 	
-2.895012794321649, 	0.000000000000000, 	1.213521771395142, 	0.000000000000000, 	0.922971726633985, 	0.000000000000000, 	1.091516563636489, 	0.000000000000000, 	
3.226378465469620, 	0.000000000000000, 	1.149169778666974, 	0.000000000000000, 	-1.695986327709386, 	0.000000000000000, 	-0.974803077355813, 	0.000000000000000, 	
-4.898035507513607, 	0.000000000000000, 	1.622719302889447, 	0.000000000000000, 	0.583891313586579, 	0.000000000000000, 	-1.677182424094957, 	0.000000000000000, 	
-1.915633132814685, 	0.000000000000000, 	-1.980150370851616, 	0.000000000000000, 	0.604538269404190, 	0.000000000000000, 	0.939862406149365, 	0.000000000000000, 	
-1.266939874246416, 	0.000000000000000, 	-1.494771249200063, 	0.000000000000000, 	0.278042784093988, 	0.000000000000000, 	0.326627416008916, 	0.000000000000000, 	
-1.914530157643303, 	0.000000000000000, 	1.908947721862196, 	0.000000000000000, 	0.531819285694044, 	0.000000000000000, 	3.056856632319658, 	0.000000000000000, 	
-0.389241827774643, 	0.000000000000000, 	-2.418606606780420, 	0.000000000000000, 	0.915299238878703, 	0.000000000000000, 	-0.098774174295283, 	0.000000000000000, 	
-0.906199428444304, 	0.000000000000000, 	0.316716451217743, 	0.000000000000000, 	-4.367700643578311, 	0.000000000000000, 	1.491687997515293, 	0.000000000000000, 	
-1.962381126288365, 	0.000000000000000, 	-0.700829196527045, 	0.000000000000000, 	3.028958963615630, 	0.000000000000000, 	-2.313461067462598, 	0.000000000000000, 	
-1.431933239886712, 	0.000000000000000, 	-0.831153039725342, 	0.000000000000000, 	3.939495598250743, 	0.000000000000000, 	0.342974753984771, 	0.000000000000000, 	
-2.768330763002974, 	0.000000000000000, 	-2.744010370019008, 	0.000000000000000, 	3.821352685212561, 	0.000000000000000, 	4.551065271455856, 	0.000000000000000, 	
3.270136437041298, 	0.000000000000000, 	-3.188028411950982, 	0.000000000000000, 	-0.777075012417436, 	0.000000000000000, 	0.097110650265216, 	0.000000000000000, 	
1.221216137608812, 	0.000000000000000, 	-1.325824244541822, 	0.000000000000000, 	-2.655296734084113, 	0.000000000000000, 	-1.074792144885704, 	0.000000000000000, 	
2.770401584439407, 	0.000000000000000, 	5.240270645610543, 	0.000000000000000, 	0.108576672208892, 	0.000000000000000, 	-1.209394350650142, 	0.000000000000000, 	
1.403344353838785, 	0.000000000000000, 	-0.299032904177277, 	0.000000000000000, 	4.074959450638227, 	0.000000000000000, 	1.718727473952107, 	0.000000000000000, 	
-3.061349227080806, 	0.000000000000000, 	-1.158596888541269, 	0.000000000000000, 	3.381858904662625, 	0.000000000000000, 	0.957339964054052, 	0.000000000000000, 	
0.179900074904899, 	0.000000000000000, 	-3.909641902506081, 	0.000000000000000, 	0.805717289408649, 	0.000000000000000, 	2.047413793928261, 	0.000000000000000, 	
-1.273580225826614, 	0.000000000000000, 	-2.681359186869971, 	0.000000000000000, 	-0.721241345822093, 	0.000000000000000, 	-1.613090681569475, 	0.000000000000000, 	
0.463138804815955, 	0.000000000000000, 	0.377223507800954, 	0.000000000000000, 	2.046550684968141, 	0.000000000000000, 	0.178508732797712, 	0.000000000000000, 	
-0.477815330358845, 	0.000000000000000, 	3.763355908332053, 	0.000000000000000, 	1.300430303035163, 	0.000000000000000, 	-0.214625793857725, 	0.000000000000000, 	
1.343267891864081, 	0.000000000000000, 	-0.340007682433245, 	0.000000000000000, 	2.062703194680005, 	0.000000000000000, 	0.042032160234235, 	0.000000000000000, 	
0.643732569732250, 	0.000000000000000, 	-1.913502543857589, 	0.000000000000000, 	3.771340762937158, 	0.000000000000000, 	1.050024807363386, 	0.000000000000000, 	
-4.440489488592649, 	0.000000000000000, 	0.444904302066643, 	0.000000000000000, 	2.898702265650048, 	0.000000000000000, 	1.953232980548558, 	0.000000000000000, 	
2.761564952735079, 	0.000000000000000, 	1.963537633260397, 	0.000000000000000, 	-2.168858472916215, 	0.000000000000000, 	-4.116235357699841, 	0.000000000000000, 	
4.183678271896528, 	0.000000000000000, 	0.600422284944681, 	0.000000000000000, 	-0.659352647255126, 	0.000000000000000, 	-0.993127338218109, 	0.000000000000000, 	
-2.463571314945747, 	0.000000000000000, 	0.937720951545881, 	0.000000000000000, 	-3.098957308429730, 	0.000000000000000, 	-2.354719140045463, 	0.000000000000000, 	
-0.417285119323949, 	0.000000000000000, 	2.187974075975947, 	0.000000000000000, 	1.101468905172585, 	0.000000000000000, 	-3.185800678152109, 	0.000000000000000, 	
2.357534709345083, 	0.000000000000000, 	0.246645606729407, 	0.000000000000000, 	4.440905650784504, 	0.000000000000000, 	-2.236807716637866, 	0.000000000000000, 	
-2.171481518317550, 	0.000000000000000, 	-2.029571795072690, 	0.000000000000000, 	0.135599790431348, 	0.000000000000000, 	-1.277965265520191, 	0.000000000000000, 	
-1.927976233157507, 	0.000000000000000, 	-5.434492783745394, 	0.000000000000000, 	-2.026375829312657, 	0.000000000000000, 	1.009666016819321, 	0.000000000000000, 	
0.238549782367247, 	0.000000000000000, 	-0.516403923971309, 	0.000000000000000, 	-0.933977817429352, 	0.000000000000000, 	0.155803015935614, 	0.000000000000000, 	
-0.396194809997929, 	0.000000000000000, 	-0.915178100253214, 	0.000000000000000, 	0.666329367985015, 	0.000000000000000, 	-1.517991149945785, 	0.000000000000000, 	
0.458266744144822, 	0.000000000000000, 	-1.242845974381418, 	0.000000000000000, 	0.057914823556477, 	0.000000000000000, 	0.994101307476875, 	0.000000000000000, 	
-2.387209849199325, 	0.000000000000000, 	0.459297048883826, 	0.000000000000000, 	0.227711405683905, 	0.000000000000000, 	0.030255073506117, 	0.000000000000000, 	
-1.323361608181337, 	0.000000000000000, 	-4.650244457426706, 	0.000000000000000, 	0.062908579526021, 	0.000000000000000, 	3.462831028244432, 	0.000000000000000, 	
1.303608183314856, 	0.000000000000000, 	-1.430415193881612, 	0.000000000000000, 	-1.672886118942142, 	0.000000000000000, 	0.992890699210099, 	0.000000000000000, 	
-0.160814531784247, 	0.000000000000000, 	-1.238132939350430, 	0.000000000000000, 	-0.589223271459376, 	0.000000000000000, 	2.326363810561534, 	0.000000000000000, 	
-4.433789496230785, 	0.000000000000000, 	1.664686987538929, 	0.000000000000000, 	-2.366128834617921, 	0.000000000000000, 	1.212421570743837, 	0.000000000000000, 	
-4.847914267690055, 	0.000000000000000, 	0.228485221404712, 	0.000000000000000, 	0.466139765470957, 	0.000000000000000, 	-1.344202776943546, 	0.000000000000000, 	
-1.012053673330574, 	0.000000000000000, 	-2.844980626424742, 	0.000000000000000, 	-1.552703722026340, 	0.000000000000000, 	-1.448830983885038, 	0.000000000000000, 	
0.127010756753980, 	0.000000000000000, 	-1.667188263752299, 	0.000000000000000, 	3.424818052085100, 	0.000000000000000, 	0.956291135453840, 	0.000000000000000, 	
-3.725533331754662, 	0.000000000000000, 	-1.584534272368832, 	0.000000000000000, 	-1.654148210472472, 	0.000000000000000, 	0.701610500675698, 	0.000000000000000, 	
0.164954538683927, 	0.000000000000000, 	-0.739260064712987, 	0.000000000000000, 	-2.167324026090101, 	0.000000000000000, 	-0.310240491909496, 	0.000000000000000, 	
-2.281790349106906, 	0.000000000000000, 	1.719655331305361, 	0.000000000000000, 	-2.997005923606441, 	0.000000000000000, 	-1.999301431556852, 	0.000000000000000, 	
-0.292229010068828, 	0.000000000000000, 	1.172317994855851, 	0.000000000000000, 	0.196734885241533, 	0.000000000000000, 	2.981365193477068, 	0.000000000000000, 	
2.637726016926352, 	0.000000000000000, 	1.434045125217982, 	0.000000000000000, 	0.883627180451827, 	0.000000000000000, 	-1.434040761445747, 	0.000000000000000, 	
-1.528891971086553, 	0.000000000000000, 	-3.306913135367542, 	0.000000000000000, 	-0.399059265470646, 	0.000000000000000, 	-0.265674394285178, 	0.000000000000000, 	
3.502591252855384, 	0.000000000000000, 	0.830301156604454, 	0.000000000000000, 	-0.220021317046083, 	0.000000000000000, 	-0.090553770476646, 	0.000000000000000, 	
0.771863477047951, 	0.000000000000000, 	1.351209629105760, 	0.000000000000000, 	3.773699756201963, 	0.000000000000000, 	0.472600118752329, 	0.000000000000000, 	
2.332825668012222, 	0.000000000000000, 	1.853747950314528, 	0.000000000000000, 	0.759515251766178, 	0.000000000000000, 	1.327112776215496, 	0.000000000000000, 	
2.518730296237868, 	0.000000000000000, 	0.764450208786353, 	0.000000000000000, 	-0.278275349491296, 	0.000000000000000, 	-0.041559465082020, 	0.000000000000000, 	
1.387166083167787, 	0.000000000000000, 	2.612996769598122, 	0.000000000000000, 	-0.385404831721799, 	0.000000000000000, 	2.005630016170309, 	0.000000000000000, 	
-0.950500047307998, 	0.000000000000000, 	-1.166884021392492, 	0.000000000000000, 	1.432973552928162, 	0.000000000000000, 	2.540370505384567, 	0.000000000000000, 	
-1.140505295054501, 	0.000000000000000, 	-3.673358835201185, 	0.000000000000000, 	-0.450691288038056, 	0.000000000000000, 	1.601024294408014, 	0.000000000000000, 	
0.773213556014045, 	0.000000000000000, 	2.973873693246168, 	0.000000000000000, 	-1.361548406382279, 	0.000000000000000, 	1.409136332424815, 	0.000000000000000, 	
-0.963382518314713, 	0.000000000000000, 	-2.031268227368161, 	0.000000000000000, 	0.983309972085586, 	0.000000000000000, 	-3.461412488471631, 	0.000000000000000, 	
-2.601124929406039, 	0.000000000000000, 	-0.533896239766343, 	0.000000000000000, 	-2.627129008866350, 	0.000000000000000, 	0.622111169161305, 	0.000000000000000, 	
-1.160926365580422, 	0.000000000000000, 	-2.406196188132628, 	0.000000000000000, 	-1.076870362758737, 	0.000000000000000, 	-1.791866820937175, 	0.000000000000000, 	
-0.749453071522325, 	0.000000000000000, 	-5.324156615990973, 	0.000000000000000, 	-1.038698022238289, 	0.000000000000000, 	-2.106629944730630, 	0.000000000000000, 	
0.659295598564773, 	0.000000000000000, 	0.520940881580988, 	0.000000000000000, 	-0.055649203928700, 	0.000000000000000, 	0.292096765423137, 	0.000000000000000, 	
-4.663743901790872, 	0.000000000000000, 	-0.125066503391666, 	0.000000000000000, 	-2.452620252445380, 	0.000000000000000, 	-0.712128227397468, 	0.000000000000000, 	
-0.048938037970968, 	0.000000000000000, 	-1.821520226003361, 	0.000000000000000, 	0.810106421304257, 	0.000000000000000, 	-0.196636623956257, 	0.000000000000000, 	
-0.701769836763804, 	0.000000000000000, 	2.460345045649201, 	0.000000000000000, 	3.506597671641116, 	0.000000000000000, 	-2.711322611972225, 	0.000000000000000, 	
-0.658079876600542, 	0.000000000000000, 	-2.040082099646173, 	0.000000000000000, 	2.201668355395807, 	0.000000000000000, 	1.181507395879711, 	0.000000000000000, 	
-1.640739552179682, 	0.000000000000000, 	-1.613393726467190, 	0.000000000000000, 	-1.156741241731352, 	0.000000000000000, 	2.527773464519963, 	0.000000000000000, 	
-0.497040638009502, 	0.000000000000000, 	-0.975817112895589, 	0.000000000000000, 	-2.866830755546166, 	0.000000000000000, 	1.120214498507878, 	0.000000000000000, 	
5.986771654661698, 	0.000000000000000, 	0.398219252656757, 	0.000000000000000, 	-3.545606013198135, 	0.000000000000000, 	0.312398099396191, 	0.000000000000000, 	
-2.265327979531788, 	0.000000000000000, 	0.792121001107366, 	0.000000000000000, 	-3.736145137670100, 	0.000000000000000, 	0.762228883650802, 	0.000000000000000, 	
2.283545661214646, 	0.000000000000000, 	3.780020629583529, 	0.000000000000000, 	3.117260228608810, 	0.000000000000000, 	-2.011159255609613, 	0.000000000000000, 	
0.279107700476072, 	0.000000000000000, 	2.003369134246936, 	0.000000000000000, 	-1.448171234480257, 	0.000000000000000, 	0.584697150310140, 	0.000000000000000, 	
0.919508663636197, 	0.000000000000000, 	-3.071349141675388, 	0.000000000000000, 	-1.555923649263667, 	0.000000000000000, 	2.232497079438850, 	0.000000000000000, 	
-0.012662139119883, 	0.000000000000000, 	0.372825540734715, 	0.000000000000000, 	2.378543590847629, 	0.000000000000000, 	1.459053407813062, 	0.000000000000000, 	
-0.967913907390927, 	0.000000000000000, 	1.322825200678212, 	0.000000000000000, 	-1.033775820061824, 	0.000000000000000, 	-1.813629552693142, 	0.000000000000000, 	
4.794348161661486, 	0.000000000000000, 	0.655279811518676, 	0.000000000000000, 	-2.224590138589720, 	0.000000000000000, 	0.595329481295766, 	0.000000000000000, 	
3.364055988866225, 	0.000000000000000, 	1.863416422998127, 	0.000000000000000, 	1.930305751828105, 	0.000000000000000, 	-0.284467053432545, 	0.000000000000000, 	
-0.923374905878938, 	0.000000000000000, 	1.922988234041399, 	0.000000000000000, 	0.310482143432719, 	0.000000000000000, 	0.332122302397134, 	0.000000000000000, 	
-1.659487472408966, 	0.000000000000000, 	-1.865943507877961, 	0.000000000000000, 	-0.186775297569864, 	0.000000000000000, 	-1.700543850628361, 	0.000000000000000, 	
0.497157959366735, 	0.000000000000000, 	-0.471244843957418, 	0.000000000000000, 	-0.432013753969948, 	0.000000000000000, 	-4.000189880113231, 	0.000000000000000, 	
-0.415335170016467, 	0.000000000000000, 	0.317311950972859, 	0.000000000000000, 	0.038393428927595, 	0.000000000000000, 	0.177219909465206, 	0.000000000000000, 	
0.531650958095143, 	0.000000000000000, 	-2.711644985175806, 	0.000000000000000, 	0.328744077805156, 	0.000000000000000, 	-0.938417707547928, 	0.000000000000000, 	
0.970379584897379, 	0.000000000000000, 	1.873649473917137, 	0.000000000000000, 	0.177938226987023, 	0.000000000000000, 	0.155609346302393, 	0.000000000000000, 	
-1.276504241867208, 	0.000000000000000, 	-0.463725075928807, 	0.000000000000000, 	-0.064748250389500, 	0.000000000000000, 	-1.725568534062385, 	0.000000000000000, 	
-0.139066584804067, 	0.000000000000000, 	1.975514554117767, 	0.000000000000000, 	-0.807063199499478, 	0.000000000000000, 	-0.326926659682788, 	0.000000000000000, 	
1.445727032487938, 	0.000000000000000, 	-0.597151107739100, 	0.000000000000000, 	2.732557531709386, 	0.000000000000000, 	-2.907130934109188, 	0.000000000000000, 	
-1.461264832679981, 	0.000000000000000, 	-1.708588604968163, 	0.000000000000000, 	3.652851925431363, 	0.000000000000000, 	0.682050868282879, 	0.000000000000000, 	
-0.281312579963294, 	0.000000000000000, 	0.554966483307825, 	0.000000000000000, 	-0.981341739340932, 	0.000000000000000, 	1.279543331141603, 	0.000000000000000, 	
0.036589747826856, 	0.000000000000000, 	2.312073745896073, 	0.000000000000000, 	1.754682200732425, 	0.000000000000000, 	-0.957515875428627, 	0.000000000000000, 	
-0.833596942819695, 	0.000000000000000, 	0.437054368791033, 	0.000000000000000, 	-0.898819399360279, 	0.000000000000000, 	-0.296050580896839, 	0.000000000000000, 	
-0.785144257649601, 	0.000000000000000, 	-2.541503089003311, 	0.000000000000000, 	2.225075846758761, 	0.000000000000000, 	-1.587290487902002, 	0.000000000000000, 	
-1.421404172056462, 	0.000000000000000, 	-3.015149802293631, 	0.000000000000000, 	1.780874288867949, 	0.000000000000000, 	-0.865812740882613, 	0.000000000000000, 	
-2.845327531197112, 	0.000000000000000, 	1.445225867774367, 	0.000000000000000, 	2.183733236584647, 	0.000000000000000, 	1.163371072749080, 	0.000000000000000, 	
0.883547693520409, 	0.000000000000000, 	-1.224093106684675, 	0.000000000000000, 	-1.854501116331044, 	0.000000000000000, 	1.783082089255796, 	0.000000000000000, 	
2.301508706196191, 	0.000000000000000, 	-0.539901944139077, 	0.000000000000000, 	1.962315832319967, 	0.000000000000000, 	-0.060709041870503, 	0.000000000000000, 	
-1.353139923300238, 	0.000000000000000, 	-1.482887537805234, 	0.000000000000000, 	1.273732601967176, 	0.000000000000000, 	-3.456609915556321, 	0.000000000000000, 	
-3.752320586540873, 	0.000000000000000, 	3.536356614978951, 	0.000000000000000, 	0.206035952043233, 	0.000000000000000, 	5.933966913773842, 	0.000000000000000, 	
-0.486633898075490, 	0.000000000000000, 	-0.329595089863342, 	0.000000000000000, 	1.496414153905337, 	0.000000000000000, 	0.137868749388880, 	0.000000000000000, 	
-0.437192030996792, 	0.000000000000000, 	2.682750615210656, 	0.000000000000000, 	-2.440234892848570, 	0.000000000000000, 	1.433910252426186, 	0.000000000000000, 	
-0.415051506104074, 	0.000000000000000, 	1.982003013708649, 	0.000000000000000, 	1.345796609972435, 	0.000000000000000, 	-2.335949513404370, 	0.000000000000000, 	
1.065988867433025, 	0.000000000000000, 	2.741844905000464, 	0.000000000000000, 	-1.754047930934362, 	0.000000000000000, 	0.229252730015575, 	0.000000000000000, 	
-0.679791016408669, 	0.000000000000000, 	-2.274097820043743, 	0.000000000000000, 	0.149802252231876, 	0.000000000000000, 	-0.139697151364830, 	0.000000000000000, 	
-2.773367420505435, 	0.000000000000000, 	-4.403400246165611, 	0.000000000000000, 	-1.468974515184135, 	0.000000000000000, 	0.664990623095844, 	0.000000000000000, 	
-3.446979775557143, 	0.000000000000000, 	1.850006428987618, 	0.000000000000000, 	-1.550866747921936, 	0.000000000000000, 	-3.632874882935257, 	0.000000000000000, 	
0.828039662992464, 	0.000000000000000, 	2.794055182632816, 	0.000000000000000, 	-0.593995716682633, 	0.000000000000000, 	0.142788156054200, 	0.000000000000000, 	
0.552461945119668, 	0.000000000000000, 	0.842127129738758, 	0.000000000000000, 	1.414335509600077, 	0.000000000000000, 	-0.311559241382430, 	0.000000000000000, 	
1.510590844695250, 	0.000000000000000, 	1.692217183824300, 	0.000000000000000, 	0.613760285711957, 	0.000000000000000, 	0.065233463207770, 	0.000000000000000, 	
-2.571912893711505, 	0.000000000000000, 	-1.707001531141341, 	0.000000000000000, 	0.673884968382041, 	0.000000000000000, 	0.889863883420103, 	0.000000000000000, 	
-2.395635435233346, 	0.000000000000000, 	1.129247296359819, 	0.000000000000000, 	0.569074704779735, 	0.000000000000000, 	6.139436017480722, 	0.000000000000000, 	
0.822158309259017, 	0.000000000000000, 	-3.289872016222589, 	0.000000000000000, 	0.417612988384414, 	0.000000000000000, 	1.493982103868165, 	0.000000000000000, 	
-0.415353391377005, 	0.000000000000000, 	0.288670764933155, 	0.000000000000000, 	-1.895650228872272, 	0.000000000000000, 	-0.139631694475020, 	0.000000000000000, 	
1.445103299005436, 	0.000000000000000, 	2.877182243683429, 	0.000000000000000, 	1.192428490172580, 	0.000000000000000, 	-5.964591921763842, 	0.000000000000000, 	
0.570859795882959, 	0.000000000000000, 	2.328333316356666, 	0.000000000000000, 	0.333755014930026, 	0.000000000000000, 	1.221901577771909, 	0.000000000000000, 	
0.943358697415568, 	0.000000000000000, 	2.793063983613067, 	0.000000000000000, 	3.163005066073616, 	0.000000000000000, 	2.098300664513867, 	0.000000000000000, 	
-3.915313164333447, 	0.000000000000000, 	-2.475766769064539, 	0.000000000000000, 	1.720472044894277, 	0.000000000000000, 	-1.273591949275665, 	0.000000000000000, 	
-1.213451272938616, 	0.000000000000000, 	0.697439404325690, 	0.000000000000000, 	-0.309902287574293, 	0.000000000000000, 	2.622575852162781, 	0.000000000000000, 	
-2.075881936219060, 	0.000000000000000, 	0.777847545691770, 	0.000000000000000, 	-3.967947986440650, 	0.000000000000000, 	-3.066503371806472, 	0.000000000000000, 	
1.193780625937845, 	0.000000000000000, 	0.214246579281311, 	0.000000000000000, 	-2.610681491162162, 	0.000000000000000, 	-1.261224183972745, 	0.000000000000000, 	
-1.165071748544285, 	0.000000000000000, 	-1.116548474834374, 	0.000000000000000, 	0.847202164846982, 	0.000000000000000, 	-3.474301529532390, 	0.000000000000000, 	
0.020799541946476, 	0.000000000000000, 	-3.868995473288166, 	0.000000000000000, 	1.757979409638067, 	0.000000000000000, 	0.868115130183109, 	0.000000000000000, 	
0.910167436737958, 	0.000000000000000, 	-1.878855115563720, 	0.000000000000000, 	1.710357104174161, 	0.000000000000000, 	-1.468933980990902, 	0.000000000000000, 	
1.799544171601169, 	0.000000000000000, 	-4.922332880027887, 	0.000000000000000, 	0.219424548939720, 	0.000000000000000, 	-0.971671113451924, 	0.000000000000000, 	
-0.940533475616266, 	0.000000000000000, 	0.122510114412152, 	0.000000000000000, 	-1.373686254916911, 	0.000000000000000, 	1.760348103896323, 	0.000000000000000, 	
0.391745067829643, 	0.000000000000000, 	2.521958505327354, 	0.000000000000000, 	-1.300693516405092, 	0.000000000000000, 	-0.538251788309178, 	0.000000000000000, 	
0.797184135810173, 	0.000000000000000, 	2.908800548982588, 	0.000000000000000, 	1.590902251655215, 	0.000000000000000, 	-1.070323714487264, 	0.000000000000000, 	
-3.349764443340999, 	0.000000000000000, 	-1.190563529731447, 	0.000000000000000, 	1.363369471291963, 	0.000000000000000, 	-1.814270299924576, 	0.000000000000000, 	
-0.023381588315711, 	0.000000000000000, 	1.719182048679569, 	0.000000000000000, 	0.839917213252626, 	0.000000000000000, 	1.006099633839122, 	0.000000000000000, 	
0.812462674381527, 	0.000000000000000, 	1.755814336346739, 	0.000000000000000, 	2.546848681206319, 	0.000000000000000, 	-1.555300208869455, 	0.000000000000000, 	
1.017053811631167, 	0.000000000000000, 	0.996591039170903, 	0.000000000000000, 	-1.228047247924881, 	0.000000000000000, 	4.809462271463009, 	0.000000000000000, 	
2.318113116151685, 	0.000000000000000, 	-1.206932520679733, 	0.000000000000000, 	1.273757685623312, 	0.000000000000000, 	0.724335352481802, 	0.000000000000000, 	
1.519876652073198, 	0.000000000000000, 	-2.749670314714158, 	0.000000000000000, 	3.424042481847581, 	0.000000000000000, 	-3.714668360421517, 	0.000000000000000, 	
1.612834197004014, 	0.000000000000000, 	-2.038234723985566, 	0.000000000000000, 	1.470938786562152, 	0.000000000000000, 	2.111634918450302, 	0.000000000000000, 	
1.030376670151787, 	0.000000000000000, 	-0.420877189003829, 	0.000000000000000, 	-1.502024800532894, 	0.000000000000000, 	0.452310749163804, 	0.000000000000000, 	
-1.606059382300987, 	0.000000000000000, 	-4.006159967834147, 	0.000000000000000, 	-2.152801208196508, 	0.000000000000000, 	1.671674089372579, 	0.000000000000000, 	
1.714536333564101, 	0.000000000000000, 	-1.011518543005344, 	0.000000000000000, 	-0.576410282180584, 	0.000000000000000, 	0.733689809480836, 	0.000000000000000, 	
1.004245602717974, 	0.000000000000000, 	1.010090391888449, 	0.000000000000000, 	3.811459513385621, 	0.000000000000000, 	-5.230621089271954, 	0.000000000000000, 	
0.678044861034399, 	0.000000000000000, 	1.255935859598107, 	0.000000000000000, 	1.674521701615288, 	0.000000000000000, 	-1.656695216761705, 	0.000000000000000, 	
1.169286028869693, 	0.000000000000000, 	0.524915416191998, 	0.000000000000000, 	2.397642885039520, 	0.000000000000000, 	2.108711400616072, 	0.000000000000000, 	
2.037618211018084, 	0.000000000000000, 	-0.623664553406925, 	0.000000000000000, 	2.984106170984409, 	0.000000000000000, 	1.132182737400932, 	0.000000000000000, 	
-2.859274340352130, 	0.000000000000000, 	-0.975550071398723, 	0.000000000000000, 	-1.359935119997407, 	0.000000000000000, 	-2.963308211050121, 	0.000000000000000, 	
-0.228726662781163, 	0.000000000000000, 	-1.411110379682043, 	0.000000000000000, 	0.741553355734225, 	0.000000000000000, 	0.497554254758309, 	0.000000000000000, 	
2.371907950598855, 	0.000000000000000, 	1.063465168988748, 	0.000000000000000, 	-0.641082692081488, 	0.000000000000000, 	-0.855439878540726, 	0.000000000000000, 	
0.578321738578726, 	0.000000000000000, 	3.005809768796194, 	0.000000000000000, 	1.961458699064065, 	0.000000000000000, 	-3.206261663772745, 	0.000000000000000, 	
-0.364431989095434, 	0.000000000000000, 	-0.263182496622273, 	0.000000000000000, 	1.843464680631139, 	0.000000000000000, 	-0.419107530229249, 	0.000000000000000, 	
1.662335873298487, 	0.000000000000000, 	-0.853687563304005, 	0.000000000000000, 	-2.584133404357169, 	0.000000000000000, 	3.466839568922895, 	0.000000000000000, 	
0.881671345091973, 	0.000000000000000, 	0.454620014206908, 	0.000000000000000, 	-1.737245187402739, 	0.000000000000000, 	2.162713238369243, 	0.000000000000000, 	
-3.868539002714486, 	0.000000000000000, 	2.014114855933826, 	0.000000000000000, 	-0.703233831811006, 	0.000000000000000, 	-3.410319935997574, 	0.000000000000000, 	
-1.851235811006584, 	0.000000000000000, 	0.909783907894036, 	0.000000000000000, 	0.091884002136728, 	0.000000000000000, 	-2.688294201131650, 	0.000000000000000, 	
-0.906134178460955, 	0.000000000000000, 	3.475054609035133, 	0.000000000000000, 	-0.573927964170323, 	0.000000000000000, 	-0.429542937515399, 	0.000000000000000, 	
0.991348618739939, 	0.000000000000000, 	1.974804904926325, 	0.000000000000000, 	0.975783450796698, 	0.000000000000000, 	-3.057119549071503, 	0.000000000000000, 	
-3.899429237481194, 	0.000000000000000, 	0.362439009175350, 	0.000000000000000, 	-1.124461670265618, 	0.000000000000000, 	1.806000360163583, 	0.000000000000000, 	
-2.768333362600288, 	0.000000000000000, 	0.244387897900379, 	0.000000000000000, 	0.908767296720926, 	0.000000000000000, 	1.254669374391882, 	0.000000000000000, 	
-1.420441929463686, 	0.000000000000000, 	-0.875658895966293, 	0.000000000000000, 	0.183824603376167, 	0.000000000000000, 	-3.361653917011686, 	0.000000000000000, 	
-0.796615630227952, 	0.000000000000000, 	-1.660226542658673, 	0.000000000000000, 	1.654439358307226, 	0.000000000000000, 	2.782812946709771, 	0.000000000000000, 	
1.418064412811531, 	0.000000000000000, 	-0.819645647243761, 	0.000000000000000, 	0.807724772592699, 	0.000000000000000, 	-0.941967976379298, 	0.000000000000000, 	
-2.312768306047469, 	0.000000000000000, 	0.872426936477443, 	0.000000000000000, 	0.919528961530845, 	0.000000000000000, 	-2.084904575264847, 	0.000000000000000, 	
-1.972464868459322, 	0.000000000000000, 	-1.050687203338466, 	0.000000000000000, 	1.659579707007902, 	0.000000000000000, 	-1.820640014705855, 	0.000000000000000, 	
-1.195078061671045, 	0.000000000000000, 	-1.639773173762048, 	0.000000000000000, 	1.616744338157063, 	0.000000000000000, 	4.019216096811563, 	0.000000000000000, 	
3.461021102549681, 	0.000000000000000, 	1.642352734361484, 	0.000000000000000, 	-0.046354693720813, 	0.000000000000000, 	-0.041936252359677, 	0.000000000000000, 	
-2.393307519480551, 	0.000000000000000, 	-0.341471634615121, 	0.000000000000000, 	-0.392073595257017, 	0.000000000000000, 	-0.219299018372730, 	0.000000000000000, 	
-2.016391579662071, 	0.000000000000000, 	-0.653096251969787, 	0.000000000000000, 	1.466353155666821, 	0.000000000000000, 	-2.872058864320412, 	0.000000000000000, 	
-2.157180779503830, 	0.000000000000000, 	0.723257479841560, 	0.000000000000000, 	3.769951308104384, 	0.000000000000000, 	-1.923392042420024, 	0.000000000000000, 	
0.644899359942840, 	0.000000000000000, 	-2.090226891621437, 	0.000000000000000, 	-0.277043982890403, 	0.000000000000000, 	-0.528271428321112, 	0.000000000000000, 	
2.518120645960652, 	0.000000000000000, 	1.040820431111488, 	0.000000000000000, 	-4.560583754742486, 	0.000000000000000, 	-0.226899614918836, 	0.000000000000000, 	
1.713331231108959, 	0.000000000000000, 	-3.293941019163642, 	0.000000000000000, 	-1.113331444648290, 	0.000000000000000, 	-1.032308423149906, 	0.000000000000000, 	
1.593774272982443, 	0.000000000000000, 	-1.246840475090529, 	0.000000000000000, 	-0.190344684920137, 	0.000000000000000, 	-1.719386356896355, 	0.000000000000000, 	
-2.827721754659679, 	0.000000000000000, 	-0.092438285279020, 	0.000000000000000, 	-0.565844430675246, 	0.000000000000000, 	-1.077916121691716, 	0.000000000000000, 	
-1.208665809504693, 	0.000000000000000, 	-2.996014266381254, 	0.000000000000000, 	2.888573323402423, 	0.000000000000000, 	2.829507048720695, 	0.000000000000000, 	
-0.859177034120755, 	0.000000000000000, 	-1.969302377743254, 	0.000000000000000, 	0.777437674525362, 	0.000000000000000, 	-0.124910190157646, 	0.000000000000000, 	
0.129875493115290, 	0.000000000000000, 	-4.192139262163992, 	0.000000000000000, 	3.023496047962126, 	0.000000000000000, 	1.149775163736637, 	0.000000000000000, 	
2.038151304801731, 	0.000000000000000, 	3.016122489841263, 	0.000000000000000, 	-4.829481812137012, 	0.000000000000000, 	-1.668436615909279, 	0.000000000000000, 	
0.958586784636918, 	0.000000000000000, 	1.550652410058678, 	0.000000000000000, 	-1.456305257976716, 	0.000000000000000, 	-0.079588392344731, 	0.000000000000000, 	
-2.453213599392345, 	0.000000000000000, 	0.296795909127105, 	0.000000000000000, 	-0.253426616607643, 	0.000000000000000, 	1.418937160028195, 	0.000000000000000, 	
-1.672949529066915, 	0.000000000000000, 	-1.620990298572947, 	0.000000000000000, 	-1.085103073196045, 	0.000000000000000, 	0.738606361195386, 	0.000000000000000, 	
-2.097831202853255, 	0.000000000000000, 	2.711952282071310, 	0.000000000000000, 	1.498539238246888, 	0.000000000000000, 	1.317457282535915, 	0.000000000000000, 	
-0.302765938349717, 	0.000000000000000, 	-0.044623707947201, 	0.000000000000000, 	2.337405215062395, 	0.000000000000000, 	-3.980689173859100, 	0.000000000000000, 	


};


/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */
uint32_t fftSize = 1024;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

/* Reference index at which max energy of bin ocuurs */
uint32_t refIndex = 213, testIndex = 0;


int32_t testvocode(void)
{

  float32_t maxValue;
  arm_status status;

  status = ARM_MATH_SUCCESS;


  /* Process the data through the CFFT/CIFFT module */
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, testInput_f32_10khz, ifftFlag, doBitReverse);

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
    arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);

  /* Calculates maxValue and returns corresponding BIN value */
    arm_max_f32(testOutput, fftSize, &maxValue, &testIndex);

  if(testIndex !=  refIndex)
  {
    status = ARM_MATH_TEST_FAILURE;
  }

  /* ----------------------------------------------------------------------
  ** Loop here if the signals fail the PASS check.
  ** This denotes a test failure
  ** ------------------------------------------------------------------- */

  if( status != ARM_MATH_SUCCESS)
  {
    while(1);
  }


  //////////////////////////////////////
  //****** this is from older code in CMSIS/FFTEX

    /*  uint32_t fftSize = 1024; 
  uint32_t ifftFlag = 0; 
  uint32_t doBitReverse = 1; 
  arm_status status; 
  arm_cfft_radix4_instance_f32 S; 


   sqrtf(0.02f);
    */	 
  /* Initialize the CFFT/CIFFT module */  
  //  status = arm_cfft_radix4_init_f32(&S, fftSize, ifftFlag, doBitReverse); 
}




/////
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }


	  if (++delwrite>=writespeed) {
	    counter+=dirryw;
	    delwrite=0;
	      }


	  /*	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }*/



	  /*	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }*/


  /*
      	for (u8 xx=0;xx<64;xx++){
  village_write[xx].start=rand()%32768;
	  village_write[xx].wrap=rand()%32768;
	  village_write[xx].offset=rand()%32768;
	  village_write[xx].dir=1;
	  village_write[xx].del=0;
	  village_write[xx].samplepos=0;
	  village_write[xx].speed=rand()%16;
	  village_write[xx].step=rand()%16;
	  village_write[xx].dirry=direction[village_write[xx].dir]*village_write[xx].step;
	  village_write[xx].samplepos=village_write[xx].start;
	  }
  */


//// from audio.c


	if (readbit){
	switch(readoverlay){
	case 0:
	  for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]=tmp+32768; 
	      audio_buffer[village_read[x].samplepos%32768]=tmp16;

	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	  }
	break;
	case 1:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]|=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]|=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 2:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]+=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]+=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 3:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      buf16[village_read[x].samplepos%32768]^=tmp+32768;
	      audio_buffer[village_read[x].samplepos%32768]^=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 4:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (buf16[village_read[x].samplepos%32768]==0) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      else if ((tmp+32768)!=0) buf16[village_read[x].samplepos%32768]%=(tmp+32768);
	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      else if (tmp16!=0) audio_buffer[village_read[x].samplepos%32768]%=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;

	case 5:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (buf16[village_read[x].samplepos%32768]==0) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      else if ((tmp+32768)!=0) buf16[village_read[x].samplepos%32768]*=(tmp+32768);
	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      else if (tmp16!=0) audio_buffer[village_read[x].samplepos%32768]*=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 6:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (buf16[village_read[x].samplepos%32768]==0) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      else if ((tmp+32768)!=0) buf16[village_read[x].samplepos%32768]&=(tmp+32768);
	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      else if (tmp16!=0) audio_buffer[village_read[x].samplepos%32768]&=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 7:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  last=0;lastt=0;
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      if (tmp>lastt) buf16[village_read[x].samplepos%32768]=tmp+32768;
	      lastt=tmp;
	      if (tmp16>last) audio_buffer[village_read[x].samplepos%32768]=tmp16;
	      last=tmp16;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);

	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	} // end switch
	}
	else{ // readbit=0
	switch(readoverlay){
	case 0: // TESTY!!
	  for (xx=0;xx<sz/2;xx++){
	    //	  xx=0;
	    //	  while(xx<sz/2){
	    src++;
	    tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	    if ((readstartoffset+(village_read[x].offset%readoffset))<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]=tmp; 

	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	  if (++delread>=readspeed) { // TEST!
		counterr+=dirry;
		delread=0;
		//		src++; xx++;
		//	  	  tmp=*(src++); 
	      }
	}
	break;
	case 1:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]|=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 2:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]+=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 3:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      audio_buffer[village_read[x].samplepos%32768]^=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 4:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      else if (tmp!=0) audio_buffer[village_read[x].samplepos%32768]%=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 5:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      else if (tmp!=0) audio_buffer[village_read[x].samplepos%32768]*=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 6:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (audio_buffer[village_read[x].samplepos%32768]==0) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      else if (tmp!=0) audio_buffer[village_read[x].samplepos%32768]&=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	case 7:
	for (xx=0;xx<sz/2;xx++){
	  src++;
	  tmp=*(src++); 
	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    
	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }
	  last=0;
	  for (x=0;x<howmanyreadvill;x++){
	    //	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset%readoffset))){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){

	      if (last>tmp) audio_buffer[village_read[x].samplepos%32768]=tmp;
	      last=tmp;
	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
		  else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
	      }
	}
	break;
	}
	}
	// END of all READS!

   	// WRITE!
	//	writeoverlay=0;
	writeoverlay=0; // TESTY!

	switch(writeoverlay){// 8 options
	case 0: // TESTY!!!
	  for (xx=0;xx<sz/2;xx++){
	  //	  xx=0;
	  mono_buffer[xx]=0;
	    //	  while (xx<sz/2){
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    //	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	    if ((writestartoffset+(village_write[x].offset%writeoffset))<=counter && village_write[x].running==1){
	      mono_buffer[xx]=audio_buffer[village_write[x].samplepos%32768];

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>=0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	    }
	    }
	  }

	  if (++delwrite>=writespeed) {
	    counter+=dirryw;//xx++;
	    //	    mono_buffer[xx]=0;
	    delwrite=0;
	      }
	  }
	break;
	case 1:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]|=audio_buffer[village_write[x].samplepos%32768];

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 2:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]+=audio_buffer[village_write[x].samplepos%32768];
	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos+=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 3:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      mono_buffer[xx]^=audio_buffer[village_write[x].samplepos%32768];
	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 4:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      if (mono_buffer[xx]!=0){
		if (audio_buffer[village_write[x].samplepos%32768]!=0) mono_buffer[xx]%=(audio_buffer[village_write[x].samplepos%32768]);}
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 5:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      if (mono_buffer[xx]!=0){
		mono_buffer[xx]*=audio_buffer[village_write[x].samplepos%32768];
	      }
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 6:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      if (mono_buffer[xx]!=0){
		mono_buffer[xx]&=audio_buffer[village_write[x].samplepos%32768];
	      }
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;
	case 7:
	for (xx=0;xx<sz/2;xx++){
	  mono_buffer[xx]=0;
	  if ((counter-writebegin)>writeend) {
	    counter=writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }
	  }	    
	  if (counter<writebegin) {
	    counter=writeend+writebegin;
	    for (x=0;x<howmanywritevill;x++){
	      village_write[x].running=1;
	  }	    
	  }
	  last=0;
	  for (x=0;x<howmanywritevill;x++){
	    if ((village_write[x].offset%writeoffset)<=counter && village_write[x].running==1){
	      /*	      if (mono_buffer[xx]!=0){
		mono_buffer[xx]=(float32_t)mono_buffer[xx]*FMODW*(float32_t)audio_buffer[village_write[x].samplepos%32768];
	      }
		  else mono_buffer[xx]=(audio_buffer[village_write[x].samplepos%32768]);
	      */
	      if (audio_buffer[village_write[x].samplepos%32768]>last) mono_buffer[xx]=audio_buffer[village_write[x].samplepos%32768];

	      last=audio_buffer[village_write[x].samplepos%32768];

	      if (++village_write[x].del>=village_write[x].step){
	      count=((village_write[x].samplepos-village_write[x].start)+village_write[x].dirry);
	      if (count<village_write[x].wrap && count>0)
	      {
		village_write[x].samplepos=village_write[x].dirry;//)%32768;
		  }
	      else
		{
		  village_write[x].running=0;
		if (village_write[x].dir==2) village_write[x].dirry=newdirection[wormdir];
		else if (village_write[x].dir==3) village_write[x].dirry=direction[adc_buffer[DOWN]&1]*village_write[x].speed;
		else village_write[x].dirry=direction[village_write[x].dir]*village_write[x].speed;

		if (village_write[x].dirry>0) village_write[x].samplepos=village_write[x].start;
		  else village_write[x].samplepos=village_write[x].start+village_write[x].wrap;
		}
	    village_write[x].del=0;
	      }
	    }
	  }
      if (++delwrite>=writespeed) {
		counter+=dirryw;
		delwrite=0;
	      }
	}
	break;

	}
	/// end of ALL WRITES!

////

villagerr village_read[128];
 unsigned char delread=0,readspeed=1;
 signed int dirry=1,readbegin=0,readend=1024,counterr=0,xx, howmanyreadvill=16,count=0,readoffset=1;

    	for (xx=0;xx<32;xx++){
	  village_read[xx].start=rand()%32768;
	  village_read[xx].wrap=rand()%32768;
	  village_read[xx].offset=rand()%32768;
	  village_read[xx].dir=1;
	  village_read[xx].del=0;
	  //village_read[xx].samplepos=0;
	  village_read[xx].speed=(rand()%16)+1;
	  village_read[xx].step=(rand()%16)+1;
	  //	  village_read[xx].dirry=direction[village_read[xx].dir]*village_read[xx].step;
	  village_read[xx].dirry=village_read[xx].step;
	  village_read[xx].samplepos=village_read[xx].start;
	    /*	  village_read[xx].start=0;
	  village_read[xx].wrap=12;
	  village_read[xx].offset=1;
	  village_read[xx].dir=1;
	  village_read[xx].del=0;
	  village_read[xx].speed=1;
	  village_read[xx].step=1;
	  village_read[xx].dirry=direction[village_read[xx].dir]*village_read[xx].step;
	  village_read[xx].samplepos=village_read[xx].start;*/
	  }

	//	 dirry=rand()%32768;xx=-32766;
	//	 int ff=254;


	//	 float freqq = (float)ff*(50.0f/48000);

	//	 printf("freq %f\n",freqq);

	int buf16[32768];
	for (int x=0;x<32768;x++){
	  buf16[x]=rand()%65536;
	}

	int tmp,tmpp; float tmppp;

	while(1){

  float c0=(float)buf16[0]/16384.0;
  float c1=(float)buf16[1]/16384.0;
  float c2=(float)buf16[2]/16384.0;

  count++;
  //    y=count+16384;
  //    y=y%32768;
  tmp=count-1;
  tmppp=(((float)(buf16[tmp%32768])/65536.0f)*c0)+(((float)(buf16[count%32768])/65536.0f)*c1)+(((float)(buf16[(count+1)%32768])/65536.0f)*c2);
  tmpp=tmppp*65536.0f;
  printf("count %d float: %d\n",count,tmpp);

	  /*	  if ((counterr-readbegin)>readend) {
	    counterr=readbegin;
	    for (unsigned char x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }
	  }	    

	  if (counterr<readbegin) {
	    counterr=readend+readbegin;
	    for (unsigned char x=0;x<howmanyreadvill;x++){
	      village_read[x].running=1;
	  }	    
	  }

	  for (unsigned char x=0;x<howmanyreadvill;x++){
	    if ((village_read[x].offset%readoffset)<=counterr && village_read[x].running==1){
	      //	      tmp16=buf16[village_read[x].samplepos%32768]-32768;
	      //	      buf16[village_read[x].samplepos%32768]=tmp+32768; ///TESTY!
	      //	      audio_buffer[village_read[x].samplepos%32768]=tmp16;

	      printf("x=%d, samplepos: %d counter %d\n",x,village_read[x].samplepos,counterr);	    

	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  village_read[x].running==0;
		  //		if (village_read[x].dir==2) village_read[x].dirry=newdirection[wormdir];
//		else if (village_read[x].dir==3) village_read[x].dirry=direction[adc_buffer[DOWN]&1]*village_read[x].speed;
//		else village_read[x].dirry=direction[village_read[x].dir]*village_read[x].speed;
//		if (village_read[x].dirry>0) village_read[x].samplepos=village_read[x].start;
//		else village_read[x].samplepos=village_read[x].start+village_read[x].wrap;
//		  village_read[x].dirry=1;
//		  village_read[x].samplepos=village_read[x].start;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
	      if (++delread>=readspeed) {
		counterr+=dirry;
		delread=0;
		}*/
	}
}
	 /*	while(1){
	  if (xx==0) xx=1;
	  dirry%=xx; 
	  xx++;
	  if (xx==32768) xx=-32768;
	printf("dirry: %d xx: %d\n", dirry,xx);
	}*/
    
	  //   for (xx=0;xx<32;xx++){

      //	    if (++delread>readspeed) {
	  counterr+=dirry;
	      //      delread=0;
	      //	      }

	  if ((counterr-readbegin)>readend) counterr=readbegin;
	  if (counterr<readbegin) counterr=readend;

	  for (unsigned char x=0;x<32;x++){
	    //	    printf("x=%d\n counterr=%d",x,counterr);
	    if ((village_read[x].offset)<=counterr && village_read[x].wrap>(counterr-(village_read[x].offset))){

	      //	      audio_buffer[village_read[x].samplepos%32768]=tmp;

	      printf("x=%d, samplepos: %d counter %d\n",x,village_read[x].samplepos,counterr);	      

	      if (++village_read[x].del>=village_read[x].step){
	      count=((village_read[x].samplepos-village_read[x].start)+village_read[x].dirry);
	      //	      printf("count %d\n",count);

	      if (count<village_read[x].wrap && count>0)
	      {
		village_read[x].samplepos+=village_read[x].dirry;//)%32768;
		  }
	      else
		{
		  // reset samplepos
		  //  village_read[x].dirry=-1;
		  village_read[x].dirry=village_read[x].speed;
		  village_read[x].samplepos=village_read[x].start;
		}
	    village_read[x].del=0;
	      }
	    }
	  }
    }
}
