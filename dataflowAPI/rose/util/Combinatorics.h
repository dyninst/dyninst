#ifndef ROSE_Combinatorics_H
#define ROSE_Combinatorics_H

#include "LinearCongruentialGenerator.h"

#include <stddef.h>
#include <algorithm>
#include <cassert>
#include <list>
#include <stdint.h>
#include <string>
#include <vector>

namespace Combinatorics {

/** Returns the factorial of @p n. */
    template<typename T>
    static T
    factorial(T n) {
        T retval = 1;
        while (n > 1) {
            T next = retval * n--;
            assert(next > retval); // overflow
            retval = next;
        }
        return retval;
    }

/** Simulate flipping a coin. Randomly returns true or false with equal probability. See also,
 *  LinearCongruentialGenerator::flip_coin(). */
    bool flip_coin();

/** Permute a vector according to the specified permutation number. The permutation number should be between zero (inclusive)
 *  and the factorial of the values size (exclusive).  A permutation number of zero is a no-op; higher permutation numbers
 *  shuffle the values in repeatable ways.  Using swap rather that erase/insert is much faster than the standard Lehmer codes,
 *  but doesn't return permutations in lexicographic order.  This function can perform approx 9.6 million permutations per
 *  second on a vector of 12 64-bit integers on Robb's machine (computing all 12! permutations in about 50 seconds). */
    template<typename T>
    static void
    permute(std::vector <T> &values/*in,out*/, uint64_t pn, size_t sz = (size_t)(-1)) {
        if ((size_t)(-1) == sz)
            sz = values.size();
        assert(sz <= values.size());
        assert(pn < factorial(sz));
        for (size_t i = 0; i < sz; ++i) {
            uint64_t radix = sz - i;
            uint64_t idx = pn % radix;
            std::swap(values[i + idx], values[i]);
            pn /= radix;
        }
    }

/** Shuffle the values of a vector.  If @p nitems is supplied then only the first @p nitems of the vector are shuffled. If
 *  @p limit is specified then the algorithm returns after at least the first @p limit elements are sufficiently shuffled. If
 *  an @p lcg is specified, then it will be used to generate the random numbers, otherwise a built-in random number generator
 *  is used. */
    template<typename T>
    void
    shuffle(std::vector <T> &vector, size_t nitems = (size_t)(-1), size_t limit = (size_t)(-1),
            LinearCongruentialGenerator *lcg = NULL) {
        static LinearCongruentialGenerator my_lcg;
        if (!lcg)
            lcg = &my_lcg;
        nitems = std::min(nitems, vector.size());
        limit = std::min(limit, nitems);

        for (size_t i = 0; i < limit; ++i)
            std::swap(vector[i], vector[lcg->next() % nitems]);
    }

/** Compute a SHA1 digest.  The returned vector will contain 20 bytes and can be converted to a string of 40 hexadecimal
 *  characters via digest_to_string().  If called when a SHA1 algorithm is not available (due to ROSE configuration) an
 *  empty vector is returned.
 * @{ */
    std::vector <uint8_t> sha1_digest(const uint8_t *data, size_t size);

    std::vector <uint8_t> sha1_digest(const std::vector <uint8_t> &data);

    std::vector <uint8_t> sha1_digest(const std::string &data);
/** @} */

/** Compute the Fowler–Noll–Vo fast string hash.  This is not a cryptographic hash. Speed is marginally slower than Murmur
 *  hash, but collision rate is slightly less.
 * @{ */
    uint64_t fnv1a64_digest(const uint8_t *data, size_t size);

    uint64_t fnv1a64_digest(const std::vector <uint8_t> &data);

    uint64_t fnv1a64_digest(const std::string &data);
/** @} */


/** Converts a binary digest to a string of hexadecimal characters.  The input can actually be any type of data and any
 *  length. The output will be twice as long as the input.  If you're using this to convert binary data to a printable format
 *  you're doing it wrong--use StringUtility::encode_base64() instead.
 * @{ */
    std::string digest_to_string(const uint8_t *data, size_t size);

    std::string digest_to_string(const std::vector <uint8_t> &digest);

    std::string digest_to_string(const std::string &data);
/** @} */

} // namespace
#endif
