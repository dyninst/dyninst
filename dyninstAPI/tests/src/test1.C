// $Id: test1.C,v 1.23 1999/05/25 20:27:01 hollings Exp $
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

int debugPrint = 0;

template class BPatch_Vector<BPatch_variableExpr*>;

BPatch *bpatch;

// control debug printf statements
#define dprintf	if (debugPrint) printf

/**************************************************************************
 * Error callback
 **************************************************************************/

#define DYNINST_NO_ERROR -1

int expectError = DYNINST_NO_ERROR;

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    char line[256];

    const char *msg = bpatch->getEnglishErrorString(num);
    bpatch->formatErrorString(line, sizeof(line), msg, params);

    if (num != expectError) {
    	printf("Error #%d (level %d): %s\n", num, level, line);

	// We consider some errors fatal.
	if (num == 101) {
	    exit(-1);
	}
    }
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
    if (cost < 0.0) {
	printf("*Error*: negative snippet cost\n");
    } else if (cost == 0.0) {
	printf("*Warning*: zero snippet cost\n");
    } else if (cost > 0.01) {
	printf("*Error*: snippet cost of %f, exceeds max expected of 0.1",
	    cost);
    }
}

//
// Replace all calls in "inFunction" to "callTo" with calls to "replacement."
// If "replacement" is NULL, them use removeFunctionCall instead of
// replaceFunctionCall.
// Returns the number of replacements that were performed.
//
int replaceFunctionCalls(BPatch_thread *appThread, BPatch_image *appImage,
			 char *inFunction, char *callTo, char *replacement,
			 int testNo, char *testName,
			 int callsExpected = -1)
{
    int numReplaced = 0;

    BPatch_Vector<BPatch_point *> *points =
	appImage->findProcedurePoint(inFunction, BPatch_subroutine);
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

    for (int n = 0; n < points->size(); n++) {
	BPatch_function *func;

	if ((func = (*points)[n]->getCalledFunction()) == NULL) continue;

	char fn[256];
	if (func->getName(fn, 256) == NULL) {
	    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	    fprintf(stderr, "    Can't get name of called function in %s\n",
		    inFunction);
	    exit(1);
	}
	if (strcmp(fn, callTo) == 0) {
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
char *locationName(BPatch_procedureLocation l)
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
	BPatch_image *appImage, char *inFunction, BPatch_procedureLocation loc,
	BPatch_snippet &snippet, int testNo, char *testName)
{
    // Find the point(s) we'll be instrumenting
    BPatch_Vector<BPatch_point *> *points =
	appImage->findProcedurePoint(inFunction, loc);

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
BPatch_snippet *makeCallSnippet(BPatch_image *appImage, char *funcName,
				int testNo, char *testName)
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
	BPatch_image *appImage, char *inFunction, BPatch_procedureLocation loc,
	char *funcName, int testNo, char *testName)
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


/**************************************************************************
 * Tests
 **************************************************************************/

//
// Start Test Case #6 - mutator side (arithmetic operators)
//
void mutatorTest6(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func6_1"
    BPatch_Vector<BPatch_point *> *point6_1 =
	appImage->findProcedurePoint("func6_1", BPatch_entry);
    BPatch_variableExpr *expr6_1 = appImage->findVariable("globalVariable6_1");
    BPatch_variableExpr *expr6_2 = appImage->findVariable("globalVariable6_2");
    BPatch_variableExpr *expr6_3 = appImage->findVariable("globalVariable6_3");
    BPatch_variableExpr *expr6_4 = appImage->findVariable("globalVariable6_4");
    BPatch_variableExpr *expr6_5 = appImage->findVariable("globalVariable6_5");
    BPatch_variableExpr *expr6_6 = appImage->findVariable("globalVariable6_6");
    BPatch_variableExpr *expr6_1a =appImage->findVariable("globalVariable6_1a");
    BPatch_variableExpr *expr6_2a =appImage->findVariable("globalVariable6_2a");
    BPatch_variableExpr *expr6_3a =appImage->findVariable("globalVariable6_3a");
    BPatch_variableExpr *expr6_4a =appImage->findVariable("globalVariable6_4a");
    BPatch_variableExpr *expr6_5a =appImage->findVariable("globalVariable6_5a");
    BPatch_variableExpr *expr6_6a =appImage->findVariable("globalVariable6_6a");
    if (!expr6_1 || !expr6_2 || !expr6_3 || !expr6_4 || 
	!expr6_5 || !expr6_6 || !expr6_1a || !expr6_2a || !expr6_3a || 
	!expr6_4a || !expr6_5a || !expr6_6a) {
	fprintf(stderr, "**Failed** test #6 (arithmetic operators)\n");
	fprintf(stderr, "    Unable to locate one of globalVariable6_?\n");
	exit(1);
    }

    BPatch_variableExpr *constVar1 = appImage->findVariable("constVar1");
    BPatch_variableExpr *constVar2 = appImage->findVariable("constVar2");
    BPatch_variableExpr *constVar3 = appImage->findVariable("constVar3");
    BPatch_variableExpr *constVar5 = appImage->findVariable("constVar5");
    BPatch_variableExpr *constVar6 = appImage->findVariable("constVar6");
    BPatch_variableExpr *constVar10 = appImage->findVariable("constVar10");
    BPatch_variableExpr *constVar60 = appImage->findVariable("constVar60");
    BPatch_variableExpr *constVar64 = appImage->findVariable("constVar64");
    BPatch_variableExpr *constVar66 = appImage->findVariable("constVar66");
    BPatch_variableExpr *constVar67 = appImage->findVariable("constVar67");
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

    // globalVariable6_3 = 66 / 3
    BPatch_arithExpr arith6_3a (BPatch_assign, *expr6_3a, BPatch_arithExpr(
      BPatch_divide, *constVar66, *constVar3));
    vect6_1.push_back(&arith6_3a);

    // globalVariable6_4 = 67 / 3
    BPatch_arithExpr arith6_4a (BPatch_assign, *expr6_4a, BPatch_arithExpr(
      BPatch_divide, *constVar67, *constVar3));
    vect6_1.push_back(&arith6_4a);

    // globalVariable6_5 = 6 * 5
    BPatch_arithExpr arith6_5a (BPatch_assign, *expr6_5a, BPatch_arithExpr(
      BPatch_times, *constVar6, *constVar5));
    vect6_1.push_back(&arith6_5a);

    // globalVariable6_6 = 10,3
    // BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a, *constVar3);
    //	BPatch_arithExpr(BPatch_seq, *constVar10, BPatch_constExpr(3)));
    BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a,
	BPatch_arithExpr(BPatch_seq, *constVar10, *constVar3));
    vect6_1.push_back(&arith6_6a);

    checkCost(BPatch_sequence(vect6_1));
    appThread->insertSnippet( BPatch_sequence(vect6_1), *point6_1);
}

void genRelTest(BPatch_image *appImage,BPatch_Vector<BPatch_snippet*> &vect7_1, 
		BPatch_relOp op, int r1, int r2, char *var1)
{
    BPatch_variableExpr *varExpr1 = appImage->findVariable(var1);
    if (!varExpr1) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate variable %s\n", var1);
	exit(1);
    }
    BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
	BPatch_boolExpr(op, BPatch_constExpr(r1), BPatch_constExpr(r2)), 
	BPatch_arithExpr(BPatch_assign, *varExpr1, BPatch_constExpr(72)));
    vect7_1.push_back(tempExpr1);

}

