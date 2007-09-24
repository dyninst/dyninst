#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

struct struct26_1 {
    int field1;
    int field2;
};

struct struct26_2 {
    int field1;
    int field2;
    int field3[10];
    struct struct26_1 field4;
};

typedef struct {
    /* void *field27_11; */
    int field27_11;
    float field27_12;
} test1_27_type1;

typedef struct {
    /* void *field27_21; */
    int field27_21;
    float field27_22;
} test1_27_type2;

typedef struct {
    int field3[10];
    struct struct26_2 field4;
} test1_27_type3;

typedef struct {
    int field3[10];
    struct struct26_2 field4;
} test1_27_type4;

/* need this variables or some compilers (AIX xlc) will removed unused
   typedefs - jkh 10/13/99 */
test1_27_type1 dummy1;
test1_27_type2 dummy2;
test1_27_type3 dummy3;
test1_27_type4 dummy4;

int test1_27_globalVariable1 = 0;
/* Note for future reference: -Wl,-bgcbypass:3 is NECESSARY for
   compilation (gcc) on AIX. Damn efficient linkers. */
int test1_27_globalVariable5[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int test1_27_globalVariable6[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float test1_27_globalVariable7[10] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
				6.0, 7.0, 8.0, 9.0};
float test1_27_globalVariable8[12];

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/*
 * Test #27 - type compatibility
 */
int test1_27_mutatee() {
  int retval, passed;

#if !defined(sparc_sun_solaris2_4) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_nt4_0) \
 && !defined(ia64_unknown_linux2_4)

    logerror("Skipped test #27 (type compatibility)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname);
    retval = 0; /* Test "passed" */
#else
    passed = (test1_27_globalVariable1 == 1);

    if (passed) {
      logerror("Passed test #27 (type compatibility)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }
#endif
    return retval;
}
