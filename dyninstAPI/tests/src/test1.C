// $Id: test1.C,v 1.52 2000/03/22 19:08:48 tikir Exp 
//
// libdyninst validation suite test #1
//    Author: Jeff Hollingsworth (1/7/97)
//        derived from a previous test by Bryan Buck
//

//  This program tests the basic features of the dyninst API.  
//	The mutatee that goes with this file is test1.mutatee.c
//	
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//	    func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#ifdef i386_unknown_nt4_0
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"
#include "test1.h"

// #include <vector.h>

int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false; // force relocation of functions

int mutateeCplusplus = 0;
int mutateeFortran = 0;
int mutateeF77 = 0;
bool runAllTests = true;
const unsigned int MAX_TEST = 35;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];
int saveWorld = 0;

template class BPatch_Vector<BPatch_variableExpr*>;
template class BPatch_Set<int>;

BPatch *bpatch;

static const char *mutateeNameRoot = "test1.mutatee";
static const char *libNameAroot = "libtestA";
static const char *libNameBroot = "libtestB";
char libNameA[64], libNameB[64];

// control debug printf statements
#define dprintf	if (debugPrint) printf

/**************************************************************************
 * Error callback
 **************************************************************************/

#define DYNINST_NO_ERROR -1

int expectError = DYNINST_NO_ERROR;

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) printf("%s\n", params[0]); }
            else
                printf("%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
        if (num != expectError) {
	  if(num != 112)
	    printf("Error #%d (level %d): %s\n", num, level, line);
        
            // We consider some errors fatal.
            if (num == 101) {
               exit(-1);
            }
        }
    }
}

void createInstPointError(BPatchErrorLevel level, int num, const char **params)
{
    if (num != 117 && num != 118)
	errorFunc(level, num, params);
}

/**************************************************************************
 * Utility functions
 **************************************************************************/

// check that the cost of a snippet is sane.  Due to differences between
//   platforms, it is impossible to check this exactly in a machine independent
//   manner.
void checkCost(BPatch_snippet snippet)
{
    float cost;
    BPatch_snippet copy;

    // test copy constructor too.
    copy = snippet;

    cost = snippet.getCost();
    dprintf("Snippet cost=%g\n", cost);
    if (cost < 0.0) {
	printf("*Error*: negative snippet cost\n");
    } else if (cost == 0.0) {
#if !defined(alpha_dec_osf4_0)
	printf("*Warning*: zero snippet cost\n");
#endif
    } else if (cost > 0.01) {
	printf("*Error*: snippet cost of %f, exceeds max expected of 0.1",
	    cost);
    }
}

int functionNameMatch(const char *gotName, const char *targetName) 
{
    if (!strcmp(gotName, targetName)) return 0;

    if (!strncmp(gotName, targetName, strlen(targetName)) &&
	(strlen(targetName) == strlen(gotName)-1) &&
	(gotName[strlen(targetName)] == '_'))
	return 0;

    return 1;
}


//
// Replace all calls in "inFunction" to "callTo" with calls to "replacement."
// If "replacement" is NULL, them use removeFunctionCall instead of
// replaceFunctionCall.
// Returns the number of replacements that were performed.
//
int replaceFunctionCalls(BPatch_thread *appThread, BPatch_image *appImage,
                         const char *inFunction, const char *callTo, 
                         const char *replacement, int testNo, 
                         const char *testName, int callsExpected = -1)
{
    int numReplaced = 0;

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction(inFunction, found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
      fprintf(stderr, "    Unable to find function %s\n",
	      inFunction);
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), inFunction);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!points || (points->size() < 1)) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to find point %s - subroutine calls\n",
		inFunction);
	exit(1);
    }

    BPatch_function *call_replacement;
    if (replacement != NULL) {
	call_replacement = appImage->findFunction(replacement);
	if (call_replacement == NULL) {
	    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	    fprintf(stderr, "    Unable to find function %s\n", replacement);
	    exit(1);
	}
    }

    for (unsigned int n = 0; n < points->size(); n++) {
	BPatch_function *func;

	if ((func = (*points)[n]->getCalledFunction()) == NULL) continue;

	char fn[256];
	if (func->getName(fn, 256) == NULL) {
	    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	    fprintf(stderr, "    Can't get name of called function in %s\n",
		    inFunction);
	    exit(1);
	}
	if (functionNameMatch(fn, callTo) == 0) {
	    if (replacement == NULL)
		appThread->removeFunctionCall(*((*points)[n]));
	    else
		appThread->replaceFunctionCall(*((*points)[n]),
					       *call_replacement);
	    numReplaced++;
	}
    }

    if (callsExpected > 0 && callsExpected != numReplaced) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Expected to find %d %s to %s in %s, found %d\n",
		callsExpected, callsExpected == 1 ? "call" : "calls",
		callTo, inFunction, numReplaced);
	exit(1);
    }


    return numReplaced;
}


//
// Return a pointer to a string identifying a BPatch_procedureLocation
//
const char *locationName(BPatch_procedureLocation l)
{
    switch(l) {
      case BPatch_entry:
	return "entry";
      case BPatch_exit:
	return "exit";
      case BPatch_subroutine:
	return "call points";
      case BPatch_longJump:
	return "long jump";
      case BPatch_allLocations:
	return "all";
      default:
	return "<invalid BPatch_procedureLocation>";
    };
}


//
// Insert "snippet" at the location "loc" in the function "inFunction."
// Returns the value returned by BPatch_thread::insertSnippet.
//
BPatchSnippetHandle *insertSnippetAt(BPatch_thread *appThread,
                               BPatch_image *appImage, const char *inFunction, 
                               BPatch_procedureLocation loc, 
                               BPatch_snippet &snippet,
                               int testNo, const char *testName)
{
    // Find the point(s) we'll be instrumenting

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction(inFunction, found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
      fprintf(stderr, "    Unable to find function %s\n",
	      inFunction);
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), inFunction);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(loc);

    if (!points) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to find point %s - %s\n",
		inFunction, locationName(loc));
	exit(-1);
    }

    checkCost(snippet);
    return appThread->insertSnippet(snippet, *points);
}

//
// Create a snippet that calls the function "funcName" with no arguments
//
BPatch_snippet *makeCallSnippet(BPatch_image *appImage, const char *funcName,
                                int testNo, const char *testName)
{
    BPatch_function *call_func = appImage->findFunction(funcName);
    if (call_func == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to find function %s\n", funcName);
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_snippet *ret = new BPatch_funcCallExpr(*call_func, nullArgs);

    if (ret == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to create snippet to call %s\n", funcName);
	exit(1);
    }

    return ret;
}

//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
BPatchSnippetHandle *insertCallSnippetAt(BPatch_thread *appThread,
                            BPatch_image *appImage, const char *inFunction,
                            BPatch_procedureLocation loc, const char *funcName,
                            int testNo, const char *testName)
{
    BPatch_snippet *call_expr =
       makeCallSnippet(appImage, funcName, testNo, testName);

    BPatchSnippetHandle *ret = insertSnippetAt(appThread, appImage,
					       inFunction, loc, *call_expr,
					       testNo, testName);
    if (ret == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to insert snippet to call function %s\n",
		funcName);
	exit(-1);
    }

    delete call_expr;

    return ret;
}

// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
BPatch_variableExpr *findVariable(BPatch_image *appImage, const char* var,
                                  BPatch_Vector <BPatch_point *> *point = NULL)
{
  //BPatch_variableExpr *FortVar = NULL;
    BPatch_variableExpr *ret = NULL;
    int i, numchars = strlen (var);
    char *lowercase = new char [numchars];
    int temp = expectError;

    if (mutateeFortran && point) {
            strcpy (lowercase, var);
            expectError = 100;
            for (i = 0; i < numchars; i++)
                lowercase [i] = tolower (lowercase [i]);
            ret = appImage->findVariable (*(*point) [0], lowercase);
        if (!ret) {
            expectError = temp;
            ret = appImage->findVariable (*(*point) [0], var);
        }
    } else {
        ret = appImage->findVariable (var);
    }

    expectError = temp;
    delete [] lowercase;
    return ret;
}

/**************************************************************************
 * Tests
 **************************************************************************/

//
// Start Test Case #1 - (zero arg function call)
//
void mutatorTest1(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func1_1"

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findBPFunction("func1_1", found_funcs)) || (0 == found_funcs.size())) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func1_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func1_1");
  }
  
  BPatch_Vector<BPatch_point *> *point1_1 = found_funcs[0]->findPoint(BPatch_entry);


    if (!point1_1 || ((*point1_1).size() == 0)) {
        fprintf(stderr, "**Failed** test #1 (zero arg function call)\n");
	fprintf(stderr, "    Unable to find entry point to \"func1_1.\"\n");
	exit(1);
    }

    BPatch_function *call1_func;

    call1_func = appImage->findFunction("call1_1");

    if (call1_func == NULL) {
        fprintf(stderr, "**Failed** test #1 (zero arg function call)\n");
	fprintf(stderr, "Unable to find function \"call1_1\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call1_args;
    BPatch_funcCallExpr call1Expr(*call1_func, call1_args);

    dprintf("Inserted snippet2\n");
    checkCost(call1Expr);
    appThread->insertSnippet(call1Expr, *point1_1);
}

//
// Start Test Case #2 - mutator side (call a four argument function)
//
void mutatorTest2(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func2_1"

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findBPFunction("func2_1", found_funcs)) || (0 == found_funcs.size())) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func2_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func2_1");
  }
  
  BPatch_Vector<BPatch_point *> *point2_1 = found_funcs[0]->findPoint(BPatch_entry);

  if (!point2_1 || ((*point2_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
    fprintf(stderr, "    Unable to find entry point to \"func2_1.\"\n");
    exit(1);
  }

    BPatch_function *call2_func;
    call2_func = appImage->findFunction("call2_1");

    if (call2_func == NULL) {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unable to find function \"call2_1.\"\n");
	exit(1);
    }

    void *ptr;

#if defined(mips_sgi_irix6_4)
    BPatch_variableExpr *pointerSizeVar = appImage->findVariable("pointerSize");
    if (!pointerSizeVar) {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unable to locate variable pointerSize\n");
	exit(1);
    }

    int pointerSize;
    if (!pointerSizeVar->readValue(&pointerSize)) {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unable to read value of variable pointerSize\n");
	exit(1);
    }

    assert(sizeof(void *) == sizeof(unsigned long) &&
	   sizeof(void *) == TEST_PTR_SIZE);

    /* Determine the size of pointer we should use dynamically. */
    if (pointerSize == 4) {
	ptr = TEST_PTR_32BIT;
    } else if (pointerSize == 8) {
	ptr = TEST_PTR_64BIT;
    } else {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unexpected value for pointerSize\n");
	exit(1);
    }
#else
    /* For platforms where there is only one possible size for a pointer. */
    ptr = TEST_PTR;
#endif

    BPatch_Vector<BPatch_snippet *> call2_args;

    BPatch_constExpr expr2_1 (0), expr2_2 (0), expr2_3 (0), expr2_4 (0);

    if (mutateeFortran) {
        BPatch_variableExpr *expr2_5 = appThread->malloc (*appImage->findType ("int"));
        BPatch_variableExpr *expr2_6 = appThread->malloc (*appImage->findType ("int"));

        expr2_1 = expr2_5->getBaseAddr ();
        expr2_2 = expr2_6->getBaseAddr ();

        BPatch_arithExpr expr2_7 (BPatch_assign, *expr2_5, BPatch_constExpr(1));
        appThread->insertSnippet (expr2_7, *point2_1);

        BPatch_arithExpr expr2_8 (BPatch_assign, *expr2_6, BPatch_constExpr(2));
        appThread->insertSnippet (expr2_8, *point2_1);

        expr2_3 = "testString2_1";
        expr2_4 = 13;
    } else {
        expr2_1 = 1;
        expr2_2 = 2;
        expr2_3 = "testString2_1";
        expr2_4 = ptr;
    }

    call2_args.push_back(&expr2_1);
    call2_args.push_back(&expr2_2);
    call2_args.push_back(&expr2_3);
    call2_args.push_back(&expr2_4);

    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    dprintf("Inserted snippet2\n");
    checkCost(call2Expr);
    appThread->insertSnippet(call2Expr, *point2_1, BPatch_callBefore, BPatch_lastSnippet);
}

//
// Start Test Case #3 - mutator side (passing variables to function)
//
void mutatorTest3(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func3_1"

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findBPFunction("func3_1", found_funcs)) || (0 == found_funcs.size())) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func3_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func3_1");
  }
  
  BPatch_Vector<BPatch_point *> *point3_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point3_1 || ((*point3_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func3_1.\"\n");
	exit(1);
    }

    BPatch_function *call3_func;
    call3_func = appImage->findFunction("call3_1");

    if (call3_func == NULL) {
	fprintf(stderr, "Unable to find function \"call3_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call3_args;

  BPatch_Vector<BPatch_function *> found_funcs2;
  if ((NULL == appImage->findBPFunction("call3_1", found_funcs2)) || (0 == found_funcs2.size())) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "call3_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs2.size(), "call3_1");
  }
  
  BPatch_Vector<BPatch_point *> *call3_1 = found_funcs2[0]->findPoint(BPatch_subroutine);

    if (!call3_1 || ((*call3_1).size() == 0)) {
        fprintf(stderr, "    Unable to find entry point to \"call3_1.\"\n");
        exit(1);
    }

    BPatch_variableExpr *expr3_1 = findVariable (appImage, "globalVariable3_1", call3_1);

    if (!expr3_1) {
        fprintf(stderr, "**Failed** test #3 (passing variables)\n");
        fprintf(stderr, "    Unable to locate variable globalVariable3_1\n");
        exit(1);
    }

    // see if we can find the address
    if (expr3_1->getBaseAddr() <= 0) {
        printf("*Error*: address %p for globalVariable3_1 is not valid\n",
            expr3_1->getBaseAddr());
    }

    BPatch_variableExpr *expr3_2 = appThread->malloc(*appImage->findType("int"));
    if (!expr3_2) {
	fprintf(stderr, "**Failed** test #3 (passing variables)\n");
	fprintf(stderr, "    Unable to create new int variable\n");
	exit(1);
    }

	BPatch_constExpr expr3_3 (expr3_1->getBaseAddr ());
	BPatch_constExpr expr3_4 (expr3_2->getBaseAddr ());

	if (mutateeFortran) {
	    call3_args.push_back (&expr3_3);
	    call3_args.push_back (&expr3_4);
	} else {
	    call3_args.push_back(expr3_1);
	    call3_args.push_back(expr3_2);
	}

    BPatch_funcCallExpr call3Expr(*call3_func, call3_args);
    checkCost(call3Expr);
    appThread->insertSnippet(call3Expr, *point3_1);

    BPatch_arithExpr expr3_5(BPatch_assign, *expr3_2, BPatch_constExpr(32));
    checkCost(expr3_5);
    appThread->insertSnippet(expr3_5, *point3_1);

    dprintf("Inserted snippet3\n");
}

