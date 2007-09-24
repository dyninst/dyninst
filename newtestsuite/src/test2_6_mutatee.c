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

/* TODO stop_process() should probably be moved into mutatee_util.h */
/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
void stop_process()
{
#ifdef i386_unknown_nt4_0
    DebugBreak();
#else

#ifdef DETACH_ON_THE_FLY
    dotf_stop_process();
    return;
#endif

#if !defined(bug_irix_broken_sigstop)
    kill(getpid(), SIGSTOP);
#else
    kill(getpid(), SIGEMT);
#endif

#endif
}

int test2_6_mutatee() {
  func1();
  stop_process();
  test_passes("test2_6");
  return 0; /* No error */
}

void func1() {
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(mips_sgi_irix6_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(ia64_unknown_linux2_4)

    void *ref;
    /* now use the dlopen interface to force an object to load. */
#if defined(alpha_dec_osf4_0)
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW);
#else
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW | RTLD_GLOBAL);
#endif

    if (!ref) {
	logerror("%s[%d]: %s\n", __FILE__, __LINE__, dlerror() );
    }
#endif
}
