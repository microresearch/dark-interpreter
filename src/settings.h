//////////////////////////////////////////////WALKERS - max 15 bits so no wrap on +1

#define HWSTART (settingsarray[0])
#define SAMPLESTART (settingsarray[1]) 
#define SAMPLESTARTREAD (settingsarray[2]) 
#define SAMPLESTARTFILT (settingsarray[3]) 
#define ANYSTART (settingsarray[4])
#define ANYSTARTREAD (settingsarray[5])
#define ANYSTARTFILT (settingsarray[6])
#define STACKSTART (settingsarray[7])
#define LMERBASE (settingsarray[8])
#define F0106ERBASE ((settingsarray[9])+1)
#define MAXIMERBASE ((settingsarray[10])+1)

#define HWWRAP ((settingsarray[11])+1)//when wrapper changes we need to redo direction array!!!
#define SAMPLEWRAP ((settingsarray[12])+1)
#define SAMPLEWRAPREAD ((settingsarray[13])+1)
#define SAMPLEWRAPFILT ((settingsarray[14])+1)
#define ANYWRAPREAD ((settingsarray[15])+1)
#define ANYWRAP ((settingsarray[16])+1)
#define ANYWRAPFILT ((settingsarray[17])+1)
#define VILLAGEWWRAP ((settingsarray[18])+1)
#define VILLAGERWRAP ((settingsarray[19])+1)
#define VILLAGEFWRAP ((settingsarray[20])+1)
#define STACKWRAP (settingsarray[21])

#define LMERCONS ((settingsarray[22])+1)
#define F0106ERCONS ((settingsarray[23])+1)
#define MAXIMERCONS ((settingsarray[24])+1)

#define HWSTEP (settingsarray[25])>>7 // 15 bits to 8 bits
#define SAMPLESTEP (settingsarray[26])>>7
#define SAMPLESTEPREAD (settingsarray[27])>>7
#define SAMPLESTEPFILT (settingsarray[28])>>7
#define ANYSTEP (settingsarray[29])>>7
#define ANYSTEPREAD (settingsarray[30])>>7
#define ANYSTEPFILT (settingsarray[31])>>7
#define VILLAGEWSTEP (settingsarray[32])>>7
#define VILLAGERSTEP (settingsarray[33])>>7
#define VILLAGEFSTEP (settingsarray[34])>>7

#define HWSPEED (settingsarray[35])>>7
#define SAMPLESPEED (settingsarray[36])>>7
#define SAMPLESPEEDREAD (settingsarray[37])>>7
#define SAMPLESPEEDFILT (settingsarray[38])>>7
#define LEAKSPEED (settingsarray[39])>>7
#define MACHINESPEED (settingsarray[40])>>7

#define HWDIR (settingsarray[41])>>13 // restricted to 2 bits
#define DATADIRR (settingsarray[42])>>13
#define DATADIRW (settingsarray[43])>>13
#define DATADIRF (settingsarray[44])>>13
#define SAMPLEDIRR (settingsarray[45])>>13
#define SAMPLEDIRW (settingsarray[46])>>13
#define SAMPLEDIRF (settingsarray[47])>>13
#define VILLAGEWDIR (settingsarray[48])>>13
#define VILLAGERDIR (settingsarray[49])>>13
#define VILLAGEFDIR (settingsarray[50])>>13

// hardware constraints - real walkers - don't have to mirror!
#define HDGENERBASE (settingsarray[51])>>7// less than 255
#define HDGENERCONS (settingsarray[52])>>7// less than 255
#define VILLAGEWSTART (settingsarray[53])>>9
#define VILLAGERSTART (settingsarray[54])>>9
#define VILLAGEFSTART (settingsarray[55])>>9

#define STACKMUCH (settingsarray[56])>>7

//////////////////////////////////////////////////

// Excess settings/actions to walkers - WHERE to set?

#define LEAKINESS (settingsarray[57])>>7
#define INFECTION (settingsarray[58])>>7
#define MUTATION (settingsarray[59])>>7
#define EXESPOT (settingsarray[60])>>10  // execution order setting already constrained upto22

/////////// BELOW is set by mirror!

#define EFFECTREAD (settingsarray[61])>>7 ///<255
#define EFFECTWRITE (settingsarray[62])>>7 
#define EFFECTFILTER (settingsarray[63])>>7 

// 64!!!!!!!!!!!!!!!