//
// Start Test Case #4 - mutator side (sequence)
//	Use the BPatch sequence operation to glue to expressions togehter.
//	The test is constructed to verify the correct execution order.
//
void mutatorTest4(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func4_1"
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findBPFunction("func4_1", found_funcs)) || (0 == found_funcs.size())) {
     fprintf(stderr, "    Unable to find function %s\n",
	    "func4_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func4_1");
  }
  
  BPatch_Vector<BPatch_point *> *point4_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point4_1 || ((*point4_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func4_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr4_1 = findVariable (appImage, "globalVariable4_1", point4_1);

    if (!expr4_1) {
	fprintf(stderr, "**Failed** test #4 (sequence)\n");
	fprintf(stderr, "    Unable to locate variable globalVariable4_1\n");
	exit(1);
    }

    BPatch_arithExpr expr4_2(BPatch_assign, *expr4_1, BPatch_constExpr(42));
    BPatch_arithExpr expr4_3(BPatch_assign, *expr4_1, BPatch_constExpr(43));

    BPatch_Vector<BPatch_snippet*> vect4_1;
    vect4_1.push_back(&expr4_2);
    vect4_1.push_back(&expr4_3);

    BPatch_sequence expr4_4(vect4_1);
    checkCost(expr4_4);
    appThread->insertSnippet(expr4_4, *point4_1);
}

//
// Start Test Case #5 - mutator side (if w.o. else)
//
void mutatorTest5(BPatch_thread *appThread, BPatch_image *appImage)
{

    // Find the entry point to the procedure "func5_2"
    
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findBPFunction("func5_2", found_funcs)) || (0 == found_funcs.size())) {
     fprintf(stderr, "    Unable to find function %s\n",
	    "func5_2");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func5_2");
  }
  
  BPatch_Vector<BPatch_point *> *point5_1 = found_funcs[0]->findPoint(BPatch_entry);  

    if (!point5_1 || ((*point5_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func5_2\".\n");
	exit(1);
    }

 BPatch_Vector<BPatch_function *> found_funcs2;
  if ((NULL == appImage->findBPFunction("func5_1", found_funcs2)) || (0 == found_funcs2.size())) {
     fprintf(stderr, "    Unable to find function %s\n",
	    "func5_1");
    exit(1);
  }
  
  if (1 < found_funcs2.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs2.size(), "func5_1");
  }
  
  BPatch_Vector<BPatch_point *> *point5_2 = found_funcs2[0]->findPoint(BPatch_subroutine);  
    
  if (!point5_2 || ((*point5_2).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func5_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr5_1 = findVariable (appImage, "globalVariable5_1", point5_2);
    BPatch_variableExpr *expr5_2 = findVariable (appImage, "globalVariable5_2", point5_2);

    if (!expr5_1 || !expr5_2) {
	fprintf(stderr, "**Failed** test #5 (1f w.o. else)\n");
	fprintf(stderr, "    Unable to locate variable globalVariable5_1 or ");
	fprintf(stderr, "    variable globalVariable5_2\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet*> vect5_1;

    // if (0 == 1) globalVariable5_1 = 52;
    BPatch_ifExpr expr5_3(
	BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0), BPatch_constExpr(1)), 
	BPatch_arithExpr(BPatch_assign, *expr5_1, BPatch_constExpr(52)));

    // if (1 == 1) globalVariable5_2 = 53;
    BPatch_ifExpr expr5_4(
	BPatch_boolExpr(BPatch_eq, BPatch_constExpr(1), BPatch_constExpr(1)), 
	BPatch_arithExpr(BPatch_assign, *expr5_2, BPatch_constExpr(53)));

    vect5_1.push_back(&expr5_3);
    vect5_1.push_back(&expr5_4);

    BPatch_sequence expr5_5(vect5_1);
    checkCost(expr5_5);
    appThread->insertSnippet(expr5_5, *point5_1);
}

//
// Start Test Case #6 - mutator side (arithmetic operators)
//
void mutatorTest6(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func6_2"

    
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findBPFunction("func6_2", found_funcs)) || (0 == found_funcs.size())) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func6_2");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func6_2");
  }
  
  BPatch_Vector<BPatch_point *> *point6_1 = found_funcs[0]->findPoint(BPatch_entry);  

  if (!point6_1 || ((*point6_1).size() == 0)) {
    fprintf(stderr, "Unable to find entry point to \"func6_2\".\n");
    exit(1);
  }

  BPatch_Vector<BPatch_function *> found_funcs2;
  if ((NULL == appImage->findBPFunction("func6_1", found_funcs2)) || (0 == found_funcs2.size())) {
     fprintf(stderr, "    Unable to find function %s\n",
	    "func6_1");
    exit(1);
  }
  
  if (1 < found_funcs2.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs2.size(), "func6_1");
  }
  
  BPatch_Vector<BPatch_point *> *point6_2 = found_funcs2[0]->findPoint(BPatch_subroutine);  

    if (!point6_2 || ((*point6_2).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func6_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr6_1, *expr6_2, *expr6_3, *expr6_4, *expr6_5, *expr6_6,
        *expr6_1a, *expr6_2a, *expr6_3a, *expr6_4a, *expr6_5a, *expr6_6a,
        *constVar1, *constVar2, *constVar3, *constVar5, *constVar6,
        *constVar10, *constVar60, *constVar64, *constVar66, *constVar67;

    expr6_1 = findVariable(appImage, "globalVariable6_1", point6_2);
    expr6_2 = findVariable(appImage, "globalVariable6_2", point6_2);
    expr6_3 = findVariable(appImage, "globalVariable6_3", point6_2);
    expr6_4 = findVariable(appImage, "globalVariable6_4", point6_2);
    expr6_5 = findVariable(appImage, "globalVariable6_5", point6_2);
    expr6_6 = findVariable(appImage, "globalVariable6_6", point6_2);
    expr6_1a = findVariable(appImage, "globalVariable6_1a", point6_2);
    expr6_2a = findVariable(appImage, "globalVariable6_2a", point6_2);
    expr6_3a = findVariable(appImage, "globalVariable6_3a", point6_2);
    expr6_4a = findVariable(appImage, "globalVariable6_4a", point6_2);
    expr6_5a = findVariable(appImage, "globalVariable6_5a", point6_2);
    expr6_6a = findVariable(appImage, "globalVariable6_6a", point6_2);

    constVar1 = findVariable(appImage, "constVar1", point6_2);
    constVar2 = findVariable(appImage, "constVar2", point6_2);
    constVar3 = findVariable(appImage, "constVar3", point6_2);
    constVar5 = findVariable(appImage, "constVar5", point6_2);
    constVar6 = findVariable(appImage, "constVar6", point6_2);
    constVar10 = findVariable(appImage, "constVar10", point6_2);
    constVar60 = findVariable(appImage, "constVar60", point6_2);
    constVar64 = findVariable(appImage, "constVar64", point6_2);
    constVar66 = findVariable(appImage, "constVar66", point6_2);
    constVar67 = findVariable(appImage, "constVar67", point6_2);

    if (!expr6_1 || !expr6_2 || !expr6_3 || !expr6_4 ||
	!expr6_5 || !expr6_6 || !expr6_1a || !expr6_2a || !expr6_3a ||
	!expr6_4a || !expr6_5a || !expr6_6a) {
	fprintf(stderr, "**Failed** test #6 (arithmetic operators)\n");
	fprintf(stderr, "    Unable to locate one of globalVariable6_?\n");
	exit(1);
    }

    if (!constVar1 || !constVar2 || !constVar3 || !constVar5 ||
	!constVar6 || !constVar10 || !constVar60 || !constVar64 || 
	!constVar66 || !constVar67) {
	fprintf(stderr, "**Failed** test #6 (arithmetic operators)\n");
	fprintf(stderr, "    Unable to locate one of constVar?\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet*> vect6_1;

    // globalVariable6_1 = 60 + 2
    BPatch_arithExpr arith6_1 (BPatch_assign, *expr6_1,
      BPatch_arithExpr(BPatch_plus,BPatch_constExpr(60), BPatch_constExpr(2)));
    vect6_1.push_back(&arith6_1);

    // globalVariable6_2 = 64 - 1
    BPatch_arithExpr arith6_2 (BPatch_assign, *expr6_2, 
      BPatch_arithExpr(BPatch_minus,BPatch_constExpr(64),BPatch_constExpr(1)));
    vect6_1.push_back(&arith6_2);

    // globalVariable6_3 = 553648128 / 25165824 = 22
    //    - make these big constants to test loading constants larger than
    //      small immediate - jkh 6/22/98
    BPatch_arithExpr arith6_3 (BPatch_assign, *expr6_3, BPatch_arithExpr(
      BPatch_divide,BPatch_constExpr(553648128),BPatch_constExpr(25165824)));
    vect6_1.push_back(&arith6_3);

    // globalVariable6_4 = 67 / 3
    BPatch_arithExpr arith6_4 (BPatch_assign, *expr6_4, BPatch_arithExpr(
      BPatch_divide,BPatch_constExpr(67),BPatch_constExpr(3)));
    vect6_1.push_back(&arith6_4);
    // globalVariable6_5 = 6 * 5
    BPatch_arithExpr arith6_5 (BPatch_assign, *expr6_5, BPatch_arithExpr(
      BPatch_times,BPatch_constExpr(6),BPatch_constExpr(5)));
    vect6_1.push_back(&arith6_5);

    // globalVariable6_6 = 10,3
    BPatch_arithExpr arith6_6 (BPatch_assign, *expr6_6, 
	BPatch_arithExpr(BPatch_seq,BPatch_constExpr(10),BPatch_constExpr(3)));
    vect6_1.push_back(&arith6_6);

    // globalVariable6_1a = 60 + 2
    BPatch_arithExpr arith6_1a (BPatch_assign, *expr6_1a, 
      BPatch_arithExpr(BPatch_plus, *constVar60, *constVar2));
    vect6_1.push_back(&arith6_1a);

    // globalVariable6_2a = 64 - 1
    BPatch_arithExpr arith6_2a (BPatch_assign, *expr6_2a, 
      BPatch_arithExpr(BPatch_minus, *constVar64, *constVar1));
    vect6_1.push_back(&arith6_2a);

    // globalVariable6_3a = 66 / 3
    BPatch_arithExpr arith6_3a (BPatch_assign, *expr6_3a, BPatch_arithExpr(
      BPatch_divide, *constVar66, *constVar3));
    vect6_1.push_back(&arith6_3a);

    // globalVariable6_4a = 67 / 3
    BPatch_arithExpr arith6_4a (BPatch_assign, *expr6_4a, BPatch_arithExpr(
      BPatch_divide, *constVar67, *constVar3));
    vect6_1.push_back(&arith6_4a);

    // globalVariable6_5a = 6 * 5
    BPatch_arithExpr arith6_5a (BPatch_assign, *expr6_5a, BPatch_arithExpr(
      BPatch_times, *constVar6, *constVar5));
    vect6_1.push_back(&arith6_5a);

    // globalVariable6_6a = 10,3
    // BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a, *constVar3);
    //	BPatch_arithExpr(BPatch_seq, *constVar10, BPatch_constExpr(3)));
    BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a,
	BPatch_arithExpr(BPatch_seq, *constVar10, *constVar3));
    vect6_1.push_back(&arith6_6a);

    checkCost(BPatch_sequence(vect6_1));
    appThread->insertSnippet( BPatch_sequence(vect6_1), *point6_1);
}

void genRelTest(BPatch_image *appImage,BPatch_Vector<BPatch_snippet*> &vect7_1,
                BPatch_relOp op, int r1, int r2, const char *var1)
{

   BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func7_1", found_funcs)) || (0 == found_funcs.size())) {
        fprintf(stderr, "    Unable to find function %s\n",
	      "func7_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func7_1");
    }

    BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point7_1 || ((*point7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr1_1 = findVariable (appImage, var1, point7_1);

    if (!expr1_1) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate variable %s\n", var1);
	exit(1);
    }
    BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
	BPatch_boolExpr(op, BPatch_constExpr(r1), BPatch_constExpr(r2)), 
	BPatch_arithExpr(BPatch_assign, *expr1_1, BPatch_constExpr(72)));
    vect7_1.push_back(tempExpr1);
}

void genVRelTest(BPatch_image *appImage,
                 BPatch_Vector<BPatch_snippet*> &vect7_1, 
                 BPatch_relOp op, BPatch_variableExpr *r1, 
                 BPatch_variableExpr *r2, const char *var1)
{

   BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func7_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func7_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func7_1");
    }

    BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point7_1 || ((*point7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr1_1 = findVariable(appImage, var1, point7_1);

    if (!expr1_1) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate variable %s\n", var1);
	exit(1);
    }
    BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
	BPatch_boolExpr(op, *r1, *r2), 
	BPatch_arithExpr(BPatch_assign, *expr1_1, BPatch_constExpr(74)));
    vect7_1.push_back(tempExpr1);
}

//
// Start Test Case #7 - mutator side (relational operators)
//
void mutatorTest7(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func7_2"

   BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func7_2", found_funcs)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func7_2");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func7_2");
    }

    BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point7_1 || ((*point7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_2\".\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet*> vect7_1;

    genRelTest(appImage, vect7_1, BPatch_lt, 0, 1, "globalVariable7_1");
    genRelTest(appImage, vect7_1, BPatch_lt, 1, 0, "globalVariable7_2");
    genRelTest(appImage, vect7_1, BPatch_eq, 2, 2, "globalVariable7_3");
    genRelTest(appImage, vect7_1, BPatch_eq, 2, 3, "globalVariable7_4");
    genRelTest(appImage, vect7_1, BPatch_gt, 4, 3, "globalVariable7_5");
    genRelTest(appImage, vect7_1, BPatch_gt, 3, 4, "globalVariable7_6");
    genRelTest(appImage, vect7_1, BPatch_le, 3, 4, "globalVariable7_7");
    genRelTest(appImage, vect7_1, BPatch_le, 4, 3, "globalVariable7_8");
    genRelTest(appImage, vect7_1, BPatch_ne, 5, 6, "globalVariable7_9");
    genRelTest(appImage, vect7_1, BPatch_ne, 5, 5, "globalVariable7_10");
    genRelTest(appImage, vect7_1, BPatch_ge, 9, 7, "globalVariable7_11");
    genRelTest(appImage, vect7_1, BPatch_ge, 7, 9, "globalVariable7_12");
    genRelTest(appImage, vect7_1, BPatch_and, 1, 1, "globalVariable7_13");
    genRelTest(appImage, vect7_1, BPatch_and, 1, 0, "globalVariable7_14");
    genRelTest(appImage, vect7_1, BPatch_or, 1, 0, "globalVariable7_15");
    genRelTest(appImage, vect7_1, BPatch_or, 0, 0, "globalVariable7_16");

   BPatch_Vector<BPatch_function *> found_funcs2;
    if ((NULL == appImage->findBPFunction("func7_1", found_funcs2)) || (0 == found_funcs2.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func7_1");
      exit(1);
    }

    if (1 < found_funcs2.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs2.size(), "func7_1");
    }

    BPatch_Vector<BPatch_point *> *func7_1 = found_funcs2[0]->findPoint(BPatch_subroutine);

    if (!func7_1 || ((*func7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *constVar0, *constVar1, *constVar2, *constVar3, *constVar4, *constVar5, *constVar6, *constVar7, *constVar9;
    constVar0 = findVariable(appImage, "constVar0", func7_1);
    constVar1 = findVariable(appImage, "constVar1", func7_1);
    constVar2 = findVariable(appImage, "constVar2", func7_1);
    constVar3 = findVariable(appImage, "constVar3", func7_1);
    constVar4 = findVariable(appImage, "constVar4", func7_1);
    constVar5 = findVariable(appImage, "constVar5", func7_1);
    constVar6 = findVariable(appImage, "constVar6", func7_1);
    constVar7 = findVariable(appImage, "constVar7", func7_1);
    constVar9 = findVariable(appImage, "constVar9", func7_1);

    if (!constVar0 || !constVar1 || !constVar2 || !constVar3 || !constVar4 ||
        !constVar5 || !constVar6 || !constVar7 || !constVar9 ) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate one of constVar?\n");
	exit(1);
    }

    genVRelTest(appImage, vect7_1, BPatch_lt, constVar0, constVar1,
        "globalVariable7_1a");
    genVRelTest(appImage, vect7_1, BPatch_lt, constVar1, constVar0, 
        "globalVariable7_2a");
    genVRelTest(appImage, vect7_1, BPatch_eq, constVar2, constVar2, 
        "globalVariable7_3a");
    genVRelTest(appImage, vect7_1, BPatch_eq, constVar2, constVar3, 
        "globalVariable7_4a");
    genVRelTest(appImage, vect7_1, BPatch_gt, constVar4, constVar3, 
        "globalVariable7_5a");
    genVRelTest(appImage, vect7_1, BPatch_gt, constVar3, constVar4, 
        "globalVariable7_6a");
    genVRelTest(appImage, vect7_1, BPatch_le, constVar3, constVar4, 
        "globalVariable7_7a");
    genVRelTest(appImage, vect7_1, BPatch_le, constVar4, constVar3, 
        "globalVariable7_8a");
    genVRelTest(appImage, vect7_1, BPatch_ne, constVar5, constVar6, 
        "globalVariable7_9a");
    genVRelTest(appImage, vect7_1, BPatch_ne, constVar5, constVar5, 
        "globalVariable7_10a");
    genVRelTest(appImage, vect7_1, BPatch_ge, constVar9, constVar7, 
        "globalVariable7_11a");
    genVRelTest(appImage, vect7_1, BPatch_ge, constVar7, constVar9, 
        "globalVariable7_12a");
    genVRelTest(appImage, vect7_1, BPatch_and, constVar1, constVar1, 
        "globalVariable7_13a");
    genVRelTest(appImage, vect7_1, BPatch_and, constVar1, constVar0, 
        "globalVariable7_14a");
    genVRelTest(appImage, vect7_1, BPatch_or, constVar1, constVar0, 
        "globalVariable7_15a");
    genVRelTest(appImage, vect7_1, BPatch_or, constVar0, constVar0, 
        "globalVariable7_16a");

    dprintf("relops test vector length is %d\n", vect7_1.size());

    checkCost(BPatch_sequence(vect7_1));
    appThread->insertSnippet( BPatch_sequence(vect7_1), *point7_1);
}

//
// Start Test Case #8 - mutator side (preserve registers - expr)
//
void mutatorTest8(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func8_1"

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func8_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func8_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func8_1");
    }

    BPatch_Vector<BPatch_point *> *point8_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point8_1 || ((*point8_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func8_1\".\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet*> vect8_1;

    BPatch_variableExpr *expr8_1 = findVariable(appImage, "globalVariable8_1", point8_1);

    if (!expr8_1) {
	fprintf(stderr, "**Failed** test #3 (passing variables)\n");
	fprintf(stderr, "    Unable to locate variable globalVariable8_1\n");
	exit(1);
    }

    BPatch_arithExpr arith8_1 (BPatch_assign, *expr8_1, 
      BPatch_arithExpr(BPatch_plus, 
	    BPatch_arithExpr(BPatch_plus, 
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(81), 
					      BPatch_constExpr(82)),
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(83), 
					      BPatch_constExpr(84))),
	    BPatch_arithExpr(BPatch_plus, 
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(85), 
					      BPatch_constExpr(86)),
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(87), 
					      BPatch_constExpr(88)))));
    vect8_1.push_back(&arith8_1);

    checkCost(BPatch_sequence(vect8_1));
    appThread->insertSnippet( BPatch_sequence(vect8_1), *point8_1);
}

