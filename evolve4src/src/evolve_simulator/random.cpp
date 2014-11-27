/***********************************************************************
 * sim_random.cpp:
 *
 * An improved random number generation package.  In addition to the standard
 * rand()/srand() like interface, this package also has a special state info
 * interface.  The initstate() routine is called with a seed, an array of
 * bytes, and a count of how many bytes are being passed in; this array is
 * then initialized to contain information for random number generation with
 * that much state information.  Good sizes for the amount of state
 * information are 32, 64, 128, and 256 bytes.  The state can be switched by
 * calling the setstate() routine with the same array as was initiallized
 * with initstate().  By default, the package runs with 128 bytes of state
 * information and generates far better random numbers than a linear
 * congruential generator.  If the amount of state information is less than
 * 32 bytes, a simple linear congruential R.N.G. is used.
 *
 * Internally, the state information is treated as an array of longs; the
 * zeroeth element of the array is the type of R.N.G. being used (small
 * integer); the remainder of the array is the state information for the
 * R.N.G.  Thus, 32 bytes of state information will give 7 longs worth of
 * state information, which will allow a degree seven polynomial.  (Note:
 * the zeroeth word of state information also has some other information
 * stored in it -- see setstate() for details).
 *
 * The random number generation technique is a linear feedback shift register
 * approach, employing trinomials (since there are fewer terms to sum up that
 * way).  In this approach, the least significant bit of all the numbers in
 * the state table will act as a linear feedback shift register, and will
 * have period 2^deg - 1 (where deg is the degree of the polynomial being
 * used, assuming that the polynomial is irreducible and primitive).  The
 * higher order bits will have longer periods, since their values are also
 * influenced by pseudo-random carries out of the lower bits.  The total
 * period of the generator is approximately deg*(2**deg - 1); thus doubling
 * the amount of state information has a vast influence on the period of the
 * generator.  Note: the deg*(2**deg - 1) is an approximation only good for
 * large deg, when the period of the shift register is the dominant factor.
 * With deg equal to seven, the period is actually much longer than the
 * 7*(2**7 - 1) predicted by this formula.
 *
 */
#include "stdafx.h"

/*
 * Compute x = (7^5 * x) mod (2^31 - 1)
 * wihout overflowing 31 bits:
 *      (2^31 - 1) = 127773 * (7^5) + 2836
 * From "Random number generators: good ones are hard to find",
 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
 * October 1988, p. 1195.
 */
static inline long good_rand(register long x)
{
	register long hi, lo;

	hi = x / 127773;
	lo = x % 127773;
	x = 16807 * lo - 2836 * hi;
	if (x <= 0)
		x += 0x7fffffff;
	return (x);
}

/*
 * seed:
 *
 * Initialize the random number generator based on the given seed.
 * Initializes state[] based on the given "seed" via a linear congruential
 * generator.  Then, the indexes are set to known locations that are exactly
 * EVOLVE_SEP4 places apart.  Lastly, it cycles the state information a given
 * number of times to get rid of any initial dependencies
 * introduced by the L.C.R.N.G.
 *
 */
EVOLVE_RANDOM *sim_random_make(unsigned long seed)
{
	EVOLVE_RANDOM *er;
	register long i;

	er = (EVOLVE_RANDOM*) CALLOC(1, sizeof(EVOLVE_RANDOM));
	ASSERT( er != NULL );

	er->state[0] = seed;
	for(i = 1; i < EVOLVE_DEG4; i++) {
		er->state[i] = good_rand( er->state[i-1] );
	}

	er->fidx = EVOLVE_SEP4;
	er->ridx = 0;

	for(i = 0; i < 10 * EVOLVE_DEG4; i++) {
		(void)sim_random(er);
	}

	return er;
}

void sim_random_delete(EVOLVE_RANDOM *er)
{
	ASSERT( er != NULL );

	FREE( er );
}

/*
 * random:
 *
 * The basic operation is to add the number at the rear index
 * into the one at the front index.  Then both indexes are advanced to
 * the next location cyclically in the table.  The value returned is the sum
 * generated, reduced to 31 bits by throwing away the "least random" low bit.
 *
 * Note: the code takes advantage of the fact that both the front and
 * rear indexes can't wrap on the same call by not testing the rear
 * index if the front one has wrapped.
 *
 * Returns a 31-bit random number.
 */
long sim_random(EVOLVE_RANDOM *er)
{
	register long i;
	register int f, r;

	f = er->fidx;
	r = er->ridx;

	er->state[f] += er->state[r];
	i = (er->state[f] >> 1) & 0x7fffffff;	/* chucking least random bit */

	if( ++f >= EVOLVE_DEG4 ) {
		f = 0;
		r += 1;
	} else if( ++r >= EVOLVE_DEG4 ) {
		r = 0;
	}

	er->fidx = f;
	er->ridx = r;

	return i;
}

