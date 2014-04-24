//////////////////////////////////////////////WALKERS - max 15 bits so no wrap on +1

#define HWSTART (settingsarray[0])
#define SAMPLESTART (settingsarray[1]) 
#define SAMPLESTARTREAD (settingsarray[2]) 
#define SAMPLESTARTFILT (settingsarray[3]) 
#define ANYSTART (settingsarray[4])
#define ANYSTARTREAD (settingsarray[5])
#define ANYSTARTFILT (settingsarray[6])
#define VILLAGEWSTART (settingsarray[7])
#define VILLAGERSTART (settingsarray[8])
#define VILLAGEFSTART (settingsarray[9])

#define HWWRAP ((settingsarray[10])+1)//when wrapper changes we need to redo direction array!!!
#define SAMPLEWRAP ((settingsarray[11])+1)
#define SAMPLEWRAPREAD ((settingsarray[12])+1)
#define SAMPLEWRAPFILT ((settingsarray[13])+1)
#define ANYWRAPREAD ((settingsarray[14])+1)
#define ANYWRAP ((settingsarray[15])+1)
#define ANYWRAPFILT ((settingsarray[16])+1)
#define VILLAGEWWRAP ((settingsarray[17])+1)
#define VILLAGERWRAP ((settingsarray[18])+1)
#define VILLAGEFWRAP ((settingsarray[19])+1)

#define HWSTEP (settingsarray[20])>>7 // 15 bits to 8 bits
#define SAMPLESTEP (settingsarray[21])>>7
#define SAMPLESTEPREAD (settingsarray[22])>>7
#define SAMPLESTEPFILT (settingsarray[23])>>7
#define ANYSTEP (settingsarray[24])>>7
#define ANYSTEPREAD (settingsarray[25])>>7
#define ANYSTEPFILT (settingsarray[26])>>7
#define VILLAGEWSTEP (settingsarray[27])>>7
#define VILLAGERSTEP (settingsarray[28])>>7
#define VILLAGEFSTEP (settingsarray[29])>>7

#define HWSPEED (settingsarray[30])>>7
#define SAMPLESPEED (settingsarray[31])>>7
#define SAMPLESPEEDREAD (settingsarray[32])>>7
#define SAMPLESPEEDFILT (settingsarray[33])>>7
#define LEAKSPEED (settingsarray[34])>>7
#define MACHINESPEED (settingsarray[35])>>7

#define HWDIR (settingsarray[36])>>13 // TODO restrict to 2 bits
#define DATADIRR (settingsarray[37])>>13
#define DATADIRW (settingsarray[38])>>13
#define DATADIRF (settingsarray[39])>>13
#define SAMPLEDIRR (settingsarray[40])>>13
#define SAMPLEDIRW (settingsarray[41])>>13
#define SAMPLEDIRF (settingsarray[42])>>13
#define VILLAGEWDIR (settingsarray[43])>>13
#define VILLAGERDIR (settingsarray[44])>>13
#define VILLAGEFDIR (settingsarray[45])>>13

// hardware constraints - real walkers - don't have to mirror!
# define HDGENERBASE (settingsarray[46])>>7// less than 255
# define HDGENERCONS (settingsarray[47])>>7// less than 255
# define LMERBASE (settingsarray[48])
# define LMERCONS ((settingsarray[49])+1)
# define F0106ERBASE ((settingsarray[50])+1)
# define F0106ERCONS ((settingsarray[51])+1)
# define MAXIMERBASE ((settingsarray[52])+1)
# define MAXIMERCONS ((settingsarray[53])+1)

//////////////////////////////////////////////////

// Excess settings/actions to walkers - WHERE to set?

#define LEAKINESS (settingsarray[54])>>7
#define INFECTION (settingsarray[55])>>7
#define MUTATION (settingsarray[56])>>7
#define EXESPOT (settingsarray[57])>>10  // execution order setting already constrained upto22

/////////// BELOW is set by mirror!

#define EFFECTREAD (settingsarray[58])>>7 ///<255
#define EFFECTWRITE (settingsarray[59])>>7 
#define EFFECTFILTER (settingsarray[60])>>7 

// 60