//
// Start Test Case #9 - mutator side (preserve registers - funcCall)
//
void mutatorTest9(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func9_1"

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func9_1", found_funcs)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func9_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func9_1");
    }

    BPatch_Vector<BPatch_point *> *point9_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point9_1 || ((*point9_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func9_1\".\n");
	exit(1);
    }

    BPatch_function *call9_func;
    call9_func = appImage->findFunction("call9_1");

    if (call9_func == NULL) {
	fprintf(stderr, "Unable to find function \"call9_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call9_args;

    BPatch_variableExpr *expr9_1 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_1 (0);
    BPatch_arithExpr arithexpr9_1 (BPatch_assign, *expr9_1, BPatch_constExpr (91));
    appThread->insertSnippet (arithexpr9_1, *point9_1);

    if (mutateeFortran) {
        constExpr9_1 = expr9_1->getBaseAddr ();
    } else {
        constExpr9_1 = 91;
    }

    call9_args.push_back(&constExpr9_1);

    BPatch_variableExpr *expr9_2 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_2 (0);
    BPatch_arithExpr arithexpr9_2 (BPatch_assign, *expr9_2, BPatch_constExpr (92));
    appThread->insertSnippet (arithexpr9_2, *point9_1);

    if (mutateeFortran) {
        constExpr9_2 = expr9_2->getBaseAddr ();
    } else {
        constExpr9_2 = 92;
    }

    call9_args.push_back(&constExpr9_2);

    BPatch_variableExpr *expr9_3 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_3 (0);
    BPatch_arithExpr arithexpr9_3 (BPatch_assign, *expr9_3, BPatch_constExpr (93));
    appThread->insertSnippet (arithexpr9_3, *point9_1);

    if (mutateeFortran) {
        constExpr9_3 = expr9_3->getBaseAddr ();
    } else {
        constExpr9_3 = 93;
    }

    call9_args.push_back(&constExpr9_3);

    BPatch_variableExpr *expr9_4 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_4 (0);
    BPatch_arithExpr arithexpr9_4 (BPatch_assign, *expr9_4, BPatch_constExpr (94));
    appThread->insertSnippet (arithexpr9_4, *point9_1);

    if (mutateeFortran) {
        constExpr9_4 = expr9_4->getBaseAddr ();
    } else {
        constExpr9_4 = 94;
    }

    call9_args.push_back(&constExpr9_4);

    BPatch_variableExpr *expr9_5 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_5 (0);
    BPatch_arithExpr arithexpr9_5 (BPatch_assign, *expr9_5, BPatch_constExpr (95));
    appThread->insertSnippet (arithexpr9_5, *point9_1);

    if (mutateeFortran) {
        constExpr9_5 = expr9_5->getBaseAddr ();
    } else {
        constExpr9_5 = 95;
    }

    call9_args.push_back(&constExpr9_5);

    BPatch_funcCallExpr call9Expr(*call9_func, call9_args);

    checkCost(call9Expr);
    appThread->insertSnippet(call9Expr, *point9_1, BPatch_callBefore, BPatch_lastSnippet);
}

//
// Start Test Case #10 - mutator side (insert snippet order)
//
void mutatorTest10(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func10_1"
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func10_1", found_funcs)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func10_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func10_1");
    }

    BPatch_Vector<BPatch_point *> *point10_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point10_1 || ((*point10_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func10_1\".\n");
	exit(1);
    }

    BPatch_function *call10_1_func;
    call10_1_func = appImage->findFunction("call10_1");

    if (call10_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call10_1.\"\n");
	exit(1);
    }

    BPatch_function *call10_2_func;
    call10_2_func = appImage->findFunction("call10_2");

    if (call10_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call10_2.\"\n");
	exit(1);
    }

    BPatch_function *call10_3_func;
    call10_3_func = appImage->findFunction("call10_3");

    if (call10_3_func == NULL) {
	fprintf(stderr, "Unable to find function \"call10_3.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call10_1Expr(*call10_1_func, nullArgs);
    BPatch_funcCallExpr call10_2Expr(*call10_2_func, nullArgs);
    BPatch_funcCallExpr call10_3Expr(*call10_3_func, nullArgs);

    checkCost(call10_2Expr);
    appThread->insertSnippet( call10_2Expr, *point10_1);

    checkCost(call10_1Expr);
    appThread->insertSnippet( call10_1Expr, *point10_1, BPatch_callBefore, 
							BPatch_firstSnippet);

    checkCost(call10_3Expr);
    appThread->insertSnippet( call10_3Expr, *point10_1, BPatch_callBefore, 
							BPatch_lastSnippet);
}

//
// Start Test Case #11 - mutator side (snippets at entry,exit,call)
//
void mutatorTest11(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func11_1"
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func11_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func11_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func11_1");
    }

    BPatch_Vector<BPatch_point *> *point11_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point11_1 || (point11_1->size() < 1)) {
	fprintf(stderr, "Unable to find point func11_1 - entry.\n");
	exit(-1);
    }

    // Find the subroutine points for the procedure "func11_1"
    BPatch_Vector<BPatch_point *> *point11_2 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!point11_2 || (point11_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func11_1 - calls.\n");
	exit(-1);
    }

    // Find the exit point to the procedure "func11_1"
    BPatch_Vector<BPatch_point *> *point11_3 = found_funcs[0]->findPoint(BPatch_exit);
 
    if (!point11_3 || (point11_3->size() < 1)) {
	fprintf(stderr, "Unable to find point func11_1 - exit.\n");
	exit(-1);
    }

    BPatch_function *call11_1_func;
    call11_1_func = appImage->findFunction("call11_1");

    if (call11_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_1.\"\n");
	exit(1);
    }

    BPatch_function *call11_2_func;
    call11_2_func = appImage->findFunction("call11_2");

    if (call11_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_2.\"\n");
	exit(1);
    }

    BPatch_function *call11_3_func;
    call11_3_func = appImage->findFunction("call11_3");

    if (call11_3_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_3.\"\n");
	exit(1);
    }

    BPatch_function *call11_4_func;
    call11_4_func = appImage->findFunction("call11_4");

    if (call11_4_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_4.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call11_1Expr(*call11_1_func, nullArgs);
    BPatch_funcCallExpr call11_2Expr(*call11_2_func, nullArgs);
    BPatch_funcCallExpr call11_3Expr(*call11_3_func, nullArgs);
    BPatch_funcCallExpr call11_4Expr(*call11_4_func, nullArgs);

    checkCost(call11_1Expr);
    appThread->insertSnippet(call11_1Expr, *point11_1);

    checkCost(call11_2Expr);
    appThread->insertSnippet(call11_2Expr, *point11_2, BPatch_callBefore);

    checkCost(call11_3Expr);
    appThread->insertSnippet(call11_3Expr, *point11_2, BPatch_callAfter);

    checkCost(call11_4Expr);
    appThread->insertSnippet(call11_4Expr, *point11_3);
}

BPatchSnippetHandle *snippetHandle12_1;
BPatch_variableExpr *varExpr12_1;

const int HEAP_TEST_UNIT_SIZE = 5000;

//
// Start Test Case #12 - mutator side (insert/remove and malloc/free)
//
void mutatorTest12a(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func12_2"
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func12_2", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func12_2");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func12_2");
    }

    BPatch_Vector<BPatch_point *> *point12_2 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point12_2 || (point12_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func12_2 - entry.\n");
	exit(-1);
    }

    varExpr12_1 = appThread->malloc(100);
    if (!varExpr12_1) {
	fprintf(stderr, "Unable to allocate 100 bytes in mutatee\n");
	exit(-1);
    }

    // Heap stress test - allocate memory until we run out, free it all
    //   and then allocate a small amount of memory.
    expectError = 66; // We're expecting a heap overflow error
    BPatch_variableExpr* memStuff[30000];
    BPatch_variableExpr *temp;
    temp = appThread->malloc(HEAP_TEST_UNIT_SIZE); 
    int count = 0;
    while (temp) {
#if defined(USES_DYNAMIC_INF_HEAP)
        if (! temp) {
	     printf("*** Inferior malloc stress test failed\n"); 
	     exit(-1);
	}
#endif /* USES_DYNAMIC_INF_HEAP */
	memStuff[count++] = temp;
	temp = appThread->malloc(HEAP_TEST_UNIT_SIZE);
#if defined(USES_DYNAMIC_INF_HEAP)
	// heap will grow indefinitely on dynamic heap platforms
	//if (count == 10000) break;
	// I get tired of waiting
	if (count == 500) break;
#endif /* USES_DYNAMIC_INF_HEAP */
	assert(count < 30000);
    }
    expectError = DYNINST_NO_ERROR;

    int freeCount = 0;
    for (int i =0; i < count; i++) {
	appThread->free(*memStuff[i]);
	freeCount++;
    }

    temp = appThread->malloc(500); 
    if (!temp) {
	printf("*** Unable to allocate memory after using then freeing heap\n");
    }

    BPatch_function *call12_1_func;
    call12_1_func = appImage->findFunction("call12_1");

    if (call12_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call12_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call12_1Expr(*call12_1_func, nullArgs);

    checkCost(call12_1Expr);
    snippetHandle12_1 = appThread->insertSnippet(call12_1Expr, *point12_2);
    if (!snippetHandle12_1) {
	fprintf(stderr,
		"Unable to insert snippet to call function \"call12_1.\"\n");
	exit(-1);
    }
}

void mutatorTest12b(BPatch_thread *appThread, BPatch_image * /*appImage*/)
{
    waitUntilStopped(bpatch, appThread, 12, "insert/remove and malloc/free");

    // remove instrumentation and free memory
    if (!appThread->deleteSnippet(snippetHandle12_1)) {
	printf("**Failed test #12 (insert/remove and malloc/free)\n");
	printf("    deleteSnippet returned an error\n");
	exit(-1);
    }
    appThread->free(*varExpr12_1);

    // continue process
    appThread->continueExecution();
}

//
// Start Test Case #13 - mutator side (paramExpr,retExpr,nullExpr)
//
void mutatorTest13(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func13_1"
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func13_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func13_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func13_1");
    }

    BPatch_Vector<BPatch_point *> *point13_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point13_1 || (point13_1->size() < 1)) {
	fprintf(stderr, "Unable to find point func13_1 - entry.\n");
	exit(-1);
    }

    BPatch_function *call13_1_func;
    call13_1_func = appImage->findFunction("call13_1");

    if (call13_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call13_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs;

    funcArgs.push_back(new BPatch_paramExpr(0));
    funcArgs.push_back(new BPatch_paramExpr(1));
    funcArgs.push_back(new BPatch_paramExpr(2));
    funcArgs.push_back(new BPatch_paramExpr(3));
    funcArgs.push_back(new BPatch_paramExpr(4));
    BPatch_funcCallExpr call13_1Expr(*call13_1_func, funcArgs);

    checkCost(call13_1Expr);
    appThread->insertSnippet(call13_1Expr, *point13_1);

    BPatch_nullExpr call13_2Expr;
    checkCost(call13_2Expr);
    appThread->insertSnippet(call13_2Expr, *point13_1);

    // now test that a return value can be read.
  BPatch_Vector<BPatch_function *> found_funcs2;
    if ((NULL == appImage->findBPFunction("func13_2", found_funcs2)) || (0 == found_funcs2.size())) {
        fprintf(stderr, "    Unable to find function %s\n",
	      "func13_2");
      exit(1);
    }

    if (1 < found_funcs2.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs2.size(), "func13_2");
    }

    BPatch_Vector<BPatch_point *> *point13_2 = found_funcs2[0]->findPoint(BPatch_exit);

    if (!point13_2 || (point13_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func13_2 - exit.\n");
	exit(-1);
    }

    BPatch_function *call13_2_func;
    call13_2_func = appImage->findFunction("call13_2");

    if (call13_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call13_2.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs2;

    BPatch_variableExpr *expr13_1;
    BPatch_retExpr *ret_var;
    BPatch_constExpr expr13_2 (0);

    if (mutateeFortran) {
        expr13_1 = appThread->malloc (*appImage->findType ("int"));
        ret_var = new BPatch_retExpr();
        BPatch_arithExpr test_arith (BPatch_assign, *expr13_1, *ret_var);
        appThread->insertSnippet (test_arith, *point13_2);
        expr13_2 = expr13_1->getBaseAddr ();
        funcArgs2.push_back (&expr13_2);
    } else {
        funcArgs2.push_back(new BPatch_retExpr());
    }

    BPatch_funcCallExpr call13_3Expr(*call13_2_func, funcArgs2);

    checkCost(call13_1Expr);
    appThread->insertSnippet(call13_3Expr, *point13_2, BPatch_callAfter, BPatch_lastSnippet);
}

//
// Start Test Case #14 - mutator side (replace function call)
//
void mutatorTest14(BPatch_thread *appThread, BPatch_image *appImage)
{
    replaceFunctionCalls(appThread, appImage, "func14_1", "func14_2", "call14_1", 
	14, "replace/remove function call", 1);
    replaceFunctionCalls(appThread, appImage, "func14_1", "func14_3", NULL,
	14, "replace/remove function call", 1);
}

//
// Start Test Case #15 - mutator side (setMutationsActive)
//
void mutatorTest15a(BPatch_thread *appThread, BPatch_image *appImage)
{
    insertCallSnippetAt(appThread, appImage, "func15_2", BPatch_entry,
            "call15_1", 15, "setMutationsActive");

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    // On the Sparc, functions containing system calls are relocated into the
    // heap when instrumented, making a special case we should check.

    // "access" makes the "access" system call, so we'll instrument it
    insertCallSnippetAt(appThread, appImage, "access", BPatch_entry,
	"call15_2", 15, "setMutationsActive");

    // We want to instrument more than one point, so do exit as well
    insertCallSnippetAt(appThread, appImage, "access", BPatch_exit,
	"call15_2", 15, "setMutationsActive");
#endif

    replaceFunctionCalls(appThread, appImage, "func15_4", "func15_3",
	"call15_3", 15, "setMutationsActive", 1);
}

void mutatorTest15b(BPatch_thread *appThread, BPatch_image * /*appImage*/)
{
    waitUntilStopped(bpatch, appThread, 15, "setMutationsActive");

    // disable mutations and continue process
    appThread->setMutationsActive(false);
    appThread->continueExecution();
    
    waitUntilStopped(bpatch, appThread, 15, "setMutationsActive");

    // re-enable mutations and continue process
    appThread->setMutationsActive(true);
    appThread->continueExecution();
}

BPatch_Vector<BPatch_snippet *> genLongExpr(BPatch_arithExpr *tail)
{
    BPatch_Vector<BPatch_snippet *> *ret;
    
    ret = new(BPatch_Vector<BPatch_snippet *>);
    for (int i=0; i < 1000; i++) {
	ret->push_back(tail);
    }
    return *ret;
}

//
// Start Test Case #16 - mutator side (if-else)
//
void mutatorTest16(BPatch_thread *appThread, BPatch_image *appImage)
{

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func16_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func16_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func16_1");
    }

    BPatch_Vector<BPatch_point *> *func16_1 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!func16_1 || ((*func16_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func16_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr16_1, *expr16_2, *expr16_3, *expr16_4, *expr16_5,
        *expr16_6, *expr16_7, *expr16_8, *expr16_9, *expr16_10;

    expr16_1 = findVariable(appImage, "globalVariable16_1", func16_1);
    expr16_2 = findVariable(appImage, "globalVariable16_2", func16_1);
    expr16_3 = findVariable(appImage, "globalVariable16_3", func16_1);
    expr16_4 = findVariable(appImage, "globalVariable16_4", func16_1);
    expr16_5 = findVariable(appImage, "globalVariable16_5", func16_1);
    expr16_6 = findVariable(appImage, "globalVariable16_6", func16_1);
    expr16_7 = findVariable(appImage, "globalVariable16_7", func16_1);
    expr16_8 = findVariable(appImage, "globalVariable16_8", func16_1);
    expr16_9 = findVariable(appImage, "globalVariable16_9", func16_1);
    expr16_10 = findVariable(appImage, "globalVariable16_10", func16_1);

    if (!expr16_1 || !expr16_2 || !expr16_3 || !expr16_4 || !expr16_5 ||
        !expr16_6 || !expr16_7 || !expr16_8 || !expr16_9 || !expr16_10) {
	fprintf(stderr, "**Failed** test #16 (if-else)\n");
	fprintf(stderr, "    Unable to locate one of globalVariable16_?\n");
	exit(1);
    }

    BPatch_arithExpr assign16_1(BPatch_assign, *expr16_1, BPatch_constExpr(1));
    BPatch_arithExpr assign16_2(BPatch_assign, *expr16_2, BPatch_constExpr(1));

    BPatch_ifExpr if16_2(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(1),
        BPatch_constExpr(1)), assign16_1, assign16_2);

    BPatch_arithExpr assign16_3(BPatch_assign, *expr16_3, BPatch_constExpr(1));
    BPatch_arithExpr assign16_4(BPatch_assign, *expr16_4, BPatch_constExpr(1));

    BPatch_ifExpr if16_3(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), assign16_3, assign16_4);

    BPatch_arithExpr assign16_5(BPatch_assign, *expr16_5, BPatch_constExpr(1));
    BPatch_arithExpr assign16_6(BPatch_assign, *expr16_6, BPatch_constExpr(1));
    BPatch_sequence longExpr16_1(genLongExpr(&assign16_5));


    BPatch_ifExpr if16_4(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), longExpr16_1, assign16_6);

    BPatch_arithExpr assign16_7(BPatch_assign, *expr16_7, BPatch_constExpr(1));
    BPatch_arithExpr assign16_8(BPatch_assign, *expr16_8, BPatch_constExpr(1));
    BPatch_sequence longExpr16_2(genLongExpr(&assign16_8));

    BPatch_ifExpr if16_5(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), assign16_7, longExpr16_2);

    BPatch_arithExpr assign16_9(BPatch_assign, *expr16_9, BPatch_constExpr(1));
    BPatch_arithExpr assign16_10(BPatch_assign, *expr16_10,BPatch_constExpr(1));
    BPatch_sequence longExpr16_3(genLongExpr(&assign16_9));
    BPatch_sequence longExpr16_4(genLongExpr(&assign16_10));

    BPatch_ifExpr if16_6(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), longExpr16_3, longExpr16_4);

    insertSnippetAt(appThread, appImage, "func16_2", BPatch_entry, if16_2,
	16, "if-else");
    insertSnippetAt(appThread, appImage, "func16_3", BPatch_entry, if16_3,
	16, "if-else");
    insertSnippetAt(appThread, appImage, "func16_4", BPatch_entry, if16_4,
	16, "if-else");
    insertSnippetAt(appThread, appImage, "func16_4", BPatch_entry, if16_5,
	16, "if-else");
    insertSnippetAt(appThread, appImage, "func16_4", BPatch_entry, if16_6,
	16, "if-else");
}

