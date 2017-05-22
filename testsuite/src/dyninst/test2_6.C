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

// $Id: test2_6.C,v 1.1 2008/10/30 19:20:25 legendre Exp $
/*
 * #Name: test2_6
 * #Desc: Load a dynamically linked library from the mutatee
 * #Dep: 
 * #Arch: !(sparc_sun_solaris2_4_test,i386_unknown_solaris2_5_test,i386_unknown_linux2_0_test,mips_sgi_irix6_4_test,alpha_dec_osf4_0_test,rs6000_ibm_aix4_1_test,ia64_unknown_linux2_4_test,x86_64_unknown_linux2_4_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test2.h"

#include "dyninst_comp.h"
class test2_6_Mutator : public DyninstMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_6_factory() {
  return new test2_6_Mutator();
}

//
// Test #6 - load a dynamically linked library from the mutatee
//	Have the mutatee use dlopen (or NT loadLibrary) to load a shared library
//	into itself.  We should then be able to see the new functions from the
//	library via getModules.
//
// static int mutatorTest(BPatch_thread *thread, BPatch_image *img)
test_results_t test2_6_Mutator::executeTest() {
    appProc->continueExecution();
    waitUntilStopped(bpatch, appProc, 6, "load a dynamically linked library");
    bool found = false;

    // see if the dlopen happened.
    char match2[256];
    sprintf(match2, "%s_module", TEST_DYNAMIC_LIB);

    // Links are now resolved at library load so compare the names (minus the extension)
    std::string noext(TEST_DYNAMIC_LIB);
    noext = noext.substr(0, noext.find_last_of("."));

    BPatch_Vector<BPatch_module *> *m = appImage->getModules();
    for (unsigned i=0; i < m->size(); i++) {
	    char name[80];
	    (*m)[i]->getName(name, sizeof(name));
	    if (strstr(name, TEST_DYNAMIC_LIB) ||
#ifdef os_aix_test
		strcmp(name, TEST_DYNAMIC_LIB_NOPATH) == 0 ||
#endif
                strcmp(name, noext.c_str()) ||
		strcmp(name, match2) == 0) {
		found = true;
		break;
	    }
    }

    if (found) {
    	logerror("Passed test #6 (load a dynamically linked library from the mutatee)\n");
	
	appProc->continueExecution();
	while (!appProc->isTerminated()) {
	  bpatch->waitForStatusChange();
	}
        return PASSED;
    } else {
    	logerror("**Failed** test #6 (load a dynamically linked library from the mutatee)\n");
	logerror("    image::getModules() did not indicate that the library had been loaded\n");
	appProc->continueExecution();
        while (!appProc->isTerminated()) {
	  bpatch->waitForStatusChange();
	}
        return FAILED;
    }
}

// extern "C" TEST_DLL_EXPORT int test2_6_mutatorMAIN(ParameterDict &param)
test_results_t test2_6_Mutator::setup(ParameterDict &param) {
  test_results_t retval = DyninstMutator::setup(param);
  if (PASSED == retval) {
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
  }

  return retval;
}
