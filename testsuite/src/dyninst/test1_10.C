/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: test1_10.C,v 1.1 2008/10/30 19:17:23 legendre Exp $
/*
 * #Name: test1_10
 * #Desc: Mutator Side - Insert Snippet Order
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_10_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_10_factory() 
{
	return new test1_10_Mutator();
}

//
// Start Test Case #10 - mutator side (insert snippet order)
//

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
	const char *fn = "test1_10_call1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call10_1_func = bpfv[0];
	bpfv.clear();

	const char *fn2 = "test1_10_call2";

	if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn2);
		return FAILED;
	}

	BPatch_function *call10_2_func = bpfv[0];
	bpfv.clear();

	const char *fn3 = "test1_10_call3";

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
	appAddrSpace->insertSnippet( call10_2Expr, *point10_1);

	dprintf("%s[%d]:  before insertSnippet 2\n", FILE__, __LINE__);

	checkCost(call10_1Expr);
	appAddrSpace->insertSnippet( call10_1Expr, *point10_1, BPatch_callBefore, 
			BPatch_firstSnippet);

	dprintf("%s[%d]:  before insertSnippet 3\n", FILE__, __LINE__);

	checkCost(call10_3Expr);
	appAddrSpace->insertSnippet( call10_3Expr, *point10_1, BPatch_callBefore, 
			BPatch_lastSnippet);

	dprintf("%s[%d]:  leaving  test1_10\n", FILE__, __LINE__);
	return PASSED;
}
