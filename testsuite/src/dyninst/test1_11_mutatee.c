#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_11_func1();
void test1_11_call1();
void test1_11_call2();
void test1_11_call3();
void test1_11_call4();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable11_1 = 0;
static int globalVariable11_2 = 0;
static int globalVariable11_3 = 0;
static int globalVariable11_4 = 0;
static int globalVariable11_5 = 0;
static int test_passed = 0;

/* Function definitions follow */

/*
 * Test #11 - snippets at entry,exit,call
 */ 	

int test1_11_mutatee() {
  int retval;
  test1_11_func1();
  if (test_passed) {
    test_passes(testname);
    retval = 0; /* Test passed */
  } else {
    retval = -1; /* Test failed */
  }
  return retval;
}

void test1_11_func1() {
    globalVariable11_1 = 1;
    func2();
    globalVariable11_1 = 3;
}

void func2() {
    globalVariable11_1 = 2;
}

void test1_11_call1()
{
    if (globalVariable11_1 == 0) globalVariable11_2 = 1;
}

void test1_11_call2()
{
    if (globalVariable11_1 == 1) globalVariable11_3 = 1;
}

void test1_11_call3()
{
    if (globalVariable11_1 == 2) globalVariable11_4 = 1;
}

void test1_11_call4()
{
    if (globalVariable11_1 == 3) globalVariable11_5 = 1;

    if (globalVariable11_2 && globalVariable11_3 &&
	globalVariable11_4 && globalVariable11_5) {
        logerror("Passed test #11 (snippets at entry,exit,call)\n");
	test_passed = 1;
    } else {
        logerror("**Failed test #11 (snippets at entry,exit,call)\n");
	if (!globalVariable11_2)
	    logerror("    entry snippet not called at the correct time\n");
	if (!globalVariable11_3)
	    logerror("    pre call snippet not called at the correct time\n");
	if (!globalVariable11_4)
	    logerror("    post call snippet not called at the correct time\n");
	if (!globalVariable11_5)
	    logerror("    exit snippet not called at the correct time\n");
    }
}
