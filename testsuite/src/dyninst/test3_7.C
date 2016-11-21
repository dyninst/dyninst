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

// $Id: test3_7.C,v 1.1 2008/10/30 19:20:45 legendre Exp $
/*
 * #Name: test3_7
 * #Desc: Tests asynchronous one-time codes
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
#define Mutatees		3

#include "dyninst_comp.h"
class test3_7_Mutator : public DyninstMutator {
  BPatch_exitType expectedSignal;
  int debugPrint;
  char *pathname;
  BPatch *bpatch;

public:
  test3_7_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test3_7_factory() {
  return new test3_7_Mutator();
}

test3_7_Mutator::test3_7_Mutator()
  : pathname(NULL), bpatch(NULL) {
  expectedSignal = ExitedViaSignal;
}

static unsigned int num_callbacks_issued = 0;

#if defined (os_osf_test)
#define TEST7_NUM_ONETIMECODE 100
#else
#define TEST7_NUM_ONETIMECODE 400
#endif

static void test7_oneTimeCodeCallback(BPatch_thread * /*thread*/,
                                void *userData,
                                void * /*returnValue*/)
{
  dprintf("%s[%d]:  inside oneTimeCode callback, iteration %d\n", __FILE__, __LINE__, num_callbacks_issued);
  num_callbacks_issued++;
  if (num_callbacks_issued == TEST7_NUM_ONETIMECODE) {
    *((bool *)userData) = true; // we are done
  }
}

//
// Start Test Case #7 - create processes and process events from each
//     Run a whole ton of asynchronous OneTimeCodes to test signal handling
//

// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test3_7_Mutator::executeTest() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_7"); // run test1 in mutatee
    child_argv[n++] = NULL;

    BPatch_process *appProc[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appProc[n]=NULL;

    num_callbacks_issued = 0;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appProc[n] = bpatch->processCreate(pathname, child_argv, NULL);
        if (!appProc[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
			if( n > 0 ) {
                            MopUpMutatees(n-1,appProc);
			}
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appProc[n]->getPid());
    }

	// Register a callback that we will use to check for done-ness
    BPatchOneTimeCodeCallback oldCallback =
        bpatch->registerOneTimeCodeCallback(test7_oneTimeCodeCallback);

    dprintf("Letting mutatee processes run a short while (2s).\n");
    for (n=0; n<Mutatees; n++) appProc[n]->continueExecution();

   ////////////////////////////
   ////////////////////////////

    //  our oneTimeCode will just be a simple call to a function that increments a global variable
    BPatch_snippet *irpcSnippets[Mutatees];

    // Build snippets for each mutatee
    for (unsigned i = 0; i < Mutatees; i++) {
        BPatch_image *appImage = appProc[i]->getImage();
      //  our oneTimeCode will just be a simple call to a function that increment
      BPatch_Vector<BPatch_function *> bpfv;
      const char *funcname = "test3_7_call1";
      if (NULL == appImage->findFunction(funcname, bpfv) || !bpfv.size()
          || NULL == bpfv[0]){
        logerror("    Unable to find function %s\n", funcname);
	MopUpMutatees(Mutatees,appProc);
        return FAILED;
      }
      BPatch_function *call7_1 = bpfv[0];

      BPatch_Vector<BPatch_snippet *> nullArgs;
      BPatch_funcCallExpr *call7_1_snip = new BPatch_funcCallExpr(*call7_1, nullArgs);
      irpcSnippets[i] = call7_1_snip;
	}

    dprintf("Pausing apps pre-iRPC...\n");
    for (n=0; n<Mutatees; n++) appProc[n]->stopExecution();

	//  Submit inferior RPCs to all of our mutatees equally...
    unsigned doneFlag = 0;
    for (unsigned int i = 0; i < TEST7_NUM_ONETIMECODE; ++i) {
      int index = i % (Mutatees);
      dprintf("%s[%d]:  issuing oneTimeCode to thread %d\n", __FILE__, __LINE__, index);
      appProc[index]->oneTimeCodeAsync(*(irpcSnippets[index]), (void *)&doneFlag);
    }
	////////////////////////////
	// RPCs are now post-and-run, not post-then-run, semantics. So post all, wait for all.
	// This holds even for async RPCs. Blame ProcControl/Windows.
	// Also in the blame PC/Win department: if you post an RPC to a running process, it *may* fail b/c of process
	// exit.
	////////////////////////////


    dprintf("Running mutatees post-iRPC...\n");
    for (n=0; n<Mutatees; n++) appProc[n]->continueExecution();
   // and wait for completion
   while( !doneFlag) {
       bpatch->waitForStatusChange();
   }

   int test7err = false;
   if (!doneFlag) {
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
            logerror("   did not receive the right # of events: got %d, expected %d\n", num_callbacks_issued, TEST7_NUM_ONETIMECODE);
            test7err = true;
   }

    dprintf("Terminating mutatee processes.\n");


    unsigned int numTerminated=0;
    for (n=0; n<Mutatees; n++) {
        bool dead = appProc[n]->terminateExecution();
        if (!dead || !(appProc[n]->isTerminated())) {
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
            logerror("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
        if(appProc[n]->terminationStatus() != expectedSignal) {
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
            logerror("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appProc[n]->getExitSignal();
        dprintf("Terminated mutatee [%d] from signal 0x%x\n", n, signalNum);
        numTerminated++;
    }

    if (numTerminated == Mutatees && !test7err) {
	logerror("Passed Test #7 (simultaneous multiple-process management - oneTimeCode)\n");
        return PASSED;
    }

    return FAILED;
}

// extern "C" TEST_DLL_EXPORT int test3_7_mutatorMAIN(ParameterDict &param)
test_results_t test3_7_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    
    return PASSED;
}
