
/* position in array//or dir//or stack push/pull:

   array[0->xx], dir=hardwaredirs below, sampledir,
   anydir, +otherdirs, stack push/pulls (0/1)x4

   simulation: function%NUM_FUNCS,delay,howmany - as settings which are pushed
   cpu: addr, wrap,CPU%31,delay
   leak: addr, wrap,CPU%31,delay
   CA: CA%NUM_CA,delay,howmany

*/

//////////////////////////////////////////////WALKERS

#define LMERSTEP (settingsarray[0])
#define LMERWRAP (settingsarray[1])
#define LMERSTART (settingsarray[2])
#define LMERSPEED (settingsarray[3])

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
#define SAMPLEWRAP (((settingsarray[17])<<8)%32768) // 32 bit max 32768
#define SAMPLESTART ((settingsarray[18])<<7)  // 32 bit max 32768
#define SAMPLESPEED (settingsarray[19])

#define SAMPLESTEPREAD (settingsarray[20])
#define SAMPLEWRAPREAD (((settingsarray[21])<<8)%32768) // 32 bit max 32768
#define SAMPLESTARTREAD ((settingsarray[22])<<7)  // 32 bit max 32768
#define SAMPLESPEEDREAD (settingsarray[23])

#define ANYSTEP (settingsarray[24])
#define ANYWRAP (settingsarray[25])
#define ANYSTART (settingsarray[26])
#define ANYSPEED (settingsarray[27])

#define ANYSTEPREAD (settingsarray[28])
#define ANYWRAPREAD (settingsarray[29])
#define ANYSTARTREAD (settingsarray[30])
#define ANYSPEEDREAD (settingsarray[31]) // 0-31 walker settings!!!

// directions from main.c

#define HDGENERDIR (settingsarray[36]%8)
#define LMERDIR (settingsarray[37]%8)
#define MAXIMERDIR (settingsarray[38]%8)
#define F0106ERDIR (settingsarray[39]%8)
#define FINALDIR (settingsarray[40]%8)

// directions from audio.c - add read dirs

#define DATADIRR ((settingsarray[41])%4)
#define DATADIRW ((settingsarray[41])%4)
#define SAMPLEDIRR ((settingsarray[42])%4)
#define SAMPLEDIRW ((settingsarray[42])%4)

//////////////////////////////////////////////////

#define LEAKSPEED (settingsarray[0])
#define MACHINESPEED (settingsarray[1])
#define LEAKINESS (settingsarray[14])
#define INFECTION (settingsarray[15])
#define MUTATION (settingsarray[16])

//////////////////////////////////

// TODO: excess settings/actions

// dirs to follow wormdir (but need to follow not just set once)
// cons, consread

#define cons (settingsarray[0])
#define consread (settingsarray[0])

/////////////////////////////////

///push settings

#define PUSHONE16BIT (settingsarray[27]) // settings for the push <<???
#define PUSHTWO16BIT (settingsarray[28]) // settings for the push <<???
#define PUSHONE8BIT (settingsarray[29])
#define PUSHTWO8BIT (settingsarray[30])
#define PUSHTHREE8BIT (settingsarray[31])
#define EXESTACKPUSH (settingsarray[32]) // TYPE so limit to 0-3

#define BEFOREDIR  36
#define BEFORESTACK 43 // limit here
#define FINALL 48 // end of end = +5 for push and pops
