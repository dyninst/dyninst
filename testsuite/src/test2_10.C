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

// $Id: test2_10.C,v 1.1 2005/09/29 20:39:00 bpellin Exp $
/*
 * #Name: test2_10
 * #Desc: Dump image
 * #Dep: 
 * #Arch: (rs6000_ibm_aix4_1,sparc_sun_sunos4_1_3,sparc_sun_solaris2_4,i386_unknown_linux2_0,mips_sgi_irix6_4,alpha_dec_osf4_0,ia64_unknown_linux2_4,x86_64_unknown_linux2_4
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

//
// Test #10 - dump image
//	This test dumps out the modified program file.  Note: only the 
//      modified executable is written, any shared libraries that have been
//	instrumented are not written.  In addition, the current dyninst
//	shared library is *NOT* written either.  Thus the results image is
//	really only useful for checking the state of instrumentation code
//	via gdb. It will crash if you try to run it.
//
int mutatorTest(BPatch_thread *thread, BPatch_image * /*appImage */)
{
#if !defined(rs6000_ibm_aix4_1) \
 && !defined(sparc_sun_sunos4_1_3) \
 && !defined(sparc_sun_solaris2_4) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(mips_sgi_irix6_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

    printf("Skipping test #10 (dump image)\n");
    printf("    BPatch_thread::dumpImage not implemented on this platform\n");
    return 0;
#else

  if (thread->isTerminated()) {
    printf( "**Failed** test #10 (dump image)\n" );
    printf("%s[%d]: mutatee in unexpected (terminated) state\n", __FILE__, __LINE__);
    return -1;
  }

    // dump image
    if (access("myimage", F_OK) == 0) {
	dprintf("File \"myimage\" exists.  Deleting it.\n");
	if (unlink("myimage") != 0) {
	    printf("Couldn't delete the file \"myimage\".  Exiting.\n");
            return -1;
	}
    }

    clearError();
    thread->dumpImage("myimage");
    int gotError = getError();
    bool imageExists = (access("myimage", F_OK) == 0);
    if (gotError || !imageExists) {
	printf("**Failed** test #10 (dump image)\n");
	if (gotError)
	    printf("    error reported by dumpImage\n");
	if (!imageExists)
	    printf("    the image file wasn't written\n");
        return -1;
    } else {
    	printf("Passed test #10 (dump image)\n");
	unlink("myimage");
        return 0;
    }
#endif
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    bool useAttach = param["useAttach"]->getInt();
    BPatch *bpatch = (BPatch *)(param["bpatch"]->getPtr());

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
