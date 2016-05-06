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

// $Id: test1_11.C,v 1.1 2008/10/30 19:17:28 legendre Exp $
/*
 * #Name: test1_11
 * #Desc: Mutator Side - Snippets at Entry,Exit,Call
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

class test1_11_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_11_factory() 
{
	return new test1_11_Mutator();
}

//
// Start Test Case #11 - mutator side (snippets at entry,exit,call)
//

test_results_t test1_11_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func11_1"
	const char *funcName = "test1_11_func1";
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

	BPatch_Vector<BPatch_point *> *point11_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point11_1 || (point11_1->size() < 1)) 
	{
		logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
		logerror("     Unable to find point %s - entry.\n", funcName);
		return FAILED;
	}

	// Find the subroutine points for the procedure "func11_1"
	BPatch_Vector<BPatch_point *> *point11_2 = found_funcs[0]->findPoint(BPatch_subroutine);

	if (!point11_2 || (point11_2->size() < 1)) 
	{
		logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
		logerror("    Unable to find point %s - calls.\n", funcName);
		return FAILED;
	}

	// Find the exit point to the procedure "func11_1"
	BPatch_Vector<BPatch_point *> *point11_3 = found_funcs[0]->findPoint(BPatch_exit);

	if (!point11_3 || (point11_3->size() < 1)) 
	{
		logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
		logerror("    Unable to find point %s - exit.\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_11_call1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call11_1_func = bpfv[0];
	bpfv.clear();

	const char *fn2 = "test1_11_call2";

	if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
		logerror("    Unable to find function %s\n", fn2);
		return FAILED;
	}

	BPatch_function *call11_2_func = bpfv[0];
	bpfv.clear();

	const char *fn3 = "test1_11_call3";

	if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed** test1_11 (Snippets at Entry,Exit,Call)\n");
		logerror("    Unable to find function %s\n", fn3);
		return FAILED;
	}

	BPatch_function *call11_3_func = bpfv[0];
	bpfv.clear();

	const char *fn4 = "test1_11_call4";

	if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
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
	appAddrSpace->insertSnippet(call11_1Expr, *point11_1);

	checkCost(call11_2Expr);
	appAddrSpace->insertSnippet(call11_2Expr, *point11_2, BPatch_callBefore);

	checkCost(call11_3Expr);
	appAddrSpace->insertSnippet(call11_3Expr, *point11_2, BPatch_callAfter);

	checkCost(call11_4Expr);
	appAddrSpace->insertSnippet(call11_4Expr, *point11_3);

	return PASSED;
}
