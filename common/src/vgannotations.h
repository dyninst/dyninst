
// Macros to make annotating the parallel parts of the code simple and easy.
// Activate with ENABLE_VG_ANNOTATIONS

#ifdef ENABLE_VG_ANNOTATIONS
#include <valgrind/helgrind.h>
#include <valgrind/drd.h>

// Annotations for libc's inlined synchronization (for locales, mostly)
#define _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(addr) ANNOTATE_HAPPENS_BEFORE(addr)
#define _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(addr)  ANNOTATE_HAPPENS_AFTER(addr)

// One or two places use function-scoped static variables for lazy singleton
// initialization. Unfortunately its a pain to mark up properly, so the following
// is a class with the same effect, but with the potential for marking.

// Caveats:
// - The value will be default-constructed with all other global statics, so for
//   efficiency use a type that has a small default constructor.
// - The value will be assigned to as part of the lazy initialization, so the
//   logic for assignment and construction needs to be equivalent.
// - Same as call_once, the given functions should be identical, otherwise weird
//   nondeterminism may occur.
// - Currently the function cannot take any arguments. Someone with more C++
//   background can fix that later if they like.

#include <mutex>
#include <functional>

template<typename T> class LazySingleton {
    T value;
    std::once_flag flag;

public:
    typedef T type;

    LazySingleton() {};
    ~LazySingleton() {};

    T& get(std::function<T()> f) {
        std::call_once(flag, [&]{
            value = f();
            ANNOTATE_HAPPENS_BEFORE(&flag);
        });
        ANNOTATE_HAPPENS_AFTER(&flag);
        return value;
    }
};

#else

#define ANNOTATE_HAPPENS_BEFORE(X)
#define ANNOTATE_HAPPENS_AFTER(X)

// Simplified form for when Valgrind isn't looking.

#include <functional>

template<typename T> class LazySingleton {
public:
    typedef T type;

    LazySingleton() {};
    ~LazySingleton() {};

    T& get(std::function<T()> f) {
        static T value = f();
        return value;
    }
};

#endif
