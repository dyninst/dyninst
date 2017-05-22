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

// $Id: test3_1.C,v 1.1 2008/10/30 19:20:33 legendre Exp $
/*
 * #Name: test3_1
 * #Desc: Create processes, process events, and kill them, no instrumentation
 * #Dep: 
 * #Arch:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#define MAX_MUTATEES	32

#include "dyninst_comp.h"
class test3_1_Mutator : public DyninstMutator {
  // These used to be static.  I don't think they need to be any more, now that
  // they're instance fields
  unsigned int Mutatees;
  int debugPrint;

  BPatch_exitType expectedSignal;
  
  BPatch *bpatch;
  char *pathname;
public:
  test3_1_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test3_1_factory() {
  return new test3_1_Mutator();
}

test3_1_Mutator::test3_1_Mutator() {
  expectedSignal = ExitedViaSignal;

  Mutatees = 3;

  bpatch = NULL;
  pathname = NULL;
} // test3_1_Mutator()

//
// Start Test Case #1 - create processes and process events from each
//     Just let them run a while, then kill them, no instrumentation added.
//
// static int mutatorTest(char *pathname, BPatch *bpatch)
// {
test_results_t test3_1_Mutator::executeTest() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_1"); // run test1 in mutatee
    child_argv[n++] = NULL;

    BPatch_process *appProc[MAX_MUTATEES];

    for (n = 0; n < MAX_MUTATEES; n++) {
        appProc[n] = NULL;
    }

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appProc[n] = bpatch->processCreate(pathname, child_argv, NULL);
        if (!appProc[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
			// If we failed on our first mutatee, we don't need to do cleanup...
			if(n > 0) {
                            MopUpMutatees(n-1,appProc);
			}
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appProc[n]->getPid());
    }

    dprintf("Letting mutatee processes run a short while (5s).\n");
    for (n=0; n<Mutatees; n++) appProc[n]->continueExecution();

    P_sleep(5);
    dprintf("Terminating mutatee processes.\n");

    unsigned int numTerminated=0;
    for (n=0; n<Mutatees; n++) {
        bool dead = appProc[n]->terminateExecution();
        if (!dead || !(appProc[n]->isTerminated())) {
            logerror("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            logerror("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
        if(appProc[n]->terminationStatus() != expectedSignal) {
            logerror("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            logerror("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appProc[n]->getExitSignal();
        dprintf("Terminated mutatee [%d] from signal 0x%x\n", n, signalNum);
        numTerminated++;
    }

    if (numTerminated == Mutatees) {
	logerror("Passed Test #1 (simultaneous multiple-process management - terminate)\n");
        return PASSED;
    }

    return FAILED;
}

// extern "C" TEST_DLL_EXPORT int test3_1_mutatorMAIN(ParameterDict &param)
// {
test_results_t test3_1_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    
//     return mutatorTest(pathname, bpatch);
    return PASSED;
} // test3_1_Mutator::setup(ParameterDict &)
