#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_26_call1();

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

/*  struct26_2 test1_26_globalVariable1;  */
struct struct26_2 test1_26_globalVariable1;
int test1_26_globalVariable2 = 26000000;
int test1_26_globalVariable3 = 26000000;
int test1_26_globalVariable4 = 26000000;
int test1_26_globalVariable5 = 26000000;
int test1_26_globalVariable6 = 26000000;
int test1_26_globalVariable7 = 26000000;

int test1_26_globalVariable8 = 26000000;
int test1_26_globalVariable9 = 26000000;
int test1_26_globalVariable10 = 26000000;
int test1_26_globalVariable11 = 26000000;
int test1_26_globalVariable12 = 26000000;
int test1_26_globalVariable13 = 26000000;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void verifyScalarValue26(const char *name, int a, int value);
static void call26_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int failed_test = FALSE;

/* Function definitions follow */

/*
 * Test #26 - field operators
 */

int test1_26_mutatee() {
  int retval;
#if !defined(sparc_sun_solaris2_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_nt4_0) \
 && !defined(ia64_unknown_linux2_4)

    logerror("Skipped test #26 (struct elements)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname); /* Well, it didn't fail.. */
    retval = 0;
#else
    int i;

    test1_26_globalVariable1.field1 = 26001001;
    test1_26_globalVariable1.field2 = 26001002;
    for (i=0; i < 10; i++) test1_26_globalVariable1.field3[i] = 26001003 + i;
    test1_26_globalVariable1.field4.field1 = 26000013;
    test1_26_globalVariable1.field4.field2 = 26000014;

    test1_26_call1();

    verifyScalarValue26("test1_26_globalVariable2", test1_26_globalVariable2, 26001001);
    verifyScalarValue26("test1_26_globalVariable3", test1_26_globalVariable3, 26001002);
    verifyScalarValue26("test1_26_globalVariable4", test1_26_globalVariable4, 26001003);
    verifyScalarValue26("test1_26_globalVariable5", test1_26_globalVariable5, 26001003+5);
    verifyScalarValue26("test1_26_globalVariable6", test1_26_globalVariable6, 26000013);
    verifyScalarValue26("test1_26_globalVariable7", test1_26_globalVariable7, 26000014);

    /* local variables */
    verifyScalarValue26("test1_26_globalVariable8", test1_26_globalVariable8, 26002001);
    verifyScalarValue26("test1_26_globalVariable9", test1_26_globalVariable9, 26002002);
    verifyScalarValue26("test1_26_globalVariable10", test1_26_globalVariable10, 26002003);
    verifyScalarValue26("test1_26_globalVariable11", test1_26_globalVariable11, 26002003+5);
    verifyScalarValue26("test1_26_globalVariable12", test1_26_globalVariable12, 26002013);
    verifyScalarValue26("test1_26_globalVariable13", test1_26_globalVariable13, 26002014);

    if (!failed_test) {
      logerror("Passed test #26 (field operators)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }
#endif
    return retval;
}

void verifyScalarValue26(const char *name, int a, int value)
{
  if (!verifyScalarValue(name, a, value, "test1_26", "field operators")) {
    failed_test = TRUE;
  }
}

void call26_2()
{
}

void test1_26_call1()
{
    int i;
    /*    struct26_2 localVariable26_1;  */
    struct struct26_2 localVariable26_1;

    localVariable26_1.field1 = 26002001;
    localVariable26_1.field2 = 26002002;
    for (i=0; i < 10; i++) localVariable26_1.field3[i] = 26002003 + i;
    localVariable26_1.field4.field1 = 26002013;
    localVariable26_1.field4.field2 = 26002014;

    /* check local variables at this point (since we known locals are still
       on the stack here. */
    call26_2();

}
