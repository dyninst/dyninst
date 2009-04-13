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

// $Id: test1_10.C,v 1.1 2008/10/30 19:17:23 legendre Exp $
/*
 * #Name: test1_10
 * #Desc: Mutator Side - Insert Snippet Order
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test1_10_Mutator : public DyninstMutator {
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test1_10_factory() {
  return new test1_10_Mutator();
}

//
// Start Test Case #10 - mutator side (insert snippet order)
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {

test_results_t test1_10_Mutator::executeTest() 
{
	dprintf("%s[%d]:  welcome to test1_10\n", FILE__, __LINE__);
	// Find the entry point to the procedure "func10_1"
	const char *funcName = "test1_10_func1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs))
			|| !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point10_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point10_1 || ((*point10_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}


	BPatch_Vector<BPatch_function *> bpfv;
	char *fn = "test1_10_call1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call10_1_func = bpfv[0];
	bpfv.clear();

	char *fn2 = "test1_10_call2";

	if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn2);
		return FAILED;
	}

	BPatch_function *call10_2_func = bpfv[0];
	bpfv.clear();

	char *fn3 = "test1_10_call3";

	if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn3);
		return FAILED;
	}

	BPatch_function *call10_3_func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> nullArgs;
	BPatch_funcCallExpr call10_1Expr(*call10_1_func, nullArgs);
	BPatch_funcCallExpr call10_2Expr(*call10_2_func, nullArgs);
	BPatch_funcCallExpr call10_3Expr(*call10_3_func, nullArgs);

	dprintf("%s[%d]:  before insertSnippet 1\n", FILE__, __LINE__);

	checkCost(call10_2Expr);
	appThread->insertSnippet( call10_2Expr, *point10_1);

	dprintf("%s[%d]:  before insertSnippet 2\n", FILE__, __LINE__);

	checkCost(call10_1Expr);
	appThread->insertSnippet( call10_1Expr, *point10_1, BPatch_callBefore, 
			BPatch_firstSnippet);

	dprintf("%s[%d]:  before insertSnippet 3\n", FILE__, __LINE__);

	checkCost(call10_3Expr);
	appThread->insertSnippet( call10_3Expr, *point10_1, BPatch_callBefore, 
			BPatch_lastSnippet);

	dprintf("%s[%d]:  leaving  test1_10\n", FILE__, __LINE__);
	return PASSED;
}
