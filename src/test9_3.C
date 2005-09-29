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

// $Id: test9_3.C,v 1.1 2005/09/29 20:39:56 bpellin Exp $
/*
 * #Name: test9_3
 * #Desc: instrument a function with arguments and save the world
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,rs6000_ibm_aix4_1,i386_unknown_linux2_0,rs6000_ibm_aix5_1
 * 5_1
 * #Notes: What to do about the remove directories/files options
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test1.h"
#include "test9.h"

//
// Start Test Case #3 - (instrument a function with arguments and save the world)
//
int mutatorTest(char *pathname, BPatch *bpatch)
{
  const char* testName = "four parameter function";
  int testNo = 3;
  int passedTest;
  char *savedDirectory;
 
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
//||  defined(rs6000_ibm_aix4_1) this fails on aix from the test case but the
//mutated binary works fine when it is run by hand 

    // Find the entry point to the procedure "func3_1"
	BPatch_image *appImage;
	BPatch_thread *appThread;

	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);


	createNewProcess(bpatch, appThread, appImage, pathname, child_argv);

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction("func3_1", found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func3_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func3_1");
  }
  
  BPatch_Vector<BPatch_point *> *point3_1 = found_funcs[0]->findPoint(BPatch_entry);

  if (!point3_1 || ((*point3_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find entry point to \"func3_1.\"\n");
    exit(1);
  }

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "call3_1";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *call2_func = bpfv[0];

    void *ptr;

    /* For platforms where there is only one possible size for a pointer. */
    ptr = TEST_PTR;

    BPatch_Vector<BPatch_snippet *> call2_args;

    BPatch_constExpr expr3_1 (0), expr2_2 (0), expr2_3 (0), expr2_4 (0);

    int mutateeFortran = isMutateeFortran(appImage);
    if (mutateeFortran) {
        BPatch_variableExpr *expr2_5 = appThread->malloc (*appImage->findType ("int"));
        BPatch_variableExpr *expr2_6 = appThread->malloc (*appImage->findType ("int"));

        expr3_1 = expr2_5->getBaseAddr ();
        expr2_2 = expr2_6->getBaseAddr ();

        BPatch_arithExpr expr2_7 (BPatch_assign, *expr2_5, BPatch_constExpr(1));
        appThread->insertSnippet (expr2_7, *point3_1);

        BPatch_arithExpr expr2_8 (BPatch_assign, *expr2_6, BPatch_constExpr(2));
        appThread->insertSnippet (expr2_8, *point3_1);

        expr2_3 = "testString3_1";
        expr2_4 = 13;
    } else {
        expr3_1 = 1;
        expr2_2 = 2;
        expr2_3 = "testString3_1";
        expr2_4 = ptr;
    }

    call2_args.push_back(&expr3_1);
    call2_args.push_back(&expr2_2);
    call2_args.push_back(&expr2_3);
    call2_args.push_back(&expr2_4);

    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    dprintf("Inserted snippet2\n");
    appThread->insertSnippet(call2Expr, *point3_1, BPatch_callBefore, BPatch_lastSnippet);

	char * dirname = saveWorld(appThread);
	savedDirectory = dirname;	
	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		if ( runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST3) )
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
