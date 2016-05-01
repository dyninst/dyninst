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

// $Id: test1_3.C,v 1.1 2008/10/30 19:18:54 legendre Exp $
/*
 * #Name: test1_3
 * #Desc: Mutator Side (passing variables to a function)
 * #Arch: all
 * #Dep: 
 * #Note: Mutatee fortran
 */

// TODO Make build system stitch these #includes onto the beginning of the
// test file

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

// ********************************************************************
// *** Everything above this line should be automatically generated ***
// ********************************************************************

#include "dyninst_comp.h"

class test1_3_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_3_factory() 
{
	return new test1_3_Mutator();
}

//
// Start Test Case #3 - mutator side (passing variables to function)
//

test_results_t test1_3_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func3_1"
	const char *funcName = "test1_3_func3_1";

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

	BPatch_Vector<BPatch_point *> *point3_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point3_1 || ((*point3_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s.\"\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_3_call3_1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	if (1 < bpfv.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, bpfv.size(), fn);
	}

	BPatch_function *call3_func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> call3_args;

	BPatch_Vector<BPatch_point *> *call3_1 = call3_func->findPoint(BPatch_subroutine);

	if (!call3_1 || ((*call3_1).size() == 0)) 
	{
		logerror("    Unable to find subroutine calls in \"call3_1.\"\n");
		return FAILED;
	}

	const char *globalVar = "test1_3_globalVariable3_1";

	BPatch_variableExpr *expr3_1 = findVariable(appImage, globalVar, call3_1);

	if (!expr3_1) 
	{
		logerror("**Failed** test #3 (passing variables)\n");
		logerror("    Unable to locate variable %s\n", globalVar);
		return FAILED;
	}

	// see if we can find the address
	if (expr3_1->getBaseAddr() <= 0) 
	{
		logerror("*Error*: address %p for %s is not valid\n",
				expr3_1->getBaseAddr(), globalVar);
	}

	BPatch_variableExpr *expr3_2 = appAddrSpace->malloc(*appImage->findType("int"));

	if (!expr3_2) 
	{
		logerror("**Failed** test #3 (passing variables)\n");
		logerror("    Unable to create new int variable\n");
		return FAILED;
	}

	BPatch_constExpr expr3_3 (expr3_1->getBaseAddr ());
	BPatch_constExpr expr3_4 (expr3_2->getBaseAddr ());

	int mutateeFortran = isMutateeFortran(appImage);

	if (mutateeFortran) 
	{
		call3_args.push_back (&expr3_3);
		call3_args.push_back (&expr3_4);
	} 
	else 
	{
		call3_args.push_back(expr3_1);
		call3_args.push_back(expr3_2);
	}

	BPatch_funcCallExpr call3Expr(*call3_func, call3_args);
	checkCost(call3Expr);
	appAddrSpace->insertSnippet(call3Expr, *point3_1);

	BPatch_arithExpr expr3_5(BPatch_assign, *expr3_2, BPatch_constExpr(32));
	checkCost(expr3_5);
	appAddrSpace->insertSnippet(expr3_5, *point3_1);

	dprintf("Inserted snippet3\n");

	return PASSED;
}
