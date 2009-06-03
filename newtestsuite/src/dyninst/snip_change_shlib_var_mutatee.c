#include "mutatee_util.h"

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

static volatile int _unused; /* move decl here to dump compiler warning - jkh */

static volatile int gv_scsv1 = 0;

int scsv1(int x)
{
	return 8 * x;
}

int snip_change_shlib_var_mutatee()
{
	/*  mutator instruments scsv1 entry and exit to (1) modify a global variable
       in a shared library (libtestB), and (2) to call a function in that library
       that returns 1 if the change was successful and 0 otherwise.  The result
       of this function call is put in gv_scsv1; */

	int dont_care = 0;
	dont_care = scsv1(5);

	if (1 != gv_scsv1)
	{
      logerror("Failed snip_change_shlib_var test\n");
	  return -1;
	}

	logerror("Passed snip_change_shlib_var test\n");
	test_passes(testname);

	return 0;
}

