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

// $Id: test3_2.C,v 1.1 2008/10/30 19:20:35 legendre Exp $
/*
 * #Name: test3_2
 * #Desc: simultaneous multiple-process management - exit
 * #Dep: 
 * #Arch: all
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#define MAX_MUTATEES	32

#include "dyninst_comp.h"
class test3_2_Mutator : public DyninstMutator {
  unsigned int Mutatees;
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test3_2_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test3_2_factory() {
  return new test3_2_Mutator();
}

test3_2_Mutator::test3_2_Mutator()
  : Mutatees(3), bpatch(NULL), pathname(NULL) {
}

//
// Start Test Case #2 - create processes and process events from each
//     Just let them run to finish, no instrumentation added.
//
// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test3_2_Mutator::executeTest() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_2"); // run test2 in mutatee
    child_argv[n++] = NULL;

    BPatch_process *appProc[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appProc[n]=NULL;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appProc[n] = bpatch->processCreate(pathname, child_argv, NULL);
        if (!appProc[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test #2 (simultaneous multiple-process management - exit)\n");
			if(n > 0) {
                            MopUpMutatees(n-1,appProc);
			}
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appProc[n]->getPid());
    }
    dprintf("Letting %d mutatee processes run.\n", Mutatees);
    for (n=0; n<Mutatees; n++) appProc[n]->continueExecution();

    unsigned int numTerminated=0;
    bool terminated[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) terminated[n]=false;

    // monitor the mutatee termination reports
    while (numTerminated < Mutatees) {
        bpatch->waitForStatusChange();
        dprintf("%s[%d]:  got status change\n", __FILE__, __LINE__);
        for (n=0; n<Mutatees; n++)
            if (!terminated[n] && (appProc[n]->isTerminated())) {
            if(appProc[n]->terminationStatus() == ExitedNormally) {
                int exitCode = appProc[n]->getExitCode();
                    if (exitCode || debugPrint)
                        dprintf("Mutatee %d exited with exit code 0x%x\n", n,
                                exitCode);
                }
                else if(appProc[n]->terminationStatus() == ExitedViaSignal) {
                    int signalNum = appProc[n]->getExitSignal();
                    if (signalNum || debugPrint)
                        dprintf("Mutatee %d exited from signal 0x%d\n", n,
                                signalNum);
                }
                terminated[n]=true;
                numTerminated++;
            }
            else if (!terminated[n] && (appProc[n]->isStopped())) {
                appProc[n]->continueExecution();
            }
    }

    if (numTerminated == Mutatees) {
	logerror("Passed Test #2 (simultaneous multiple-process management - exit)\n");
        return PASSED;
    }
	return FAILED;
}

// extern "C" TEST_DLL_EXPORT int test3_2_mutatorMAIN(ParameterDict &param)
test_results_t test3_2_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    
    return PASSED;
}
