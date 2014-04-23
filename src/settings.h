
/* position in array//or dir//or stack push/pull:

   array[0->xx], dir=hardwaredirs below, sampledir,
   anydir, +otherdirs, stack push/pulls (0/1)x4

   simulation: function%NUM_FUNCS,delay,howmany - as settings which are pushed
   cpu: addr, wrap,CPU%31,delay
   leak: addr, wrap,CPU%31,delay
   CA: CA%NUM_CA,delay,howmany

*/

//////////////////////////////////////////////WALKERS

#define HWSTART (settingsarray[0])
#define SAMPLESTART (settingsarray[4]) 
#define SAMPLESTARTREAD (settingsarray[5]) 
#define SAMPLESTARTFILT (settingsarray[6]) 
#define ANYSTART (settingsarray[7])
#define ANYSTARTREAD (settingsarray[8])
#define ANYSTARTFILT (settingsarray[9])

#define HWWRAP (settingsarray[10])//when wrapper changes we need to redo direction array!!!

#define SAMPLEWRAP (settingsarray[14])
#define SAMPLEWRAPREAD (settingsarray[15])
#define SAMPLEWRAPFILT (settingsarray[16])
#define ANYWRAPREAD (settingsarray[17])
#define ANYWRAP (settingsarray[18])
#define ANYWRAPFILT (settingsarray[19])

#define HWSTEP (settingsarray[20])
#define SAMPLESTEP (settingsarray[24])
#define SAMPLESTEPREAD (settingsarray[25])
#define SAMPLESTEPFILT (settingsarray[26])
#define ANYSTEP (settingsarray[27])
#define ANYSTEPREAD (settingsarray[28])
#define ANYSTEPFILT (settingsarray[29])

#define HWSPEED (settingsarray[30])
#define SAMPLESPEED (settingsarray[34])
#define SAMPLESPEEDREAD (settingsarray[35])
#define SAMPLESPEEDFILT (settingsarray[36])

// CONSTRAINT/micro-macro settings!
// we just attack wraps directly

//////////////////////////////////////////////////

// directions

#define HWDIR (settingsarray[37]) //TODO redo?
#define DATADIRR (settingsarray[42]) // TODO restrict to 4
#define DATADIRW (settingsarray[43])
#define DATADIRF (settingsarray[44])
#define SAMPLEDIRR (settingsarray[45])
#define SAMPLEDIRW (settingsarray[46])
#define SAMPLEDIRF (settingsarray[47])

// hardware constraints - real walkers
# define HDGENERBASE (settingsarray[48])// less than 255
# define HDGENERCONS (settingsarray[49])// less than 255
# define LMERBASE (settingsarray[50])
# define LMERCONS (settingsarray[51])
# define F0106ERBASE (settingsarray[52])
# define F0106ERCONS (settingsarray[53])
# define MAXIMERBASE (settingsarray[54])
# define MAXIMERCONS (settingsarray[55])

//////////////////////////////////////////////////

// Excess settings/actions to walkers

#define LEAKSPEED (settingsarray[56])
#define MACHINESPEED (settingsarray[57])
#define LEAKINESS (settingsarray[58])
#define INFECTION (settingsarray[59])
#define MUTATION (settingsarray[60])
#define EFFECTREAD (settingsarray[61]) ///<255
#define EFFECTWRITE (settingsarray[62]) 
#define EFFECTFILTER (settingsarray[63]) 
#define EXESPOT (settingsarray[63])  // execution order setting already constrained

/////////////////////////////////

// redo/redo in main.c
