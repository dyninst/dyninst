#ifndef ROSE_LinearCongruentialGenerator_H
#define ROSE_LinearCongruentialGenerator_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/** Linear congruential generator.  Generates a repeatable sequence of pseudo-random numbers. */
class LinearCongruentialGenerator {
public:
    /** Initialize the generator with a random seed. */
    LinearCongruentialGenerator() { init(); }

    /** Initialize the generator with a seed. The seed determines which sequence of numbers is returned. */
    LinearCongruentialGenerator(int seed) : seed_(seed), value_(seed) { }

    /** Random initialization. This uses /dev/urandom or /dev/random to initailize the sequence. */
    void init();

    /** Reset the sequence back to the first value. */
    void reset() { value_ = seed_; }

    /** Start a new sequence of random values. The seed identifies which sequence is returned. */
    void reseed(int seed) { value_ = seed_ = seed; }

    /** Return the seed for the current sequence. */
    int seed() const { return seed_; }

    /** Return the last returned value again. */
    uint64_t again() const { return value_; }

    /** Return the maximum possible value. */
    uint64_t max();

    /** Return the next value in the sequence.  If nbits is specified, then only the low-order bits are randomized and the high
     *  order bits are always cleared.  For instance, to get only positive values for casting to a 32-bit signed integer, use
     *  nbits=31.  The @p niter indicates the number of values that should be consumed, all of which are exclusive-ORed
     *  together to create the return value.  If @p niter is zero this function returns zero without consume any values.
     * @{ */
    uint64_t next(size_t nbits = 64, size_t niter = 1);

    uint64_t operator()() { return next(); }
    /** @} */

    /** Return a random boolean value. */
    bool flip_coin();

protected:
    int seed_;
    uint64_t value_;
};

typedef LinearCongruentialGenerator LCG;

#endif
