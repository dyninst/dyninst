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

// $Id: test2_2.C,v 1.1 2008/10/30 19:20:20 legendre Exp $
/*
 * #Name: test2_2
 * #Desc: Try to run a createProcess on a file that is not an executable file
 * #Dep: 
 * #Arch: all
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

#include "dyninst_comp.h"
class test2_2_Mutator : public DyninstMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_2_factory() {
  return new test2_2_Mutator();
}


//
// Test #2 - try to execute a file that is not a valid program
//	Try to run a createProcess on a file that is not an executable file 
//	(via mutatorMAIN).
//
// static int mutatorTest(BPatch *bpatch, bool useAttach)
test_results_t test2_2_Mutator::executeTest() {
//     if (useAttach) {
// 	logerror("Skipping test #2 (try to execute a file that is not a valid program)\n");
// 	logerror("    not relevant with -attach option\n");
// 	return SKIPPED;
//     }

    // try to run a file that is not a valid program

    const char *mutatee_name;
#ifdef os_windows_test
    mutatee_name = "nul:";
#else
    mutatee_name = "/dev/null";
#endif

    clearError();
    BPatch_process *ret = BPatch::bpatch->processCreate(mutatee_name, NULL, NULL);


    int gotError = getError();
    if (ret || !gotError) {
	logerror("**Failed** test #2 (try to execute a file that is not a valid program)\n");
	if (ret)
	    logerror("    created a thread handle for invalid executable\n");
	if (!gotError)
	    logerror("    the error callback should have been called but wasn't\n");
        return FAILED;
    } else {
	logerror("Passed test #2 (try to execute a file that is not a valid program)\n");
        return PASSED;
    }
}

// extern "C" TEST_DLL_EXPORT int test2_2_mutatorMAIN(ParameterDict &param)
test_results_t test2_2_Mutator::setup(ParameterDict &param) {
  bool useAttach = ((create_mode_t) param["createmode"]->getInt()) == USEATTACH;
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  
  // Get log file pointers
//   FILE *outlog = (FILE *)(param["outlog"]->getPtr());
//   FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//   setOutputLog(outlog);
//   setErrorLog(errlog);
    if (useAttach) {
	logerror("Skipping test #2 (try to execute a file that is not a valid program)\n");
	logerror("    not relevant with -attach option\n");
	return SKIPPED;
    }

  return PASSED;

}
