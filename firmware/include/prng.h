#ifndef __PRNG_H_
#define __PRNG_H_

#include "stdint.h"

/*
 * Seeds the pseudo random number generator.
 *
 * Uses a part of the unique device ID to seed the generator.
 *
 */
void prng_seed();

/*
 * Samples uniform random variable.
 *
 * Uses a fast software pseudo random number generator to generate uniformly
 * distributed unsigned integers from the given range.
 *
 * @param min Minimum value of support
 * @param max Maximum value of support
 *
 * @returns Random unsigned integer from given range
 *
 */
uint32_t prng_urand(uint32_t min, uint32_t max);

/*
 *
 * Looks up distribution 'scale' from a table in NVM
 *
 * The exact shape of the optimized waiting distribution depends on the
 * charging times of the node. We precalculate optimized scales for different
 * waiting times and store them in a LUT.
 *
 * @param t_chr Current waiting time in flync ticks (10ms)
 *
 * @returns Scale parameter of optimized geometric distribution
 *
 */
float lookup_scale(unsigned int t_chr);

/*
 * Samples geometric distribution using inverse transform sampling
 *
 * ITF allows to sample an arbitrary distribution by evaluating the inverse
 * of the cdf of the distribution at a uniform random number.
 *
 * @param p shape parameter of geometric distribution
 *
 * @returns Random unsigned integer according to geometric distribution
 *
 */
unsigned int geometric_itf_sample(float p);

#endif /* __PRNG_H_ */