//////////////////////////////////////////////WALKERS - can be 16 bits so need to fix WRAP!

#ifdef LACH

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
#define HDGENERBASE (settingsarray[10]>>8) //-LACH_TODO
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

#define VILLAGEWWRAP ((settingsarray[21]>>9)+1) // 7 BITs=128
#define VILLAGERWRAP ((settingsarray[22]>>9)+1) 
#define VILLAGEFWRAP ((settingsarray[23]>>9)+1) //-LACH_TODO

#define HDGENERCONS ((settingsarray[24]>>8)+1)//-LACH_TODO
#define LMERCONS ((settingsarray[25]>>1)+1)//-LACH_TODO
#define F0106ERCONS ((settingsarray[26]>>1)+1)//-LACH_TODO
#define MAXIMERCONS ((settingsarray[27]>>1)+1)//-LACH_TODO

#define LMEROFFSET (settingsarray[28]>>1)//-LACH_TODO
#define LMEROFFSETTWO (settingsarray[29]>>1)//-LACH_TODO
#define F0106EROFFSET (settingsarray[30]>>1)//-LACH_TODO
#define MAXIMEROFFSET (settingsarray[31]>>1)//-LACH_TODO

#define HWSTEP (settingsarray[32]>>8) // 8 bits //-LACH_TODO
#define SAMPLESTEP (settingsarray[33]>>8)
#define SAMPLESTEPREAD (settingsarray[34]>>8)
#define SAMPLESTEPFILT (settingsarray[35]>>8)//-LACH_TODO

#define ANYSTEP (settingsarray[36]>>8)
#define ANYSTEPREAD (settingsarray[37]>>8)
#define ANYSTEPFILT (settingsarray[38]>>8)//-LACH_TODO
#define VILLAGEWSTEP (settingsarray[39]>>8)
#define VILLAGERSTEP (settingsarray[40]>>8)
#define VILLAGEFSTEP (settingsarray[41]>>8)//-LACH_TODO

#define HWSPEED (settingsarray[42]>>8) // 8 bits//-LACH_TODO
#define SAMPLESPEED (settingsarray[43]>>8)
#define SAMPLESPEEDREAD (settingsarray[44]>>8)
#define SAMPLESPEEDFILT (settingsarray[45]>>8)//-LACH_TODO
//#define LEAKSPEED (settingsarray[46]>>8)
//#define MACHINESPEED (settingsarray[47]>>8)

#define EFFROFFSET (settingsarray[46]>>9) // 7 bits as 128 //-LACH_TODO
#define EFFFOFFSET (settingsarray[47]>>9) // 7 bits as 128//-LACH_TODO

/// float mods in audio.c

#define FMOD ((float32_t)settingsarray[48]/65536.0f)
#define FMODF ((float32_t)settingsarray[49]/65536.0f)
#define FMODW ((float32_t)settingsarray[50]/65536.0f)// LEAVE ALL IN FOR LACH

#define SAMPLEEXPAND (settingsarray[51]>>1) 
#define SAMPLEREXPAND (settingsarray[52]>>1)
#define SAMPLEFEXPAND (settingsarray[53]>>1)//-LACH_TODO

/////
#define HWDIR (settingsarray[54]>>15) //-LACH_TODO
#define DATADIRR (settingsarray[55]>>15)
#define DATADIRW (settingsarray[56]>>15)
#define DATADIRF (settingsarray[57]>>15)//-LACH_TODO
#define SAMPLEDIRR (settingsarray[58]>>15)
#define SAMPLEDIRW (settingsarray[59]>>15)
#define SAMPLEDIRF (settingsarray[60]>>15)//-LACH_TODO
#define VILLAGEWDIR (settingsarray[61]>>15)
#define VILLAGERDIR (settingsarray[62]>>15)
#define VILLAGEFDIR (settingsarray[63]>>15)//-LACH_TODO
#endif
