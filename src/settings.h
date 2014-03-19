/*

simulation: function%NUM_FUNCS,delay,howmany - are these not set on a push!
cpu: addr, wrap,CPU%31,delay
leak: addr, wrap,CPU%31,delay
CA: CA%NUM_CA,delay,howmany - set on a push as generic 4 values from array

*/

#define LEAKINESS (settingsarray[0])
#define INFECTION (settingsarray[1])
#define MUTATION (settingsarray[2])
#define LMERWORMFLAG (settingsarray[3])
#define LMERSTEP (settingsarray[4])
#define LMERSPEED (settingsarray[5])
#define MAXIMERWORMFLAG (settingsarray[6])
#define MAXIMERSTEP (settingsarray[7])
#define MAXIMERSPEED (settingsarray[8])
#define f0106ERWORMFLAG (settingsarray[9])
#define f0106ERSTEP (settingsarray[10])
#define f0106ERSPEED (settingsarray[11])
#define HDGENERWORMFLAG (settingsarray[12])
#define HDGENERSTEP (settingsarray[13])
#define HDGENERSPEED (settingsarray[14])

#define LEAKSPEED (settingsarray[15])
#define MACHINESPEED (settingsarray[16])

#define SAMPLESTEP (settingsarray[16])
#define SAMPLEWRAP (settingsarray[17])
#define SAMPLESTART (settingsarray[18])
#define SAMPLESPEED (settingsarray[19])

#define ANYSTEP (settingsarray[20])
#define ANYSPEED (settingsarray[21])
#define EDGERASSETTING (settingsarray[22])
#define INSTEP (settingsarray[23])
#define EDGESTEP (settingsarray[24])
#define EDGESPEED (settingsarray[25])

/* then +4 for generic push settings

so array is 29 but then for settings we have: +2 or +4 push/pull, 

dir=hardwaredirs below=4, sampledir, anydir, +otherdirs

so aim for 64 with extra settings and so on...

*/