void genVRelTest(BPatch_image *appImage,
		 BPatch_Vector<BPatch_snippet*> &vect7_1, 
		 BPatch_relOp op, BPatch_variableExpr *r1, 
		 BPatch_variableExpr *r2, char *var1)
{
    BPatch_variableExpr *varExpr1 = appImage->findVariable(var1);
    if (!varExpr1) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate variable %s\n", var1);
	exit(1);
    }
    BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
	BPatch_boolExpr(op, *r1, *r2), 
	BPatch_arithExpr(BPatch_assign, *varExpr1, BPatch_constExpr(74)));
    vect7_1.push_back(tempExpr1);

}

//
// Start Test Case #7 - mutator side (relational operators)
//
void mutatorTest7(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func7_1"
    BPatch_Vector<BPatch_point *> *point7_1 =
	appImage->findProcedurePoint("func7_1", BPatch_entry);
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

    BPatch_variableExpr *constVar0 = appImage->findVariable("constVar0");
    BPatch_variableExpr *constVar1 = appImage->findVariable("constVar1");
    BPatch_variableExpr *constVar2 = appImage->findVariable("constVar2");
    BPatch_variableExpr *constVar3 = appImage->findVariable("constVar3");
    BPatch_variableExpr *constVar4 = appImage->findVariable("constVar4");
    BPatch_variableExpr *constVar5 = appImage->findVariable("constVar5");
    BPatch_variableExpr *constVar6 = appImage->findVariable("constVar6");
    BPatch_variableExpr *constVar7 = appImage->findVariable("constVar7");
    BPatch_variableExpr *constVar9 = appImage->findVariable("constVar9");
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
    BPatch_Vector<BPatch_point *> *point8_1 =
	appImage->findProcedurePoint("func8_1", BPatch_entry);
    BPatch_Vector<BPatch_snippet*> vect8_1;

    BPatch_variableExpr *expr8_1 = appImage->findVariable("globalVariable8_1");
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
    BPatch_Vector<BPatch_point *> *point9_1 =
	appImage->findProcedurePoint("func9_1", BPatch_entry);

    BPatch_function *call9_func = appImage->findFunction("call9_1");
    if (call9_func == NULL) {
	fprintf(stderr, "Unable to find function \"call9_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call9_args;

    BPatch_constExpr constExpr91(91);
    call9_args.push_back(&constExpr91);

    BPatch_constExpr constExpr92(92);
    call9_args.push_back(&constExpr92);

    BPatch_constExpr constExpr93(93);
    call9_args.push_back(&constExpr93);

    BPatch_constExpr constExpr94(94);
    call9_args.push_back(&constExpr94);

    BPatch_constExpr constExpr95(95);
    call9_args.push_back(&constExpr95);

    BPatch_funcCallExpr call9Expr(*call9_func, call9_args);

    checkCost(call9Expr);
    appThread->insertSnippet( call9Expr, *point9_1);
}


//
// Start Test Case #10 - mutator side (insert snippet order)
//
void mutatorTest10(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func10_1"
    BPatch_Vector<BPatch_point *> *point10_1 =
	appImage->findProcedurePoint("func10_1", BPatch_entry);

    BPatch_function *call10_1_func = appImage->findFunction("call10_1");
    if (call10_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call10_1.\"\n");
	exit(1);
    }

    BPatch_function *call10_2_func = appImage->findFunction("call10_2");
    if (call10_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call10_2.\"\n");
	exit(1);
    }

    BPatch_function *call10_3_func = appImage->findFunction("call10_3");
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
    BPatch_Vector<BPatch_point *> *point11_1 =
	appImage->findProcedurePoint("func11_1", BPatch_entry);
    if (!point11_1 || (point11_1->size() < 1)) {
	fprintf(stderr, "Unable to find point func11_1 - entry.\n");
	exit(-1);
    }

    // Find the subroutine points for the procedure "func11_1"
    BPatch_Vector<BPatch_point *> *point11_2 =
	appImage->findProcedurePoint("func11_1", BPatch_subroutine);
    if (!point11_2 || (point11_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func11_1 - calls.\n");
	exit(-1);
    }

    // Find the exit point to the procedure "func11_1"
    BPatch_Vector<BPatch_point *> *point11_3 =
	appImage->findProcedurePoint("func11_1", BPatch_exit);
    if (!point11_3 || (point11_3->size() < 1)) {
	fprintf(stderr, "Unable to find point func11_1 - exit.\n");
	exit(-1);
    }

    BPatch_function *call11_1_func = appImage->findFunction("call11_1");
    if (call11_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_1.\"\n");
	exit(1);
    }

    BPatch_function *call11_2_func = appImage->findFunction("call11_2");
    if (call11_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_2.\"\n");
	exit(1);
    }

    BPatch_function *call11_3_func = appImage->findFunction("call11_3");
    if (call11_3_func == NULL) {
	fprintf(stderr, "Unable to find function \"call11_3.\"\n");
	exit(1);
    }

    BPatch_function *call11_4_func = appImage->findFunction("call11_4");
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
    BPatch_Vector<BPatch_point *> *point12_2 =
	appImage->findProcedurePoint("func12_2", BPatch_entry);
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
	memStuff[count++] = temp;
	temp = appThread->malloc(HEAP_TEST_UNIT_SIZE);
