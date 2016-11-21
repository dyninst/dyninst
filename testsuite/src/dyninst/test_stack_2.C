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

// $Id: test_stack_2.C,v 1.1 2008/10/30 19:22:20 legendre Exp $
/*
 * #Name: test8_2
 * #Desc: getCallStack in signal handler
 * #Dep: 
 * #Arch: i386_unknown_linux2_0_test,sparc_sun_solaris2_4_test,ia64_unknown_linux2_4_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test_stack_2_Mutator : public DyninstMutator {
private:
  BPatch *bpatch;

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_stack_2_factory() {
  return new test_stack_2_Mutator();
}

// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test_stack_2_Mutator::executeTest() {
#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 || defined(amd64_unknown_freebsd7_0_test) \
 || defined(i386_unknown_freebsd7_0_test)
    appProc->continueExecution();

    static const frameInfo_t correct_frame_info[] = {

#if defined( os_linux_test ) && (defined( arch_x86_test ) || defined( arch_x86_64_test ))
	{ true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if !defined(rs6000_ibm_aix4_1_test)
	{ false, false, BPatch_frameNormal, NULL },
#endif	
	{ true,  false, BPatch_frameNormal, "stop_process_" },
	{ true,  false, BPatch_frameNormal, "test_stack_2_func4" },
	{ true,  false, BPatch_frameNormal, "sigalrm_handler" },
	{ true,  false, BPatch_frameSignal, NULL },
	{ true,  false, BPatch_frameNormal, "test_stack_2_func3" },
	{ true,  false, BPatch_frameNormal, "test_stack_2_func2" },
	{ true,  false, BPatch_frameNormal, "test_stack_2_func1" },
	{ true,  false, BPatch_frameNormal, "test_stack_2_mutateeTest" },
	{ true,  false, BPatch_frameNormal, "main" }
    };

    if (waitUntilStopped(bpatch, appProc, 2, "getCallStack in signal handler") < 0) {
        appProc->terminateExecution();
      return FAILED;
    }

    if (checkStack(appThread,
		   correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   2, "getCallStack in signal handler")) {
	logerror("Passed test #2 (getCallStack in signal handler)\n");
    } else {
        appProc->terminateExecution();
      return FAILED;
    }

    appProc->continueExecution();
    while (!appProc->isTerminated()) {
      bpatch->waitForStatusChange();
    }
#else
    logerror("Skipping test #2 (getCallStack in signal handler)\n");
    logerror("    feature not implemented on this platform\n");
#endif

    return PASSED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test8_2_mutatorMAIN(ParameterDict &param)
test_results_t test_stack_2_Mutator::setup(ParameterDict &param) {
#ifdef os_windows_test
  return SKIPPED;
#else
  DyninstMutator::setup(param);
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  return PASSED;
#endif
}
