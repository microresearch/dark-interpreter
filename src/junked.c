
//life

unsigned char runfulllife(unsigned char* cellies){
  unsigned char sum;
  unsigned int x;
  static unsigned char flag=0;
  unsigned char *newcells, *cells;

  if ((flag&0x01)==0) {
    cells=(unsigned char*)cellies; newcells=(unsigned char*)cellies+32768;
  }
  else {
    cells=(unsigned char*)cellies+32768; newcells=(unsigned char*)cellies;
  }      

  for (x=unit->celllen+1;x<(32768-unit->celllen-1);x++){ // could be less of a large chunk to process
    sum=(cells[x-1]&1)+(cells[x+1]&1)+(cells[x-unit->celllen]&1)+(cells[x+unit->celllen]&1)+(cells[x-unit->celllen-1]&1)+(cells[x-unit->celllen+1]&1)+(cells[x+unit->celllen-1]&1)+(cells[x+unit->celllen+1]&1);

    /*    if (sum==3 || (sum+(cells[x]&0x01)==3)) newcells[x]=255; /// 
	  else newcells[x]=0;*/
    if ((cells[x]&1)==1 && sum<2) newcells[x]=0;
    else if ((cells[x]&1)==1 && sum>3) newcells[x]=0;
    else if ((cells[x]&1)==0 && sum==3) newcells[x]=255;
    else newcells[x]=cells[x];

    printf("%c",cells[x]);
  }
  
  flag^=0x01;
  return sum;
}

/*
1- CPU/instruction sets from microbd. DONE (mostly) as leaky.c

2- cellular automata: 1d,2d, classic, flexible - re-do all (again states 8 bits) as CA.c

3- worms, langton's ants,turmites -

langton.c -> langtonrev.c (now in CPUs but could also be as datagens - depends how we divide up)

http://en.wikipedia.org/wiki/Langton%27s_ant

+ hodge.c, hodgenet.c and older BD hodge
*/
//6-SIR section

// SIR: we we could resolve edges? redo completely for speed, also all
// inits from workingbuffer

uint16_t SIR(uint16_t delay, uint16_t speed, uint16_t *workingbuffer){ // sudden death!
  static uint8_t flag=0;
  uint16_t cell,cell1,cell2,cell3,sum=0;
  uint16_t *newcells, *cells;
  int16_t x,y,k,p;

  if (flag==0) {
    cells=workingbuffer; newcells=&workingbuffer[MAXDIV2];
  }
  else {
    cells=&workingbuffer[MAXDIV2]; newcells=workingbuffer;
  }      

  /* cell = cells[MAXDIV2]>>8; */
  /* newcells[0]=cell<<8; */
  /* if (cell >= k) newcells[0] = recovered<<8;                                                  */
  /* else if ((cell>32 && cell<k)) newcells[0]+=255; */
  /* else if (cell == susceptible) { */
  /*   cell = cells[MAXDIV2]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[CELLLEN]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[1]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /*   cell = cells[MAXDIV2-CELLLEN]>>8; */
  /*   if (cell>32 && cell<k) if (rand()%10 < p) newcells[0] = 64<<8;        */
  /* 	} */

  /////

  for (x=1;x<MAXDIV2;x++){
        cell = cells[x]>>8;
	newcells[x]=cells[x];
	if (cell >= k) newcells[x] = recovered<<8;                                                 
	else if (cell>8){
	  newcells[x]+=255; // aging                                                       
	}
	else { // susceptible
	  sum++;
	  y=x-1;
	  cell = cells[y]>>8;
	  y=(x+CELLLEN)%MAXDIV2;
	  cell1 = cells[y]>>8;
	  y=(x+1)%MAXDIV2;
	  cell2 = cells[y]>>8;
	  if ((x-CELLLEN)<0) y=MAXDIV2-(x+CELLLEN);
	  else y=x-CELLLEN;
	  cell3 = cells[y]>>8;

      if ( (cell>0 && cell<k) ||
	   (cell1>0 && cell1<k) ||
	   (cell2>0 && cell2<k) ||
	   (cell3>0 && cell3<k))
	{
	if (rand()%10 < p) newcells[x] = 9<<8;       
	}
	}}

  //SPEED UP: can also pull out loop for 0->celllen and for last celllen

  flag^=0x01;
  return sum;
}
