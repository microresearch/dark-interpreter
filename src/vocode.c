// first test dsplib

#ifndef PCSIM
#include "arm_math.h"
#include "arm_const_structs.h"
#endif

#define TEST_LENGTH_SAMPLES 2048

static float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES];
static float32_t testOutput[TEST_LENGTH_SAMPLES/2];

/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */
uint32_t fftSize = 1024;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

/* Reference index at which max energy of bin ocuurs */
uint32_t refIndex = 213, testIndex = 0;


int32_t testvocode(void)
{

  float32_t maxValue;

  /* Process the data through the CFFT/CIFFT module */
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, testInput_f32_10khz, ifftFlag, doBitReverse);

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
    arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);

  /* Calculates maxValue and returns corresponding BIN value */
    arm_max_f32(testOutput, fftSize, &maxValue, &testIndex);

  //////////////////////////////////////
  //****** this is from older code in CMSIS/FFTEX

  uint32_t fftSize = 1024; 
  uint32_t ifftFlag = 0; 
  uint32_t doBitReverse = 1; 
  arm_status status; 
  arm_cfft_radix4_instance_f32 S; 


  //  sqrtf(0.02f);
	 
  /* Initialize the CFFT/CIFFT module */  
  //  status = arm_cfft_radix4_init_f32(&S, fftSize, ifftFlag, doBitReverse); 
}


