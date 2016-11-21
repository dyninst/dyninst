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

// $Id: test2_3.C,v 1.1 2008/10/30 19:20:21 legendre Exp $
/*
 * #Name: test2_3
 * #Desc: Attatch to an invalid pid
 * #Dep: 
 * #Arch: !sparc_sun_sunos4_1_3_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

#include "dyninst_comp.h"
class test2_3_Mutator : public DyninstMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_3_factory() {
  return new test2_3_Mutator();
}

//
// Test #3 - attach to an invalid pid
//	Try to attach to an invalid pid number (65539).
//
// static int mutatorTest(BPatch *bpatch, bool useAttach) 
test_results_t test2_3_Mutator::executeTest() {
    // attach to an an invalid pid
    clearError();
    BPatch_process *ret = bpatch->processAttach(NULL, 65539);
    int gotError = getError();
    if (ret || !gotError) {
	logerror("**Failed** test #3 (attach to an invalid pid)\n");
	if (ret)
    	    logerror("    created a thread handle for invalid executable\n");
	if (!gotError)
	    logerror("    the error callback should have been called but wasn't\n");
        return FAILED;
    } else {
	logerror("Passed test #3 (attach to an invalid pid)\n");
        return PASSED;
    }
}

// extern "C" TEST_DLL_EXPORT int test2_3_mutatorMAIN(ParameterDict &param)
test_results_t test2_3_Mutator::setup(ParameterDict &param) {
   bool useAttach = ((create_mode_t) param["createmode"]->getInt()) == USEATTACH;
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
  
  // Get log file pointers
//   FILE * outlog = (FILE *)(param["outlog"]->getPtr());
//   FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//   setOutputLog(outlog);
//   setErrorLog(errlog);

    if ( !useAttach ) {
      logerror("Skipping test #3 (attach to an invalid pid)\n");
      logerror("    test only makes sense in attach mode\n");
      return SKIPPED;
    }
  return PASSED;

}
