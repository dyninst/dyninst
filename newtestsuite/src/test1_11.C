/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test1_11.C,v 1.1 2007/09/24 16:36:23 cooksey Exp $
/*
 * #Name: test1_11
 * #Desc: Mutator Side - Snippets at Entry,Exit,Call
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "TestMutator.h"

class test1_11_Mutator : public TestMutator {
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test1_11_factory() {
  return new test1_11_Mutator();
}

//
// Start Test Case #11 - mutator side (snippets at entry,exit,call)
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_11_Mutator::preExecution() {
    // Find the entry point to the procedure "func11_1"
  const char *funcName = "test1_11_func1";
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(funcName, found_funcs))
	|| !found_funcs.size()) {
      logerror("    Unable to find function %s\n", funcName);
      return FAILED;
    }

    if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), funcName);
    }

    BPatch_Vector<BPatch_point *> *point11_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point11_1 || (point11_1->size() < 1)) {
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("     Unable to find point %s - entry.\n", funcName);
      return FAILED;
    }

    // Find the subroutine points for the procedure "func11_1"
    BPatch_Vector<BPatch_point *> *point11_2 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!point11_2 || (point11_2->size() < 1)) {
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("    Unable to find point %s - calls.\n", funcName);
      return FAILED;
    }

    // Find the exit point to the procedure "func11_1"
    BPatch_Vector<BPatch_point *> *point11_3 = found_funcs[0]->findPoint(BPatch_exit);
 
    if (!point11_3 || (point11_3->size() < 1)) {
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("    Unable to find point %s - exit.\n", funcName);
      return FAILED;
    }



    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "test1_11_call1";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("    Unable to find function %s\n", fn);
      return FAILED;
    }

    BPatch_function *call11_1_func = bpfv[0];
    bpfv.clear();

    char *fn2 = "test1_11_call2";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("    Unable to find function %s\n", fn2);
      return FAILED;
    }

    BPatch_function *call11_2_func = bpfv[0];
    bpfv.clear();

    char *fn3 = "test1_11_call3";
    if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("    Unable to find function %s\n", fn3);
      return FAILED;
    }
    BPatch_function *call11_3_func = bpfv[0];
    bpfv.clear();

    char *fn4 = "test1_11_call4";
    if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
      logerror("    Unable to find function %s\n", fn4);
      return FAILED;
    }
    BPatch_function *call11_4_func = bpfv[0];

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

    return PASSED;
}
