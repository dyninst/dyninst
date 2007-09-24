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

// $Id: test1_23.C,v 1.1 2007/09/24 16:37:26 cooksey Exp $
/*
 * #Name: test1_23
 * #Desc: Local Variables
 * #Dep: !mips_sgi_irix6_4
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

// static int mutateeFortran;

#include "TestMutator.h"
class test1_23_Mutator : public TestMutator {
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test1_23_factory() {
  return new test1_23_Mutator();
}

//
// Start Test Case #23 - local variables
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_23_Mutator::preExecution() {
// #if !defined(mips_sgi_irix6_4)
  if (isMutateeFortran(appImage)) {
    return SKIPPED;
  }
  // if (!mutateeFortran) {

  const char *funcName = "test1_23_call1";
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(funcName, found_funcs, 1)) 
      || !found_funcs.size()) {
    logerror("    Unable to find function %s\n", funcName);
    return FAILED;
  }
        
  if (1 < found_funcs.size()) {
    logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	     __FILE__, __LINE__, found_funcs.size(), funcName);

  }

  BPatch_Vector<BPatch_point *> *point23_calls = found_funcs[0]->findPoint(BPatch_subroutine);    
  if (!point23_calls || (point23_calls->size() < 1)) {
    logerror("**Failed** test #23 (local variables)\n");
    logerror("  Unable to find point %s - subroutine calls\n", funcName);
    return FAILED;
  }
  /* We only want the first one... */
  BPatch_Vector<BPatch_point *> point23_1;
  point23_1.push_back((*point23_calls)[0]);

  BPatch_variableExpr *var1 = appImage->findVariable(*(point23_1[0]),
						     "localVariable23_1");
  BPatch_variableExpr *var2 = appImage->findVariable(*(point23_1[0]),
						     "test1_23_shadowVariable1");
  BPatch_variableExpr *var3 = appImage->findVariable("test1_23_shadowVariable2");
  BPatch_variableExpr *var4 = appImage->findVariable("test1_23_globalVariable1");
    
  if (!var1 || !var2 || !var3 || !var4) {
    logerror("**Failed** test #23 (local variables)\n");
    if (!var1)
      logerror("  can't find local variable localVariable23_1\n");
    if (!var2)
      logerror("  can't find local variable test1_23_shadowVariable1\n");
    if (!var3)
      logerror("  can't find global variable test1_23_shadowVariable2\n");
    return FAILED;
  }
    
  BPatch_arithExpr expr23_1(BPatch_assign, *var1, BPatch_constExpr(2300001));
  BPatch_arithExpr expr23_2(BPatch_assign, *var2, BPatch_constExpr(2300012));
  BPatch_arithExpr expr23_3(BPatch_assign, *var3, BPatch_constExpr(2300023));
  BPatch_arithExpr expr23_4(BPatch_assign, *var4, *var1);
    
  BPatch_Vector<BPatch_snippet *> exprs;
    
  exprs.push_back(&expr23_1);
  exprs.push_back(&expr23_2);
  exprs.push_back(&expr23_3);
  exprs.push_back(&expr23_4);
    
  BPatch_sequence allParts(exprs);
    
  // this should not be needed???  JAW
  //BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);
    
  appThread->insertSnippet(allParts, point23_1);
  // }
// #endif
    return PASSED;
}
