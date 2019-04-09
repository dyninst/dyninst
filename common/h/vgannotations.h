
// Macros to make annotating the parallel parts of the code simple and easy.
// Activate with ENABLE_VG_ANNOTATIONS

#ifdef ENABLE_VG_ANNOTATIONS
#include <valgrind/helgrind.h>
#include <valgrind/drd.h>

// Annotations for libc's inlined synchronization (for locales, mostly)
#define _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(addr) ANNOTATE_HAPPENS_BEFORE(addr)
#define _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(addr)  ANNOTATE_HAPPENS_AFTER(addr)

// Function-scope static variables are a pain to mark properly. Usage:
//   static int x = foo();
// becomes:
//   int x = STATICIFY(foo());
// This macro assumes the type is MoveConstructable, probably.
#define LAZY_ONCE(EXPR) *({ \
    static struct _helper { \
        _helper(decltype(EXPR)& v) : _v(v) { \
            ANNOTATE_HAPPENS_BEFORE(&_v); \
        } \
        _helper(decltype(EXPR) v) : _v(v) { \
            ANNOTATE_HAPPENS_BEFORE(&_v); \
        } \
        ~_helper() {} \
        decltype(EXPR) _v; \
    } _help(EXPR); \
    ANNOTATE_HAPPENS_AFTER(&_help._v); \
    &_help._v; \
})

#else

#define ANNOTATE_HAPPENS_BEFORE(X)
#define ANNOTATE_HAPPENS_AFTER(X)

// Simplifed form for when Valgrind isn't looking.
#define LAZY_ONCE(EXPR) *({ \
    static auto value = EXPR; \
    &value; \
})

#endif
