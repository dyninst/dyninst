
/* Test application (Mutatee) */

/* $Id: test5.mutatee.C,v 1.9 2003/07/18 15:44:10 schendel Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "cpp_test.h"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

int isAttached = 0;
int mutateeCplusplus = 1;

#ifndef COMPILER
#define COMPILER ""
#endif
const char Builder_id[]=COMPILER; /* defined on compile line */

/* Empty functions are sometimes compiled too tight for entry and exit
   points.  The following macro is used to flesh out these
   functions. (expanded to use on all platforms for non-gcc compilers jkh 10/99) */
volatile int dummy3__;

#define DUMMY_FN_BODY \
  int dummy1__ = 1; \
  int dummy2__ = 2; \
  dummy3__ = dummy1__ + dummy2__

/* control debug printf statements */
#define dprintf	if (debugPrint) printf
int debugPrint = 0;

#define TRUE	1
#define FALSE	0

#define MAX_TEST 12

int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}

/*
 * The test functions.
 */

void cpp_test_util::call_cpp(int test)
{
   passedTest[test] = TRUE;

   switch (test) {

    case  1 : {
                 cout << "Passed test #1 (C++ argument pass)" << endl;
		 break;
    }

    case  2 : {
		 cout << "Passed test #2 (overload function)" << endl;
		 break;
    }
    
    case  3 : {
		 cout << "Passed test #3 (overload operator)" << endl;
		 break;
    }

    case  4 : {
		 cout << "Passed test #4 (static member)" << endl;
		 break;
    }
    
    case  5 : {
		 cout << "Passed test #5 (namespace)" << endl;
		 break;
    }

    case  7 : {
		 cout << "Passed test #7 (template)" << endl;
		 break;
    }

    case  8 : {
		 cout << "Passed test #8 (declaration)" << endl;
		 break;
    }

    case  9 : {
		 cout << "Passed test #9 (derivation)" << endl;
		 break;
    }

    case  12 : {
		 cout << "Passed test #12 (C++ member function)" << endl;
		 break;
    }

    default : {
                 cerr << "\tInvalid test "<< test <<" requested" << endl;
                 cerr << "\tThis invalid test# is most likely caused by the C++ class member function argument passing" << endl;
		 break;
    }

  }
}


void arg_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #1 (argument)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[1] = TRUE;

#else

  int test = 1;
  int arg2 = 1;

  call_cpp(test, arg2);
#endif
}

void arg_test::arg_pass(int test)
{
   if (test != 1) {
    cerr << "**Failed** test #1 (C++ argument pass)" << endl;
    cerr << "    Pass in an incorrect parameter value" << endl;
    return;
   }
   cpp_test_util::call_cpp(test);
}

void arg_test::dummy()
{
  DUMMY_FN_BODY;
}

void arg_test::call_cpp(const int arg1, int & arg2, int arg3)
{
   const int m = 8;
   int n = 6;
   int & reference = n;

   dummy(); // place to change the value of arg3 from CPP_DEFLT_ARG to 1 

   if ( 1 != arg3 ) {
     cerr << "**Failed** test #1 (C++ argument pass)" << endl;
     cerr << "    Default argument value is not changed " << endl;
   }

   if ( arg1 == arg2 )  arg2 = CPP_DEFLT_ARG;
}


void overload_func_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #2 (function overload)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[2] = TRUE;

#else


   call_cpp("test overload function");

   call_cpp(2);

   call_cpp(2, 2.0);
#endif
}


void overload_func_test::call_cpp(char * arg1)
{
  DUMMY_FN_BODY;
}


void overload_func_test::call_cpp(int arg1)
{
  DUMMY_FN_BODY;
}


void overload_func_test::call_cpp(int arg1, float arg2)
{
  DUMMY_FN_BODY;
}


void overload_op_test::func_cpp()
{
   overload_op_test test;
   ++test;
}

void overload_op_test_call_cpp(int arg)
{
   if ( arg == 3 ) {
     passedTest[arg] = TRUE;
     cout << "Passed test #3 (overload operator)" << endl;
   } else {
     cerr << "**Failed** test #3 (overload operator)" << endl;
     cerr << "    Overload operator++ return wrong value " << endl;
   }
}