//
// Start Test Case #17 - mutator side (return values from func calls)
// Verify that instrumentation inserted at a subroutine's exit point
// doesn't clobber its return value.
// Method: the mutatee's func17_1 (first and only) exit is instrumented to
// call call17_1 with parameter (constant) "1"; func17_2's (first and only)
// exit is similarly instrumented to call call17_2(1); a subsequent test in
// the mutatee compares the return values of func17_1 and func17_2.
// (No examination is made of the return values of call17_1 or call17_2.)
//
void mutatorTest17(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func17_1"
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func17_1", found_funcs)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func17_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func17_1");
    }

    BPatch_Vector<BPatch_point *> *point17_1 = found_funcs[0]->findPoint(BPatch_exit);

    if (!point17_1 || (point17_1->size() < 1)) {
	fprintf(stderr, "Unable to find point func17_1 - exit.\n");
	exit(-1);
    }

    BPatch_function *call17_1_func;
    call17_1_func = appImage->findFunction("call17_1");

    if (call17_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call17_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs;

    BPatch_variableExpr *var17_1 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr17_1 (0);
    BPatch_arithExpr arithExpr17_1 (BPatch_assign, *var17_1, BPatch_constExpr (1));
    appThread->insertSnippet (arithExpr17_1, *point17_1);

    if (mutateeFortran) {
        constExpr17_1 = var17_1->getBaseAddr ();
    } else {
        constExpr17_1 = 1;
    }

    funcArgs.push_back (&constExpr17_1);

    BPatch_funcCallExpr call17_1Expr(*call17_1_func, funcArgs);
    checkCost(call17_1Expr);
    appThread->insertSnippet(call17_1Expr, *point17_1, BPatch_callAfter, BPatch_lastSnippet);

    // Find the exit point to the procedure "func17_2"
  BPatch_Vector<BPatch_function *> found_funcs2;
    if ((NULL == appImage->findBPFunction("func17_2", found_funcs2)) || (0 == found_funcs2.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func17_2");
      exit(1);
    }

    if (1 < found_funcs2.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs2.size(), "func17_2");
    }

    BPatch_Vector<BPatch_point *> *point17_2 = found_funcs2[0]->findPoint(BPatch_exit);

    if (!point17_2 || (point17_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func17_2 - exit.\n");
	exit(-1);
    }

    BPatch_function *call17_2_func;
    call17_2_func = appImage->findFunction("call17_2");

    if (call17_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call17_2.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs2;

    BPatch_variableExpr *var17_2 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr17_2 (0);
    BPatch_arithExpr arith17_2 (BPatch_assign, *var17_2, BPatch_constExpr (1));
    appThread->insertSnippet (arith17_2, *point17_2);

    if (mutateeFortran) {
        constExpr17_2 = var17_2->getBaseAddr ();
    } else {
        constExpr17_2 = 1;
    }

    funcArgs2.push_back (&constExpr17_2);

    BPatch_funcCallExpr call17_2Expr(*call17_2_func, funcArgs2);
    checkCost(call17_2Expr);

    // test interface to call into insertSnippet with only one parameter
    BPatch_point aPoint = *(*point17_2)[0];
    appThread->insertSnippet(call17_2Expr, aPoint, BPatch_callAfter, BPatch_lastSnippet);
}

//
// Start Test Case #18 - mutator side (read/write a variable in the mutatee)
//
void mutatorTest18(BPatch_thread *appThread, BPatch_image *appImage)
{
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func18_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func18_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func18_1");
    }

    BPatch_Vector<BPatch_point *> *func18_1 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!func18_1 || ((*func18_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func18_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr18_1 = findVariable(appImage, "globalVariable18_1", func18_1);

/* Initialization must be done, because C would have done initialization at declaration */
    if (mutateeFortran) {
        BPatch_arithExpr arith18_1 (BPatch_assign, *expr18_1, BPatch_constExpr (42));
        appThread->oneTimeCode (arith18_1);
    }

    if (expr18_1 == NULL) {
	fprintf(stderr, "**Failed** test #18 (read/write a variable in the mutatee)\n");
	fprintf(stderr, "    Unable to locate globalVariable18_1\n");
	exit(1);
    }

    int n;
    expr18_1->readValue(&n);

    if (n != 42) {
	fprintf(stderr, "**Failed** test #18 (read/write a variable in the mutatee)\n");
	fprintf(stderr, "    value read from globalVariable18_1 was %d, not 42 as expected\n", n);
	exit(1);
    }

    n = 17;
    expr18_1->writeValue(&n,true); //ccw 31 jul 2002
}

void test19_oneTimeCodeCallback(BPatch_thread *thread,
				void *userData,
				void *returnValue)
{
    *(int *)userData = 1;
}

//
// Start Test Case #19 - mutator side (oneTimeCode)
//
void mutatorTest19(BPatch_thread *appThread, BPatch_image *appImage)
{
    waitUntilStopped(bpatch, appThread, 19, "oneTimeCode");

    BPatch_function *call19_1_func;
    call19_1_func = appImage->findFunction("call19_1");

    if (call19_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call19_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call19_1Expr(*call19_1_func, nullArgs);
    checkCost(call19_1Expr);

    appThread->oneTimeCode(call19_1Expr);

    // Let the mutatee run to check the result
    appThread->continueExecution();

    // Wait for the next test
    waitUntilStopped(bpatch, appThread, 19, "oneTimeCode");

    BPatch_function *call19_2_func;
    call19_2_func = appImage->findFunction("call19_2");

    if (call19_2_func == NULL) {
        fprintf(stderr, "Unable to find function \"call19_2.\"\n");
        exit(1);
    }

    BPatch_funcCallExpr call19_2Expr(*call19_2_func, nullArgs);
    checkCost(call19_2Expr);

    int callbackFlag = 0;

    // Register a callback that will set the flag callbackFlag
    BPatchOneTimeCodeCallback oldCallback = 
	bpatch->registerOneTimeCodeCallback(test19_oneTimeCodeCallback);

    appThread->oneTimeCodeAsync(call19_2Expr, (void *)&callbackFlag);

    // Wait for the callback to be called
    while (!appThread->isTerminated() && !callbackFlag) ;

    // Restore old callback (if there was one)
    bpatch->registerOneTimeCodeCallback(oldCallback);

    // Let the mutatee run to check the result and then go on to the next test
    appThread->continueExecution();
}

//
// Start Test Case #20 - mutator side (instrumentation at arbitrary points)
//
void mutatorTest20(BPatch_thread *appThread, BPatch_image *appImage)
{
    BPatch_function *call20_1_func;
    call20_1_func = appImage->findFunction("call20_1");

    if (call20_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call20_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call20_1Expr(*call20_1_func, nullArgs);
    checkCost(call20_1Expr);

    BPatch_function *f;
    f = appImage->findFunction("func20_2");

    if (f == NULL) {
	fprintf(stderr, "Unable to find function \"func20_2.\"\n");
	exit(1);
    }

    BPatch_point *p = NULL;
    bool found_one = false;

    if (f->getSize() == 0) {
	fprintf(stderr, "**Failed** test #20 (instrumentation at arbitrary points)\n");
	fprintf(stderr, "    getSize returned a size of 0 for the function \"func20_2\"\n");
	exit(1);
    }

    /* We expect certain errors from createInstPointAtAddr. */
    BPatchErrorCallback oldError =
	bpatch->registerErrorCallback(createInstPointError);

    for (unsigned int i = 0; i < f->getSize(); i+= 1) {
	p = appImage->createInstPointAtAddr((char *)f->getBaseAddr() + i);

	if (p) {
	    if (p->getPointType() == BPatch_instruction) {
		found_one = true;
		if (appThread->insertSnippet(call20_1Expr, *p) == NULL) {
		    fprintf(stderr,
		      "Unable to insert snippet into function \"func20_2.\"\n");
		    exit(1);
		}
	    }
	}
    }

    bpatch->registerErrorCallback(oldError);

    if (!found_one) {
	fprintf(stderr, "Unable to find a point to instrument in function \"func20_2.\"\n");
	exit(1);
    }
}


//
// Start Test Case #21 - mutator side (findFunction in module)
//
// There is no corresponding failure (test2) testing because the only
// bad input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.

void readyTest21or22(BPatch_thread *appThread)
{
    char libA[128], libB[128];
    sprintf(libA, "./%s", libNameA);
    sprintf(libB, "./%s", libNameB);
#if !defined(i386_unknown_nt4_0) && !defined(alpha_dec_osf4_0)
    if (!mutateeFortran) {
	if (! appThread->loadLibrary(libA)) {
	     fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	     fprintf(stderr, "  Mutator couldn't load %s into mutatee\n", libNameA);
	     exit(1);
	}
	if (! appThread->loadLibrary(libB)) {
	     fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	     fprintf(stderr, "  Mutator couldn't load %s into mutatee\n", libNameB);
	     exit(1);
	}
    }
#endif
}

void mutatorTest21(BPatch_thread *, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(ia64_unknown_linux2_4) \
 || defined(mips_sgi_irix6_4) \
 || defined(rs6000_ibm_aix4_1)

    // Lookup the libtestA.so and libtestB.so modules that we've just loaded

    if (mutateeFortran) {
	return;
    }

    BPatch_module *modA = NULL;
    BPatch_module *modB = NULL;
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    if (!mods || mods->size() == 0) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't search modules of mutatee\n");
	 exit(1);
    }

    for (unsigned int i = 0; i < mods->size() && !(modA && modB); i++) {
	 char buf[1024];
	 BPatch_module *m = (*mods)[i];
	 m->getName(buf, 1024);
	 // module names sometimes have "_module" appended
	 if (!strncmp(libNameA, buf, strlen(libNameA)))
	      modA = m;
	 else if (!strncmp(libNameB, buf, strlen(libNameB)))
	      modB = m;
    }
    if (! modA || ! modB) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't find shlib in mutatee\n");
	 fflush(stdout);
	 exit(1);
    }

    // Find the function CALL21_1 in each of the modules
    BPatch_function *funcA = modA->findFunction("call21_1");
    BPatch_function *funcB = modB->findFunction("call21_1");
    if (! funcA) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't find a function in %s\n", libNameA);
	 exit(1);
    }
    if (! funcB) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't find a function in %s\n", libNameB);
	 exit(1);
    }
    // Kludgily test whether the functions are distinct
    if (funcA->getBaseAddr() == funcB->getBaseAddr()) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr,
	        "  Mutator cannot distinguish two functions from different shlibs\n");
	 exit(1);
    }
#endif
}


