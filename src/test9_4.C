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

// $Id: test9_4.C,v 1.1 2005/09/29 20:39:57 bpellin Exp $
/*
 * #Name: test9_4
 * #Desc: call writeValue and save the world
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,rs6000_ibm_aix4_1,i386_unknown_linux2_0,x86_64_unknown_linux2_4
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
// Start Test Case #4 - (call writeValue and save the world)
//
int mutatorTest(char *pathname, BPatch *bpatch)
{
  const char* testName = "call writeValue and save the world";
  int testNo = 4;
  int passedTest;
  char *savedDirectory;
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
//||  defined(rs6000_ibm_aix4_1) this fails on aix from the test case but the
//mutated binary works fine when it is run by hand 

	BPatch_image *appImage;
	BPatch_thread *appThread;

	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);


	createNewProcess(bpatch, appThread, appImage, pathname, child_argv);

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func4_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func4_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func4_1");
    }

    BPatch_Vector<BPatch_point *> *func4_1 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!func4_1 || ((*func4_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func4_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr4_1 = findVariable(appImage, "globalVariable4_1", func4_1);

    if (expr4_1 == NULL) {
	fprintf(stderr, "**Failed** test #4 (read/write a variable in the mutatee)\n");
	fprintf(stderr, "    Unable to locate globalVariable4_1\n");
	exit(1);
    }

    int n;
    expr4_1->readValue(&n);

    if (n != 42) {
	fprintf(stderr, "**Failed** test #4 (read/write a variable in the mutatee)\n");
	fprintf(stderr, "    value read from globalVariable4_1 was %d, not 42 as expected\n", n);
	exit(1);
    }

    n = 17;
    expr4_1->writeValue(&n,true); //ccw 31 jul 2002


	char * dirname = saveWorld(appThread);
	savedDirectory = dirname;
	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/
 
	if( retValue == 0){
	
		if ( runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST4) )
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
