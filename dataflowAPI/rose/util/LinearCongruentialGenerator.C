#define __STDC_LIMIT_MACROS

#include "LinearCongruentialGenerator.h"
#include "../integerOps.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#ifndef _MSC_VER

void
LinearCongruentialGenerator::init() {
    if (0 == access("/dev/urandom", R_OK)) {                      // try non-blocking version first
        int fd = open("/dev/urandom", O_RDONLY);
        assert(fd >= 0);
        ssize_t nread __attribute__((unused)) = read(fd, &seed_, sizeof seed_);
        assert(nread == sizeof seed_);
        value_ = seed_;
        close(fd);
    } else if (0 == access("/dev/random", R_OK)) {                // this one might block for a while
        int fd = open("/dev/random", O_RDONLY);
        assert(fd >= 0);
        ssize_t nread __attribute__((unused)) = read(fd, &seed_, sizeof seed_);
        assert(nread == sizeof seed_);
        value_ = seed_;
        close(fd);
    } else {
        // We don't know if srand() has been called yet, so we must assume that it hasn't.
        struct timeval tv;
        int status __attribute__((unused)) = gettimeofday(&tv, NULL);
        assert(status >= 0);
        srand(tv.tv_sec ^ tv.tv_usec); // tv_sec is too slow, tv_usec might not be too discrete
        seed_ = value_ = rand();
    }
}

#else
void
LinearCongruentialGenerator::init()
{
}
#endif

uint64_t
LinearCongruentialGenerator::next(size_t nbits, size_t niter) {
#if 0
    uint64_t retval = 0;
    for (size_t i=0; i<niter; ++i) {
        // These are the values used by MMIX written by Donald Knuth. All 64 bits are returned.
        value_ = 6364136223846793005ull * value_ + 1442695040888963407ull;
        retval ^= value_;
    }
#else
    // multiplier and addend are from java.util.Random and we avoid the low-order bits
    uint64_t retval = 0;
    static const uint64_t a_lo = 0xece66d;              // low-order 24 bits, done this way for 32-bit machines
    static const uint64_t a_hi = 0x5de;                 // high-order bits
    static const uint64_t c = 11;
    for (size_t i = 0; i < niter; ++i) {
        value_ = ((a_hi << 24) | a_lo) * value_ + c;
        uint64_t v = (value_ >> 17) & 0x3fffff; // 22 bits: 17 (inclusive) through 39 (exclusive)
        value_ = ((a_hi << 24) | a_lo) * value_ + c;
        v |= ((value_ >> 18) & 0x3fffff) << 22; // 22 bits: 18 (inclusive) through 40 (exclusive)
        value_ = ((a_hi << 24) | a_lo) * value_ + c;
        v |= ((value_ >> 19) & 0x0fffff) << 44; // 20 bits: 19 (inclusive) through 39 (exclusive)
        retval ^= v;
    }
#endif

    return retval & IntegerOps::genMask<uint64_t>(nbits);
}

#ifndef _MSC_VER

uint64_t
LinearCongruentialGenerator::max() {
    // all 64 bits are returned by next()
    return UINT64_MAX;
}

bool
LinearCongruentialGenerator::flip_coin() {
    return 0 == next(1);
}

#endif