//
// Start Test Case #22 - mutator side (replace function)
//
// There is no corresponding failure (test2) testing because the only
// invalid input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.
void mutatorTest22(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)

    if (mutateeFortran) {
	return;
    }

    char errbuf[1024]; errbuf[0] = '\0';
    BPatch_module *modA = NULL;
    BPatch_module *modB = NULL;

    // Assume that a prior test (mutatorTest21) has loaded the
    // libraries libtestA.so and libtestB.so into the mutator.
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    if (!mods || mods->size() == 0) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find shlib in mutatee\n");
         exit(1);
    }
    // Lookup the libtestA.so and libtestB.so modules
    for (unsigned int i = 0; i < mods->size() && !(modA && modB); i++) {
	 char buf[1024];
	 BPatch_module *m = (*mods)[i];
	 m->getName(buf, 1024);
	 // module names sometimes have "_module" appended
	 if (!strncmp(libNameA, buf, strlen(libNameA)))
	      modA = m;
	 else if (!strncmp(libNameB, buf, strlen(libNameB)))
	      modB = m;
    }
    if (! modA || ! modB) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find dynamically loaded modules\n");
	 exit(1);
    }
    
    //  Mutatee function replacement scheme:

    //  function      module     replaced    global
    //                         or called?   

    //  call22_1       a.out     replaced         1       global is the index
    //  call22_2       a.out       called         1       of the global variable
    //  call22_3       a.out     replaced         2       in test1.mutatee updated
    //  call22_4    libtestA       called         2       by the function
    //  call22_5A   libtestA     replaced         3
    //  call22_5B   libtestB       called         3
    //  call22_6    libtestA     replaced         4
    //  call22_7       a.out       called         4

    // Both of each pair of functions (e.g., call22_1, call22_2)
    // increments a global variable.  The mutatee will test that the
    // variable has been updated only be the "called" function.

    // Replace an a.out with another a.out function
    BPatch_function *call22_1func = appImage->findFunction("call22_1");
    BPatch_function *call22_2func = appImage->findFunction("call22_2");
    if (! call22_1func || ! call22_2func) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 exit(1);
    }
    if (! appThread->replaceFunction(*call22_1func, *call22_2func)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (a.out -> a.out)\n");
	 exit(1);
    }

    // Replace an a.out function with a shlib function
    BPatch_function *call22_3func = appImage->findFunction("call22_3");
    BPatch_function *call22_4func = modA->findFunction("call22_4");
    if (! call22_3func || ! call22_4func) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 exit(1);
    }
    if (! appThread->replaceFunction(*call22_3func, *call22_4func)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (a.out -> shlib)\n");
	 exit(1);
    }

    // Replace a shlib function with a shlib function
    BPatch_function *call22_5Afunc = modA->findFunction("call22_5");
    BPatch_function *call22_5Bfunc = modB->findFunction("call22_5");
    if (! call22_5Afunc || ! call22_5Bfunc) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 exit(1);
    }
    if (! appThread->replaceFunction(*call22_5Afunc, *call22_5Bfunc)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (shlib -> shlib)\n");
    }

    // Replace a shlib function with an a.out function
    BPatch_function *call22_6func = modA->findFunction("call22_6");
    BPatch_function *call22_7func = appImage->findFunction("call22_7");
    if (! call22_6func || ! call22_7func) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 exit(1);
    }
    if (! appThread->replaceFunction(*call22_6func, *call22_7func)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (shlib -> a.out)\n");
	 exit(1);
    }

#endif
}

//
// Start Test Case #23 - local variables
//
void mutatorTest23(BPatch_thread *appThread, BPatch_image *appImage)
{
#if !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0)
    if (!mutateeFortran) {
        //     First verify that we can find a local variable in call23_1
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("call23_1", found_funcs, 1)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "call23_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "call23_1");
    }

    BPatch_Vector<BPatch_point *> *point23_1 = found_funcs[0]->findPoint(BPatch_subroutine);

        assert(point23_1);

        BPatch_variableExpr *var1 = appImage->findVariable(*(*point23_1)[0],
            "localVariable23_1");
        BPatch_variableExpr *var2 = appImage->findVariable(*(*point23_1)[0],
            "shadowVariable23_1");
        BPatch_variableExpr *var3 = appImage->findVariable("shadowVariable23_2");
        BPatch_variableExpr *var4 = appImage->findVariable("globalVariable23_1");

        if (!var1 || !var2 || !var3 || !var4) {
            fprintf(stderr, "**Failed** test #23 (local variables)\n");
            if (!var1)
                fprintf(stderr, "  can't find local variable localVariable23_1\n");
            if (!var2)
                fprintf(stderr, "  can't find local variable shadowVariable23_1\n");
            if (!var3)
                fprintf(stderr,"  can't find global variable shadowVariable23_2\n");
            return;
        }

        BPatch_arithExpr expr23_1(BPatch_assign, *var1, BPatch_constExpr(2300001));
        BPatch_arithExpr expr23_2(BPatch_assign, *var2, BPatch_constExpr(2300012));
        BPatch_arithExpr expr23_3(BPatch_assign, *var3, BPatch_constExpr(2300023));
        BPatch_arithExpr expr23_4(BPatch_assign, *var4, *var1);

        BPatch_Vector<BPatch_snippet *> exprs;

        exprs.push_back(&expr23_4); // put this one first so it isn't clobbered
        exprs.push_back(&expr23_1);
        exprs.push_back(&expr23_2);
        exprs.push_back(&expr23_3);

        BPatch_sequence allParts(exprs);
	
	// this should not be needed???  JAW
	//BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);

	if (!point23_1 || (point23_1->size() < 1)) {
            fprintf(stderr, "**Failed** test #23 (local variables)\n");
            fprintf(stderr, "  Unable to find point call23_1 - subroutine calls\n");
            exit(1);
        }
        appThread->insertSnippet(allParts, *point23_1);
    }
#endif
}

//
// Start Test Case #24 - array variables
//
void mutatorTest24(BPatch_thread *appThread, BPatch_image *appImage)
{
#if !defined(mips_sgi_irix6_4)
    if (!mutateeFortran) {
        //     First verify that we can find function call24_1
        BPatch_function *call24_1func = appImage->findFunction("call24_1");
        if (call24_1func == NULL) {
            fprintf(stderr, "Unable to find function \"call24_1\".\n");
           return;
        }

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("call24_1", found_funcs, 1)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "call24_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "call24_1");
    }

    BPatch_Vector<BPatch_point *> *temp = found_funcs[0]->findPoint(BPatch_subroutine);

        //     Then verify that we can find a local variable in call24_1
    if (!temp) {
            fprintf(stderr, "**Failed** test #24 (array variables)\n");
            fprintf(stderr, "  can't find function call24_1\n");
            return;
        } else {
            dprintf("Found %d callsites in function call24_1\n", temp->size());
        }

        BPatch_Vector<BPatch_point *> *point24_1  =
	    new(BPatch_Vector<BPatch_point *>);
        point24_1->push_back((*temp)[0]);

	BPatch_Vector<BPatch_point *> *point24_2 = found_funcs[0]->findPoint(BPatch_exit);
	BPatch_Vector<BPatch_point *> *point24_3 = found_funcs[0]->findPoint(BPatch_entry);
 
	assert(point24_1 && point24_2 && point24_3);

        BPatch_variableExpr *lvar;
        BPatch_variableExpr *gvar[10];

        for (int i=1; i <= 9; i++) {
            char name[80];

            sprintf(name, "globalVariable24_%d", i);
            gvar[i] = appImage->findVariable(name);
            if (!gvar[i]) {
                fprintf(stderr, "**Failed** test #24 (array variables)\n");
                fprintf(stderr, "  can't find variable globalVariable24_%d\n", i);
                return;
            }
        }

        lvar = appImage->findVariable(*(*point24_1)[0], "localVariable24_1");
        if (!lvar) {
            fprintf(stderr, "**Failed** test #24 (array variables)\n");
            fprintf(stderr, "  can't find variable localVariable24_1\n");
            return;
        }

        //     globalVariable24_1[1] = 2400001
        BPatch_arithExpr assignment1(BPatch_assign,
            BPatch_arithExpr(BPatch_ref, *gvar[1], BPatch_constExpr(1)),
        BPatch_constExpr(2400001));
        appThread->insertSnippet(assignment1, *point24_1);

        //     globalVariable24_1[globalVariable24_2] = 2400002
        BPatch_arithExpr assignment2(BPatch_assign,
            BPatch_arithExpr(BPatch_ref, *gvar[1], *gvar[2]),
        BPatch_constExpr(2400002));
        appThread->insertSnippet(assignment2, *point24_1);

        //     globalVariable24_3 = globalVariable24_1[79]
        BPatch_arithExpr assignment3(BPatch_assign, *gvar[3],
            BPatch_arithExpr(BPatch_ref, *gvar[1], BPatch_constExpr(79)));
        appThread->insertSnippet(assignment3, *point24_1);

        //     globalVariable24_5 = globalVariable24_1[globalVariable24_4]
        BPatch_arithExpr assignment4(BPatch_assign, *gvar[5],
            BPatch_arithExpr(BPatch_ref, *gvar[1], *gvar[4]));
        appThread->insertSnippet(assignment4, *point24_1);

        // now the local variables
        //     localVariable24_1[1] = 2400005
        BPatch_arithExpr assignment5(BPatch_assign,
            BPatch_arithExpr(BPatch_ref, *lvar, BPatch_constExpr(1)),
            BPatch_constExpr(2400005));
        appThread->insertSnippet(assignment5, *point24_1);

        //     localVariable24_1[globalVariable24_2] = 2400006
        BPatch_arithExpr assignment6(BPatch_assign,
            BPatch_arithExpr(BPatch_ref, *lvar, *gvar[2]),
            BPatch_constExpr(2400006));
        appThread->insertSnippet(assignment6, *point24_1);

        //     globalVariable24_6 = localVariable24_1[79]
        BPatch_arithExpr assignment7(BPatch_assign, *gvar[6],
            BPatch_arithExpr(BPatch_ref, *lvar, BPatch_constExpr(79)));
        appThread->insertSnippet(assignment7, *point24_1);

        //     globalVariable24_7 = localVariable24_1[globalVariable24_4]
        BPatch_arithExpr assignment8(BPatch_assign, *gvar[7],
            BPatch_arithExpr(BPatch_ref, *lvar, *gvar[4]));
        appThread->insertSnippet(assignment8, *point24_1);

        // now test multi-dimensional arrays
        //	   globalVariable24_8[2][3] = 2400011
        BPatch_arithExpr assignment9(BPatch_assign,
            BPatch_arithExpr(BPatch_ref, BPatch_arithExpr(BPatch_ref, *gvar[8],
	    BPatch_constExpr(2)), BPatch_constExpr(3)), BPatch_constExpr(2400011));
        appThread->insertSnippet(assignment9, *point24_1);

        // globalVariable24_9 = globalVariable24_8[7][9]
        BPatch_arithExpr assignment10(BPatch_assign, *gvar[9],
            BPatch_arithExpr(BPatch_ref, BPatch_arithExpr(BPatch_ref, *gvar[8],
            BPatch_constExpr(7)), BPatch_constExpr(9)));
      appThread->insertSnippet(assignment10, *point24_1);
    }
#endif
}

//
// Start Test Case #25 - unary operators
//
void mutatorTest25(BPatch_thread *appThread, BPatch_image *appImage)
{
	// Used as hack for Fortran to allow assignment of a pointer to an int
	bpatch->setTypeChecking (false);
#if !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0)
    //     First verify that we can find a local variable in call25_1
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("call25_1", found_funcs)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "call25_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "call25_1");
    }

    BPatch_Vector<BPatch_point *> *point25_1 = found_funcs[0]->findPoint(BPatch_entry);

    assert(point25_1);
//    assert(point25_1 && (point25_1->size() == 1));

    BPatch_variableExpr *gvar[8];

    for (int i=1; i <= 7; i++) {
        char name[80];

        sprintf (name, "globalVariable25_%d", i);
        gvar [i] = findVariable (appImage, name, point25_1);

	if (!gvar[i]) {
	    fprintf(stderr, "**Failed** test #25 (unary operaors)\n");
	    fprintf(stderr, "  can't find variable globalVariable25_%d\n", i);
	    exit(-1);
	}
    }

    //     globalVariable25_2 = &globalVariable25_1
#if !defined(sparc_sun_solaris2_4) && \
    !defined(rs6000_ibm_aix4_1) && \
    !defined(alpha_dec_osf4_0) && \
    !defined(i386_unknown_linux2_0) && \
    !defined(ia64_unknown_linux2_4) && \
    !defined(i386_unknown_solaris2_5) && \
    !defined(i386_unknown_nt4_0)

    // without type info need to inform
    BPatch_type *type = appImage->findType("void *");
    assert(type);
    gvar[2]->setType(type);
#endif

    BPatch_arithExpr assignment1(BPatch_assign, *gvar[2],
	BPatch_arithExpr(BPatch_addr, *gvar[1]));

    appThread->insertSnippet(assignment1, *point25_1);

    // 	   globalVariable25_3 = *globalVariable25_2
    //		Need to make sure this happens after the first one
    BPatch_arithExpr assignment2(BPatch_assign, *gvar[3],
	BPatch_arithExpr(BPatch_deref, *gvar[2]));
    appThread->insertSnippet(assignment2, *point25_1,  BPatch_callBefore,
	    BPatch_lastSnippet);

    // 	   globalVariable25_5 = -globalVariable25_4
    BPatch_arithExpr assignment3(BPatch_assign, *gvar[5],
	BPatch_arithExpr(BPatch_negate, *gvar[4]));
    appThread->insertSnippet(assignment3, *point25_1);

    // 	   globalVariable25_7 = -globalVariable25_6
    BPatch_arithExpr assignment4(BPatch_assign, *gvar[7],
	BPatch_arithExpr(BPatch_negate, *gvar[6]));
    appThread->insertSnippet(assignment4, *point25_1);
#endif
	bpatch->setTypeChecking (true);
}


