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

// $Id: test3_5.C,v 1.1 2008/10/30 19:20:41 legendre Exp $
/*
 * #Name: test3_5
 * #Desc: sequential multiple-process management - abort
 * #Dep: 
 * #Arch:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#include "dyninst_comp.h"
class test3_5_Mutator : public DyninstMutator {
  unsigned int Mutatees;
  int debugPrint;
  char *pathname;
  BPatch *bpatch;

public:
  test3_5_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test3_5_factory() {
  return new test3_5_Mutator();
}

test3_5_Mutator::test3_5_Mutator()
  : Mutatees(3), pathname(NULL), bpatch(NULL) {
}

//
// Start Test Case #5 - create one process, wait for it to exit.  Then 
//     create a second one and wait for it to exit.  Repeat as required.
//     Differs from test 3 in that the mutatee processes terminate with
//     abort rather than exit.
//
// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test3_5_Mutator::executeTest() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_5"); // run test5 in mutatee
    child_argv[n++] = NULL;

    for (n=0; n<Mutatees; n++) {
        // Start the mutatee
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appProc = bpatch->processCreate(pathname, child_argv, NULL);
        if (!appProc) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** Test #5 (sequential multiple-process management - abort)\n");
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appProc->getPid());

        appProc->continueExecution();

        while (!appProc->isTerminated()) {
            if (appProc->isStopped())
                appProc->continueExecution();
            bpatch->waitForStatusChange();
        }

        if(appProc->terminationStatus() == ExitedNormally) {
            int exitCode = appProc->getExitCode();
           if (exitCode || debugPrint)
               dprintf("Mutatee %d exited with exit code 0x%x\n", n, exitCode);
        } else if(appProc->terminationStatus() == ExitedViaSignal) {
            int signalNum = appProc->getExitSignal();
           if (signalNum || debugPrint)
               dprintf("Mutatee %d exited from signal 0x%d\n", n, signalNum);
        }
    }

    logerror("Passed Test #5 (sequential multiple-process management - abort)\n");
    return PASSED;
}

// extern "C" TEST_DLL_EXPORT int test3_5_mutatorMAIN(ParameterDict &param)
test_results_t test3_5_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    
    return PASSED;
}
