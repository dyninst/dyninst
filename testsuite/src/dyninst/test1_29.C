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

// $Id: test1_29.C,v 1.1 2008/10/30 19:18:45 legendre Exp $
/*
 * #Name: test1_29
 * #Desc: getParent/Child
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"
#include "dyninst_comp.h"

class test1_29_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_29_factory() 
{
	return new test1_29_Mutator();
}

static bool printSrcObj(BPatch_sourceObj *p, int level)
{
	unsigned int i;
	bool ret = true;

	BPatch_Vector<BPatch_sourceObj *> curr;

	if (!p) return(true);

	switch (p->getSrcType()) 
	{
		case BPatch_sourceProgram:
			if (level != 0) ret = false;
			break;

		case BPatch_sourceModule: 
			if (level != 1) ret = false;
			break;

		case BPatch_sourceFunction: 
			if (level != 2) ret = false;
			break;

		default:
			logerror("<unknown type>");
	}

	if (!p->getSourceObj(curr)) 
	{
		// eveything down to functions should have something
		return((level == 2) ? true : false);
	}

	for (i=0; i < curr.size(); i++) 
	{
		p = curr[i];
		ret = printSrcObj(p, level+1) && ret;
	}

	return ret;
}

//
// Start Test Case #29 - getParent/Child
//

test_results_t test1_29_Mutator::executeTest() 
{
	BPatch_sourceObj *p;
	bool passedTest;

	p = (BPatch_sourceObj *) appImage;
	passedTest = printSrcObj(p, 0);

	if (!passedTest) 
	{
		logerror("**Failed** test #29 (class BPatch_srcObj)\n");
		return FAILED;
	}

	const char *funcName = "test1_29_func1";
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

	BPatch_Vector<BPatch_point *> *point29_1 = found_funcs[0]->findPoint(BPatch_entry);

	assert (point29_1);

	//BPatch_variableExpr *expr29_1 = findVariable(appImage, "test1_29_globalVariable1", point29_1);
	BPatch_variableExpr *expr29_1 = appImage->findVariable("test1_29_globalVariable1");

	if (expr29_1 == NULL) 
	{
		logerror("**Failed** test #29 (class BPatch_srcObj)\n");
		logerror("    Unable to locate test1_29_globalVariable1\n");
		return FAILED;
	}
	setExpectError(DYNINST_NO_ERROR);

	int n = 1;
	if (!expr29_1->writeValue(&n,true))
	{
		logerror("**Failed** test #29 (class BPatch_srcObj)\n");
		logerror("    Unable to write test1_29_globalVariable1\n");
		return FAILED;
	}

	return PASSED;
}