//
// Start Test Case #26 - struct elements
//
void mutatorTest26(BPatch_thread *appThread, BPatch_image *appImage)
{
#if !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0)

    if (!mutateeFortran) {
        //     First verify that we can find a local variable in call26_1
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("call26_1", found_funcs, 1)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "call26_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "call26_1");
    }

    BPatch_Vector<BPatch_point *> *point26_1 = found_funcs[0]->findPoint(BPatch_subroutine);
    BPatch_Vector<BPatch_point *> *point26_2 = found_funcs[0]->findPoint(BPatch_subroutine);

    assert(point26_1 && (point26_1->size() == 1));
        assert(point26_2);

        BPatch_variableExpr *lvar;
        BPatch_variableExpr *gvar[14];

        int i;
        for (i=1; i <= 13; i++) {
            char name[80];

            sprintf (name, "globalVariable26_%d", i);
            gvar [i] = findVariable(appImage, name, point26_2);

            if (!gvar[i]) {
                fprintf(stderr, "**Failed** test #26 (struct elements)\n");
                fprintf(stderr, "  can't find variable globalVariable26_%d\n", i);
                exit(-1);
            }
        }

        // start of code for globalVariable26_1
        BPatch_Vector<BPatch_variableExpr *> *fields = gvar[1]->getComponents();
    	if (!fields) {
	    fprintf(stderr, "**Failed** test #26 (struct elements)\n");
	    fprintf(stderr, "  struct lacked correct number of elements\n");
	    exit(-1);
    	}

        for (i=0; i < 4; i++) {
            char fieldName[80];
            sprintf(fieldName, "field%d", i+1);
            if (!(*fields)[i]->getName())
                printf("NULL NAME!\n");
            if (strcmp(fieldName, (*fields)[i]->getName())) {
                printf("field %d of the struct is %s, not %s\n",
                    i+1, fieldName, (*fields)[i]->getName());
                return;
            }
        }

	    // 	   globalVariable26_2 = globalVariable26_1.field1
    	BPatch_arithExpr assignment1(BPatch_assign, *gvar[2], *((*fields)[0]));
	    appThread->insertSnippet(assignment1, *point26_2);

    	// 	   globalVariable26_3 = globalVariable26_1.field2
	    BPatch_arithExpr assignment2(BPatch_assign, *gvar[3], *((*fields)[1]));
    	appThread->insertSnippet(assignment2, *point26_2);

	    // 	   globalVariable26_4 = globalVariable26_1.field3[0]
    	BPatch_arithExpr assignment3(BPatch_assign, *gvar[4],
		BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
    	appThread->insertSnippet(assignment3, *point26_2);

	    // 	   globalVariable26_5 = globalVariable26_1.field3[5]
    	BPatch_arithExpr assignment4(BPatch_assign, *gvar[5],
		BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
    	appThread->insertSnippet(assignment4, *point26_2);

	    BPatch_Vector<BPatch_variableExpr *> *subfields =
		(*fields)[3]->getComponents();
	    assert(subfields != NULL);

    	// 	   globalVariable26_6 = globalVariable26_1.field4.field1
	    BPatch_arithExpr assignment5(BPatch_assign, *gvar[6], *((*subfields)[0]));
    	appThread->insertSnippet(assignment5, *point26_2);

	    // 	   globalVariable26_7 = globalVariable26_1.field4.field2
    	BPatch_arithExpr assignment6(BPatch_assign, *gvar[7], *((*subfields)[1]));
	    appThread->insertSnippet(assignment6, *point26_2);

    	// start of code for localVariable26_1
        expectError = 100;
        lvar = appImage->findVariable(*(*point26_1) [0], "localVariable26_1");
        if (!lvar)
            lvar = appImage->findVariable(*(*point26_1) [0], "localvariable26_1");
	assert(lvar);
	expectError = DYNINST_NO_ERROR;

    	fields = lvar->getComponents();
    	if (!fields || (fields->size() < 4)) {
	    fprintf(stderr, "**Failed** test #26 (struct elements)\n");
	    fprintf(stderr, "  struct lacked correct number of elements\n");
	    exit(-1);
    	}

	    for (i=0; i < 4; i++) {
		 char fieldName[80];
		 sprintf(fieldName, "field%d", i+1);
		 if (strcmp(fieldName, (*fields)[i]->getName())) {
	    	  printf("field %d of the local struct is %s, not %s\n",
			  i+1, fieldName, (*fields)[i]->getName());
		      return;
		 }
    	}

	    // 	   globalVariable26_8 = localVariable26_1.field1
    	BPatch_arithExpr assignment7(BPatch_assign, *gvar[8], *((*fields)[0]));
	    appThread->insertSnippet(assignment7, *point26_1);

    	// 	   globalVariable26_9 = localVariable26_1.field2
	    BPatch_arithExpr assignment8(BPatch_assign, *gvar[9], *((*fields)[1]));
    	appThread->insertSnippet(assignment8, *point26_1);

	    // 	   globalVariable26_10 = localVariable26_1.field3[0]
    	BPatch_arithExpr assignment9(BPatch_assign, *gvar[10],
		BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
    	appThread->insertSnippet(assignment9, *point26_1);

	    // 	   globalVariable26_11 = localVariable26_1.field3[5]
    	BPatch_arithExpr assignment10(BPatch_assign, *gvar[11],
		BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
    	appThread->insertSnippet(assignment10, *point26_1);

	    subfields = (*fields)[3]->getComponents();
    	assert(subfields != NULL);

	    // 	   globalVariable26_12 = localVariable26_1.field4.field1
    	BPatch_arithExpr assignment11(BPatch_assign, *gvar[12], *((*subfields)[0]));
	    appThread->insertSnippet(assignment11, *point26_1);

	    // 	   globalVariable26_13 = localVariable26_1.field4.field2
    	BPatch_arithExpr assignment12(BPatch_assign, *gvar[13], *((*subfields)[1]));
	    appThread->insertSnippet(assignment12, *point26_1);
    }
#endif
}

//
// Start Test Case #27 - type compatibility
//
void mutatorTest27(BPatch_thread *, BPatch_image *appImage)
{
#if !defined(mips_sgi_irix6_4)

    BPatch_type *type27_1 = appImage->findType("type27_1");
    BPatch_type *type27_2 = appImage->findType("type27_2");
    BPatch_type *type27_3 = appImage->findType("type27_3");
    BPatch_type *type27_4 = appImage->findType("type27_4");

    if (!type27_1 || !type27_2 || !type27_3 || !type27_4) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr, "    Unable to locate one of type27_{1,2,3,4}\n");
	return;
    }

    if (!type27_1->isCompatible(type27_2)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_1 reported as incompatibile with type27_2\n");
	return;
    }

    if (!type27_2->isCompatible(type27_1)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_2 reported as incompatibile with type27_1\n");
	return;
    }

    if (!type27_3->isCompatible(type27_3)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_3 reported as incompatibile with type27_4\n");
	return;
    }

    if (!type27_4->isCompatible(type27_3)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_4 reported as incompatibile with type27_3\n");
	return;
    }

    expectError = 112; // We're expecting type conflicts here
    if (type27_1->isCompatible(type27_3)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_1 reported as compatibile with type27_3\n");
	return;
    }

    if (type27_4->isCompatible(type27_2)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_4 reported as compatibile with type27_2\n");
	return;
    }
    expectError = DYNINST_NO_ERROR;

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func27_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func27_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func27_1");
    }

    BPatch_Vector<BPatch_point *> *point27_1 = found_funcs[0]->findPoint(BPatch_entry);

    assert (point27_1);

    BPatch_variableExpr *expr27_5, *expr27_6, *expr27_7, *expr27_8;

    expr27_5 = findVariable(appImage, "globalVariable27_5", point27_1);
    expr27_6 = findVariable(appImage, "globalVariable27_6", point27_1);
    expr27_7 = findVariable(appImage, "globalVariable27_7", point27_1);
    expr27_8 = findVariable(appImage, "globalVariable27_8", point27_1);

    assert(expr27_5 && expr27_6 && expr27_7 && expr27_8);

    BPatch_type *type27_5 = const_cast<BPatch_type *> (expr27_5->getType());
    BPatch_type *type27_6 = const_cast<BPatch_type *> (expr27_6->getType());
    BPatch_type *type27_7 = const_cast<BPatch_type *> (expr27_7->getType());
    BPatch_type *type27_8 = const_cast<BPatch_type *> (expr27_8->getType());

    assert(type27_5 && type27_6 && type27_7 && type27_8);

    if (!type27_5->isCompatible(type27_6)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_5 reported as incompatibile with type27_6\n");
	return;
    }

    // difderent number of elements
    expectError = 112; // We're expecting type conflicts here
    if (type27_5->isCompatible(type27_7)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_5 reported as compatibile with type27_7\n");
	return;
    }

    // same # of elements, different type
    if (type27_5->isCompatible(type27_8)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_5 reported as compatibile with type27_8\n");
	return;
    }

    // all ok, set the global variable, depends on test 18 working
    BPatch_variableExpr *expr27_1 = findVariable(appImage, "globalVariable27_1", point27_1);

    if (expr27_1 == NULL) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr, "    Unable to locate globalVariable27_1\n");
	return;
    }
    expectError = DYNINST_NO_ERROR;

    int n = 1;
    expr27_1->writeValue(&n, true); //ccw 31 jul 2002
#endif
}

//
// Start Test Case #28 - user defined fields
//
void mutatorTest28(BPatch_thread *appThread, BPatch_image *appImage)
{
    int i;

    //	   Create the types
    BPatch_type *intType = appImage->findType("int");
    assert(intType);

    BPatch_Vector<char *> names;
    BPatch_Vector<BPatch_type *> types;

    if (mutateeFortran) {
	return;
    }

    names.push_back(const_cast<char*>("field1"));
    names.push_back(const_cast<char*>("field2"));
    types.push_back(intType);
    types.push_back(intType);

    //	struct28_1 { int field1, int field 2; }
    BPatch_type *struct28_1 = bpatch->createStruct("struct28_1", names, types);

    names.push_back(const_cast<char*>("field3"));
    names.push_back(const_cast<char*>("field4"));
    BPatch_type *intArray = bpatch->createArray("intArray", intType, 0, 9);
    types.push_back(intArray);
    types.push_back(struct28_1);

    // struct28_2 { int field1, int field 2, int field3[10],struct26_1 field4 } 
    BPatch_type *struct28_2 = bpatch->createStruct("struct28_2", names, types);

    // now create variables of these types.
    BPatch_variableExpr *globalVariable28_1 = 
	appImage->findVariable("globalVariable28_1");
    globalVariable28_1->setType(struct28_2);

    //     Next verify that we can find a local variable in call28
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("call28_1", found_funcs)) || (0 == found_funcs.size())) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "call28_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "call28_1");
    }

    BPatch_Vector<BPatch_point *> *point28 = found_funcs[0]->findPoint(BPatch_entry);

    assert(point28 && (point28->size() == 1));

    BPatch_variableExpr *gvar[8];

    for (i=1; i <= 7; i++) {
	char name[80];

	sprintf(name, "globalVariable28_%d", i);
	gvar[i] = appImage->findVariable(name);
	if (!gvar[i]) {
	    fprintf(stderr, "**Failed** test #28 (user defined fields)\n");
	    fprintf(stderr, "  can't find variable globalVariable28_%d\n", i);
	    exit(-1);
	}
    }

    // start of code for globalVariable28
    BPatch_Vector<BPatch_variableExpr *> *fields = gvar[1]->getComponents();
    assert(fields && (fields->size() == 4));

    for (i=0; i < 4; i++) {
	 char fieldName[80];
	 sprintf(fieldName, "field%d", i+1);
	 if (strcmp(fieldName, (*fields)[i]->getName())) {
	      printf("field %d of the struct is %s, not %s\n",
		  i+1, fieldName, (*fields)[i]->getName());
	      return;
	 }
    }

    // 	   globalVariable28 = globalVariable28.field1
    BPatch_arithExpr assignment1(BPatch_assign, *gvar[2], *((*fields)[0]));
    appThread->insertSnippet(assignment1, *point28);

    // 	   globalVariable28 = globalVariable28.field2
    BPatch_arithExpr assignment2(BPatch_assign, *gvar[3], *((*fields)[1]));
    appThread->insertSnippet(assignment2, *point28);

    // 	   globalVariable28 = globalVariable28.field3[0]
    BPatch_arithExpr assignment3(BPatch_assign, *gvar[4], 
	BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
    appThread->insertSnippet(assignment3, *point28);

    // 	   globalVariable28 = globalVariable28.field3[5]
    BPatch_arithExpr assignment4(BPatch_assign, *gvar[5], 
	BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
    appThread->insertSnippet(assignment4, *point28);

    BPatch_Vector<BPatch_variableExpr *> *subfields = 
	(*fields)[3]->getComponents();
    assert(subfields != NULL);

    // 	   globalVariable28 = globalVariable28.field4.field1
    BPatch_arithExpr assignment5(BPatch_assign, *gvar[6], *((*subfields)[0]));
    appThread->insertSnippet(assignment5, *point28);

    // 	   globalVariable28 = globalVariable28.field4.field2
    BPatch_arithExpr assignment6(BPatch_assign, *gvar[7], *((*subfields)[1]));
    appThread->insertSnippet(assignment6, *point28);
}

bool printSrcObj(BPatch_sourceObj *p, int level)
{
    unsigned int i;
    bool ret = true;

    BPatch_Vector<BPatch_sourceObj *> curr;

    if (!p) return(true);

    switch (p->getSrcType()) {
	case BPatch_sourceProgram:
	    if (level != 0) ret = false;
	    break;

	case BPatch_sourceModule: 
	    if (level != 1) ret = false;
	    break;

	case BPatch_sourceFunction: 
	    if (level != 2) ret = false;
	    break;

	default:
	    printf("<unknown type>");
    }

    if (!p->getSourceObj(curr)) {
	// eveything down to functions should have something
	return((level == 2) ? true : false);
    }

    for (i=0; i < curr.size(); i++) {
	p = curr[i];
	ret = printSrcObj(p, level+1) && ret;
    }

    return ret;
}

//
// Start Test Case #29 - getParent/Child
//
void mutatorTest29(BPatch_thread *, BPatch_image *appImage)
{
    BPatch_sourceObj *p;

    p = (BPatch_sourceObj *) appImage;
    passedTest[29] = printSrcObj(p, 0);

    if (!passedTest[29]) {
	fprintf(stderr, "**Failed** test #29 (class BPatch_srcObj)\n");
	return;
    }

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func29_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func29_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func29_1");
    }

    BPatch_Vector<BPatch_point *> *point29_1 = found_funcs[0]->findPoint(BPatch_entry);

    assert (point29_1);

    BPatch_variableExpr *expr29_1 = findVariable(appImage, "globalVariable29_1", point29_1);

    if (expr29_1 == NULL) {
	fprintf(stderr, "**Failed** test #29 (class BPatch_srcObj)\n");
	fprintf(stderr, "    Unable to locate globalVariable27_1\n");
	return;
    }
    expectError = DYNINST_NO_ERROR;

    int n = 1;
    expr29_1->writeValue(&n,true); //ccw 31 jul 2002
}

//
// Start Test Case #30 - (line information)
//
void mutatorTest30(BPatch_thread *appThread, BPatch_image *appImage)
{

#if defined(sparc_sun_solaris2_4) || \
    defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(ia64_unknown_linux2_4) || \
    defined(i386_unknown_nt4_0) ||\
    defined(rs6000_ibm_aix4_1) || \
    defined(alpha_dec_osf4_0)

  unsigned n;
  unsigned long baseAddr,lastAddr;
  unsigned int call30_1_line_no;
  unsigned short lineNo;
  char fileName[256];

	if (mutateeFortran) {
	    return;
	} 
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findBPFunction("func30_1", found_funcs)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func30_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func30_1");
    }


    BPatch_Vector<BPatch_point *> *point30_1 = found_funcs[0]->findPoint(BPatch_entry);
	//instrument with the function that will set the line number

	if (!point30_1 || (point30_1->size() < 1)) {
		fprintf(stderr, "Unable to find point func30_1 - entry.\n");
		exit(-1);
	}
	BPatch_function *call30_1func = appImage->findFunction("call30_1");
	if (call30_1func == NULL) {
		fprintf(stderr, "Unable to find function \"call30_1.\"\n");
		exit(1);
	}
	BPatch_Vector<BPatch_snippet *> nullArgs;
	BPatch_funcCallExpr call30_1Expr(*call30_1func, nullArgs);

	checkCost(call30_1Expr);
    	appThread->insertSnippet(call30_1Expr, *point30_1);

	//get the line number of the function call30_1
	BPatch_variableExpr *expr30_7 = 
		appImage->findVariable("globalVariable30_7");
	if (expr30_7 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_7\n");
        	exit(1);
    	}
	expr30_7->readValue(&n);
	call30_1_line_no = (unsigned)(n+1);

	//get the base addr and last addr of the function call30_1
	baseAddr = (unsigned long)(call30_1func->getBaseAddr());
	lastAddr = baseAddr + call30_1func->getSize();

	//now write the base address and last address of the function
	BPatch_variableExpr *expr30_8 = 
			appImage->findVariable("globalVariable30_8");
	if (expr30_8 == NULL) {
		fprintf(stderr, "**Failed** test #30 (line information)\n");
		fprintf(stderr, "    Unable to locate globalVariable30_8\n");
	}

	BPatch_variableExpr *expr30_9 = 
			appImage->findVariable("globalVariable30_9");
	if (expr30_9 == NULL) {
		fprintf(stderr, "**Failed** test #30 (line information)\n");
		fprintf(stderr, "    Unable to locate globalVariable30_9\n");
	}

	expr30_8->writeValue(&baseAddr);
	expr30_9->writeValue(&lastAddr);
	
	
	//check getLineAddr for appImage
	BPatch_variableExpr *expr30_3 =
			appImage->findVariable("globalVariable30_3");
	if (expr30_3 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_3\n");
        	exit(1);
    	}
	BPatch_Vector<unsigned long> buffer1; 
	if(appImage->getLineToAddr("test1.mutatee.c",call30_1_line_no,buffer1))
	{
    		n = buffer1[0];
    		expr30_3->writeValue(&n);
	}

	//check getLineAddr for module
	BPatch_variableExpr *expr30_4 =
			appImage->findVariable("globalVariable30_4");
	if (expr30_4 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_4\n");
        	exit(1);
    	}
	BPatch_Vector<BPatch_module*>* appModules = appImage->getModules();
	for(unsigned int i=0;i<appModules->size();i++){
		char mname[256];
		(*appModules)[i]->getName(mname,255);mname[255] = '\0';
		if(!strncmp(mname,"test1.mutatee.c",15)){
			BPatch_Vector<unsigned long> buffer2;
			if((*appModules)[i]->getLineToAddr(
					call30_1_line_no,buffer2))
			{
				n = buffer2[0];
				expr30_4->writeValue(&n);
			}
			break;
		}
	}

	//check getLineAddr works for the function
	BPatch_variableExpr *expr30_5 =
		appImage->findVariable("globalVariable30_5");
	if (expr30_5 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_5\n");
        	exit(1);
	}
	BPatch_Vector<unsigned long> buffer3; 
	if(call30_1func->getLineToAddr(call30_1_line_no,buffer3))
	{
		n = buffer3[0];
		expr30_5->writeValue(&n);
	}

	//check whether getLineFile works for appThread
	BPatch_variableExpr *expr30_6 =
		appImage->findVariable("globalVariable30_6");
	if (expr30_6 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_6\n");
        	exit(1);
	}
	/* since the first line address of a function changes with the
	   compiler type (gcc,native) we need to check with next address
	   etc. Instead I use the last address of the function*/
	if(appThread->getLineAndFile(lastAddr-1,lineNo,fileName,256)){
		n = lineNo;
		expr30_6->writeValue(&n);
	}
#endif
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

typedef BPatch_Vector<BPatch_point * > point_vector;
// typedef vector<BPatchSnippetHandle * > handle_vector;

