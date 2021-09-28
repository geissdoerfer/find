#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include <float.h>
#include <math.h>

#include "peripherals.h"

static uint32_t prng_x;

/* This symbol is defined in the object file generated from the binary LUT */
extern volatile unsigned char _binary__build_opt_scale_bin_start;

void prng_seed(uint32_t seed) { prng_x = NRF_FICR->DEVICEID[0]; }

__attribute__((long_call, section(".ramfunctions"))) uint32_t
prng_urand(uint32_t min, uint32_t max) {
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123;
  uint32_t t;
  t = prng_x ^ (prng_x << 11);
  prng_x = y;
  y = z;
  z = w;
  w = w ^ (w >> 19) ^ (t ^ (t >> 8));
  return w % (max - min) + min;
}

/* For some reason, this function fails when optimized */
__attribute__((optimize("O0")))
__attribute__((long_call, section(".ramfunctions"))) float
lookup_scale(unsigned int t_chr) {
  float *scale_tab = (float *)&_binary__build_opt_scale_bin_start;

  if (t_chr < 10)
    return scale_tab[0];
  else if (t_chr > 2560)
    return scale_tab[255];

  volatile int idx_low = (t_chr / 10) - 1;
  volatile float val_low = scale_tab[idx_low];
  volatile float val_high = scale_tab[t_chr / 10];
  float frac = (float)(t_chr % 10) / 10.0f;
  return val_low + frac * (val_high - val_low);
}

__attribute__((long_call, section(".ramfunctions"))) unsigned int
geometric_itf_sample(float p) {
  float y = (float)prng_urand(0, 4096) / 4096;
  unsigned int res = (unsigned int)(logf(1 - y) / logf(1 - p) - 1.0f);
  return res;
}