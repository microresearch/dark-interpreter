
/* position in array//or dir//or stack push/pull:

   array[0->xx], dir=hardwaredirs below, sampledir,
   anydir, +otherdirs, stack push/pulls (0/1)x4

   simulation: function%NUM_FUNCS,delay,howmany - as settings which are pushed
   cpu: addr, wrap,CPU%31,delay
   leak: addr, wrap,CPU%31,delay
   CA: CA%NUM_CA,delay,howmany

*/

//////////////////////////////////////////////WALKERS

#define MAXIMERSTART (settingsarray[0])
#define LMERSTART (settingsarray[1])
#define F0106ERSTART (settingsarray[2])
#define HDGENERSTART (settingsarray[3])
#define SAMPLESTART (settingsarray[4]) 
#define SAMPLESTARTREAD (settingsarray[5]) 
#define SAMPLESTARTFILT (settingsarray[6]) 
#define ANYSTART (settingsarray[7])
#define ANYSTARTREAD (settingsarray[8])
#define ANYSTARTFILT (settingsarray[9])


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

#define LMERSTEP (settingsarray[20])
#define MAXIMERSTEP (settingsarray[21])
#define F0106ERSTEP (settingsarray[22])
#define HDGENERSTEP (settingsarray[23])
#define SAMPLESTEP (settingsarray[24])
#define SAMPLESTEPREAD (settingsarray[25])
#define SAMPLESTEPFILT (settingsarray[26])
#define ANYSTEP (settingsarray[27])
#define ANYSTEPREAD (settingsarray[28])
#define ANYSTEPFILT (settingsarray[29])

#define LMERSPEED (settingsarray[30])
#define MAXIMERSPEED (settingsarray[31])
#define F0106ERSPEED (settingsarray[32])
#define HDGENERSPEED (settingsarray[33])
#define SAMPLESPEED (settingsarray[34])
#define SAMPLESPEEDREAD (settingsarray[35])
#define SAMPLESPEEDFILT (settingsarray[36])

// CONSTRAINT/micro-macro settings!
// we just attack wraps directly

//////////////////////////////////////////////////

// directions from main.c

#define HDGENERDIR (settingsarray[37]) //TODO redo?
#define LMERDIR (settingsarray[38])
#define MAXIMERDIR (settingsarray[39])
#define F0106ERDIR (settingsarray[40])
#define FINALDIR (settingsarray[41])

// directions from audio.c - add read dirs

#define DATADIRR (settingsarray[42]) // TODO restrict to 4
#define DATADIRW (settingsarray[43])
#define DATADIRF (settingsarray[44])
#define SAMPLEDIRR (settingsarray[45])
#define SAMPLEDIRW (settingsarray[46])
#define SAMPLEDIRF (settingsarray[47])

// CONSTRAINT/micro-macro settings!
// we just attack wraps directly

//////////////////////////////////////////////////


//////////////////////////////////////////////////

#define LEAKSPEED (settingsarray[49])
#define MACHINESPEED (settingsarray[50])
#define LEAKINESS (settingsarray[51])
#define INFECTION (settingsarray[52])
#define MUTATION (settingsarray[53])

//////////////////////////////////

// TODO: excess settings/actions

// dirs to follow wormdir (but need to follow not just set once)-halved in some/all cases?
// mirror read and write heads as an action
// that grains can set datagen start/wrap settings also (grain spawn mode)

// villageread, villagewrite=0-2

#define EFFECTREAD (settingsarray[54]) 
//nofilter. bit0=clip/noclip; 1=buffer.2=buffer <<3 = 0-6(7 options)= total =63
// else options = 0-14 = total=128

#define EFFECTWRITE (settingsarray[55]) 
#define EFFECTFILTER (settingsarray[56]) 

// alter all/some wraps

// when wrapper changes we need to redo direction array!!!

/////////////////////////////////

///push settings-TODO: if we use these any more?

#define PUSHONE16BIT (settingsarray[57]) // settings for the push <<???
#define PUSHTWO16BIT (settingsarray[58]) // settings for the push <<???
#define PUSHONE8BIT (settingsarray[59])
#define PUSHTWO8BIT (settingsarray[60])
#define PUSHTHREE8BIT (settingsarray[61])
#define EXESTACKPUSH (settingsarray[62]) // TYPE so limit to 0-3

#define BEFOREDIR  0
#define BEFORESTACK 0 // limit here
#define FINALL 0 // end of end = +5 for push and pops
