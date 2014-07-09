//////////////////////////////////////////////WALKERS - can be 16 bits so need to fix WRAP!

#ifdef LACH

#define SAMPLESTART ((settingsarray[0])>>1) 
#define SAMPLESTARTREAD ((settingsarray[1])>>1) 

#define ANYSTART ((settingsarray[2])>>1) 
#define ANYSTARTREAD ((settingsarray[3])>>1) 

#define VILLAGEWSTART (settingsarray[4]>>10) //64 *2
#define VILLAGERSTART (settingsarray[5]>>10)

#define SAMPLEWRAP ((settingsarray[6]>>1)+1)
#define SAMPLEWRAPREAD ((settingsarray[7]>>1)+1)

#define ANYWRAPREAD ((settingsarray[8]>>1)+1)
#define ANYWRAP ((settingsarray[9]>>1)+1)

#define VILLAGEWWRAP ((settingsarray[10]>>10)+1) // 6 BITs=64
#define VILLAGERWRAP ((settingsarray[11]>>10)+1) 

#define SAMPLESTEP ((settingsarray[12]>>12)+1)
#define SAMPLESTEPREAD ((settingsarray[13]>>12)+1)

#define ANYSTEP ((settingsarray[14]>>12)+1)
#define ANYSTEPREAD ((settingsarray[15]>>12)+1)

#define VILLAGEWSTEP ((settingsarray[16]>>12)+1)
#define VILLAGERSTEP ((settingsarray[17]>>12)+1)

#define SAMPLESPEED ((settingsarray[18]>>12)+1)
#define SAMPLESPEEDREAD ((settingsarray[19]>>12)+1)

#define FMOD ((float32_t)settingsarray[20]/65536.0f)
#define FMODF ((float32_t)settingsarray[21]/65536.0f)
#define FMODW ((float32_t)settingsarray[22]/65536.0f)// LEAVE ALL IN FOR LACH
#define FOLDTOP (settingsarray[23]) 
#define FOLDOFFSET (settingsarray[24]) 
#define DATADIRR (settingsarray[25]>>15)
#define DATADIRW (settingsarray[26]>>15)
#define SAMPLEDIRR (settingsarray[27]>>15)
#define SAMPLEDIRW (settingsarray[28]>>15)
#define VILLAGERDIR (settingsarray[29]>>15)
#define VILLAGEWDIR (settingsarray[30]>>15)
#define MACHINESPEED (settingsarray[31]>>9)

#define THREADCOUNT (settingsarray[32]>>10) // 6 bits
#define VILLAGESTACKPOS (((settingsarray[33]>>10)+1)*2) // 6 bits
#define STACKPOS (settingsarray[34]>>10)
#define STACKPOSY (settingsarray[35]>>10)

#else

#define HWSTART ((settingsarray[0])>>1) //-LACH_TODO
#define SAMPLESTART ((settingsarray[1])>>1) 
#define SAMPLESTARTREAD ((settingsarray[2])>>1) 
#define SAMPLESTARTFILT ((settingsarray[3])>>1) //-LACH_TODO
#define ANYSTART ((settingsarray[4])>>1) 
#define ANYSTARTREAD ((settingsarray[5])>>1) 
#define ANYSTARTFILT ((settingsarray[6])>>1) //-LACH_TODO
#define VILLAGEWSTART (settingsarray[7]>>10) //64 *2
#define VILLAGERSTART (settingsarray[8]>>10)
#define VILLAGEFSTART (settingsarray[9]>>10)//-LACH_TODO
//#define HDGENERBASE (settingsarray[10]>>8) //-LACH_TODO HOLE!
#define LMERBASE ((settingsarray[11])>>1) //-LACH_TODO
#define F0106ERBASE ((settingsarray[12])>>1)//-LACH_TODO 
#define MAXIMERBASE ((settingsarray[13])>>1) //-LACH_TODO

