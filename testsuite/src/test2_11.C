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

// $Id: test2_11.C,v 1.3 2005/11/22 19:42:06 bpellin Exp $
/*
 * #Name: test2_11
 * #Desc: getDisplacedInstructions
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//
// Test 11 - getDisplacedInstructions
//	This function tests the getDisplacedInstructions instructions methods.
//	Currently this tests is only enabled on AIX platforms.
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func11_1", found_funcs, 1)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func11_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func11_1");
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_entry);

    if (points == NULL) {
	printf("**Failed** test #11 (getDisplacedInstructions)\n");
	printf("    unable to locate function \"func11_1\".\n");
        return -1;
    }

    char buf[128];
    memset(buf, 128, 0);
    int nbytes = (*points)[0]->getDisplacedInstructions(128, buf);
    if (nbytes < 0 || nbytes > 128) {
	printf("**Failed** test #11 (getDisplacedInstructions)\n");
	printf("    getDisplacedInstructions returned a strange number of bytes (%d)\n", nbytes);
        return -1;
    }
    int i;
    for (i = 0; i < nbytes; i++) {
	if (buf[i] != 0) break;
    }
    if (i == nbytes) {
	printf("**Failed** test #11 (getDisplacedInstructions)\n");
	printf("    getDisplacedInstructions doesn't seem to have returned any instructions\n");
        return -1;
    }
    printf("Passed test #11 (getDisplacedInstructions)\n");
    return 0;
}

extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
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
