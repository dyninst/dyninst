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

// $Id: test2_7.C,v 1.1 2007/09/24 16:39:28 cooksey Exp $
/*
 * #Name: test2_7
 * #Desc: Load a dynamically linked lbibraryr from the mutator
 * #Dep: 
 * #Arch: !(sparc_sun_solaris2_4,i386_unknown_solaris2_5,i386_unknown_linux2_0,mips_sgi_irix6_4,rs6000_ibm_aix4_1,ia64_unknown_linux2_4,x86_64_unknown_linux2_4)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test2.h"

#include "TestMutator.h"
class test2_7_Mutator : public TestMutator {
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test2_7_factory() {
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
test_results_t test2_7_Mutator::preExecution() {
#if !defined(os_solaris) && !defined(os_linux) && !defined(os_irix) && \
    !defined(os_aix) && !defined(os_windows)
    logerror("Skipping test #7 (load a dynamically linked library from the mutator)\n");
    logerror("    feature not implemented on this platform\n");
    return SKIPPED;
#else
    test_results_t result;

    if (!appThread->loadLibrary(TEST_DYNAMIC_LIB2)) {
    	logerror("**Failed** test #7 (load a dynamically linked library from the mutator)\n");
	logerror("    BPatch_thread::loadLibrary returned an error\n");
        result = FAILED;
    } else {
	// see if it worked
	bool found = false;
	char match2[256];
	sprintf(match2, "%s_module", TEST_DYNAMIC_LIB2);
	BPatch_Vector<BPatch_module *> *m = appImage->getModules();
	for (unsigned int i=0; i < m->size(); i++) {
		char name[80];
		(*m)[i]->getName(name, sizeof(name));
#if defined(os_windows)
        //Windows files don't have case sensitive names, so make
        //sure we have a consistent name.
        lcase(name);
#endif
		if (strstr(name, TEST_DYNAMIC_LIB2) ||
#ifdef rs6000_ibm_aix4_1
		    strcmp(name, TEST_DYNAMIC_LIB2_NOPATH) == 0 ||
#endif
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

// extern "C" TEST_DLL_EXPORT int test2_7_mutatorMAIN(ParameterDict &param)
// {
//     bool useAttach = param["useAttach"]->getInt();
//     BPatch *bpatch = (BPatch *)(param["bpatch"]->getPtr());

//     BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

//     // Read the program's image and get an associated image object
//     BPatch_image *appImage = appThread->getImage();

//     // Get log file pointers
//     FILE *outlog = (FILE *)(param["outlog"]->getPtr());
//     FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//     setOutputLog(outlog);
//     setErrorLog(errlog);

//     // Signal the child that we've attached
//     if (useAttach) {
// 	signalAttached(appThread, appImage);
//     }

//     // This calls the actual test to instrument the mutatee
//     return mutatorTest(appThread, appImage);
// }
