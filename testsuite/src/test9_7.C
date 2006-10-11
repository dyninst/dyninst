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

// $Id: test9_7.C,v 1.4 2006/10/11 21:54:26 cooksey Exp $
/*
 * #Name: test9_7
 * #Desc: instrument entry point of main and first basic block in main
 * #Dep: 
 * #Arch: 
 * #Notes: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test9.h"

//
// Start Test Case #7 - (instrument entry point of main and first basic block in main)
//
static int mutatorTest(char *pathname, BPatch *bpatch)
{
  char* testName = "instrument entry point of main and first basic block in main";
  int testNo = 7;
  char *savedDirectory;
 
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)

	const char* child_argv[MAX_TEST+5];
	
	buildArgs(child_argv, pathname, testNo);

	BPatch_thread *appThread;
	BPatch_image *appImage;
	if ( !createNewProcess(bpatch, appThread, appImage, pathname, child_argv) )
        {
           logerror("**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
           return -1;
        }
	
	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction("main", found_funcs)) || !found_funcs.size()) {
		logerror("    Unable to find function main\n");
                return -1;
	}
  
	if (1 < found_funcs.size()) {
		logerror("%s[%d]:  WARNING  : found %d functions named main.  Using the first.\n", 
			__FILE__, __LINE__, found_funcs.size());
	}
  
	BPatch_point *point=NULL;

	const BPatch_Vector<BPatch_point*>* points = found_funcs[0]->findPoint(BPatch_subroutine);
	char buff[1024];
	bool foundIt = false;

	for(int i=0; !foundIt && i< points->size(); i++){

		(*points)[i]->getCalledFunction()->getName(buff,1023);
		if( !strncmp(buff,"firstBasicBlock",15)){
			foundIt = true;
			point = (*points)[i];
		}
	}

	if (!point ){
		logerror("**Failed** test #%d (%s)\n", testNo,testName);
		logerror("    Unable to find call to firstBasicBlock() in main()\n");
                return -1;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction("funcIncrGlobalMain", bpfv) || !bpfv.size()
     		|| NULL == bpfv[0]){
		logerror("**Failed** test #%d (%s)\n", testNo, testName);
		logerror("    Unable to find function funcIncrGlobalMain\n");
                return -1;
	}
	BPatch_function *call1_func = bpfv[0];
  
	BPatch_Vector<BPatch_snippet *> call1_args;
	BPatch_funcCallExpr call1Expr(*call1_func, call1_args);
  
	appThread->insertSnippet(call1Expr, *point);

	instrumentToCallZeroArg(appThread, appImage, "main", "funcIncrGlobalMainBy2", testNo, testName);
	char* dirname=saveWorld(appThread);	
	savedDirectory = dirname;

	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0 ){
	
		if ( runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST7) )
                {
                   return 0;
                } else {
                   return -1;
                }
	}else{
		logerror("**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
                return -1;
	}

	//appThread->terminateExecution();
#else
	logerror("Skipped Test #%d: not implemented on this platform\n",testNo);
        return 0;

#endif	
}

extern "C" int test9_7_mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    char *pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());

    // Get log file pointers
    FILE *outlog = (FILE *)(param["outlog"]->getPtr());
    FILE *errlog = (FILE *)(param["errlog"]->getPtr());
    setOutputLog(outlog);
    setErrorLog(errlog);

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return mutatorTest(pathname, bpatch);
}
