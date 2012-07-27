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

// $Id: test2_1.C,v 1.1 2008/10/30 19:20:09 legendre Exp $
/*
 * #Name: test2_1
 * #Desc: Run an executable that does not exist
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
class test2_1_Mutator : public DyninstMutator {
  bool useAttach;
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT DLLEXPORT TestMutator *test2_1_factory() {
  return new test2_1_Mutator();
}

//
// Test #1 - run an executable that does not exist
//	Attempt to run a program file that does not exist.  Should return a
//	null values from createProcess (called via mutatorMAIN)
//

// static int mutatorTest(BPatch *bpatch, bool useAttach)
test_results_t test2_1_Mutator::executeTest() {

    if (useAttach) {
	logerror("Skipping test #1 (run an executable that does not exist)\n");
	logerror("    not relevant with -attach option\n");
        return SKIPPED;
    } else {
	// try to run a program that does not exist
        clearError();
        BPatch_process *ret = BPatch::bpatch->processCreate("./noSuchFile", NULL, NULL);
        bool gotError = getError();
	if (ret || !gotError) {
	    logerror("**Failed** test #1 (run an executable that does not exist)\n");
	    if (ret)
		logerror("    created a thread handle for a non-existant file\n");
	    if (!gotError)
		logerror("    the error callback should have been called but wasn't\n");
            return FAILED;
	} else {
	    logerror("Passed test #1 (run an executable that does not exist)\n");
            return PASSED;
	}
    }
}

// extern "C" TEST_DLL_EXPORT int test2_1_mutatorMAIN(ParameterDict &param)
test_results_t test2_1_Mutator::setup(ParameterDict &param) {
   useAttach = ((create_mode_t) param["createmode"]->getInt()) == USEATTACH;
  bpatch = (BPatch *)(param["bpatch"]->getPtr());

  if (useAttach) {
    logerror("Skipping test #1 (run an executable that does not exist)\n");
    logerror("    not relevant with -attach option\n");
    return SKIPPED;
  } else {
    return PASSED;
  }
}
