
/* position in array//or dir//or stack push/pull:

   array[0->xx], dir=hardwaredirs below, sampledir,
   anydir, +otherdirs, stack push/pulls (0/1)x4

   simulation: function%NUM_FUNCS,delay,howmany - as settings which are pushed
   cpu: addr, wrap,CPU%31,delay
   leak: addr, wrap,CPU%31,delay
   CA: CA%NUM_CA,delay,howmany

*/

// settings to add - each walker should be constrained by start and wrap

// main.c

#define LEAKSPEED (settingsarray[0])
#define MACHINESPEED (settingsarray[1])
#define LEAKINESS (settingsarray[14])
#define INFECTION (settingsarray[15])
#define MUTATION (settingsarray[16])


#define LMERWORMFLAG (settingsarray[2])
#define LMERSTEP (settingsarray[3])
#define LMERSPEED (settingsarray[4])

#define MAXIMERWORMFLAG (settingsarray[5])
#define MAXIMERSTEP (settingsarray[6])
#define MAXIMERSPEED (settingsarray[7])

#define F0106ERWORMFLAG (settingsarray[8])
#define F0106ERSTEP (settingsarray[9])
#define F0106ERSPEED (settingsarray[10])

#define HDGENERWORMFLAG (settingsarray[11])
#define HDGENERSTEP (settingsarray[12])
#define HDGENERSPEED (settingsarray[13])


// audio.c

#define SAMPLESTEP (settingsarray[17])
#define SAMPLEWRAP (((settingsarray[18])<<8)%32768) // 32 bit max 32768
#define SAMPLESTART ((settingsarray[19])<<7)  // 32 bit max 32768
#define SAMPLESPEED (settingsarray[20])

//add reading samplestuff, start and wraps

#define ANYSTEP (settingsarray[21])
#define ANYSPEED (settingsarray[22])


#define PUSHONE16BIT (settingsarray[27]) // settings for the push <<???
#define PUSHTWO16BIT (settingsarray[28]) // settings for the push <<???
#define PUSHONE8BIT (settingsarray[29])
#define PUSHTWO8BIT (settingsarray[30])
#define PUSHTHREE8BIT (settingsarray[31])
#define EXESTACKPUSH (settingsarray[32]) // TYPE so limit to 0-3

// add start and wraps

#define FINALWORMFLAG (settingsarray[33])
#define FINALSTEP (settingsarray[34])
#define FINALSPEED (settingsarray[35])

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


#define BEFOREDIR  36
#define BEFORESTACK 43 // limit here
#define FINALL 48 // end of end = +5 for push and pops
