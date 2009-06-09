#include "cpp_test.h"

#include "mutatee_util.h"

#define MAX_TEST_CPP 12
int passedTestCPP[MAX_TEST_CPP+1];

int CPP_DEFLT_ARG = CPP_DEFLT_ARG_VAL;

/*
 * The test functions.
 */

void cpp_test_util::call_cpp(int test)
{
   passedTestCPP[test] = TRUE;

   switch (test) {

    case  1 : {
      logerror("Passed test #1 (C++ argument pass)\n");
      break;
    }

    case  2 : {
      logerror("Passed test #2 (overload function)\n");
      break;
    }
    
    case  3 : {
      logerror("Passed test #3 (overload operator)\n");
      break;
    }

    case  4 : {
      logerror("Passed test #4 (static member)\n");
      break;
    }
    
    case  5 : {
      logerror("Passed test #5 (namespace)\n");
      break;
    }

    case  7 : {
      logerror("Passed test #7 (template)\n");
      break;
    }

    case  8 : {
      logerror("Passed test #8 (declaration)\n");
      break;
    }

    case  9 : {
      logerror("Passed test #9 (derivation)\n");
      break;
    }

    case  12 : {
      logerror("Passed test #12 (C++ member function)\n");
      break;
    }

    default : {
      logerror("\tInvalid test %d requested\n", test);
      logerror("\tThis invalid test# is most likely caused by the C++ class member function argument passing\n");
      break;
    }

  }
}
