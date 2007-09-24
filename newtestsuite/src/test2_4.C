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

// $Id: test2_4.C,v 1.1 2007/09/24 16:39:23 cooksey Exp $
/*
 * #Name: test2_4
 * #Desc: Attach to a protected pid
 * #Dep: 
 * #Arch: !sparc_sun_sunos4_1_3
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

#include "TestMutator.h"
class test2_4_Mutator : public TestMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test2_4_factory() {
  return new test2_4_Mutator();
}

//
// Test #4 - attach to a protected pid
//	Try to attach to a "protected pid" number.  This should test the
//	procfs attach to a pid that we don't have "rw" access to. The pid
//	selected is pid #1 which is a OS kernel process that user processes
//	can't read or write.
//

// static int mutatorTest(BPatch *bpatch, bool useAttach)
test_results_t test2_4_Mutator::execute() {
    // attach to an a protected pid
    clearError();
    BPatch_thread *ret = bpatch->attachProcess(NULL, 1);
    int gotError = getError();
    if (ret || !gotError) {
	logerror("**Failed** test #4 (attach to a protected pid)\n");
	if (ret)
    	    logerror("    created a thread handle for invalid executable\n");
	if (!gotError)
	    logerror("    the error callback should have been called but wasn't\n");
        return FAILED;
    } else {
	logerror("Passed test #4 (attach to a protected pid)\n");
        return PASSED;
    }
}

// extern "C" TEST_DLL_EXPORT int test2_4_mutatorMAIN(ParameterDict &param)
test_results_t test2_4_Mutator::setup(ParameterDict &param) {
  int useAttach = param["useAttach"]->getInt();
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  
#if defined(sparc_sun_sunos4_1_3)
    logerror("Skipping test #4 (attach to a protected pid)\n");
    logerror("    attach is not supported on this platform\n");
    return SKIPPED;
#else
    if ( !useAttach )
    {
      logerror("Skipping test #4 (attach to a protected pid)\n");
      logerror("    test doesn't make sense without useAttach\n");
      return SKIPPED;
    }
#endif

  return PASSED;
}
