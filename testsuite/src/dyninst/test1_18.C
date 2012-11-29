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

// $Id: test1_18.C,v 1.1 2008/10/30 19:18:03 legendre Exp $
/*
 * #Name: test1_18
 * #Desc: Mutator Side - Read/Write a variable in the mutatee
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

class test1_18_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_18_factory() 
{
	return new test1_18_Mutator();
}

//
// Start Test Case #18 - mutator side (read/write a variable in the mutatee)
//

test_results_t test1_18_Mutator::executeTest() 
{
	const char *funcName = "test1_18_func1";
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

	BPatch_Vector<BPatch_point *> *func18_1 = found_funcs[0]->findPoint(BPatch_subroutine);

	if (!func18_1 || ((*func18_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	const char *varName = "test1_18_globalVariable1";
	BPatch_variableExpr *expr18_1 = findVariable(appImage, varName, func18_1);

	/* Initialization must be done, because C would have done initialization at declaration */

	if (expr18_1 == NULL) 
	{
		logerror("**Failed** test #18 (read/write a variable in the mutatee)\n");
		logerror("    Unable to locate %s\n", varName);
		return FAILED;
	}

	int expectedReadValue = 42;
	
	// We can't get globals initialized with FORTRAN in time to read their non-zero values...
	if (isMutateeFortran(appImage)) 
	{
	  expectedReadValue = 0;
	}
	
	int n;
	expr18_1->readValue(&n);

	if (n != expectedReadValue) 
	{
		logerror("**Failed** test #18 (read/write a variable in the mutatee)\n");
		logerror("    value read from %s was %d, not %d as expected\n",
				varName, n, expectedReadValue);
		return FAILED;
	}

	n = 17;

	if (!expr18_1->writeValue(&n,true))
	{
		logerror("%s[%]:  failed to writeValue()\n", FILE__, __LINE__);
		return FAILED;
	}

	return PASSED;
} // test1_18_Mutator::executeTest()
