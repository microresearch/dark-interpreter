
/* position in array//or dir//or stack push/pull:

   array[0->xx], dir=hardwaredirs below, sampledir,
   anydir, +otherdirs, stack push/pulls (0/1)x4

   simulation: function%NUM_FUNCS,delay,howmany - as settings which are pushed
   cpu: addr, wrap,CPU%31,delay
   leak: addr, wrap,CPU%31,delay
   CA: CA%NUM_CA,delay,howmany

*/

//////////////////////////////////////////////WALKERS

// these as chunks or do all steps, then wraps usw.

#define LMERSTEP (settingsarray[0])
#define LMERWRAP (settingsarray[1])
#define LMERSTART (settingsarray[2])
#define LMERSPEED (settingsarray[3]) //+4 for mirroring ops across walkers

#define MAXIMERSTEP (settingsarray[4])
#define MAXIMERWRAP (settingsarray[5])
#define MAXIMERSTART (settingsarray[6])
#define MAXIMERSPEED (settingsarray[7])

#define F0106ERSTEP (settingsarray[8])
#define F0106ERWRAP (settingsarray[9])
#define F0106ERSTART (settingsarray[10])
#define F0106ERSPEED (settingsarray[11])

#define HDGENERSTEP (settingsarray[12])
#define HDGENERWRAP (settingsarray[13])
#define HDGENERSTART (settingsarray[14])
#define HDGENERSPEED (settingsarray[15])

#define SAMPLESTEP (settingsarray[16])
#define SAMPLEWRAP (settingsarray[17])
#define SAMPLESTART (settingsarray[18])  // 32 bit max 32768
#define SAMPLESPEED (settingsarray[19])

#define SAMPLESTEPREAD (settingsarray[20])
#define SAMPLEWRAPREAD (settingsarray[21])
#define SAMPLESTARTREAD (settingsarray[22])  // 32 bit max 32768
#define SAMPLESPEEDREAD (settingsarray[23])

#define ANYSTEP (settingsarray[24])
#define ANYWRAP (settingsarray[25])
#define ANYSTART (settingsarray[26])
#define ANYSPEED (settingsarray[27])

#define ANYSTEPREAD (settingsarray[28])
#define ANYWRAPREAD (settingsarray[29])
#define ANYSTARTREAD (settingsarray[30])
#define ANYSPEEDREAD (settingsarray[31]) // 0-31 =32 (4x8) walker settings!!!

// CONSTRAINT/micro-macro settings!
// we just attack wraps directly

//////////////////////////////////////////////////

// directions from main.c

#define HDGENERDIR (settingsarray[40]) //TODO redo?
#define LMERDIR (settingsarray[41])
#define MAXIMERDIR (settingsarray[42])
#define F0106ERDIR (settingsarray[43])
#define FINALDIR (settingsarray[44])

// directions from audio.c - add read dirs

#define DATADIRR (settingsarray[45]) // TODO restrict to 4
#define DATADIRW (settingsarray[46])
#define SAMPLEDIRR (settingsarray[47])
#define SAMPLEDIRW (settingsarray[48])

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

/////////////////////////////////

///push settings

#define PUSHONE16BIT (settingsarray[54]) // settings for the push <<???
#define PUSHTWO16BIT (settingsarray[55]) // settings for the push <<???
#define PUSHONE8BIT (settingsarray[56])
#define PUSHTWO8BIT (settingsarray[57])
#define PUSHTHREE8BIT (settingsarray[58])
#define EXESTACKPUSH (settingsarray[59]) // TYPE so limit to 0-3

#define BEFOREDIR  0
#define BEFORESTACK 0 // limit here
#define FINALL 0 // end of end = +5 for push and pops
