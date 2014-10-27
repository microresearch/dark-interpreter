#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "simulation.h"
#include "vocode.h"

const LADSPA_Data decay_table[] =
{
  1/100.0,
  1/100.0, 1/100.0, 1/100.0,
  1/125.0, 1/125.0, 1/125.0,
  1/166.0, 1/166.0, 1/166.0,
  1/200.0, 1/200.0, 1/200.0,
  1/250.0, 1/250.0, 1/250.0
};

/* useful macros */
#undef CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/*****************************************************************************/

VocoderInstance* instantiateVocoder(void) {
  VocoderInstance* vocoder = (VocoderInstance *)malloc(sizeof(VocoderInstance));

  //  if (vocoder == NULL)
  //    return NULL;

  vocoder->SampleRate = 48000;
  vocoder->num_bands = -1;
  vocoder->ctrlBandCount=16;
  vocoder->mainvol = 1.0 * AMPLIFIER;

  for (u8 x=0;x<MAX_BANDS;x++){
    vocoder->ctrlBandLevels[x]=1.0f;
    vocoder->bands_out[x].oldval = 0.0f;
  }

  return vocoder;
}

/*****************************************************************************/

/*****************************************************************************/

// vocoder_do_bandpasses /*fold00*/
void vocoder_do_bandpasses(struct bandpass *bands, LADSPA_Data sample,
			   VocoderInstance *vocoder)
{
  u8 i;
  for (i=0; i < vocoder->num_bands; i++)
    {
      bands[i].high1 = sample - bands[i].f * bands[i].mid1 - bands[i].low1;
      bands[i].mid1 += bands[i].high1 * bands[i].c;
      bands[i].low1 += bands[i].mid1;

      bands[i].high2 = bands[i].low1 - bands[i].f * bands[i].mid2
	- bands[i].low2;
      bands[i].mid2 += bands[i].high2 * bands[i].c;
      bands[i].low2 += bands[i].mid2;
      bands[i].y = bands[i].high2 * bands[i].att;
    }
}

/* Run a vocoder instance for a block of SampleCount samples. */

// TODO: how to swap round channels/bands

void runVocoder(VocoderInstance *vocoder, float *formant, float *carrier, float *out, unsigned int SampleCount)
{
  u8 i, j, numbands;
  float a;
  LADSPA_Data x, c;

  vocoder->portFormant=formant;
  vocoder->portCarrier=carrier;
  vocoder->portOutput=out;

  numbands = vocoder->ctrlBandCount;
  if (numbands < 1 || numbands > MAX_BANDS) numbands = MAX_BANDS;

  /* initialize bandpass information if num_bands control has changed,
     or on first run */
  if (vocoder->num_bands != numbands)
    {
      vocoder->num_bands = numbands;

      for(i=0; i < numbands; i++)
	{
	  memset(&vocoder->bands_formant[i], 0, sizeof(struct bandpass));

	  a = 16.0 * i/(float)numbands;  // stretch existing bands

	  if (a < 4.0)
	    vocoder->bands_formant[i].freq = 150 + 420 * a / 4.0;
	  else
	    vocoder->bands_formant[i].freq = 600 * powf (1.23, a - 4.0);

	  c = vocoder->bands_formant[i].freq * 2 * M_PI / vocoder->SampleRate;
	  vocoder->bands_formant[i].c = c * c;

	  vocoder->bands_formant[i].f = 0.4/c;
	  vocoder->bands_formant[i].att =
	    1/(6.0 + ((expf (vocoder->bands_formant[i].freq
			    / vocoder->SampleRate) - 1) * 10));

	  memcpy(&vocoder->bands_carrier[i],
		 &vocoder->bands_formant[i], sizeof(struct bandpass));

	  vocoder->bands_out[i].decay = decay_table[(int)a];
	  vocoder->bands_out[i].level =
	    CLAMP (vocoder->ctrlBandLevels[i], 0.0, 1.0);
	}
    }
  else		       /* get current values of band level controls */
    {
      for (i = 0; i < numbands; i++)
	vocoder->bands_out[i].level = CLAMP (vocoder->ctrlBandLevels[i],
					     0.0, 1.0);
    }

  ///////////////////////////////////
  for (i=0; i < SampleCount; i++)
    {
      vocoder_do_bandpasses (vocoder->bands_carrier,
			     vocoder->portCarrier[i], vocoder);
      vocoder_do_bandpasses (vocoder->bands_formant,
			     vocoder->portFormant[i], vocoder);

      vocoder->portOutput[i] = 0.0;
      for (j=0; j < numbands; j++)
	{
	  vocoder->bands_out[j].oldval = vocoder->bands_out[j].oldval
	    + (fabsf (vocoder->bands_formant[j].y)
	       - vocoder->bands_out[j].oldval)
	    * vocoder->bands_out[j].decay;
	  x = vocoder->bands_carrier[j].y * vocoder->bands_out[j].oldval;
	  vocoder->portOutput[i] += x * vocoder->bands_out[j].level;
	}
      vocoder->portOutput[i] *= vocoder->mainvol;
    }
}
