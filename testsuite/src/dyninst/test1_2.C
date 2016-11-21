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

// $Id: test1_2.C,v 1.1 2008/10/30 19:18:17 legendre Exp $
/*
 * #Name: test1_2
 * #Desc: Mutator Side (call a four argument function)
 * #Arch: all
 * #Dep: 
 * #Notes: Uses mutateeFortran variable
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"
#include "test1.h"

#include "dyninst_comp.h"

class test1_2_Mutator : public DyninstMutator {
	virtual DLLEXPORT test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator *test1_2_factory() 
{
	return new test1_2_Mutator();
}

//
// Start Test Case #2 - mutator side (call a four argument function)
//

test_results_t DLLEXPORT test1_2_Mutator::executeTest() 
{
	const char *funcName = "test1_2_func2_1";
	const char* testName = "four parameter function";
	int testNo = 2;

	// Find the entry point to the procedure "func2_1"

	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction(funcName, found_funcs))
			|| !found_funcs.size()) 
	{
		logerror("\tUnable to find function %s\n",
				funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point2_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point2_1 || ((*point2_1).size() == 0)) 
	{
		logerror("**Failed** test #%d (%s)\n", testNo, testName);
		logerror("    Unable to find entry point to \"%s.\"\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_2_call2_1";

	if (NULL == appImage->findFunction(fn, bpfv) 
			|| !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed** test #%d (%s)\n", testNo, testName);
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call2_func = bpfv[0];

	void *ptr;

#if defined(mips_sgi_irix6_4_test) || defined(arch_x86_64_test) || defined (ppc64_linux)

	unsigned pointer_size = pointerSize(appImage);

	/* Determine the size of pointer we should use dynamically. */

	if (pointer_size == 4) 
	{
		ptr = TEST_PTR_32BIT;
	}
	else if (pointer_size == 8) 
	{
		ptr = TEST_PTR_64BIT;
	} else 
	{
		logerror("**Failed** test #2 (four parameter function)\n");
		logerror("    Unexpected value for pointerSize\n");
		return FAILED;
	}

#else

	/* For platforms where there is only one possible size for a pointer. */
	ptr = TEST_PTR;

#endif

	BPatch_Vector<BPatch_snippet *> call2_args;

	BPatch_constExpr expr2_1 (0), expr2_2 (0), expr2_3 (0), expr2_4 (0);

	int mutateeFortran = isMutateeFortran(appImage);

	if (mutateeFortran) 
	{
		BPatch_variableExpr *expr2_5 = appAddrSpace->malloc (*appImage->findType ("int"));
		BPatch_variableExpr *expr2_6 = appAddrSpace->malloc (*appImage->findType ("int"));

		expr2_1 = expr2_5->getBaseAddr ();
		expr2_2 = expr2_6->getBaseAddr ();

		BPatch_arithExpr expr2_7 (BPatch_assign, *expr2_5, BPatch_constExpr(1));
		appAddrSpace->insertSnippet (expr2_7, *point2_1);

		BPatch_arithExpr expr2_8 (BPatch_assign, *expr2_6, BPatch_constExpr(2));
		appAddrSpace->insertSnippet (expr2_8, *point2_1);

		expr2_3 = "testString2_1";
		expr2_4 = 13;
	} 
	else 
	{
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
	appAddrSpace->insertSnippet(call2Expr, *point2_1, BPatch_callBefore, BPatch_lastSnippet);

	return PASSED;
}
