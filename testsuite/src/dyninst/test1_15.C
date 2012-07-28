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

// $Id: test1_15.C,v 1.1 2008/10/30 19:17:48 legendre Exp $
/*
 * #Name: test1_15
 * #Desc: Mutator Side - setMutationsActive
 * #Dep: 
 * #Notes: Combined Result of 15a and 15b from previous test, we may want to consider recombining them
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_15_Mutator : public DyninstMutator {

	virtual bool hasCustomExecutionPath() { return true; }
	virtual test_results_t executeTest();
	int mutatorTest15a();
	int mutatorTest15b();
};

extern "C" DLLEXPORT  TestMutator *test1_15_factory() 
{
	return new test1_15_Mutator();
}

//
// Start Test Case #15 - mutator side (setMutationsActive)
//

int test1_15_Mutator::mutatorTest15a() 
{
	BPatch_process *proc = appThread->getProcess();
	assert(proc);

	RETURNONFAIL(insertCallSnippetAt(proc, appImage, "test1_15_func2",
				BPatch_entry, "test1_15_call1", 15,
				"setMutationsActive"));

	RETURNONFAIL(replaceFunctionCalls(proc, appImage, "test1_15_func4",
				"test1_15_func3",	"test1_15_call3", 15,
				"setMutationsActive", 1));

	return 0;
}

int test1_15_Mutator::mutatorTest15b() 
{
    RETURNONFAIL(waitUntilStopped(BPatch::bpatch, appProc, 15, "setMutationsActive"));

	// disable mutations and continue process
        appProc->setMutationsActive(false);
        appProc->continueExecution();

        RETURNONFAIL(waitUntilStopped(BPatch::bpatch, appProc, 15, "setMutationsActive"));

	// re-enable mutations and continue process
        appProc->setMutationsActive(true);
        appProc->continueExecution();

	return 0;
}

test_results_t test1_15_Mutator::executeTest() 
{
	if (mutatorTest15a() != 0) 
	{
		return FAILED;
	}

        while (!appProc->isStopped())
	{
		BPatch::bpatch->waitForStatusChange();
	}

        appProc->continueExecution();

	if (mutatorTest15b() != 0) 
	{
		return FAILED;
	}

	return PASSED;
}
