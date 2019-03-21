
// Macros to make annotating the parallel parts of the code simple and easy.
// Activate with ENABLE_VG_ANNOTATIONS

#ifdef ENABLE_VG_ANNOTATIONS
#include <valgrind/helgrind.h>
#include <valgrind/drd.h>

// Function-static variables are a pain to mark properly. Usage:
//   static int x = foo();
// becomes:
//   int x = STATICIFY(foo());
#define STATICIFY(EXPR) ([]() -> decltype(EXPR)& { \
    bool mine = false; \
    static auto value = [&mine](){ \
        mine = true; \
        return EXPR; \
    }(); \
    if(mine) ANNOTATE_HAPPENS_BEFORE(&value); \
    else ANNOTATE_HAPPENS_AFTER(&value); \
    return value; \
}())

#else

#define ANNOTATE_HAPPENS_BEFORE(X)
#define ANNOTATE_HAPPENS_AFTER(X)

// Simplifed form for when Valgrind isn't looking.
#define STATICIFY(EXPR) ([]() -> decltype(EXPR)& { \
    static auto value = EXPR; \
    return value; \
}())

#endif
