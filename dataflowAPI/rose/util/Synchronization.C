// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#include "Synchronization.h"

#include <boost/random/uniform_smallint.hpp>
#if BOOST_VERSION >= 104700
    #include <boost/random/mersenne_twister.hpp>
    #define SAWYER_PRN_GENERATOR boost::random::mt11213b
    #define SAWYER_UNIFORM_SIZE_T boost::random::uniform_smallint<size_t>
#else
    // Boost 1.45 and 1.46 say that mt11213b is only 44% as fast as rand48. Also, these things were not part of the
    // boost::random namespace in those versions.
    #include <boost/random/linear_congruential.hpp>         // 64% as fast as mersenne_twister according to boost 1.59
    #define SAWYER_PRN_GENERATOR boost::rand48
    #define SAWYER_UNIFORM_SIZE_T boost::uniform_smallint<size_t>
#endif

namespace Sawyer {

// In order to get thread safety, you probably need to add "-pthread" to all GCC compile and link lines.  It is not sufficient
// to only add "-lpthreads" to the link lines.
#if SAWYER_MULTI_THREADED == 0
# ifdef _MSC_VER
#  pragma message("Sawyer multi-threading is disabled; even documented thread-safe functions are unsafe!")
# else
#  warning "Sawyer multi-threading is disabled; even documented thread-safe functions are unsafe!"
# endif
#endif

static SAWYER_THREAD_TRAITS::RecursiveMutex bigMutex_;

// thread-safe
SAWYER_EXPORT SAWYER_THREAD_TRAITS::RecursiveMutex&
bigMutex() {
    return bigMutex_;
}

// thread-safe
SAWYER_EXPORT size_t
fastRandomIndex(size_t n) {
    assert(n > 0);

    // FIXME[Robb Matzke 2015-12-08]: This leaks memory when threads that use this function are destroyed, but the common
    // denominator across compilers is that thread-local storage can only be applied to POD types.
    static SAWYER_THREAD_LOCAL SAWYER_PRN_GENERATOR *generator = NULL;
    if (!generator)
        generator = new SAWYER_PRN_GENERATOR;

    SAWYER_UNIFORM_SIZE_T distributor(0, n-1);
    return distributor(*generator);
}

} // namespace
