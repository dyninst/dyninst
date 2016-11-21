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

// $Id: test1_8.C,v 1.1 2008/10/30 19:19:58 legendre Exp $
/*
 * #Name: test1_8
 * #Desc: Mutator Side - Preserve Registers - expr
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

class test1_8_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_8_factory() 
{
	return new test1_8_Mutator();
}

//
// Start Test Case #8 - mutator side (preserve registers - expr)
//

test_results_t test1_8_Mutator::executeTest() 
{
	// Find the entry point to the procedure "test1_8_func1"
	const char *funcName = "test1_8_func1";
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

	BPatch_Vector<BPatch_point *> *point8_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point8_1 || ((*point8_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet*> vect8_1;
	const char *globalVar = "test1_8_globalVariable1";
	BPatch_variableExpr *expr8_1 = findVariable(appImage, globalVar,
			point8_1);

	if (!expr8_1) 
	{
		logerror("**Failed** test #3 (passing variables)\n");
		logerror("    Unable to locate variable %s\n", globalVar);
		return FAILED;
	}

	BPatch_arithExpr arith8_1 (BPatch_assign, *expr8_1, 
			BPatch_arithExpr(BPatch_plus, 
				BPatch_arithExpr(BPatch_plus, 
					BPatch_arithExpr(BPatch_plus, BPatch_constExpr(81), 
						BPatch_constExpr(82)),
					BPatch_arithExpr(BPatch_plus, BPatch_constExpr(83), 
						BPatch_constExpr(84))),
				BPatch_arithExpr(BPatch_plus, 
					BPatch_arithExpr(BPatch_plus, BPatch_constExpr(85), 
						BPatch_constExpr(86)),
					BPatch_arithExpr(BPatch_plus, BPatch_constExpr(87), 
						BPatch_constExpr(88)))));
	vect8_1.push_back(&arith8_1);

	checkCost(BPatch_sequence(vect8_1));
	appAddrSpace->insertSnippet( BPatch_sequence(vect8_1), *point8_1);

	return PASSED;
}
