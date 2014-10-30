// start with fft/inverse and work up

//from teensy: analyze_fft256 see also data_windows there
//also maybe faster filters there and using routines from dspinst.h

#include "simulation.h"
#include "PV_vocoder.h"

const int16_t AudioWindowHanning256[] __attribute__ ((aligned (4))) = {
     0,     5,    20,    45,    80,   124,   179,   243,   317,   401,
   495,   598,   711,   833,   965,  1106,  1257,  1416,  1585,  1763,
  1949,  2145,  2349,  2561,  2782,  3011,  3249,  3494,  3747,  4008,
  4276,  4552,  4834,  5124,  5421,  5724,  6034,  6350,  6672,  7000,
  7334,  7673,  8018,  8367,  8722,  9081,  9445,  9812, 10184, 10560,
 10939, 11321, 11707, 12095, 12486, 12879, 13274, 13672, 14070, 14471,
 14872, 15275, 15678, 16081, 16485, 16889, 17292, 17695, 18097, 18498,
 18897, 19295, 19692, 20086, 20478, 20868, 21255, 21639, 22019, 22397,
 22770, 23140, 23506, 23867, 24224, 24576, 24923, 25265, 25602, 25932,
 26258, 26577, 26890, 27196, 27496, 27789, 28076, 28355, 28627, 28892,
 29148, 29398, 29639, 29872, 30097, 30314, 30522, 30722, 30913, 31095,
 31268, 31432, 31588, 31733, 31870, 31997, 32115, 32223, 32321, 32410,
 32489, 32558, 32618, 32667, 32707, 32737, 32757, 32767, 32767, 32757,
 32737, 32707, 32667, 32618, 32558, 32489, 32410, 32321, 32223, 32115,
 31997, 31870, 31733, 31588, 31432, 31268, 31095, 30913, 30722, 30522,
 30314, 30097, 29872, 29639, 29398, 29148, 28892, 28627, 28355, 28076,
 27789, 27496, 27196, 26890, 26577, 26258, 25932, 25602, 25265, 24923,
 24576, 24224, 23867, 23506, 23140, 22770, 22397, 22019, 21639, 21255,
 20868, 20478, 20086, 19692, 19295, 18897, 18498, 18097, 17695, 17292,
 16889, 16485, 16081, 15678, 15275, 14872, 14471, 14070, 13672, 13274,
 12879, 12486, 12095, 11707, 11321, 10939, 10560, 10184,  9812,  9445,
  9081,  8722,  8367,  8018,  7673,  7334,  7000,  6672,  6350,  6034,
  5724,  5421,  5124,  4834,  4552,  4276,  4008,  3747,  3494,  3249,
  3011,  2782,  2561,  2349,  2145,  1949,  1763,  1585,  1416,  1257,
  1106,   965,   833,   711,   598,   495,   401,   317,   243,   179,
   124,    80,    45,    20,     5,     0,
};


static void copy_to_fft_buffer(void *destination, const void *source)
{
	const int16_t *src = (const int16_t *)source;
	uint32_t *dst = (uint32_t *)destination;

	for (int i=0; i < 256; i++) {
		*dst++ = *src++;  // real sample plus a zero for imaginary
	}
}

static void  copy_from_fft_buffer(void *destination, const void *source)
{
	int16_t *dst = (int16_t *)destination;
	const uint32_t *src = (uint32_t *)source;
	for (int i=0; i < 256; i++) {
	  *dst++ = *(src++);  // real sample plus a zero for imaginary
	  //	    dst[i] += src[2*i];
			}
}

static void apply_window_to_fft_buffer(void *buffer, const void *window)
{
	int16_t *buf = (int16_t *)buffer;
	const int16_t *win = (int16_t *)window;;

	for (int i=0; i < 256; i++) {
		int32_t val = *buf * *win++;
		//*buf = signed_saturate_rshift(val, 16, 15);
		*buf = val >> 15;
		buf += 2;
	}

}


void pvvocprocess(PV *unit, int16_t* inbuffer, int16_t* outbuffer){ // 256 samples

  /// try without overlap/window
  //  memcpy(unit->buffer,inbuffer,512);
  //copy_to_fft_buffer(unit->buffer, inbuffer);
  apply_window_to_fft_buffer(unit->buffer, AudioWindowHanning256);
  arm_cfft_q15(&unit->fft_inst, unit->buffer,0,0);
  // invert
  arm_cfft_q15(&unit->fft_inst, unit->buffer,1,0);
  // copy 256 from fft buffer????
  copy_from_fft_buffer(outbuffer,unit->buffer);
  //  memcpy(outbuffer,inbuffer,512);
  // do we need windows
}

/////NOTES

/*arm_status arm_cfft_radix4_init_q15(
  arm_cfft_radix4_instance_q15 * S,
  uint16_t fftLen,
  uint8_t ifftFlag, inverse=1;
  uint8_t bitReverseFlag)
*/


/*

	AudioAnalyzeFFT256() : AudioStream(1, inputQueueArray),
	  window(AudioWindowHanning256), prevblock(NULL), count(0),
	  naverage(8), outputflag(false) {
		arm_cfft_radix4_init_q15(&fft_inst, 256, 0, 1);
	}
	bool available() {
		if (outputflag == true) {
			outputflag = false;
			return true;
		}
		return false;
	}
	float read(unsigned int binNumber) {
		if (binNumber > 127) return 0.0;
		return (float)(output[binNumber]) * (1.0 / 16384.0);
	}
	float read(unsigned int binFirst, unsigned int binLast) {
		if (binFirst > binLast) {
			unsigned int tmp = binLast;
			binLast = binFirst;
			binFirst = tmp;
		}
		if (binFirst > 127) return 0.0;
		if (binLast > 127) binLast = 127;
		uint32_t sum = 0;
		do {
			sum += output[binFirst++];
		} while (binFirst < binLast);
		return (float)sum * (1.0 / 16384.0);
	}
	void averageTogether(uint8_t n) {
		if (n == 0) n = 1;
		naverage = n;
	}
	void windowFunction(const int16_t *w) {
		window = w;
	}
	virtual void update(void);
	uint16_t output[128] __attribute__ ((aligned (4)));
private:
	const int16_t *window;
	audio_block_t *prevblock;
	int16_t buffer[512] __attribute__ ((aligned (4)));
	uint32_t sum[128];
	uint8_t count;
	uint8_t naverage;
	bool outputflag;
	audio_block_t *inputQueueArray[1];
	arm_cfft_radix4_instance_q15 fft_inst;
};

*/
