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

// $Id: test1_37.C,v 1.1 2008/10/30 19:19:24 legendre Exp $
/*
 * #Name: test1_37
 * #Desc: Instrument Loops
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"
#include "BPatch_point.h"
#include "BPatch_basicBlockLoop.h"

#include "test_lib.h"
#include "Callbacks.h"
#include "dyninst_comp.h"

class test1_37_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_37_factory() 
{
	return new test1_37_Mutator();
}

//
// Start Test Case #37 - (loop instrumentation)
//

// sort basic blocks ascending by block number
static void sort_blocks(BPatch_Vector<BPatch_basicBlock*> &a, int n) 
{
	for (int i=0; i<n-1; i++) 
	{
		for (int j=0; j<n-1-i; j++)
			if (a[j+1]->getBlockNumber() < a[j]->getBlockNumber()) 
			{
				BPatch_basicBlock* tmp = a[j]; 
				a[j] = a[j+1];
				a[j+1] = tmp;
			}
	}
}

/* This method instruments the entry and exit edges of a loop with 
   the passed-in function. It accomplishes this by looking up the entry
   and exit blocks of the loop, finding the edges that do not come from
   or target other blocks in the loop (respectively), and instrumenting
   those edges. So effectively, this is a test of both our loop detection
   and edge instrumentation facilities. Two for one, yay!
 */

static void instrumentLoops(BPatch_addressSpace *appAddrSpace, BPatch_image *appImage,
			    BPatch_Vector<BPatch_basicBlockLoop*> &loops,
			    BPatch_snippet &snippet)
{            
	// for each loop (set of basic blocks)
	for (unsigned int i = 0; i < loops.size(); i++) 
	{
		BPatch_flowGraph *cfg; 
		BPatch_Vector<BPatch_point*> * exits;
		BPatch_Vector<BPatch_point*> * entries;

		cfg = loops[i]->getFlowGraph();

		// Find loop entry and exit points
		entries = cfg->findLoopInstPoints(BPatch_locLoopEntry,
				loops[i]);
		exits = cfg->findLoopInstPoints(BPatch_locLoopExit,
				loops[i]);
		// instrument those points      

		if (entries->size() == 0) 
		{
			logerror("**Failed** test #37 (instrument loops)\n");
			logerror("   Unable to find loop entry inst point.\n");
		}

		if (exits->size() == 0) 
		{
			logerror("**Failed** test #37 (instrument loops)\n");
			logerror("   Unable to find loop exit inst point.\n");
		}

		unsigned int j;
		BPatch_point *p = NULL;

		for (j=0;j<entries->size();j++) 
		{
			p = (*entries)[j];

			BPatchSnippetHandle * han =
				appAddrSpace->insertSnippet(snippet, *p, BPatch_callBefore);

			// did we insert the snippet?
			if (han == NULL) 
			{
				logerror("**Failed** test #37 (instrument loops)\n");
				logerror("   Unable to insert snippet at loop entry.\n");
                                //cfg->dump();
                        }
		}
		for (j=0;j<exits->size();j++) 
		{
			p = (*exits)[j];

			BPatchSnippetHandle * han =
				appAddrSpace->insertSnippet(snippet, *p, BPatch_callBefore);

			// did we insert the snippet?
			if (han == NULL) 
			{
				logerror("**Failed** test #37 (instrument loops)\n");
				logerror("   Unable to insert snippet at loop exit.\n");
                                //cfg->dump();
			}
		}

		// we are responsible for releasing the point vectors
		delete entries;
		delete exits;

		BPatch_Vector<BPatch_basicBlockLoop*> lps;
		loops[i]->getOuterLoops(lps);

		// recur with this loop's outer loops
		instrumentLoops(appAddrSpace, appImage, lps, snippet);
	}
}

static int instrumentFuncLoopsWithCall(BPatch_addressSpace *appAddrSpace, 
				       BPatch_image *appImage,
				       const char *call_func,
				       const char *inc_func)
{
	// get function * for call_func
	BPatch_Vector<BPatch_function *> funcs;

	appImage->findFunction(call_func, funcs);
	BPatch_function *func = funcs[0];

	// get function * for inc_func
	BPatch_Vector<BPatch_function *> funcs2;
	appImage->findFunction(inc_func, funcs2);
	BPatch_function *incVar = funcs2[0];

	if (func == NULL || incVar == NULL) 
	{
		logerror("**Failed** test #37 (instrument loops)\n");
		logerror("    Unable to get funcions.\n");
		return -1;
	}

	// create func expr for incVar
	BPatch_Vector<BPatch_snippet *> nullArgs;
	BPatch_funcCallExpr callInc(*incVar, nullArgs);
	checkCost(callInc);

	// instrument the function's loops
	BPatch_flowGraph *cfg = func->getCFG();
	BPatch_Vector<BPatch_basicBlockLoop*> loops;
	cfg->getOuterLoops(loops);

	instrumentLoops(appAddrSpace, appImage, loops, callInc);

	return 0;
}

static int instrumentFuncLoopsWithInc(BPatch_addressSpace *appAddrSpace, 
				      BPatch_image *appImage,
				      const char *call_func,
				      const char *var)
{
	// get function * for call_func
	BPatch_Vector<BPatch_function *> funcs;

	appImage->findFunction(call_func, funcs);
	BPatch_function *func = funcs[0];

	if (func == NULL) 
	{
		logerror("**Failed** test #37 (instrument loops)\n");
		logerror("    Unable to get funcions.\n");
		return -1;
	}

	// Look up global variable
	const BPatch_variableExpr *varexpr = appImage->findVariable(var);
	if (varexpr == NULL) {
	  logerror("**FAILED** test #37 (instrument loops)\n");
	  logerror("      Unable to find global variable\n");
	  return -1;
	}

	// Create increment snippet
	BPatch_arithExpr assign(BPatch_assign,
				*varexpr,
				BPatch_arithExpr(BPatch_plus,
						 *varexpr,
						 BPatch_constExpr(1)));


	// instrument the function's loops
	BPatch_flowGraph *cfg = func->getCFG();
	BPatch_Vector<BPatch_basicBlockLoop*> loops;
	cfg->getOuterLoops(loops);

	instrumentLoops(appAddrSpace, appImage, loops, assign);

	return 0;
}

test_results_t test1_37_Mutator::executeTest() 
{
	if (isMutateeFortran(appImage)) 
	{
		return SKIPPED;
	} 

	if (instrumentFuncLoopsWithInc(appAddrSpace, appImage,
				"test1_37_call1", "globalVariable37_1") < 0) 
	{
		return FAILED;
	}

	if (instrumentFuncLoopsWithInc(appAddrSpace, appImage,
				       "test1_37_call2", "globalVariable37_2") < 0) 
	{
		return FAILED;
	}

	if (instrumentFuncLoopsWithCall(appAddrSpace, appImage,
				"test1_37_call3", "test1_37_inc3") < 0) 
	{
		return FAILED;
	}

	return PASSED;
}