void instrument_entry_points( BPatch_thread * app_thread,
			      BPatch_image * ,
			      BPatch_function * func,
			      BPatch_snippet * code )
{
  assert( func != 0 );
  assert( code != 0 );

//   handle_vector * list_of_handles = new handle_vector;

  int null_entry_point_count = 0;
  int failed_snippet_insertion_count = 0;

  point_vector * entries = func->findPoint( BPatch_entry );
  assert( entries != 0 );

  for( unsigned int i = 0; i < entries->size(); i++ )
    {
      BPatch_point * point = ( * entries )[ i ];
      if( point == 0 )
	{
	  null_entry_point_count++;
	}
      else
	{
	  BPatchSnippetHandle * result =
	    app_thread->insertSnippet( * code,
				       * point, BPatch_callBefore, BPatch_firstSnippet );
	  if( result == 0 )
	    {
	      failed_snippet_insertion_count++;
	    }
// 	  else
// 	    {
// 	      list_of_handles->push_back( result );
// 	    }
	}
    }

  delete code;

//   return * list_of_handles;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

void instrument_exit_points( BPatch_thread * app_thread,
			     BPatch_image * ,
			     BPatch_function * func,
			     BPatch_snippet * code )
{
  assert( func != 0 );
  assert( code != 0 );

//   handle_vector * list_of_handles = new handle_vector;

  int null_exit_point_count = 0;
  int failed_snippet_insertion_count = 0;

  point_vector * exits = func->findPoint( BPatch_exit );
  assert( exits != 0 );

  for( unsigned int i = 0; i < exits->size(); i++ )
    {
      BPatch_point * point = ( * exits )[ i ];
      if( point == 0 )
	{
	  null_exit_point_count++;
	}
      else
	{
	  BPatchSnippetHandle * result =
	    app_thread->insertSnippet( * code,
				       * point, BPatch_callAfter, BPatch_firstSnippet );
	  if( result == 0 )
	    {
	      failed_snippet_insertion_count++;
	    }
// 	  else
// 	    {
// 	      list_of_handles->push_back( result );
// 	    }
	}
    }

  delete code;

//   return * list_of_handles;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

//
// Start Test Case #31 - (non-recursive base tramp)
//
void mutatorTest31( BPatch_thread * appThread, BPatch_image * appImage )
{
   const char * foo_name = "func31_2";
   const char * bar_name = "func31_3";
   const char * baz_name = "func31_4";

   BPatch_image * app_image = appImage;
   BPatch_thread * app_thread = appThread;
  
   BPatch_function *foo_function;
   foo_function = app_image->findFunction(foo_name);
   
   if( foo_function == 0 )
   {
      fprintf( stderr, "Cannot find \"%s\" function.",
	       foo_name );
      exit( -1 );
   }
   
   BPatch_function *bar_function;
   bar_function = app_image->findFunction(bar_name);
   
   if( bar_function == 0 )
   {
      fprintf( stderr, "Cannot find \"%s\" function.",
	       bar_name );
      exit( -1 );
   }
   
   BPatch_function *baz_function;
   baz_function = app_image->findFunction(baz_name);
   
   if( baz_function == 0 )
   {
      fprintf( stderr, "Cannot find \"%s\" function.",
	       baz_name );
      exit( -1 );
    }
   
   bool old_value = BPatch::bpatch->isTrampRecursive();
   BPatch::bpatch->setTrampRecursive( false );
   
   BPatch_Vector<BPatch_snippet *> foo_args;
   BPatch_snippet * foo_snippet =
      new BPatch_funcCallExpr( * bar_function,
                               foo_args );
   instrument_entry_points( app_thread, app_image, foo_function, foo_snippet );
   
   BPatch_Vector<BPatch_snippet *> bar_args_1;
   bar_args_1.push_back( new BPatch_constExpr( 1 ) );
   BPatch_snippet * bar_snippet_1 =
      new BPatch_funcCallExpr( * baz_function,
                               bar_args_1 );
   instrument_entry_points(app_thread, app_image, bar_function, bar_snippet_1);

   BPatch_Vector<BPatch_snippet *> bar_args_2;
   bar_args_2.push_back( new BPatch_constExpr( 2 ) );
   BPatch_snippet * bar_snippet_2 =
      new BPatch_funcCallExpr( * baz_function,
                               bar_args_2 );
   instrument_exit_points(app_thread, app_image, bar_function, bar_snippet_2);
   
   BPatch::bpatch->setTrampRecursive( old_value );
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

//
// Start Test Case #32 - (recursive base tramp)
//
void mutatorTest32( BPatch_thread * appThread, BPatch_image * appImage )
{
  const char * foo_name = "func32_2";
  const char * bar_name = "func32_3";
  const char * baz_name = "func32_4";

  BPatch_image * app_image = appImage;
  BPatch_thread * app_thread = appThread;

  BPatch_function * foo_function;
  foo_function = app_image->findFunction(foo_name);

  if( foo_function == 0 )
  {
    fprintf( stderr, "Cannot find \"%s\" function.",
       foo_name );
    exit( -1 );
  }

  BPatch_function * bar_function;
  bar_function = app_image->findFunction(bar_name);

  if( bar_function == 0 )
    {
      fprintf( stderr, "Cannot find \"%s\" function.",
	       bar_name );
      exit( -1 );
    }
  BPatch_function * baz_function;
  baz_function = app_image->findFunction(baz_name);

  if( baz_function == 0 )
    {
      fprintf( stderr, "Cannot find \"%s\" function.",
	       baz_name );
      exit( -1 );
    }

  bool old_value = BPatch::bpatch->isTrampRecursive();
  BPatch::bpatch->setTrampRecursive( true );

  BPatch_Vector<BPatch_snippet *> foo_args;
  BPatch_snippet * foo_snippet =
    new BPatch_funcCallExpr( * bar_function,
			     foo_args );
  instrument_entry_points( app_thread, app_image, foo_function, foo_snippet );

  BPatch_Vector<BPatch_snippet *> bar_args_1;

  if (mutateeFortran) {
    BPatch_variableExpr *expr32_1 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr expr32_2 = expr32_1->getBaseAddr ();

    BPatch_arithExpr expr32_3 (BPatch_assign, *expr32_1, BPatch_constExpr(1));

    appThread->oneTimeCode (expr32_3);
    bar_args_1.push_back (&expr32_2);
  } else {
    bar_args_1.push_back (new BPatch_constExpr (1));
  }

  bar_args_1.push_back (new BPatch_constExpr (1));
  BPatch_snippet * bar_snippet_1 =
    new BPatch_funcCallExpr( * baz_function,
			     bar_args_1 );
  instrument_entry_points( app_thread, app_image, bar_function, bar_snippet_1 );

  BPatch_Vector<BPatch_snippet *> bar_args_2;

  if (mutateeFortran) {
    BPatch_variableExpr *expr32_4 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr expr32_5 = expr32_4->getBaseAddr ();

    BPatch_arithExpr expr32_6 (BPatch_assign, *expr32_4, BPatch_constExpr (2));
    appThread->oneTimeCode (expr32_6);
    bar_args_2.push_back (&expr32_5);
  } else {
    bar_args_2.push_back (new BPatch_constExpr (2));
  }

  BPatch_snippet * bar_snippet_2 =
    new BPatch_funcCallExpr( * baz_function,
			     bar_args_2 );
  instrument_exit_points( app_thread, app_image, bar_function, bar_snippet_2 );

  BPatch::bpatch->setTrampRecursive( old_value );
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

//
// Start Test Case #33 - (control flow graphs)
//

bool hasBackEdge(BPatch_basicBlock *bb, BPatch_Set<int> visited)
{
    if (visited.contains(bb->getBlockNumber()))
	return true;

    visited.insert(bb->getBlockNumber());

    BPatch_Vector<BPatch_basicBlock*> targets;
    bb->getTargets(targets);

    unsigned int i;
    for (i = 0; i < targets.size(); i++) {
	if (hasBackEdge(targets[i], visited))
	    return true;
    }

    return false;
}

void mutatorTest33( BPatch_thread * /*appThread*/, BPatch_image * appImage )
{
    unsigned int i;

    BPatch_function *func2;
    BPatch_function *func3;

    if (mutateeFortran) {
	return;
    }

    func2 = appImage->findFunction("func33_2");

    if (func2 == NULL) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Unable to find function func33_2\n");
	exit(1);
    }

    BPatch_flowGraph *cfg = func2->getCFG();
    if (cfg == NULL) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Unable to get control flow graph of func33_2\n");
	exit(1);
    }

    /*
     * Test for consistency of entry basic blocks.
     */
    BPatch_Vector<BPatch_basicBlock*> entry_blocks;
    cfg->getEntryBasicBlock(entry_blocks);

    if (entry_blocks.size() != 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected %d entry basic blocks in func33_2, should have been one.\n", entry_blocks.size());
    }

    for (i = 0; i < entry_blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> sources;
	entry_blocks[i]->getSources(sources);
	if (sources.size() > 0) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  An entry basic block has incoming edges in the control flow graph\n");
	    exit(1);
	}

    	BPatch_Vector<BPatch_basicBlock*> targets;
	entry_blocks[i]->getTargets(targets);
	if (targets.size() < 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs\n");
	    fprintf(stderr, "  An entry basic block has no outgoing edges in the control flow graph\n");
	    exit(1);
	}
    }

    /*
     * Test for consistency of exit basic blocks.
     */
    BPatch_Vector<BPatch_basicBlock*> exit_blocks;
    cfg->getExitBasicBlock(exit_blocks);

    if (exit_blocks.size() != 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected %d exit basic blocks in func33_2, should have been one.\n", exit_blocks.size());
    }

    for (i = 0; i < exit_blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> sources;
	exit_blocks[i]->getSources(sources);
	if (sources.size() < 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  An exit basic block has no incoming edges in the control flow graph\n");
	    exit(1);
	}

	BPatch_Vector<BPatch_basicBlock*> targets;
	exit_blocks[i]->getTargets(targets);
	if (targets.size() > 0) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  An exit basic block has outgoing edges in the control flow graph\n");
	    exit(1);
	}
    }

    /*
     * Check structure of control flow graph.
     */
    BPatch_Set<BPatch_basicBlock*> blocks;
    cfg->getAllBasicBlocks(blocks);
    if (blocks.size() < 4) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected %d basic blocks in func33_2, should be at least four.\n", blocks.size());
	exit(1);
    }

    BPatch_basicBlock **block_elements = new BPatch_basicBlock*[blocks.size()];
    blocks.elements(block_elements);

    bool foundOutDegreeTwo = false;
    bool foundInDegreeTwo = false;
    int blocksNoIn = 0, blocksNoOut = 0;

    for (i = 0; i < (unsigned int) blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> in;
	BPatch_Vector<BPatch_basicBlock*> out;

	block_elements[i]->getSources(in);
	block_elements[i]->getTargets(out);

	if (in.size() == 0)
	    blocksNoIn++;

	if (out.size() == 0)
	    blocksNoOut++;

	if (in.size() > 2 || out.size() > 2) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with %d incoming edges and %d\n", in.size(), out.size());
	    fprintf(stderr, "  outgoing edges - neither should be greater than two.\n");
	    exit(1);
	} else if (in.size() > 1 && out.size() > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with %d incoming edges and %d\n", in.size(), out.size());
	    fprintf(stderr, "  outgoing edges - only one should be greater than one.\n");
	    exit(1);
	} else if (in.size() == 0 && out.size() == 0) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with no incoming or outgoing edges.\n");
	    exit(1);
	} else if (in.size() == 2) {
	    assert(out.size() <= 1);

	    if (foundInDegreeTwo) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Detected two basic blocks in func33_2 with in degree two, there should only\n");
		fprintf(stderr, "  be one.\n");
		exit(1);
	    }
	    foundInDegreeTwo = true;

	    if (in[0]->getBlockNumber() == in[1]->getBlockNumber()) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Two edges go to the same block (number %d).\n", in[0]->getBlockNumber());
		exit(1);
	    }
	} else if (out.size() == 2) {
	    assert(in.size() <= 1);

	    if (foundOutDegreeTwo) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Detected two basic blocks in func33_2 with out degree two, there should only\n");
		fprintf(stderr, "  be one.\n");
		exit(1);
	    }
	    foundOutDegreeTwo = true;

	    if (out[0]->getBlockNumber() == out[1]->getBlockNumber()) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Two edges go to the same block (number %d).\n", out[0]->getBlockNumber());
		exit(1);
	    }
	} else if (in.size() > 1 || out.size() > 1) {
	    /* Shouldn't be able to get here. */
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with %d incoming edges and %d\n", in.size(), out.size());
	    fprintf(stderr, "  outgoing edges.\n");
	    exit(1);
	}
    }

    delete [] block_elements;
    
    if (blocksNoIn > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected more than one block in func33_2 with no incoming edges.\n");
	    exit(1);
    }

    if (blocksNoOut > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected more than block in func33_2 with no outgoing edges.\n");
	    exit(1);
    }

    if (!foundOutDegreeTwo) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Did not detect the \"if\" statement in func33_2.\n");
	    exit(1);
    }

    /*
     * Check for loops (there aren't any in the function we're looking at).
     */
    BPatch_Set<int> empty;
    if (hasBackEdge(entry_blocks[0], empty)) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected a loop in func33_2, there should not be one.\n");
	exit(1);
    }

    /*
     * Now check a function with a switch statement.
     */
    func3 = appImage->findFunction("func33_3");

    if (func3 == NULL) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Unable to find function func33_3\n");
	exit(1);
    }

    BPatch_flowGraph *cfg3 = func3->getCFG();
    if (cfg3 == NULL) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Unable to get control flow graph of func33_3\n");
	exit(1);
    }

    BPatch_Set<BPatch_basicBlock*> blocks3;
    cfg3->getAllBasicBlocks(blocks3);
    if (blocks3.size() < 10) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected %d basic blocks in func33_3, should be at least ten.\n", blocks3.size());
	exit(1);
    }

    block_elements = new BPatch_basicBlock*[blocks3.size()];
    blocks3.elements(block_elements);

    bool foundSwitchIn = false;
    bool foundSwitchOut = false;
    bool foundRangeCheck = false;
    for (i = 0; i < (unsigned int)blocks3.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> in;
	BPatch_Vector<BPatch_basicBlock*> out;

	block_elements[i]->getSources(in);
	block_elements[i]->getTargets(out);

	if (!foundSwitchOut && out.size() >= 10 && in.size() <= 1) {
	    foundSwitchOut = true;
	} else if (!foundSwitchIn && in.size() >= 10 && out.size() <= 1) {
	    foundSwitchIn = true;
	} else if (!foundRangeCheck && out.size() == 2 && in.size() <= 1) {
	    foundRangeCheck = true;
	} else if (in.size() > 1 && out.size() > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Found basic block in func33_3 with unexpected number of edges.\n");
	    fprintf(stderr, "  %d incoming edges, %d outgoing edges.\n",
		    in.size(), out.size());
	    exit(1);
	}
    }

    if (!foundSwitchIn || !foundSwitchOut) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	if (!foundSwitchIn)
	    fprintf(stderr,"  Did not find \"switch\" statement in func33_3.\n");
	if (!foundSwitchOut)
	    fprintf(stderr,"  Did not find block afer \"switch\" statement.\n");
	exit(1);
    }

    /* Check dominator info. */
    BPatch_Vector<BPatch_basicBlock*> entry3;
    cfg3->getEntryBasicBlock(entry3);
    if (entry3.size() != 1) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected %d entry basic blocks in func33_3, should have been one.\n", entry_blocks.size());
	exit(1);
    }

    for (i = 0; i < (unsigned int) blocks3.size(); i++) {
	if (!entry3[0]->dominates(block_elements[i])) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Entry block does not dominate all blocks in func33_3\n");
	    exit(1);
	}
    }

    delete [] block_elements;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int numContainedLoops(BPatch_basicBlockLoop *loop)
{
    BPatch_Vector<BPatch_basicBlockLoop*> containedLoops;
    loop->getContainedLoops(containedLoops);

    return containedLoops.size();
}

int numBackEdges(BPatch_basicBlockLoop *loop)
{
    BPatch_Vector<BPatch_basicBlock*> backEdges;
    loop->getBackEdges(backEdges);

    return backEdges.size();
}