int overload_op_test::operator++()
{
  return (cpp_test_util::CPP_TEST_UTIL_VAR);
}


void static_test_call_cpp(int test)
{
   passedTest[test] = TRUE;
   cout << "Passed test #4 (static member)" << endl;
}

int static_test::count = 0;

void static_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #4 (static member)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[4] = TRUE;

#else

   static_test obj1, obj2;

   if ((obj1.call_cpp()+1) != obj2.call_cpp()) {
      cerr << "**Failed** test #4 (static member)" << endl;
      cerr << "    C++ objects of the same class have different static members " << endl;
   }
#endif
}

static int local_file_var = 3;

void namespace_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #5 (namespace)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[5] = TRUE;

#else
  int local_fn_var = local_file_var;

  class_variable = local_fn_var;

  if ( 1024 != ::CPP_DEFLT_ARG)
    cout << "::CPP_DEFLT_ARG init value wrong" <<endl;
  if ( 0 != cpp_test_util::CPP_TEST_UTIL_VAR )
    cout <<"cpp_test_util::CPP_TEST_UTIL_VAR int value wrong"<<endl;
#endif
}


void sample_exception::response()
{
   DUMMY_FN_BODY;
}


void exception_test_call_cpp(int arg)
{
   if ( arg == 6 ) {
      passedTest[arg] = TRUE;
      cout << "Passed test #6 (exception)" << endl;
   } else {
      cerr << "**Failed** test #6 (exception)" << endl;
   }
}


void exception_test::call_cpp()
{
    throw sample_exception();
}


void exception_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #6 (exception)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[6] = TRUE;

#else

   try {
          int testno = 6;
          call_cpp();
   }
   catch ( sample_exception & ex) {
     ex.response();
   }
   catch ( ... ) {
     cerr << "**Failed** test #6 (exception)" << endl;
     cerr << "    Does not catch appropriate exception" << endl;
     throw;
   }
#endif
}

#ifdef rs6000_ibm_aix4_1
/* xlC's static libC has strangely undefined symbols, so just fake them ... */
int SOMClassClassData;
int SOMObjectClassData;
#else
/* xlC also doesn't like these, so just skip them ... */
template class sample_template <int>;
template class sample_template <char>;
template class sample_template <double>;
#endif

template <class T> T sample_template <T>::content()
{
   T  ret = item;
   return (ret);
}

void template_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #7 (template)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[7] = TRUE;

#else

  int int7 = 7;
  char char7 = 'c';
  sample_template <int> item7_1(int7);
  sample_template <char> item7_2(char7);
 
  item7_1.content();
  item7_2.content();
#endif
}

void template_test_call_cpp(int test)
{
   passedTest[test] = TRUE;
   cout << "Passed test #7 (template)" << endl;
}

