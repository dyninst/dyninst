/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test3_7.C,v 1.3 2008/06/18 19:58:25 carl Exp $
/*
 * #Name: test3_7
 * #Desc: Tests asynchronous one-time codes
 * #Dep: 
 * #Arch:
 * #Notes:useAttach does not apply
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#define MAX_MUTATEES	32
#define Mutatees		3

#include "TestMutator.h"
class test3_7_Mutator : public TestMutator {
  BPatch_exitType expectedSignal;
  int debugPrint;
  char *pathname;
  BPatch *bpatch;
  const unsigned int TIMEOUT; // Timeout in seconds

public:
  test3_7_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test3_7_factory() {
  return new test3_7_Mutator();
}

test3_7_Mutator::test3_7_Mutator()
  : pathname(NULL), bpatch(NULL), TIMEOUT(120) {
#if defined(os_windows)
  expectedSignal = ExitedNormally;
#else
  expectedSignal = ExitedViaSignal;
#endif
}

static unsigned int num_callbacks_issued = 0;

#if defined (os_osf)
#define TEST7_NUM_ONETIMECODE 100
#else
#define TEST7_NUM_ONETIMECODE 400
#endif

static void test7_oneTimeCodeCallback(BPatch_thread * /*thread*/,
                                void *userData,
                                void * /*returnValue*/)
{
  dprintf("%s[%d]:  inside oneTimeCode callback\n", __FILE__, __LINE__);
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
test_results_t test3_7_Mutator::execute() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_7"); // run test1 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    num_callbacks_issued = 0;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
            MopUpMutatees(n-1,appThread);
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread[n]->getPid());
	// Register for cleanup
	registerPID(appThread[n]->getProcess()->getPid());
    }

	// Register a callback that we will use to check for done-ness
    BPatchOneTimeCodeCallback oldCallback =
        bpatch->registerOneTimeCodeCallback(test7_oneTimeCodeCallback);

    dprintf("Letting mutatee processes run a short while (2s).\n");
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

   ////////////////////////////
   ////////////////////////////

    //  our oneTimeCode will just be a simple call to a function that increments a global variable
    BPatch_snippet *irpcSnippets[Mutatees];

    // Build snippets for each mutatee
    for (unsigned i = 0; i < Mutatees; i++) {
      BPatch_image *appImage = appThread[i]->getImage();
      //  our oneTimeCode will just be a simple call to a function that increment
      BPatch_Vector<BPatch_function *> bpfv;
      char *funcname = "test3_7_call1";
      if (NULL == appImage->findFunction(funcname, bpfv) || !bpfv.size()
          || NULL == bpfv[0]){
        logerror("    Unable to find function %s\n", funcname);
        exit(1);
      }
      BPatch_function *call7_1 = bpfv[0];

      BPatch_Vector<BPatch_snippet *> nullArgs;
      BPatch_funcCallExpr *call7_1_snip = new BPatch_funcCallExpr(*call7_1, nullArgs);
      irpcSnippets[i] = call7_1_snip;
	}

    dprintf("Pausing apps pre-iRPC...\n");
    for (n=0; n<Mutatees; n++) appThread[n]->stopExecution();

	//  Submit inferior RPCs to all of our mutatees equally...
    unsigned doneFlag = 0;
    for (unsigned int i = 0; i < TEST7_NUM_ONETIMECODE; ++i) {
      int index = i % (Mutatees);
      dprintf("%s[%d]:  issuing oneTimeCode to thread %d\n", __FILE__, __LINE__, index);
      appThread[index]->oneTimeCodeAsync(*(irpcSnippets[index]), (void *)&doneFlag);
    }

    dprintf("Running mutatees post-iRPC...\n");
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

   ////////////////////////////
   ////////////////////////////

   // and wait for completion/timeout
   int timeout = 0;
   while (!doneFlag && (timeout < TIMEOUT)) {
     P_sleep(1);
     bpatch->pollForStatusChange();
     timeout++;
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
        bool dead = appThread[n]->terminateExecution();
        if (!dead || !(appThread[n]->isTerminated())) {
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
            logerror("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
        if(appThread[n]->terminationStatus() != expectedSignal) {
            logerror("**Failed** test #7 (simultaneous multiple-process management - oneTimeCode)\n");
            logerror("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appThread[n]->getExitSignal();
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

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return PASSED;
}
