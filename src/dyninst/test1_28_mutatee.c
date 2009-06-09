#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_28_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

struct test1_28_struct1 {
    int field1;
    int field2;
};

struct test1_28_struct2 {
    int field1;
    int field2;
    int field3[10];
    struct test1_28_struct1 field4;
};

char test1_28_globalVariable1[sizeof(struct test1_28_struct2)];
int test1_28_globalVariable2 = 28000000;
int test1_28_globalVariable3 = 28000000;
int test1_28_globalVariable4 = 28000000;
int test1_28_globalVariable5 = 28000000;
int test1_28_globalVariable6 = 28000000;
int test1_28_globalVariable7 = 28000000;
int test1_28_globalVariable8 = 28000000;	

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void verifyScalarValue28(const char *name, int a, int value);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int test_failed = FALSE;

/* Function definitions follow */

/*
 * Test #28 - field operators
 */
struct test1_28_struct2 *temp;

int test1_28_mutatee() {
    int retval;
    int i;

    temp = (struct test1_28_struct2 *) test1_28_globalVariable1;

    temp->field1 = 28001001;
    temp->field2 = 28001002;
    for (i=0; i < 10; i++) temp->field3[i] = 28001003 + i;
    temp->field4.field1 = 28000013;
    temp->field4.field2 = 28000014;

    test1_28_call1();

    verifyScalarValue28("test1_28_globalVariable2", test1_28_globalVariable2, 28001001);
    verifyScalarValue28("test1_28_globalVariable3", test1_28_globalVariable3, 28001002);
    verifyScalarValue28("test1_28_globalVariable4", test1_28_globalVariable4, 28001003);
    verifyScalarValue28("test1_28_globalVariable5", test1_28_globalVariable5, 28001003+5);
    verifyScalarValue28("test1_28_globalVariable6", test1_28_globalVariable6, 28000013);
    verifyScalarValue28("test1_28_globalVariable7", test1_28_globalVariable7, 28000014);

    if (!test_failed) {
      logerror("Passed test #28 (user defined fields)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }
    return retval;
}

void verifyScalarValue28(const char *name, int a, int value)
{
  if (!verifyScalarValue(name, a, value, "test1_28", "user defined fields")) {
    test_failed = TRUE;
  }
}

void test1_28_call1()
{
    int i = 42;

    int j = i;

    for (j=0; j < 400; j++);
}
