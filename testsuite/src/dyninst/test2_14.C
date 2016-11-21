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

// $Id: test2_14.C,v 1.1 2008/10/30 19:20:18 legendre Exp $
/*
 * #Name: test2_14
 * #Desc: Delete Thread
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test2_14_Mutator : public DyninstMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_14_factory() {
  return new test2_14_Mutator();
}

//
// Test 14 - Look through the thread list and make sure the thread has
//    been deleted as required.

// static int mutatorTest(BPatch_thread *thread, BPatch_image *appImage)
test_results_t test2_14_Mutator::executeTest() {
    killMutatee(appProc);
    delete appProc;
   
    bool failed_this = false;
    BPatch_Vector<BPatch_process *> *threads = bpatch->getProcesses();
    for (unsigned int i=0; i < threads->size(); i++) {
	if ((*threads)[i] == appProc) {
	    logerror("**Failed** test #14 (delete thread)\n"); 
	    logerror("    thread %p was deleted, but getThreads found it\n",
                     appProc); // DO NOT try to dereference here...
	    failed_this = true;
	}
    }
    // Because this uses the thread form of terminateExecution (deprecated),
    // it also kills the process.  This means we'll access deleted memory if we try to terminate the process.
    // So set the thread and proc to NULL, ensuring that we won't crash on any system with a real debug heap
    // (e.g. Windows.)
    // This means that the test teardown needs to pass these NULLifications back to the component...
    appThread = NULL;
    appProc = NULL;

    if (!failed_this) {
	logerror("Passed test #14 (delete thread)\n");
        return PASSED;
    } else {
        return FAILED;
    }
}

// extern "C" TEST_DLL_EXPORT int test2_14_mutatorMAIN(ParameterDict &param)
test_results_t test2_14_Mutator::setup(ParameterDict &param) {
    bool createmode = param["createmode"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());

    appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    appProc = appThread->getProcess();

    // Read the program's image and get an associated image object
    appImage = appProc->getImage();

    // Signal the child that we've attached
    if (createmode == USEATTACH) {
       signalAttached(appImage);
    }

    return PASSED;
}
