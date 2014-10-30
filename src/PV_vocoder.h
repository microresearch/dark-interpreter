
typedef struct
{
  uint16_t output[128] __attribute__ ((aligned (4)));
  int16_t buffer[512] __attribute__ ((aligned (4)));
  uint32_t sum[128];
  const arm_cfft_instance_q15 fft_inst;
} PV;

void pvvocprocess(PV *unit, int16_t* inbuffer, int16_t* outbuffer);
