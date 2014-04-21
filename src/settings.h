
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

// hardware constraints
# define HDGENERBASE (settingsarray[48])>>8 // less than 255
# define HDGENERCONS (settingsarray[49])>>8 // less than 255
# define LMERBASE (settingsarray[50])
# define LMERCONS (settingsarray[51])
# define F0106ERBASE (settingsarray[52])
# define F0106ERCONS (settingsarray[53])
# define MAXIMERBASE (settingsarray[54])
# define MAXIMERCONS (settingsarray[55])

//////////////////////////////////////////////////


//////////////////////////////////////////////////

#define LEAKSPEED (settingsarray[56])
#define MACHINESPEED (settingsarray[57])
#define LEAKINESS (settingsarray[58])
#define INFECTION (settingsarray[59])
#define MUTATION (settingsarray[60])

//////////////////////////////////

// TODO: excess settings/actions

// dirs to follow wormdir (but need to follow not just set once)-halved in some/all cases?
// mirror read and write heads as an action
// that grains can set datagen start/wrap settings also (grain spawn mode)

// villageread, villagewrite=0-2

#define EFFECTREAD (settingsarray[61]) 
//nofilter. bit0=clip/noclip; 1=buffer.2=buffer <<3 = 0-6(7 options)= total =63
// else options = 0-14 = total=128

#define EFFECTWRITE (settingsarray[62]) 
#define EFFECTFILTER (settingsarray[63]) 

/////////////////////////////////
