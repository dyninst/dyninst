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

// $Id: test1_13.C,v 1.1 2008/10/30 19:17:38 legendre Exp $
/*
 * #Name: test1_13
 * #Desc: Mutator Side - paramExpr,retExpr,nullExpr
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

class test1_13_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_13_factory() 
{
	return new test1_13_Mutator();
}

//
// Start Test Case #13 - mutator side (paramExpr,retExpr,nullExpr)
//

test_results_t test1_13_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func13_1"
	const char *funcName = "test1_13_func1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point13_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point13_1 || (point13_1->size() < 1)) 
	{
		logerror("Unable to find point %s - entry.\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_13_call1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call13_1_func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> funcArgs;

	funcArgs.push_back(new BPatch_paramExpr(0));
	funcArgs.push_back(new BPatch_paramExpr(1));
	funcArgs.push_back(new BPatch_paramExpr(2));
	funcArgs.push_back(new BPatch_paramExpr(3));
	funcArgs.push_back(new BPatch_paramExpr(4));
	BPatch_funcCallExpr call13_1Expr(*call13_1_func, funcArgs);

	checkCost(call13_1Expr);
	appAddrSpace->insertSnippet(call13_1Expr, *point13_1);

	BPatch_nullExpr call13_2Expr;
	checkCost(call13_2Expr);
	appAddrSpace->insertSnippet(call13_2Expr, *point13_1);

	// now test that a return value can be read.
	const char *funcName2 = "test1_13_func2";
	BPatch_Vector<BPatch_function *> found_funcs2;

	if ((NULL == appImage->findFunction(funcName2, found_funcs2)) || !found_funcs2.size()) 
	{
		logerror("    Unable to find function %s\n", funcName2);
		return FAILED;
	}

	if (1 < found_funcs2.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs2.size(), funcName2);
	}

	BPatch_Vector<BPatch_point *> *point13_2 = found_funcs2[0]->findPoint(BPatch_exit);

	if (!point13_2 || (point13_2->size() < 1)) 
	{
		logerror("Unable to find point %s - exit.\n", funcName2);
		return FAILED;
	}

	bpfv.clear();

	const char *fn2 = "test1_13_call2";

	if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn2);
		return FAILED;
	}

	BPatch_function *call13_2_func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> funcArgs2;

	BPatch_variableExpr *expr13_1;
	BPatch_retExpr *ret_var;
	BPatch_constExpr expr13_2 (0);

	int mutateeFortran = isMutateeFortran(appImage);

	if (mutateeFortran) 
	{
		expr13_1 = appAddrSpace->malloc (*appImage->findType ("int"));
		ret_var = new BPatch_retExpr();
		BPatch_arithExpr test_arith (BPatch_assign, *expr13_1, *ret_var);
		appAddrSpace->insertSnippet (test_arith, *point13_2);
		expr13_2 = expr13_1->getBaseAddr ();
		funcArgs2.push_back (&expr13_2);
	} 
	else 
	{
		funcArgs2.push_back(new BPatch_retExpr());
	}

	BPatch_funcCallExpr call13_3Expr(*call13_2_func, funcArgs2);

	checkCost(call13_1Expr);
	BPatchSnippetHandle* goodRetExpr = appAddrSpace->insertSnippet(call13_3Expr, *point13_2, BPatch_callAfter, BPatch_lastSnippet);
	if(!goodRetExpr) {
		logerror("Failed: couldn't insert return expression at exit point of func13_2\n");
		return FAILED;
	}
	// Preserving this so that nobody gets a "clever" idea in the future.
	// There is inconsistency across our various platforms/compilers as to whether
	// type information's absence is used to signify "this returns void", or whether it means
	// there is no type information. This means we have to fail toward no type info = untyped expression
	// and allow insertion.
	// However, gcc *does* overload this to "void return == no return type info", so we can't actually test
	// that a void function can't take a return value snippet.

	// This insertion *should* fail, as we're trying to instrument the return point of a
	// void function.
	//if(insertSnippetAt(appAddrSpace, appImage, "test1_13_func3", BPatch_exit, call13_3Expr, 13, 
	//		   "Test type system: void functions can't take retExprs")) {
	//  logerror("Failed: retExpr inserted into void function\n");
	//  
	//  return FAILED;
	//}
	
	
	return PASSED;
} // test1_13_Mutator::executeTest()
