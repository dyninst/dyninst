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

// $Id: test1_34.C,v 1.1 2008/10/30 19:19:12 legendre Exp $
/*
 * #Name: test1_34
 * #Desc: Loop Information
 * #Dep: 
 * #Arch: !os_windows && !os_aix_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_34_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_34_factory() 
{
	return new test1_34_Mutator();
}

static int numContainedLoops(BPatch_basicBlockLoop *loop)
{
	BPatch_Vector<BPatch_basicBlockLoop*> containedLoops;
	loop->getContainedLoops(containedLoops);

	return containedLoops.size();
}

//
// Start Test Case #34 - (loop information)
//

test_results_t test1_34_Mutator::executeTest() 
{
	unsigned int i;

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_34_func2";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *func2 = bpfv[0];

	BPatch_flowGraph *cfg = func2->getCFG();

	if (cfg == NULL) 
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("  Unable to get control flow graph of %s\n", fn);
		return FAILED;
	}

	BPatch_Vector<BPatch_basicBlockLoop*> loops;
	cfg->getLoops(loops);

	if (loops.size() != 4) 
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("  Detected %d loops, should have been four.\n",
				loops.size());
		return FAILED;
	}

	/*
	 * Find the loop that contains two loops (that should be the outermost
	 * one).
	 */
	BPatch_basicBlockLoop *outerLoop = NULL;

	for (i = 0; i < loops.size(); i++) 
	{
		if (numContainedLoops(loops[i]) == 3) 
		{
			outerLoop = loops[i];
			break;
		}
	}

	if (outerLoop == NULL) 
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("  Unable to find a loop containing two other loops.\n");
		return FAILED;
	}

	BPatch_Vector<BPatch_basicBlockLoop*> insideOuterLoop;
	outerLoop->getContainedLoops(insideOuterLoop);
	assert(insideOuterLoop.size() == 3);

	bool foundFirstLoop = false;
	int deepestLoops = 0;

	for (i = 0; i < insideOuterLoop.size(); i++) 
	{
		BPatch_Vector<BPatch_basicBlockLoop*> tmpLoops;
		insideOuterLoop[i]->getContainedLoops(tmpLoops);

		if (tmpLoops.size() == 1)  /* The first loop has one nested inside. */
		{
			if (foundFirstLoop) 
			{
				logerror("**Failed** test #34 (loop information)\n");
				logerror("  Found more than one second-level loop with one nested inside.\n");
				return FAILED;
			}
			foundFirstLoop = true;

			if (numContainedLoops(tmpLoops[0]) != 0) 
			{
				logerror("**Failed** test #34 (loop information)\n");
				logerror("  The first loop at the third level should not have any loops nested inside,\n");
				logerror("  but %d were detected.\n",
						numContainedLoops(tmpLoops[0]));
				return FAILED;
			}

		} 
		else if (tmpLoops.size() == 0)  /* The second loop has none nested. */
		{
			if (deepestLoops >= 2) 
			{
				logerror("**Failed** test #34 (loop information)\n");
				logerror("  Found more than two loops without any nested inside.\n");
				return FAILED;
			}
			deepestLoops++;

		} 
		else 
		{ /* All loops should be recognized above. */
			logerror("**Failed** test #34 (loop information)\n");
			logerror("  Found a loop containing %d loops, should be one or  none.\n", tmpLoops.size());
			return FAILED;
		}
	}

	if (!foundFirstLoop || deepestLoops < 2) 
	{
		/* We shouldn't be able to get here. */
		logerror("**Failed** test #34 (loop information)\n");
		if (!foundFirstLoop)
			logerror("  Could not find the first nested loop.\n");
		if (deepestLoops < 2)
			logerror("  Could not find all the deepest level loops.\n");
		return FAILED;
	}

	// test getOuterLoops
	// i'd like to be able to swap the order of BPatch_flowGraph::loops
	// around so that the hasAncestor code is tested

	BPatch_Vector<BPatch_basicBlockLoop*> outerLoops;
	cfg->getOuterLoops(outerLoops);

	if (outerLoops.size() != 1) 
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("  Detected %d outer loops, should have been one.\n",
				outerLoops.size());
		return FAILED;
	}

	BPatch_Vector<BPatch_basicBlockLoop*> outerLoopChildren;
	outerLoops[0]->getOuterLoops(outerLoopChildren);

	if (outerLoopChildren.size() != 2) 
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("  Detected %d outer loops, should have been two.\n",
				outerLoopChildren.size());
		return FAILED;
	}

	BPatch_Vector<BPatch_basicBlockLoop*> outerLoopGrandChildren0;
	outerLoopChildren[0]->getOuterLoops(outerLoopGrandChildren0);

	BPatch_Vector<BPatch_basicBlockLoop*> outerLoopGrandChildren1;
	outerLoopChildren[1]->getOuterLoops(outerLoopGrandChildren1);

	// one has no children, the other has 1 child
	if (!((outerLoopGrandChildren0.size() == 0 || 
					outerLoopGrandChildren1.size() == 0) &&
				(outerLoopGrandChildren0.size() == 1 || 
				 outerLoopGrandChildren1.size() == 1))) 
	{
		logerror("**Failed** test #34 (loop information)\n");
		logerror("  Detected %d and %d outer loops, should have been zero and one.\n",
				outerLoopGrandChildren0.size(), 
				outerLoopGrandChildren1.size());
		return FAILED;
    }

    return PASSED;
}
