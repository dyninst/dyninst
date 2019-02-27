
// Macros to make annotating the parallel parts of the code simple and easy.
// Activate with ENABLE_VG_ANNOTATIONS

#ifdef ENABLE_VG_ANNOTATIONS
#include <valgrind/helgrind.h>
#include <valgrind/drd.h>

// Helgrind and DRD don't exactly agree on how to markup normal mutexes.
// These macros lean towards Helgrind's semantics while involving DRD
#define ANNOTATE_MUTEX_CREATE(_L, _rec) VALGRIND_HG_MUTEX_INIT_POST(_L, _rec)
#define ANNOTATE_MUTEX_LOCK_PRE(_L, _try) VALGRIND_HG_MUTEX_LOCK_PRE(_L, _try)
#define ANNOTATE_MUTEX_LOCK_POST(_L) \
    do { \
        VALGRIND_HG_MUTEX_LOCK_POST(_L); \
    } while(0)
        //VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__DRD_ANNOTATE_RWLOCK_ACQUIRED, 
        //                                _L, 1, 0, 0, 0); 
#define ANNOTATE_MUTEX_UNLOCK_PRE(_L) VALGRIND_HG_MUTEX_UNLOCK_PRE(_L)
#define ANNOTATE_MUTEX_UNLOCK_POST(_L) \
    do { \
        VALGRIND_HG_MUTEX_UNLOCK_PRE(_L); \
    } while(0)
        //VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__DRD_ANNOTATE_RWLOCK_RELEASED, 
         //                               _L, 1, 0, 0, 0); 

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
#define ANNOTATE_MUTEX_CREATE(X, Y)
#define ANNOTATE_MUTEX_LOCK_PRE(X, Y)
#define ANNOTATE_MUTEX_LOCK_POST(X)
#define ANNOTATE_MUTEX_UNLOCK_PRE(X)
#define ANNOTATE_MUTEX_UNLOCK_POST(X)

// Simplifed form for when Valgrind isn't looking.
#define STATICIFY(EXPR) ([]() -> decltype(EXPR)& { \
    static auto value = EXPR; \
    return value; \
}())

#endif
