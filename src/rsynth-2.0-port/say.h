/* say.h
*/

#include "darray.h"



extern short *pWavBuffer;
extern darray_t wav_elm;
extern unsigned short wav_len;

void PhonemeToWaveData(char *phone, int len, int verbose);
void FreePhonemeToWaveData(void);