void decl_test::func_cpp()
{
#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
    printf("Skipped test #8 (declaration)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[8] = TRUE;

#else

   int CPP_DEFLT_ARG = 8;

   if ( 8 != CPP_DEFLT_ARG )
     cout <<"CPP_DEFLT_ARG init value wrong"<<endl;
   if ( 1024 != ::CPP_DEFLT_ARG )
     cout <<"::CPP_DEFLT_ARG init value wrong"<<endl;
   if ( 0 != cpp_test_util::CPP_TEST_UTIL_VAR )
     cout <<"cpp_test_util::CPP_TEST_UTIL_VAR int value wrong"<<endl;
#endif
}

void decl_test::call_cpp(int test)
{
   if (test != 8) {
       cerr << "**Failed** test #8 (C++ argument pass)" << endl;
       cerr << "    Pass in an incorrect parameter value" << endl;
       return;
   }
   cpp_test_util::CPP_TEST_UTIL_VAR = ::CPP_DEFLT_ARG;
   cpp_test_util::call_cpp(test);
}


void derivation_test::func_cpp()
{
   passedTest[9] = TRUE;
   cout << "Passed test #9 (derivation)" << endl;

}

void stdlib_test1::func_cpp()
{
#if defined(sparc_sun_solaris2_4) \
 || defined(mips_sgi_irix6_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(alpha_dec_osf4_0) \
 || defined(ia64_unknown_linux2_4)
     cout << "Passed test #10 (find standard C++ library)" << endl;
     passedTest[10] = TRUE;
#else
    cout << "Skipped test #10 (find standard C++ library)" << endl;
    cout << "\t- not implemented on this platform" << endl;
    passedTest[10] = TRUE;
#endif

}

void stdlib_test2::call_cpp()
{
   DUMMY_FN_BODY;
}

void stdlib_test2::func_cpp()
{
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0)
    cout<<"Passed test #11 (replace function in standard C++ library)"<<endl;
    passedTest[11] = TRUE;
#else
    cout<<"Skipped test #11 (replace function in standard C++ library)"<<endl;
    cout<<"\t- not implemented on this platform"<<endl;
    passedTest[11] = TRUE;
#endif

}


int func_test::func2_cpp() const
{
    return CPP_TEST_UTIL_VAR;
}


void func_test::func_cpp()
{
   int int12 = func2_cpp();
   call_cpp(int12);
}


//Global Vars for C++ tests

arg_test test1; 
overload_func_test test2;
overload_op_test test3;
static_test test4;
namespace_test test5;
exception_test test6;
template_test test7;
decl_test test8; 
derivation_test test9;
stdlib_test1 test10;
stdlib_test2 test11;
func_test test12;


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifdef i386_unknown_nt4_0
#define USAGE "Usage: test5.mutatee [-attach] [-verbose] -run <num> .."
#else
#define USAGE "Usage: test5.mutatee [-attach <fd>] [-verbose] -run <num> .."
#endif

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
    unsigned int i, j;
    unsigned int testsFailed = 0;
    int useAttach = FALSE;
#ifndef i386_unknown_nt4_0
    int pfd;
#endif
 
    for (j=0; j <= MAX_TEST; j++) {
        runTest[j] = FALSE;
	passedTest[j] = FALSE;
    }

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
	    if (++i >= argc) {
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	    }
	    pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-runall")) {
            dprintf("selecting all tests\n");
            for (j=1; j <= MAX_TEST; j++) runTest[j] = TRUE;
        } else if (!strcmp(argv[i], "-run")) {
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        dprintf("selecting test %d\n", testId);
                        runTest[testId] = TRUE;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    /* end of test list */
		    break;
                }
            }
            i=j-1;
        } else {
            fprintf(stderr, "%s\n", USAGE);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [C++]:\"%s\"\n", argv[0], Builder_id);
    if (argc==1) exit(0);

    if (useAttach) {
#ifndef i386_unknown_nt4_0
	char ch = 'T';
	if (write(pfd, &ch, sizeof(char)) != sizeof(char)) {
	    fprintf(stderr, "*ERROR*: Writing to pipe\n");
	    exit(-1);
	}
	close(pfd);
#endif
	printf("Waiting for mutator to attach...\n");
    	while (!checkIfAttached()) ;
	printf("Mutator attached.  Mutatee continuing.\n");
    }

    if (runTest[1]) test1.func_cpp();
    if (runTest[2]) test2.func_cpp();
    if (runTest[3]) test3.func_cpp();
    if (runTest[4]) test4.func_cpp();
    if (runTest[5]) test5.func_cpp();
    if (runTest[6]) test6.func_cpp();
    if (runTest[7]) test7.func_cpp();
    if (runTest[8]) test8.func_cpp();
    if (runTest[9]) test9.func_cpp();
    if (runTest[10]) test10.func_cpp();
    if (runTest[11]) test11.func_cpp();
    if (runTest[12]) test12.func_cpp();

    /* See how we did running the tests. */
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) testsFailed++;
    }

    if (!testsFailed) {
        printf("All tests passed\n");
    } else {
	printf("**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
	    for (i=1; i <= MAX_TEST; i++) {
	      printf("%s ", runTest[i] ?  (passedTest[i] ? "P" : "F") : "*"); 
	    }
	    printf("\n");
    }

    dprintf("Mutatee %s terminating.\n", argv[0]);
    return (testsFailed ? 127 : 0);
}
