//////////////////////////////////////////////WALKERS - can be 16 bits so need to fix WRAP!

#define HWSTART ((settingsarray[0])>>1)

#define SAMPLESTART ((settingsarray[1])>>1) 
#define SAMPLESTARTREAD ((settingsarray[2])>>1) 
#define SAMPLESTARTFILT ((settingsarray[3])>>1) 
#define ANYSTART ((settingsarray[4])>>1) 
#define ANYSTARTREAD ((settingsarray[5])>>1) 
#define ANYSTARTFILT ((settingsarray[6])>>1) 

#define STACKSTART ((settingsarray[7])>>1) // ??? for villager
#define LMERBASE ((settingsarray[8])>>1) 
#define F0106ERBASE ((settingsarray[9])>>1)
#define MAXIMERBASE ((settingsarray[10])>>1) 

#define HWWRAP ((settingsarray[11]>>1)+1) // allowed - can all hit 32768
#define SAMPLEWRAP ((settingsarray[12]>>1)+1)
#define SAMPLEWRAPREAD ((settingsarray[13]>>1)+1)
#define SAMPLEWRAPFILT ((settingsarray[14]>>1)+1)
#define ANYWRAPREAD ((settingsarray[15]>>1)+1)
#define ANYWRAP ((settingsarray[16]>>1)+1)
#define ANYWRAPFILT ((settingsarray[17]>>1)+1)
#define VILLAGEWWRAP ((settingsarray[18]>>10)+1) // 8 BITs
#define VILLAGERWRAP ((settingsarray[19]>>10)+1) // 8 bits
#define VILLAGEFWRAP ((settingsarray[20]>>10)+1) // 8 BITS
#define STACKWRAP ((settingsarray[21]>>1)+1)

#define LMERCONS ((settingsarray[22]>>1)+1)
#define F0106ERCONS ((settingsarray[23]>>1)+1)
#define MAXIMERCONS ((settingsarray[24]>>1)+1)

#define HWSTEP (settingsarray[25]>>8) // 16 bits to 8 bits
#define SAMPLESTEP (settingsarray[26]>>8)
#define SAMPLESTEPREAD (settingsarray[27]>>8)
#define SAMPLESTEPFILT (settingsarray[28]>>8)
#define ANYSTEP (settingsarray[29]>>8)
#define ANYSTEPREAD (settingsarray[30]>>8)
#define ANYSTEPFILT (settingsarray[31]>>8)
#define VILLAGEWSTEP (settingsarray[32]>>8)
#define VILLAGERSTEP (settingsarray[33]>>8)
#define VILLAGEFSTEP (settingsarray[34]>>8)

#define HWSPEED (settingsarray[35]>>8) // 8 bits
#define SAMPLESPEED (settingsarray[36]>>8) // 16 bits to 3 bits
#define SAMPLESPEEDREAD (settingsarray[37]>>8)
#define SAMPLESPEEDFILT (settingsarray[38]>>8)
#define LEAKSPEED (settingsarray[39]>>8)
#define MACHINESPEED (settingsarray[40]>>8)

// hardware constraints - real walkers - don't have to mirror!
#define HDGENERBASE (settingsarray[41]>>8)// less than 255
#define HDGENERCONS (settingsarray[42]>>8)// less than 255
#define VILLAGEWSTART (settingsarray[43]>>10) //64 *2
#define VILLAGERSTART (settingsarray[44]>>10)
#define VILLAGEFSTART (settingsarray[45]>>10)

/// float mods in audio.c

#define FMOD ((float32_t)settingsarray[46]/65536.0f)
#define FMODF ((float32_t)settingsarray[47]/65536.0f)
#define FMODW ((float32_t)settingsarray[48]/65536.0f)//

//////////////////////////////////////////////////

// Excess settings/actions to walkers - WHERE to set?

#define STACKFUNC (settingsarray[49]>>11)
#define STACKMUCH (settingsarray[50]>>10) 

#define SAMPLEEXPAND (settingsarray[51]>>1) // test TODO for expansion
#define SAMPLEREXPAND (settingsarray[52]>>1) // test TODO for expansion
#define SAMPLEFEXPAND (settingsarray[53]>>1) // test TODO for expansion

/// writeexpand above-51, readexpand-52, filterexpand-53

//#define EFFECTREAD (settingsarray[51]>>9) /// 7 bits =128 // TESTY out of mirror
//#define EFFECTWRITE (settingsarray[52]>>9) 
//#define EFFECTFILTER (settingsarray[53]>>9) 
/////
#define HWDIR (settingsarray[54]>>15) // restricted to 1 bit
#define DATADIRR (settingsarray[55]>>15)
#define DATADIRW (settingsarray[56]>>15)
#define DATADIRF (settingsarray[57]>>15)
#define SAMPLEDIRR (settingsarray[58]>>15)
#define SAMPLEDIRW (settingsarray[59]>>15)
#define SAMPLEDIRF (settingsarray[60]>>15)
#define VILLAGEWDIR (settingsarray[61]>>15)
#define VILLAGERDIR (settingsarray[62]>>15)
#define VILLAGEFDIR (settingsarray[63]>>15)

//foldback settings now in new array

//#define FOLDSSTART (settingsarray[64]>>1)) 
//#define FOLDSWRAP (settingsarray[65]>>9)// 6 bits 
//(FOLDD[1]>>9)
//(FOLDD[0]>>1)

//#define FOLDDSTART (settingsarray[66])>>10) // 6 bits
//(FOLDD[2])>>10)
//#define FOLDDWRAP (settingsarray[67]>>9)
//(FOLDD[3]>>9)
//#define FOLDSPEED (settingsarray[68]>>10) //unused...