#if defined(USES_DYNAMIC_INF_HEAP)
	// heap will grow indefinitely on dynamic heap platforms
	if (count == 1000) break; 
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

    BPatch_function *call12_1_func = appImage->findFunction("call12_1");
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
    BPatch_Vector<BPatch_point *> *point13_1 =
	appImage->findProcedurePoint("func13_1", BPatch_entry);
    if (!point13_1 || (point13_1->size() < 1)) {
	fprintf(stderr, "Unable to find point func13_1 - entry.\n");
	exit(-1);
    }

    BPatch_function *call13_1_func = appImage->findFunction("call13_1");
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
    BPatch_Vector<BPatch_point *> *point13_2 =
	appImage->findProcedurePoint("func13_2", BPatch_exit);
    if (!point13_2 || (point13_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func13_2 - exit.\n");
	exit(-1);
    }

    BPatch_function *call13_2_func = appImage->findFunction("call13_2");
    if (call13_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call13_2.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs2;
    funcArgs2.push_back(new BPatch_retExpr());
    BPatch_funcCallExpr call13_3Expr(*call13_2_func, funcArgs2);

    checkCost(call13_1Expr);
    appThread->insertSnippet(call13_3Expr, *point13_2);
}


//
// Start Test Case #14 - mutator side (replace function call)
//
void mutatorTest14(BPatch_thread *appThread, BPatch_image *appImage)
{
    replaceFunctionCalls(appThread, appImage,
		         "func14_1", "func14_2", "call14_1",
			 14, "replace/remove function call", 1);
    replaceFunctionCalls(appThread, appImage,
			 "func14_1", "func14_3", NULL,
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
    BPatch_variableExpr *expr16_1=appImage->findVariable("globalVariable16_1");
    BPatch_variableExpr *expr16_2=appImage->findVariable("globalVariable16_2");
    BPatch_variableExpr *expr16_3=appImage->findVariable("globalVariable16_3");
    BPatch_variableExpr *expr16_4=appImage->findVariable("globalVariable16_4");
    BPatch_variableExpr *expr16_5=appImage->findVariable("globalVariable16_5");
    BPatch_variableExpr *expr16_6=appImage->findVariable("globalVariable16_6");
    BPatch_variableExpr *expr16_7=appImage->findVariable("globalVariable16_7");
    BPatch_variableExpr *expr16_8=appImage->findVariable("globalVariable16_8");
    BPatch_variableExpr *expr16_9=appImage->findVariable("globalVariable16_9");
    BPatch_variableExpr *expr16_10=appImage->findVariable("globalVariable16_10");
    if (!expr16_1 || !expr16_2 || !expr16_3 || !expr16_4 || !expr16_5 ||
        !expr16_6 || !expr16_7 || !expr16_8 || !expr16_9 || !expr16_10) {
	fprintf(stderr, "**Failed** test #16 (if-else)\n");
	fprintf(stderr, "    Unable to locate one of globalVariable16_?\n");
	exit(1);
    }

    BPatch_arithExpr assign16_1(BPatch_assign, *expr16_1, BPatch_constExpr(1));
    BPatch_arithExpr assign16_2(BPatch_assign, *expr16_2, BPatch_constExpr(1));

    BPatch_ifExpr if16_2(BPatch_boolExpr(BPatch_eq,
	                                 BPatch_constExpr(1),
					 BPatch_constExpr(1)),
			 assign16_1, assign16_2);

    BPatch_arithExpr assign16_3(BPatch_assign, *expr16_3, BPatch_constExpr(1));
    BPatch_arithExpr assign16_4(BPatch_assign, *expr16_4, BPatch_constExpr(1));

    BPatch_ifExpr if16_3(BPatch_boolExpr(BPatch_eq,
	                                 BPatch_constExpr(0),
					 BPatch_constExpr(1)),
			 assign16_3, assign16_4);

    BPatch_arithExpr assign16_5(BPatch_assign, *expr16_5, BPatch_constExpr(1));
    BPatch_arithExpr assign16_6(BPatch_assign, *expr16_6, BPatch_constExpr(1));
    BPatch_sequence longExpr16_1(genLongExpr(&assign16_5));


    BPatch_ifExpr if16_4(BPatch_boolExpr(BPatch_eq,
	                                 BPatch_constExpr(0),
					 BPatch_constExpr(1)),
			 longExpr16_1, assign16_6);

    BPatch_arithExpr assign16_7(BPatch_assign, *expr16_7, BPatch_constExpr(1));
    BPatch_arithExpr assign16_8(BPatch_assign, *expr16_8, BPatch_constExpr(1));
    BPatch_sequence longExpr16_2(genLongExpr(&assign16_8));

    BPatch_ifExpr if16_5(BPatch_boolExpr(BPatch_eq,
	                                 BPatch_constExpr(0),
					 BPatch_constExpr(1)),
			 assign16_7, longExpr16_2);

    BPatch_arithExpr assign16_9(BPatch_assign, *expr16_9, BPatch_constExpr(1));
    BPatch_arithExpr assign16_10(BPatch_assign, *expr16_10,BPatch_constExpr(1));
    BPatch_sequence longExpr16_3(genLongExpr(&assign16_9));
    BPatch_sequence longExpr16_4(genLongExpr(&assign16_10));

    BPatch_ifExpr if16_6(BPatch_boolExpr(BPatch_eq,
	                                 BPatch_constExpr(0),
					 BPatch_constExpr(1)),
			 longExpr16_3, longExpr16_4);

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
//
void mutatorTest17(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func17_1"
    BPatch_Vector<BPatch_point *> *point17_1 =
	appImage->findProcedurePoint("func17_1", BPatch_exit);
    if (!point17_1 || (point17_1->size() < 1)) {
	fprintf(stderr, "Unable to find point func17_1 - entry.\n");
	exit(-1);
    }

    BPatch_function *call17_1_func = appImage->findFunction("call17_1");
    if (call17_1_func == NULL) {
	fprintf(stderr, "Unable to find function \"call17_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs;
    funcArgs.push_back(new BPatch_paramExpr(1));
    BPatch_funcCallExpr call17_1Expr(*call17_1_func, funcArgs);
    checkCost(call17_1Expr);
    appThread->insertSnippet(call17_1Expr, *point17_1);

    // Find the exit point to the procedure "func17_2"
    BPatch_Vector<BPatch_point *> *point17_2 =
	appImage->findProcedurePoint("func17_2", BPatch_exit);
    if (!point17_2 || (point17_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func17_2 - entry.\n");
	exit(-1);
    }

    BPatch_function *call17_2_func = appImage->findFunction("call17_2");
    if (call17_2_func == NULL) {
	fprintf(stderr, "Unable to find function \"call17_2.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> funcArgs2;
    funcArgs2.push_back(new BPatch_constExpr(1));
    BPatch_funcCallExpr call17_2Expr(*call17_2_func, funcArgs2);
    checkCost(call17_2Expr);

    // test interface to call into insertSnippet with only one parameter
    const BPatch_point aPoint = *(*point17_2)[0];
    appThread->insertSnippet(call17_2Expr, aPoint, 
	BPatch_callBefore, BPatch_lastSnippet);
}

//
// Start Test Case #18 - mutator side (read/write a variable in the mutatee)
//
void mutatorTest18(BPatch_thread *appThread, BPatch_image *appImage)
{
    BPatch_variableExpr *expr18_1 =appImage->findVariable("globalVariable18_1");
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
    expr18_1->writeValue(&n);
}

//
// Start Test Case #19 - mutator side (oneTimeCode)
//
void mutatorTest19(BPatch_thread *appThread, BPatch_image *appImage)
{
    waitUntilStopped(bpatch, appThread, 19, "oneTimeCode");

    BPatch_function *call19_1func = appImage->findFunction("call19_1");
    if (call19_1func == NULL) {
	fprintf(stderr, "Unable to find function \"call19_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call19_1Expr(*call19_1func, nullArgs);
    checkCost(call19_1Expr);

    appThread->oneTimeCode(call19_1Expr);

    appThread->continueExecution();
#ifdef i386_unknown_nt4_0
    Sleep(1000);
#else
    sleep(1);           /* wait for child to continue */
#endif

    BPatch_function *call19_2func = appImage->findFunction("call19_2");
    if (call19_2func == NULL) {
        fprintf(stderr, "Unable to find function \"call19_2.\"\n");
        exit(1);
    }

    BPatch_funcCallExpr call19_2Expr(*call19_2func, nullArgs);
    checkCost(call19_2Expr);

    appThread->oneTimeCode(call19_2Expr);
}

//
// Start Test Case #20 - mutator side (instrumentation at arbitrary points)
//
void mutatorTest20(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
    BPatch_function *call20_1func = appImage->findFunction("call20_1");
    if (call20_1func == NULL) {
	fprintf(stderr, "Unable to find function \"call20_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call20_1Expr(*call20_1func, nullArgs);
    checkCost(call20_1Expr);


    BPatch_function *f = appImage->findFunction("func20_2");
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

    for (int i = 0; i < f->getSize(); i+= 4) {
	void *addr = (char *)f->getBaseAddr() + i;

	p = appImage->createInstPointAtAddr((char *)f->getBaseAddr() + i);

	if (p) {
	    if (p->getPointType() == BPatch_address) {
		found_one = true;
		if (appThread->insertSnippet(call20_1Expr, *p) == NULL) {
		    fprintf(stderr,
		      "Unable to insert snippet into function \"func20_2.\"\n");
		    exit(1);
		}
	    }
    	    delete p;
	}
    }

    if (!found_one) {
	fprintf(stderr, "Unable to find a point to instrument in function \"func20_2.\"\n");
	exit(1);
    }
#endif
}


//
// Start Test Case #21 - mutator side (findFunction in module)
//
// There is no corresponding failure (test2) testing because the only
// bad input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.
void mutatorTest21(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(mips_sgi_irix6_4) || defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(alpha_dec_osf4_0)
    if (! appThread->loadLibrary("./libtestA.so")) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't load libtestA.so into mutatee\n");
	 exit(1);
    }
    if (! appThread->loadLibrary("./libtestB.so")) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't load libtestB.so into mutatee\n");
	 exit(1);
    }

    // Lookup the libtestA.so and libtestB.so modules that we've just loaded
    BPatch_module *modA = NULL;
    BPatch_module *modB = NULL;
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    if (!mods || mods->size() == 0) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't search modules of mutatee\n");
	 exit(1);
    }
    for (int i = 0; i < mods->size() && !(modA && modB); i++) {
	 char buf[1024];
	 BPatch_module *m = (*mods)[i];
	 m->getName(buf, 1024);
	 if (! strcmp("libtestA.so", buf))
	      modA = m;
	 else if (! strcmp("libtestB.so", buf))
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
	 fprintf(stderr, "  Mutator couldn't find a function in libtestA.so\n");
	 exit(1);
    }
    if (! funcB) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't find a function in libtestB.so\n");
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
#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
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
    for (int i = 0; i < mods->size() && !(modA && modB); i++) {
	 char buf[1024];
	 BPatch_module *m = (*mods)[i];
	 m->getName(buf, 1024);
#if defined(mips_sgi_irix6_4)
	 // On MIPS, module names are appended with "_module"
	 if (! strcmp("libtestA.so_module", buf))
	      modA = m;
	 else if (! strcmp("libtestB.so_module", buf))
	      modB = m;
#else
	 if (! strcmp("libtestA.so", buf))
	      modA = m;
	 else if (! strcmp("libtestB.so", buf))
	      modB = m;
#endif
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
	 exit(1);
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

void mutatorMAIN(char *pathname, bool useAttach)
{
    BPatch_thread *appThread;

    // Create an instance of the bpatch library
    bpatch = new BPatch;

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);

    char *child_argv[4];
   
    int n = 0;
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = "-verbose";
    child_argv[n] = NULL;

    if (useAttach) {
	int pid = startNewProcessForAttach(pathname, child_argv);
	if (pid < 0) {
	    printf("*ERROR*: unable to start tests due to error creating mutatee process\n");
	    exit(-1);
	}

	appThread = bpatch->attachProcess(pathname, pid);
    } else {
	appThread = bpatch->createProcess(pathname, child_argv,NULL);
    }

    if (appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach)
	signalAttached(appThread, appImage);

    int i;
    BPatch_Vector<BPatch_module *> *m = appImage->getModules();
    for (i=0; i < m->size(); i++) {
        // dprintf("func %s\n", (*m)[i]->name());
    }
    BPatch_Vector<BPatch_function *> *p1 = (*m)[0]->getProcedures();

    BPatch_Vector<BPatch_function *> *p = appImage->getProcedures();
    for (i=0; i < p->size(); i++) {
        // dprintf("func %s\n", (*p)[i]->name());
    }
    // Start Test Case #1 - mutator side (call a zero argument function)

    // Find the entry point to the procedure "func1_1"
    BPatch_Vector<BPatch_point *> *point1_1 =
	appImage->findProcedurePoint("func1_1", BPatch_entry);

    if ((*point1_1).size() == 0) {
        fprintf(stderr, "**Failed** test #1 (zero arg function call)\n");
	fprintf(stderr, "    Unable to find entry point to \"func1_1.\"\n");
	exit(1);
    }

    BPatch_function *call1_func = appImage->findFunction("call1_1");
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

    //
    // Start Test Case #2 - mutator side (call a three argument function)
    //

    // Find the entry point to the procedure "func2_1"
    BPatch_Vector<BPatch_point *> *point2_1 =
	appImage->findProcedurePoint("func2_1", BPatch_entry);

    if (!point2_1 || ((*point2_1).size() == 0)) {
	fprintf(stderr, "**Failed** test #2 (three parameter function)\n");
	fprintf(stderr, "    Unable to find entry point to \"func2_1.\"\n");
	exit(1);
    }

    BPatch_function *call2_func = appImage->findFunction("call2_1");
    if (call2_func == NULL) {
	fprintf(stderr, "**Failed** test #2 (three parameter function)\n");
	fprintf(stderr, "    Unable to find function \"call2_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call2_args;
    BPatch_constExpr expr2_1(1);
    BPatch_constExpr expr2_2(2);
    BPatch_constExpr expr2_3("testString2_1");
    call2_args.push_back(&expr2_1);
    call2_args.push_back(&expr2_2);
    call2_args.push_back(&expr2_3);

    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    dprintf("Inserted snippet2\n");
    checkCost(call2Expr);
    appThread->insertSnippet(call2Expr, *point2_1);

    //
    // Start Test Case #3 - mutator side (passing variables to function)
    //

    // Find the entry point to the procedure "func3_1"
    BPatch_Vector<BPatch_point *> *point3_1 =
	appImage->findProcedurePoint("func3_1", BPatch_entry);

    if (!point3_1 || ((*point3_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func3_1.\"\n");
	exit(1);
    }

    BPatch_function *call3_func = appImage->findFunction("call3_1");
    if (call3_func == NULL) {
	fprintf(stderr, "Unable to find function \"call3_1.\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call3_args;

    BPatch_variableExpr *expr3_1 = appImage->findVariable("globalVariable3_1");
    if (!expr3_1) {
	fprintf(stderr, "**Failed** test #3 (passing variables)\n");
	fprintf(stderr, "    Unable to locate variable globalVariable3_1\n");
	exit(1);
    }
    // see if we can find the address
    if (expr3_1->getBaseAddr() <= 0) {
	printf("*Error*: address %d for globalVariable3_1 is not valid\n",
		(int) expr3_1->getBaseAddr());
    }

    BPatch_variableExpr *expr3_2 = appThread->malloc(*appImage->findType("int"));
    if (!expr3_2) {
	fprintf(stderr, "**Failed** test #3 (passing variables)\n");
	fprintf(stderr, "    Unable to create new int variable\n");
	exit(1);
    }

    call3_args.push_back(expr3_1);
    call3_args.push_back(expr3_2);

    BPatch_funcCallExpr call3Expr(*call3_func, call3_args);
    checkCost(call3Expr);
    appThread->insertSnippet(call3Expr, *point3_1);

    BPatch_arithExpr expr3_3(BPatch_assign, *expr3_2, BPatch_constExpr(32));
    checkCost(expr3_3);
    appThread->insertSnippet(expr3_3, *point3_1);

    dprintf("Inserted snippet3\n");
    //
    // Start Test Case #4 - mutator side (sequence)
    //

    // Find the entry point to the procedure "func4_1"
    BPatch_Vector<BPatch_point *> *point4_1 =
	appImage->findProcedurePoint("func4_1", BPatch_entry);


    BPatch_variableExpr *expr4_1 = appImage->findVariable("globalVariable4_1");
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

    //
    // Start Test Case #5 - mutator side (if w.o. else)
    //

    // Find the entry point to the procedure "func5_1"
    BPatch_Vector<BPatch_point *> *point5_1 =
	appImage->findProcedurePoint("func5_1", BPatch_entry);
    BPatch_variableExpr *expr5_1 = appImage->findVariable("globalVariable5_1");
    BPatch_variableExpr *expr5_2 = appImage->findVariable("globalVariable5_2");
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
    mutatorTest6(appThread, appImage);
    mutatorTest7(appThread, appImage);

    mutatorTest8(appThread, appImage);

    mutatorTest9(appThread, appImage);

    mutatorTest10(appThread, appImage);

    mutatorTest11(appThread, appImage);

    mutatorTest12a(appThread, appImage);
    mutatorTest13(appThread, appImage);
    mutatorTest14(appThread, appImage);
    mutatorTest15a(appThread, appImage);
    mutatorTest16(appThread, appImage);
    mutatorTest17(appThread, appImage);

    mutatorTest18(appThread, appImage);

    mutatorTest20(appThread, appImage);
    mutatorTest21(appThread, appImage);    // find function in module
    mutatorTest22(appThread, appImage);    // replaceFunction

    // Start of code to continue the process.  All mutations made
    // above will be in place before the mutatee begins its tests.

    dprintf("starting program execution.\n");
    appThread->continueExecution();

    // Test poll for status change
    bool changed = bpatch->pollForStatusChange();

    mutatorTest12b(appThread, appImage);
    mutatorTest15b(appThread, appImage);
    mutatorTest19(appThread, appImage);

    while (!appThread->isTerminated())
	bpatch->waitForStatusChange();

    dprintf("Done.\n");
}

//
// main - decide our role and call the correct "main"
//
main(int argc, char *argv[])
{
    bool useAttach = false;

    for (int i=1; i < argc; i++) {
	if (!strcmp(argv[i], "-verbose")) {
	    debugPrint = 1;
	} else if (!strcmp(argv[i], "-attach")) {
	    useAttach = true;
	} else {
	    fprintf(stderr, "Usage: test1 [-attach] [-verbose]\n");
	    exit(-1);
	}
    }

#if defined(sparc_sun_sunos4_1_3)
    if (useAttach) {
        printf("Attach is not supported on this platform.\n");
        exit(1);
    }
#endif


#ifdef i386_unknown_nt4_0
    mutatorMAIN("test1.mutatee.exe", useAttach);
#else
    mutatorMAIN("./test1.mutatee", useAttach);
#endif

    return 0;
}
