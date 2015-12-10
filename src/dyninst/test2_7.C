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

// $Id: test2_7.C,v 1.1 2008/10/30 19:20:27 legendre Exp $
/*
 * #Name: test2_7
 * #Desc: Load a dynamically linked lbibraryr from the mutator
 * #Dep: 
 * #Arch: !(sparc_sun_solaris2_4_test,i386_unknown_solaris2_5_test,i386_unknown_linux2_0_test,mips_sgi_irix6_4_test,rs6000_ibm_aix4_1_test,ia64_unknown_linux2_4_test,x86_64_unknown_linux2_4_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_object.h"


#include "test_lib.h"
#include "test2.h"

#include "dyninst_comp.h"
class test2_7_Mutator : public DyninstMutator {
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_7_factory() {
  return new test2_7_Mutator();
}

static void lcase(char *s) {
    while (*s) {
        if (*s >= 'A' && *s <= 'Z')
            *s = *s - 'A' + 'a';
        s++;
    }
}

// static int mutatorTest(BPatch_thread *thread, BPatch_image *img)
test_results_t test2_7_Mutator::executeTest() {
#if !defined(os_linux_test) && \
    !defined(os_aix_test) && !defined(os_windows_test) && \
    !defined(os_freebsd_test)
    logerror("Skipping test #7 (load a dynamically linked library from the mutator)\n");
    logerror("    feature not implemented on this platform\n");
    return SKIPPED;
#else
    test_results_t result;

    if (!appProc->loadLibrary(TEST_DYNAMIC_LIB2)) {
    	logerror("**Failed** test #7 (load a dynamically linked library from the mutator)\n");
	logerror("    BPatch_thread::loadLibrary returned an error\n");
        result = FAILED;
    } else {
	// see if it worked
	bool found = false;
	char match2[256];
	sprintf(match2, "%s_module", TEST_DYNAMIC_LIB2);

        // Links are now resolved at library load so compare the names (minus the extension)
        std::string noext(TEST_DYNAMIC_LIB2);
        noext = noext.substr(0, noext.find_last_of("."));

	BPatch_Vector<BPatch_object *> obj;
	appImage->getObjects(obj);
	for (auto i = obj.begin(); i != obj.end(); i++) {
		char name[80];
		strncpy(name, (*i)->name().c_str(), 80);
#if defined(os_windows_test)
        //Windows files don't have case sensitive names, so make
        //sure we have a consistent name.
        lcase(name);
#endif
		if (strstr(name, TEST_DYNAMIC_LIB2) ||
#ifdef os_aix_test
		    strcmp(name, TEST_DYNAMIC_LIB2_NOPATH) == 0 ||
#endif
                    strstr(name, noext.c_str()) ||
		    strcmp(name, match2) == 0) {
		    found = true;
		    break;
		}
	}
	if (found) {
	    logerror("Passed test #7 (load a dynamically linked library from the mutator)\n");
            result = PASSED;
	} else {
	    logerror("**Failed** test #7 (load a dynamically linked library from the mutator)\n");
	    logerror("    image::getModules() did not indicate that the library had been loaded\n");
            result = FAILED;
	}
    }

    if (PASSED == result) {
      // Write a 'passed' flag in the mutatee so it can correctly print out
      // the human-readable summary for this test
      BPatch_variableExpr *passed_expr =
	appImage->findVariable("test2_7_passed");
      if (passed_expr == NULL) {
	logerror("**Failed** test #7 (load a dynamically linked library from the mutator)\n");
	logerror("    Unable to locate test2_7_passed\n");
	result = FAILED;
      } else {
	int pvalue = 1;
	passed_expr->writeValue(&pvalue);
      }
    }

    return result;
#endif
}
