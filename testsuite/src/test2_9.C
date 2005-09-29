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

// $Id: test2_9.C,v 1.1 2005/09/29 20:39:13 bpellin Exp $
/*
 * #Name: test2_9
 * #Desc: dump core but do not terminate
 * #Dep: 
 * #Arch: (sparc_sun_sunos4_1_3,sparc_sun_solaris2_4)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

//
// Test #9 - dump core but do not terminate
//	This test dumps the core file from the mutatee process, without having
//	the process terminate exuection.  It looks for the creation of the file
//	"mycore" in the current directory.
//      
int mutatorTest(BPatch_thread *thread, BPatch_image * /*appImage*/)
{
#if !defined(sparc_sun_sunos4_1_3) \
 && !defined(sparc_sun_solaris2_4) 
    printf("Skipping test #9 (dump core but do not terminate)\n");
    printf("    BPatch_thread::dumpCore not implemented on this platform\n");
    return 0;
#else

    // dump core, but do not terminate.
    // this doesn't seem to do anything - jkh 7/12/97
    if (access("mycore", F_OK) == 0) {
        dprintf("File \"mycore\" exists.  Deleting it.\n");
	if (unlink("mycore") != 0) {
	    printf("Couldn't delete the file \"mycore\".  Exiting.\n");
	    exit(-1);
	}
    }

    clearError();
    thread->dumpCore("mycore", true);
    bool coreExists = (access("mycore", F_OK) == 0);
    int gotError = getError();
    if (gotError || !coreExists) {
	printf("**Failed** test #9 (dump core but do not terminate)\n");
	if (gotError)
	    printf("    error reported by dumpCore\n");
	if (!coreExists)
	    printf("    the core file wasn't written\n");
        return -1;
    } else {
	unlink("mycore");
    	printf("Passed test #9 (dump core but do not terminate)\n");
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
