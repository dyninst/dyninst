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

// $Id: test2_13.C,v 1.1 2005/09/29 20:39:03 bpellin Exp $
/*
 * #Name: test2_13
 * #Desc: loadLibrary failure test
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0,alpha_dec_osf4_0)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

BPatch *bpatch;

char loadLibErrStr[256] = "no error";
void llErrorFunc(BPatchErrorLevel level, int num, const char **params)
{

  char line[256];
  const char *msg = bpatch->getEnglishErrorString(num);
  bpatch->formatErrorString(line, sizeof(line), msg, params);

  if (num == 124) {
    strcpy(loadLibErrStr, line);
  }
  else {
    printf("Unexpected Error #%d (level %d): %s\n", num, level, line);
  }

}

// Start Test Case #13 - (loadLibrary failure test)
int mutatorTest( BPatch_thread * appThread, BPatch_image * appImage )
{
#if !defined(i386_unknown_nt4_0) \
 && !defined(alpha_dec_osf4_0)

  if (appThread->isTerminated()) {
    printf( "**Failed** test #13 (dlopen failure reporting test)\n" );
    printf("%s[%d]: mutatee in unexpected (terminated) state\n", __FILE__, __LINE__);
    return -1;
  }

  BPatchErrorCallback oldErrorFunc = bpatch->registerErrorCallback(llErrorFunc);
  
  if (appThread->loadLibrary("noSuchLibrary.Ever")) {
    fprintf(stderr, "**Failed** test #13 (failure reporting for loadLibrary)\n");
    return -1;
  }
  else {
    if (!strcmp(loadLibErrStr, "no error")) {
      printf( "**Failed** test #13 (dlopen failure reporting test)\n" );
      printf( "\tno error string produced\n" );
      return -1;
    }
    else {
      printf( "Passed test #13 (dlopen failure test: %s)\n", loadLibErrStr);
      return 0;
    }
  }
  bpatch->registerErrorCallback(oldErrorFunc);
#else
  printf( "Skipped test #13 (dlopen failure reporting test)\n" );
  printf( "\t- not implemented on this platform\n" );
  return 0;
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
