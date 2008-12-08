#include <signal.h>
#include <dlfcn.h>

#include "mutatee_util.h"
#include "test2.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

void func1();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* TODO stop_process() should probably be moved into mutatee_util */
/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
void stop_process()
{
#ifdef os_windows_test
    DebugBreak();
#else
    kill(getpid(), SIGSTOP);
#endif
}

int test2_6_mutatee() {
  func1();
  stop_process();
  test_passes("test2_6");
  return 0; /* No error */
}

void func1() {
/* #if defined(sparc_sun_solaris2_4_test) \ */
/*  || defined(i386_unknown_solaris2_5_test) \ */
/*  || defined(i386_unknown_linux2_0_test) \ */
/*  || defined(x86_64_unknown_linux2_4_test) /\* Blind duplication - Ray *\/ \ */
/*  || defined(mips_sgi_irix6_4_test) \ */
/*  || defined(alpha_dec_osf4_0_test) \ */
/*  || defined(rs6000_ibm_aix4_1_test) \ */
/*  || defined(ia64_unknown_linux2_4_test) */

    void *ref;
    /* now use the dlopen interface to force an object to load. */
#if defined(alpha_dec_osf4_0_test)
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW);
#else
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW | RTLD_GLOBAL);
#endif

    if (!ref) {
	logerror("%s[%d]: %s\n", __FILE__, __LINE__, dlerror() );
    }
/* #endif */
}
