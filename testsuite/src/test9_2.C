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

// $Id: test9_2.C,v 1.1 2005/09/29 20:39:55 bpellin Exp $
/*
 * #Name: test9_2
 * #Desc: Instrument many function calls and save the world
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,rs6000_ibm_aix4_1,i386_unknown_linux2_0,x86_64_unknown_linux2_4,rs6000_ibm_aix5_1
 * 5_1
 * #Notes: What to do about the remove directories/files options
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test9.h"


//
// Start Test Case #2 - (instrument many function calls and save the world)
//
int mutatorTest(char *pathname, BPatch *bpatch)
{
  char* testName = "instrument many function calls and save the world";
  int testNo = 2;
  int passedTest;
  char *savedDirectory;
 
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);

	char instrumentee[15];
	char patch[15];
	int i;
	BPatch_image *appImage;
	BPatch_thread *appThread;

	createNewProcess(bpatch, appThread, appImage, pathname, child_argv);


	for( i = 1;i<1001; i++){
		sprintf(instrumentee, "func2_%d", i);
		sprintf(patch, "call2_%d", i);

		instrumentToCallZeroArg(appThread, appImage, instrumentee, patch, testNo, testName);
	}

	char * dirname = saveWorld(appThread);
	savedDirectory = dirname;
	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		if ( runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST2) )
                {
                   return 0;
                } else {
		  fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
                  return -1;
                }
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);

	}
#else
	fprintf(stderr,"Skipped Test #%d: not implemented on this platform\n",testNo);
        return 0;
#endif	
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    char *pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return mutatorTest(pathname, bpatch);
}
