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

// $Id: test1_36.C,v 1.1 2008/10/30 19:19:19 legendre Exp $
/*
 * #Name: test1_36
 * #Desc: Callsite Parameter Referencing
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_36_Mutator : public DyninstMutator {

	virtual test_results_t executeTest();

	BPatch_arithExpr *makeTest36paramExpr(BPatch_snippet *expr, int paramId);
};

extern "C" DLLEXPORT  TestMutator *test1_36_factory() 
{
	return new test1_36_Mutator();
}

BPatch_arithExpr *test1_36_Mutator::makeTest36paramExpr(BPatch_snippet *expr,
		int paramId)
{
	if (isMutateeFortran(appImage)) 
	{
		// Fortran is call by reference
		BPatch_arithExpr *derefExpr = new BPatch_arithExpr(BPatch_deref, *(new BPatch_paramExpr(paramId)));
		assert(derefExpr);
		return new BPatch_arithExpr(BPatch_assign, *expr, *derefExpr);
	} 
	else 
	{
		return new BPatch_arithExpr(BPatch_assign, *expr, *(new BPatch_paramExpr(paramId)));
	}
}

//
// Start Test Case #36 - (callsite parameter referencing)
//

test_results_t test1_36_Mutator::executeTest() 
{
	const char *funcName = "test1_36_func1";
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

	BPatch_Vector<BPatch_point *> *all_points36_1 =
		found_funcs[0]->findPoint(BPatch_subroutine);

	if (!all_points36_1 || (all_points36_1->size() < 1)) 
	{
		logerror("Unable to find point %s - subroutines.\n", funcName);
		return FAILED;
	}

	const char *funcName2 = "test1_36_call1";
	BPatch_point *point36_1 = NULL;

	for (unsigned i=0; i<(*all_points36_1).size(); i++) 
	{
		BPatch_point *cur_point = (*all_points36_1)[i];
		if (cur_point == NULL) 
			continue;

		BPatch_function *func = cur_point->getCalledFunction();
		char funcname[100];

		if (!func) 
			continue;

		if (func->getName(funcname, 99)) 
		{
			if (strstr(funcname, funcName2))
				point36_1 = cur_point;
		}
	}

	if (point36_1 == NULL) 
	{
		logerror("Unable to find callsite %s\n", funcName2);
		return FAILED;
	}

	BPatch_variableExpr *expr36_1 = findVariable(appImage,
			"test1_36_globalVariable1", 
			all_points36_1);
	BPatch_variableExpr *expr36_2 = findVariable(appImage,
			"test1_36_globalVariable2", 
			all_points36_1);
	BPatch_variableExpr *expr36_3 = findVariable(appImage,
			"test1_36_globalVariable3", 
			all_points36_1);
	BPatch_variableExpr *expr36_4 = findVariable(appImage,
			"test1_36_globalVariable4", 
			all_points36_1);
	BPatch_variableExpr *expr36_5 = findVariable(appImage,
			"test1_36_globalVariable5", 
			all_points36_1);
	BPatch_variableExpr *expr36_6 = findVariable(appImage,
			"test1_36_globalVariable6", 
			all_points36_1);
	BPatch_variableExpr *expr36_7 = findVariable(appImage,
			"test1_36_globalVariable7", 
			all_points36_1);
	BPatch_variableExpr *expr36_8 = findVariable(appImage,
			"test1_36_globalVariable8", 
			all_points36_1);
	BPatch_variableExpr *expr36_9 = findVariable(appImage,
			"test1_36_globalVariable9", 
			all_points36_1);
	BPatch_variableExpr *expr36_10 = findVariable(appImage,
			"test1_36_globalVariable10", 
			all_points36_1);

	if (expr36_1 == NULL || expr36_2 == NULL || expr36_3 == NULL ||
			expr36_4 == NULL || expr36_5 == NULL || expr36_6 == NULL ||
			expr36_7 == NULL || expr36_8 == NULL || expr36_9 == NULL ||
			expr36_10 == NULL)
	{
		logerror("**Failed** test #36 (callsite parameter referencing)\n");
		logerror("    Unable to locate at least one of "
				"test1_36_globalVariable{1...10}\n");
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet *> snippet_seq;
	snippet_seq.push_back(makeTest36paramExpr(expr36_1, 0));
	snippet_seq.push_back(makeTest36paramExpr(expr36_2, 1));
	snippet_seq.push_back(makeTest36paramExpr(expr36_3, 2));
	snippet_seq.push_back(makeTest36paramExpr(expr36_4, 3));
	snippet_seq.push_back(makeTest36paramExpr(expr36_5, 4));
	snippet_seq.push_back(makeTest36paramExpr(expr36_6, 5));
	snippet_seq.push_back(makeTest36paramExpr(expr36_7, 6));
	snippet_seq.push_back(makeTest36paramExpr(expr36_8, 7));
	snippet_seq.push_back(makeTest36paramExpr(expr36_9, 8));
	snippet_seq.push_back(makeTest36paramExpr(expr36_10, 9));
	
    BPatch_sequence seqExpr(snippet_seq);

	checkCost(seqExpr);
	appAddrSpace->insertSnippet(seqExpr, *point36_1);

	return PASSED;
}