//
// Start Test Case #34 - (loop information)
//
void mutatorTest34( BPatch_thread * /*appThread*/, BPatch_image * appImage )
{
    unsigned int i;

    BPatch_function *func2;
    func2 = appImage->findFunction("func34_2");

    if (func2 == NULL) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Unable to find function func34_2\n");
	exit(1);
    }

    BPatch_flowGraph *cfg = func2->getCFG();
    if (cfg == NULL) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Unable to get control flow graph of func34_2\n");
	exit(1);
    }

    BPatch_Vector<BPatch_basicBlockLoop*> loops;
    cfg->getLoops(loops);
    if (loops.size() != 4) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Detected %d loops, should have been four.\n",
		loops.size());
	exit(1);
    }

    /*
     * Find the loop that contains two loops (that should be the outermost
     * one).
     */
    BPatch_basicBlockLoop *outerLoop = NULL;
    for (i = 0; i < loops.size(); i++) {
	if (numContainedLoops(loops[i]) == 3) {
	    outerLoop = loops[i];
	    break;
	}
    }

    if (outerLoop == NULL) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Unable to find a loop containing two other loops.\n");
	exit(1);
    }

    if (numBackEdges(outerLoop) != 1) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  There should be exactly one backedge in the outer loops, but there are %d\n", numBackEdges(outerLoop));
	exit(1);
    }

    BPatch_Vector<BPatch_basicBlockLoop*> insideOuterLoop;
    outerLoop->getContainedLoops(insideOuterLoop);
    assert(insideOuterLoop.size() == 3);

    bool foundFirstLoop = false;
    int deepestLoops = 0;
    for (i = 0; i < insideOuterLoop.size(); i++) {
	BPatch_Vector<BPatch_basicBlockLoop*> tmpLoops;
	insideOuterLoop[i]->getContainedLoops(tmpLoops);

	if (tmpLoops.size() == 1) { /* The first loop has one nested inside. */
	    if (foundFirstLoop) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  Found more than one second-level loop with one nested inside.\n");
		exit(1);
	    }
	    foundFirstLoop = true;

	    if (numBackEdges(insideOuterLoop[i]) != 1) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  There should be exactly one backedge in the first inner loop, but there are %d\n", numBackEdges(tmpLoops[0]));
		exit(1);
	    }

	    if (numBackEdges(tmpLoops[0]) != 1) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  There should be exactly one backedge in the third level loop, but there are %d\n", numBackEdges(tmpLoops[0]));
		exit(1);
	    }

	    if (numContainedLoops(tmpLoops[0]) != 0) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  The first loop at the third level should not have any loops nested inside,\n");
		fprintf(stderr, "  but %d were detected.\n",
			numContainedLoops(tmpLoops[0]));
		exit(1);
	    }

	} else if(tmpLoops.size() == 0) { /* The second loop has none nested. */
	    if (deepestLoops >= 2) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  Found more than two loops without any nested inside.\n");
		exit(1);
	    }
	    deepestLoops++;

	    if (numBackEdges(insideOuterLoop[i]) != 1) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  Unexpected number of backedges in loop (%d)\n", numBackEdges(insideOuterLoop[i]));
		exit(1);
	    }
	} else { /* All loops should be recognized above. */
	    fprintf(stderr, "**Failed** test #34 (loop information)\n");
	    fprintf(stderr, "  Found a loop containing %d loops, should be one or  none.\n", tmpLoops.size());
	    exit(1);
	}
    }

    if (!foundFirstLoop || deepestLoops < 2) {
	/* We shouldn't be able to get here. */
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	if (!foundFirstLoop)
	    fprintf(stderr, "  Could not find the first nested loop.\n");
	if (deepestLoops < 2)
	    fprintf(stderr, "  Could not find all the deepest level loops.\n");
	exit(1);
    }
}

// Start Test Case #35 - (function relocation)
void mutatorTest35( BPatch_thread * appThread, BPatch_image * appImage )
{
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4)

    const char * foo_name = "call35_1";

    if (mutateeFortran)
	return;


    BPatch_function * foo_function = appImage->findFunction(foo_name);
    if( foo_function == 0 )
    {
      fprintf( stderr, "Cannot find \"%s\" function.",
	       foo_name );
      exit( -1 );
    }

    BPatch_Vector<BPatch_point *> *point35_1 =  
	foo_function->findPoint(BPatch_subroutine);

    assert(point35_1);

    BPatch_variableExpr *var1 = appImage->findVariable(*(*point35_1)[0], 
	"localVariable35_1");
    BPatch_variableExpr *var2 = appImage->findVariable(*(*point35_1)[0], 
	"localVariable35_2");
    BPatch_variableExpr *var3 = appImage->findVariable(*(*point35_1)[0], 
	"total35_1");
    BPatch_variableExpr *var4 = appImage->findVariable(*(*point35_1)[0], 
	"total35_2");

    if (!var1 || !var2 || !var3 || !var4 ) {
	fprintf(stderr, "**Failed** test #35 (function relocation)\n");
	if (!var1) 
	    fprintf(stderr, "  can't find local variable localVariable35_1\n");
	if (!var2) 
	    fprintf(stderr, "  can't find local variable localVariable35_2\n");
        if (!var3) 
	    fprintf(stderr, "  can't find local variable total35_1\n");
        if (!var4) 
	    fprintf(stderr, "  can't find local variable total35_2\n");
	return;
    }

    BPatch_snippet * snippet35_1 = 
      new BPatch_arithExpr(BPatch_assign, *var1, BPatch_constExpr(7));

    BPatch_snippet * snippet35_2 = 
      new BPatch_arithExpr(BPatch_assign, *var2, BPatch_constExpr(5));

    BPatch_snippet * snippet35_3 = 
      new BPatch_arithExpr(BPatch_assign, *var4, *var3);

    BPatch_point * call_1 = ( (* point35_1)[0] );
    assert( call_1 != 0 );
    
    BPatch_point * call_2 = ( (* point35_1)[2] );
    assert( call_2 != 0 );
    
    appThread->insertSnippet( * snippet35_3, * call_2, BPatch_callAfter, BPatch_firstSnippet );
    appThread->insertSnippet( * snippet35_2, * call_1, BPatch_callBefore, BPatch_firstSnippet );
    appThread->insertSnippet( * snippet35_1, * call_1, BPatch_callBefore, BPatch_firstSnippet );

#endif   
}    


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int mutatorMAIN(char *pathname, bool useAttach)
{
    BPatch_thread *appThread;
	char *dirName;

    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);

    char *child_argv[MAX_TEST+5];

    int n = 0;
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    if (runAllTests) {
        child_argv[n++] = const_cast<char*>("-runall"); // signifies all tests
    } else {
        child_argv[n++] = const_cast<char*>("-run");
        for (unsigned int j=1; j <= MAX_TEST; j++) {
            if (runTest[j]) {
        	char str[5];
        	sprintf(str, "%d", j);
        	child_argv[n++] = strdup(str);
            }
        }
    }

    child_argv[n] = NULL;

    if (useAttach) {
	int pid = startNewProcessForAttach(pathname, child_argv);
	if (pid < 0) {
	    printf("*ERROR*: unable to start tests due to error creating mutatee process\n");
	    exit(-1);
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
	}
        P_sleep(1); // let the mutatee catch its breath for a moment
	appThread = bpatch->attachProcess(pathname, pid);
    } else {
	appThread = bpatch->createProcess(pathname, child_argv,NULL);
    }

    if (appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
    /* this is only supported on sparc solaris  and linux*/
	/* this call tells the process to collect data for the 
	save the world functionality
	*/	
	if(saveWorld){
		appThread->startSaveWorld();
	}	
#endif

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
	signalAttached(appThread, appImage);
    }

    // determine whether mutatee is C or C++
    BPatch_variableExpr *isCxx = appImage->findVariable("mutateeCplusplus");
    if (isCxx == NULL) {
	fprintf(stderr, "  Unable to locate variable \"mutateeCplusplus\""
                 " -- assuming 0!\n");
    } else {
        isCxx->readValue(&mutateeCplusplus);
        dprintf("Mutatee is %s.\n", mutateeCplusplus ? "C++" : "C");
    }

    // determine whether mutatee is Fortran
    BPatch_variableExpr *isF = appImage->findVariable("mutateeFortran");
    if (isF == NULL) {
	fprintf(stderr, "  Unable to locate variable \"mutateeFortran\""
                 " -- assuming 0!\n");
    } else {
        isF->readValue(&mutateeFortran);
        dprintf("Mutatee is %s.\n", mutateeFortran ? "Fortran" : "C/C++");
    }

    // determine whether mutatee is F77
    BPatch_variableExpr *isF77 = appImage->findVariable("mutateeF77");
    if (isF77 == NULL) {
	fprintf(stderr, "  Unable to locate variable \"mutateeF77\""
                 " -- assuming 0!\n");
    } else {
        isF77->readValue(&mutateeF77);
        dprintf("Mutatee is %s.\n", mutateeF77 ? "F77" : "not F77");
    }
    /*
    unsigned int i;
    BPatch_Vector<BPatch_module *> *m = appImage->getModules();
    for (i=0; i < m->size(); i++) {
         dprintf("func %s\n", (*m)[i]->name());
    }
    BPatch_Vector<BPatch_function *> *p = appImage->getProcedures();
    for (i=0; i < p->size(); i++) {
         dprintf("func %s\n", (*p)[i]->name());
    }
    */
    if (runTest[1]) mutatorTest1(appThread, appImage);
    if (runTest[2]) mutatorTest2(appThread, appImage);
    if (runTest[3]) mutatorTest3(appThread, appImage);
    if (runTest[4]) mutatorTest4(appThread, appImage);
    if (runTest[5]) mutatorTest5(appThread, appImage);
    if (runTest[6]) mutatorTest6(appThread, appImage);
    if (runTest[7]) mutatorTest7(appThread, appImage);
    if (runTest[8]) mutatorTest8(appThread, appImage);
    if (runTest[9]) mutatorTest9(appThread, appImage);
    if (runTest[10]) mutatorTest10(appThread, appImage);
    if (runTest[11]) mutatorTest11(appThread, appImage);

    if (runTest[12]) mutatorTest12a(appThread, appImage);

    if (runTest[13]) mutatorTest13(appThread, appImage);
    if (runTest[14]) mutatorTest14(appThread, appImage);

    if (runTest[15]) mutatorTest15a(appThread, appImage);

    if (runTest[16]) mutatorTest16(appThread, appImage);

    if (runTest[17]) mutatorTest17(appThread, appImage);
    if (runTest[18]) mutatorTest18(appThread, appImage);
    if (runTest[20]) mutatorTest20(appThread, appImage);

    if (runTest[21] || runTest[22]) readyTest21or22(appThread);
    if (runTest[21]) mutatorTest21(appThread, appImage);
    if (runTest[22]) mutatorTest22(appThread, appImage);
    if (runTest[23]) mutatorTest23(appThread, appImage);
    if (runTest[24]) mutatorTest24(appThread, appImage);
    if (runTest[25]) mutatorTest25(appThread, appImage);
    if (runTest[26]) mutatorTest26(appThread, appImage);
    if (runTest[27]) mutatorTest27(appThread, appImage);
    if (runTest[28]) mutatorTest28(appThread, appImage);
    if (runTest[29]) mutatorTest29(appThread, appImage);
    if (runTest[30]) mutatorTest30(appThread, appImage);

    if( runTest[ 31 ] ) mutatorTest31( appThread, appImage );
    if( runTest[ 32 ] ) mutatorTest32( appThread, appImage );

    if( runTest[ 33 ] ) mutatorTest33( appThread, appImage );
    if( runTest[ 34 ] ) mutatorTest34( appThread, appImage );

    if( runTest[ 35 ] ) mutatorTest35( appThread, appImage );


    
    /* the following bit of code saves the mutatee in its mutated state to the
	file "originalmutateename"_mutated

	test cases 12, 15, 19 fail on the saved mutatee because they 
	expect the mutator to be there to wait for the mutatee and change
	things as the mutatee runs.

	test cases 18, 27, 29 fail on the saved mutatee because they
	require the mutator to writeValue() data into the mutatee.  If you
	want these to pass you need to set saveWorld to true in the writeValue()
	call in the mutator then save the mutatee.

	test case 22 fails on the saved mutatee because it deals with shared
	libraries.

	On linux test 20 also fails.  On x86, to instrument and arbitrary
	instruction, we place a 0xcc breakpoint in the mutatee and have
	the mutator catch it. Obviously with no mutator that will not
	work.
*/

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
    /* this is only supported on sparc solaris and linux*/

    	if( saveWorld ) {
        	char *mutatedName = new char[strlen(pathname) + strlen("_mutated") +1];
        	strcpy(mutatedName, pathname);
        	strcat(mutatedName, "_mutated");
        	dirName = appThread->dumpPatchedImage(mutatedName);
		if(dirName){
			printf(" The mutated binary is stored in: %s\n",dirName);
			delete [] dirName;
		}else{
			printf("Error: No directory name returned\n");
		}
		//appThread->detach(false);
	}
#endif
    // Start of code to continue the process.  All mutations made
    // above will be in place before the mutatee begins its tests.

    dprintf("starting program execution.\n");
    appThread->continueExecution();

    // Test poll for status change
    if (runTest[12]) mutatorTest12b(appThread, appImage);
    if (runTest[15]) mutatorTest15b(appThread, appImage);
    if (runTest[19]) mutatorTest19(appThread, appImage);

    while (!appThread->isTerminated())
	bpatch->waitForStatusChange();

    int exitCode = appThread->terminationStatus();
    if (exitCode || debugPrint) printf("Mutatee exit code 0x%x\n", exitCode);

    dprintf("Done.\n");
    return(exitCode);
}

//
// main - decide our role and call the correct "main"
//
int
main(unsigned int argc, char *argv[])
{
    char mutateeName[128];
    char libRTname[256];

    bool N32ABI = false;
    bool useAttach = false;

    strcpy(mutateeName,mutateeNameRoot);
    strcpy(libNameA,libNameAroot);
    strcpy(libNameB,libNameBroot);
    libRTname[0]='\0';

    if (!getenv("DYNINSTAPI_RT_LIB")) {
	 fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");
         exit(-1);
#endif
    } else
         strcpy((char *)libRTname, (char *)getenv("DYNINSTAPI_RT_LIB"));

    unsigned int i;
    // by default run all tests
    for (i=1; i <= MAX_TEST; i++) {
        runTest[i] = true;
        passedTest[i] = false;
    }

    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "-v+", 3) == 0)    errorPrint++;
        if (strncmp(argv[i], "-v++", 4) == 0)   errorPrint++;
	if (strncmp(argv[i], "-verbose", 2) == 0) {
	    debugPrint = 1;
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
	}else if (!strcmp(argv[i], "-saveworld")) {
		saveWorld = 1;
#endif
	} else if (!strcmp(argv[i], "-V")) {
            fprintf (stdout, "%s\n", V_libdyninstAPI);
            if (libRTname[0])
                fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
	} else if (!strcmp(argv[i], "-attach")) {
	    useAttach = true;
	} else if (!strcmp(argv[i], "-skip")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = false;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-run")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = true;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-mutatee")) {
	    i++;
            if (*argv[i]=='_')
                strcat(mutateeName,argv[i]);
            else
                strcpy(mutateeName,argv[i]);
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
            N32ABI = true;
#endif
	} else {
	    fprintf(stderr, "Usage: test1 "
		    "[-V] [-verbose] [-attach] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
		    "[-saveworld] "
#endif 
                    "[-mutatee <test1.mutatee>] "
		    "[-run <test#> <test#> ...] "
		    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
	}
    }

    if (!runAllTests) {
        printf("Running Tests: ");
	for (unsigned int j=1; j <= MAX_TEST; j++) {
	    if (runTest[j]) printf("%d ", j);
	}
	printf("\n");
    }

    // patch up the default compiler in mutatee name (if necessary)
    if (!strstr(mutateeName, "_"))
#if defined(i386_unknown_nt4_0)
        strcat(mutateeName,"_VC");
#else
        strcat(mutateeName,"_gcc");
#endif
    if (N32ABI || strstr(mutateeName,"_n32")) {
        // patch up file names based on alternate ABI (as necessary)
        if (!strstr(mutateeName, "_n32")) strcat(mutateeName,"_n32");
        strcat(libNameA,"_n32");
        strcat(libNameB,"_n32");
    }
		
    // patch up the platform-specific filename extensions
#if defined(i386_unknown_nt4_0)
    if (!strstr(mutateeName, ".exe")) strcat(mutateeName,".exe");
    strcat(libNameA,".dll");
    strcat(libNameB,".dll");
#else
    strcat(libNameA,".so");
    strcat(libNameB,".so");
#endif

    int exitCode = mutatorMAIN(mutateeName, useAttach);

    return exitCode;
}
