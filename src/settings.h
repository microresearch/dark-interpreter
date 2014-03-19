
/* position in array//or dir//or stack push/pull:

   array[0->xx], dir=hardwaredirs below, sampledir,
   anydir, +otherdirs, stack push/pulls (0/1)x4

   simulation: function%NUM_FUNCS,delay,howmany - as settings which are pushed
   cpu: addr, wrap,CPU%31,delay
   leak: addr, wrap,CPU%31,delay
   CA: CA%NUM_CA,delay,howmany

*/

// main.c

#define LEAKINESS (settingsarray[0])
#define INFECTION (settingsarray[1])
#define MUTATION (settingsarray[2])
#define LMERWORMFLAG (settingsarray[3])
#define LMERSTEP (settingsarray[4])
#define LMERSPEED (settingsarray[5])
#define MAXIMERWORMFLAG (settingsarray[6])
#define MAXIMERSTEP (settingsarray[7])
#define MAXIMERSPEED (settingsarray[8])
#define F0106ERWORMFLAG (settingsarray[9])
#define F0106ERSTEP (settingsarray[10])
#define F0106ERSPEED (settingsarray[11])
#define HDGENERWORMFLAG (settingsarray[12])
#define HDGENERSTEP (settingsarray[13])
#define HDGENERSPEED (settingsarray[14])

#define LEAKSPEED (settingsarray[15])
#define MACHINESPEED (settingsarray[16])

// audio.c

#define SAMPLESTEP (settingsarray[17])
#define SAMPLEWRAP ((settingsarray[18])<<8) // 32 bit
#define SAMPLESTART (settingsarray[19])
#define SAMPLESPEED (settingsarray[20])

#define ANYSTEP (settingsarray[21])
#define ANYSPEED (settingsarray[22])
#define EDGERASSETTING ((settingsarray[23])<<8) // 32 bit
#define INSTEP (settingsarray[24])
#define EDGESTEP (settingsarray[25])
#define EDGESPEED (settingsarray[26])

#define PUSHONE16BIT (settingsarray[27])
#define PUSHTWO16BIT (settingsarray[28])
#define PUSHONE8BIT (settingsarray[29])
#define PUSHTWO8BIT (settingsarray[30])
#define PUSHTHREE8BIT (settingsarray[31])
#define EXESTACKPUSH (settingsarray[32])

#define FINALWORMFLAG (settingsarray[33])
#define FINALSTEP (settingsarray[34])
#define FINALSPEED (settingsarray[35])

// directions from main.c

#define HDGENERDIR (settingsarray[36]%8)
#define LMERDIR (settingsarray[37]%8)
#define MAXIMERDIR (settingsarray[38]%8)
#define F0106ERDIR (settingsarray[39]%8)
#define FINALDIR (settingsarray[40]%8)

// directions from audio.c

#define SAMPLEDIR (settingsarray[41])
#define ANYDIR (settingsarray[42])


#define BEFOREDIR  36
#define BEFORESTACK 43 // limit here
#define FINALL 53 // end of end = +10 TODO:check!
/* 

stack+10=48odd + extra settings

dir=hardwaredirs below=4, sampledir, anydir, 

+otherdirs

so aim for 64??? but at end we have datagenwalker for settingsarray

*/
