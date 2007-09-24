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

// $Id: test_stack_1.C,v 1.1 2007/09/24 16:41:22 cooksey Exp $
/*
 * #Name: test8_1
 * #Desc: getCallStack
 * #Dep: 
 * #Arch: !alpha_dec_osf4_0
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "TestMutator.h"
class test_stack_1_Mutator : public TestMutator {
private:
  BPatch *bpatch;

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test_stack_1_factory() {
  return new test_stack_1_Mutator();
}

// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test_stack_1_Mutator::execute() {
    appThread->continueExecution();
    static const frameInfo_t correct_frame_info[] = {
#if defined( os_linux ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
	{ true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if !defined(rs6000_ibm_aix4_1)
	{ false, false, BPatch_frameNormal, NULL },
#endif
#if !defined(i386_unknown_nt4_0)
	{ true,  false, BPatch_frameNormal, "stop_process_" },
#endif
	{ true,  false, BPatch_frameNormal, "test_stack_1_func3" },
	{ true,  false, BPatch_frameNormal, "test_stack_1_func2" },
	{ true,  false, BPatch_frameNormal, "test_stack_1_func1" },
	{ true,  false, BPatch_frameNormal, "test_stack_1_mutateeTest" },
	{ true,  false, BPatch_frameNormal, "main" },
    };

    if (waitUntilStopped(bpatch, appThread, 1, "getCallStack") < 0) {
      appThread->getProcess()->terminateExecution();
      return FAILED;
    }

    if (checkStack(appThread,
		   correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   1, "getCallStack")) {
	logerror("Passed test #1 (getCallStack)\n");

    } else {
      appThread->getProcess()->terminateExecution();
       return FAILED;
    }

    appThread->continueExecution();
    while (!appThread->getProcess()->isTerminated()) {
      bpatch->waitForStatusChange();
    }
    //fprintf(stderr, "[%s:%u] - mutator continued mutatee\n", __FILE__, __LINE__); /*DEBUG*/

    return PASSED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test8_1_mutatorMAIN(ParameterDict &param)
test_results_t test_stack_1_Mutator::setup(ParameterDict &param) {
  TestMutator::setup(param);
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  return PASSED;
}
