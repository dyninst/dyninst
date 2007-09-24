#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4)

#ifndef Fortran
#ifdef __cplusplus
extern "C" int test1_35_call1();
#else
extern int test1_35_call1();
#endif
#endif
#endif

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test1_35_mutatee() {
#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4)  

#if !defined Fortran

    int value;
    value = test1_35_call1();

    if( value != 35 )
    {
      logerror( "**Failed** test #35 (function relocation)\n" );

      logerror( "    total = %d, should be 35.\n", value );

      switch(value)
      {
        case 1: logerror( "    Entry instrumentation did not work.\n" ); break;
        case 2: logerror( "    Exit instrumentation did not work.\n" ); break;
        default: logerror("    Take a look at the call to call35_2.\n" ); break;
      }
      return -1; /* Test failed */
    }

    test_passes(testname);
    logerror( "Passed test #35 (function relocation)\n" );
    return 0; /* Test passed */
#endif
#else
    test_passes(testname);
    logerror( "Skipped test #35 (function relocation)\n" );
#if defined(i386_unknown_nt4_0)
    logerror( "\t- test not implemented for this platform\n" );
#else
#if defined(ia64_unknown_linux2_4)
    logerror( "\t- not applicable to this platform.\n" );
#else
    logerror( "\t- not implemented on this platform\n" );
#endif
#endif
    return 0; /* Test "passed" */
#endif
}