#define HWWRAP ((settingsarray[14]>>1)+1) //-LACH_TODO
#define SAMPLEWRAP ((settingsarray[15]>>1)+1)
#define SAMPLEWRAPREAD ((settingsarray[16]>>1)+1)
#define SAMPLEWRAPFILT ((settingsarray[17]>>1)+1)//-LACH_TODO
#define ANYWRAPREAD ((settingsarray[18]>>1)+1)
#define ANYWRAP ((settingsarray[19]>>1)+1)
#define ANYWRAPFILT ((settingsarray[20]>>1)+1)//-LACH_TODO

#define VILLAGEWWRAP ((settingsarray[21]>>10)+1) // 6 BITs=64
#define VILLAGERWRAP ((settingsarray[22]>>10)+1) 
#define VILLAGEFWRAP ((settingsarray[23]>>10)+1) //-LACH_TODO

#define HDGENERCONS ((settingsarray[24]>>8)+1)//-LACH_TODO
#define LMERCONS ((settingsarray[25]>>1)+1)//-LACH_TODO
#define F0106ERCONS ((settingsarray[26]>>1)+1)//-LACH_TODO
#define MAXIMERCONS ((settingsarray[27]>>1)+1)//-LACH_TODO

#define LMEROFFSET (settingsarray[28]>>1)//-LACH_TODO
#define LMEROFFSETTWO (settingsarray[29]>>1)//-LACH_TODO
#define F0106EROFFSET (settingsarray[30]>>1)//-LACH_TODO
#define MAXIMEROFFSET (settingsarray[31]>>1)//-LACH_TODO

#define HWSTEP ((settingsarray[32]>>12)+1) // 8 bits //-LACH_TODO

#define SAMPLESTEP ((settingsarray[33]>>12)+1)
#define SAMPLESTEPREAD ((settingsarray[34]>>12)+1)
#define SAMPLESTEPFILT ((settingsarray[35]>>12)+1)//-LACH_TODO

#define ANYSTEP ((settingsarray[36]>>12)+1)
#define ANYSTEPREAD ((settingsarray[37]>>12)+1)
#define ANYSTEPFILT ((settingsarray[38]>>12)+1)//-LACH_TODO
#define VILLAGEWSTEP ((settingsarray[39]>>12)+1)
#define VILLAGERSTEP ((settingsarray[40]>>12)+1)
#define VILLAGEFSTEP ((settingsarray[41]>>12)+1)//-LACH_TODO

#define HWSPEED ((settingsarray[42]>>10)+1) // 8 bits//-LACH_TODO
#define SAMPLESPEED ((settingsarray[43]>>12)+1)
#define SAMPLESPEEDREAD ((settingsarray[44]>>12)+1)
#define SAMPLESPEEDFILT ((settingsarray[45]>>12)+1)//-LACH_TODO

#define FMOD ((float32_t)settingsarray[46]/65536.0f)
#define FMODF ((float32_t)settingsarray[47]/65536.0f)
#define FMODW ((float32_t)settingsarray[48]/65536.0f)// LEAVE ALL IN FOR LACH

#define FOLDTOP (settingsarray[49]) 
#define FOLDOFFSET (settingsarray[50]) 
#define MACHINESPEED (settingsarray[51]>>8)

#define HWDIR (settingsarray[52]>>15) //-LACH_TODO
#define DATADIRR (settingsarray[53]>>15)
#define DATADIRW (settingsarray[54]>>15)
#define DATADIRF (settingsarray[55]>>15)//-LACH_TODO
#define SAMPLEDIRR (settingsarray[56]>>15)
#define SAMPLEDIRW (settingsarray[57]>>15)
#define SAMPLEDIRF (settingsarray[58]>>15)//-LACH_TODO
#define VILLAGERDIR (settingsarray[59]>>15)
#define VILLAGEWDIR (settingsarray[60]>>15)
#define VILLAGEFDIR (settingsarray[61]>>15)//-LACH_TODO

#define THREADCOUNT (settingsarray[62]>>10) // 6 bits
#define VILLAGESTACKPOS (((settingsarray[63]>>10)+1)*2) // 6 bits
#define STACKPOS (settingsarray[64]>>10)
#define STACKPOSY (settingsarray[65]>>10)
#endif
