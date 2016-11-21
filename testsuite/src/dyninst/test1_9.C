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

// $Id: test1_9.C,v 1.1 2008/10/30 19:20:03 legendre Exp $
/*
 * #Name: test1_9
 * #Desc: Mutator Side - Preserve Registers - FuncCall
 * #Arch: all
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

class test1_9_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_9_factory() 
{
	return new test1_9_Mutator();
}

//
// Start Test Case #9 - mutator side (preserve registers - funcCall)
//

test_results_t test1_9_Mutator::executeTest() 
{
	// Find the entry point to the procedure "test1_9_func1"
	const char *funcName = "test1_9_func1";
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

	BPatch_Vector<BPatch_point *> *point9_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point9_1 || ((*point9_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_9_call1";
	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call9_func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> call9_args;

	BPatch_variableExpr *expr9_1 = appAddrSpace->malloc (*appImage->findType ("int"));
	BPatch_constExpr constExpr9_1 (0);
	BPatch_arithExpr arithexpr9_1 (BPatch_assign, *expr9_1, BPatch_constExpr (91));
	appAddrSpace->insertSnippet (arithexpr9_1, *point9_1);

	int mutateeFortran = isMutateeFortran(appImage);

	if (mutateeFortran) 
	{
		constExpr9_1 = expr9_1->getBaseAddr ();
	} 
	else 
	{
		constExpr9_1 = 91;
	}

	call9_args.push_back(&constExpr9_1);

	BPatch_variableExpr *expr9_2 = appAddrSpace->malloc (*appImage->findType ("int"));
	BPatch_constExpr constExpr9_2 (0);
	BPatch_arithExpr arithexpr9_2 (BPatch_assign, *expr9_2, BPatch_constExpr (92));
	appAddrSpace->insertSnippet (arithexpr9_2, *point9_1);

	if (mutateeFortran) 
	{
		constExpr9_2 = expr9_2->getBaseAddr ();
	} 
	else 
	{
		constExpr9_2 = 92;
	}

	call9_args.push_back(&constExpr9_2);

	BPatch_variableExpr *expr9_3 = appAddrSpace->malloc (*appImage->findType ("int"));
	BPatch_constExpr constExpr9_3 (0);
	BPatch_arithExpr arithexpr9_3 (BPatch_assign, *expr9_3, BPatch_constExpr (93));
	appAddrSpace->insertSnippet (arithexpr9_3, *point9_1);

	if (mutateeFortran) 
	{
		constExpr9_3 = expr9_3->getBaseAddr ();
	} 
	else 
	{
		constExpr9_3 = 93;
	}

	call9_args.push_back(&constExpr9_3);

	BPatch_variableExpr *expr9_4 = appAddrSpace->malloc (*appImage->findType ("int"));
	BPatch_constExpr constExpr9_4 (0);
	BPatch_arithExpr arithexpr9_4 (BPatch_assign, *expr9_4, BPatch_constExpr (94));
	appAddrSpace->insertSnippet (arithexpr9_4, *point9_1);

	if (mutateeFortran) 
	{
		constExpr9_4 = expr9_4->getBaseAddr ();
	} 
	else 
	{
		constExpr9_4 = 94;
	}

	call9_args.push_back(&constExpr9_4);

	BPatch_variableExpr *expr9_5 = appAddrSpace->malloc (*appImage->findType ("int"));
	BPatch_constExpr constExpr9_5 (0);
	BPatch_arithExpr arithexpr9_5 (BPatch_assign, *expr9_5, BPatch_constExpr (95));
	appAddrSpace->insertSnippet (arithexpr9_5, *point9_1);

	if (mutateeFortran) 
	{
		constExpr9_5 = expr9_5->getBaseAddr ();
	} 
	else 
	{
		constExpr9_5 = 95;
	}

	call9_args.push_back(&constExpr9_5);

	BPatch_funcCallExpr call9Expr(*call9_func, call9_args);

	checkCost(call9Expr);
	appAddrSpace->insertSnippet(call9Expr, *point9_1, BPatch_callBefore, BPatch_lastSnippet);

	return PASSED;
}
