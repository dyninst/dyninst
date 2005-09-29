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

// $Id: test2_6.C,v 1.1 2005/09/29 20:39:10 bpellin Exp $
/*
 * #Name: test2_6
 * #Desc: Load a dynamically linked library from the mutatee
 * #Dep: 
 * #Arch: !(sparc_sun_solaris2_4,i386_unknown_solaris2_5,i386_unknown_linux2_0,mips_sgi_irix6_4,alpha_dec_osf4_0,rs6000_ibm_aix4_1,ia64_unknown_linux2_4,x86_64_unknown_linux2_4)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test2.h"

BPatch *bpatch;

//
// Test #6 - load a dynamically linked library from the mutatee
//	Have the mutatee use dlopen (or NT loadLibrary) to load a shared library
//	into itself.  We should then be able to see the new functions from the
//	library via getModules.
//
int mutatorTest(BPatch_thread *thread, BPatch_image *img)
{
#if !defined(sparc_sun_solaris2_4) \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(mips_sgi_irix6_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

    printf("Skipping test #6 (load a dynamically linked library from the mutatee)\n");
    printf("    feature not implemented on this platform\n");
    return 0;
#else

    thread->continueExecution();
    waitUntilStopped(bpatch, thread, 6, "load a dynamically linked library");
    bool found = false;

    // see if the dlopen happended.
    char match2[256];
    sprintf(match2, "%s_module", TEST_DYNAMIC_LIB);
    BPatch_Vector<BPatch_module *> *m = img->getModules();
    for (unsigned i=0; i < m->size(); i++) {
	    char name[80];
	    (*m)[i]->getName(name, sizeof(name));
	    if (strstr(name, TEST_DYNAMIC_LIB) ||
#ifdef rs6000_ibm_aix4_1
		strcmp(name, TEST_DYNAMIC_LIB_NOPATH) == 0 ||
#endif
		strcmp(name, match2) == 0) {
		found = true;
		break;
	    }
    }

    if (found) {
    	printf("Passed test #6 (load a dynamically linked library from the mutatee)\n");
        return 0;
    } else {
    	printf("**Failed** test #6 (load a dynamically linked library from the mutatee)\n");
	printf("    image::getModules() did not indicate that the library had been loaded\n");
        return -1;
    }
#endif
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());

    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
	signalAttached(appThread, appImage);
    }

    // This calls the actual test to instrument the mutatee
    return mutatorTest(appThread, appImage);
}
