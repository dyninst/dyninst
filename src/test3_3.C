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

// $Id: test3_3.C,v 1.2 2005/11/22 19:42:20 bpellin Exp $
/*
 * #Name: test3_3
 * #Desc: instrument multiple processes
 * #Dep: 
 * #Arch: all
 * #Notes: useAttach does not apply
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

const unsigned int MAX_MUTATEES = 32;
unsigned int Mutatees=3;
int debugPrint;

//
// read the result code written to the file test3.out.<pid> and return it
//
int readResult(int pid)
{
    int ret;
    FILE *fp;
    char filename[80];

    sprintf(filename, "test3.out.%d", pid);
    fp = fopen(filename, "r");
    if (!fp) {
	printf("ERROR: unable to open output file %s\n", filename);
	return -1;
    }
    fscanf(fp, "%d\n", &ret);
    fclose(fp);
    // don't need the file any longer so delete it now
    unlink(filename);

    return ret;
}

//
// Start Test Case #3 - create processes and insert different code into
//     each one.  The code sets a global variable which the mutatee then
//     writes to a file.  After all mutatees exit, the mutator reads the
//     files to verify that the correct code ran in each mutatee.
//     The first mutator should write a 1 to the file, the second a 2, etc.
//     If no code is patched into the mutatees, the value is 0xdeadbeef.
//
int mutatorTest(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("3");	    // run test3 in mutatee
    child_argv[n++] = NULL;

    int pid[MAX_MUTATEES];
    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            printf("*ERROR*: unable to create handle%d for executable\n", n);
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(n-1,appThread);
            return -1;
        }
        pid[n] = appThread[n]->getPid();
        dprintf("Mutatee %d started, pid=%d\n", n, pid[n]);
    }

    // Instrument mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Instrumenting %d/%d\n", n, Mutatees);

        const char *Func="func3_1";
        const char *Var="test3ret";
        const char *Call="call3_1";
        BPatch_image *img = appThread[n]->getImage();

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == img->findFunction(Func, found_funcs, 1)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      Func);
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), Func);
    }

    BPatch_Vector<BPatch_point *> *point = found_funcs[0]->findPoint(BPatch_entry);

        if (!point || (*point).size() == 0) {
            printf("  Unable to find entry point to \"%s\".\n", Func);
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return -1;
        }
        BPatch_variableExpr *var = img->findVariable(Var);
        if (var == NULL) {
            printf("  Unable to find variable \"%s\".\n", Var);
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return -1;
        }

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == img->findFunction(Call, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  printf("  Unable to find target function \"%s\".\n", Call);
	  printf("**Failed** test #3 (instrument multiple processes)\n");
          return -1;
	}

	BPatch_function *callFunc = bpfv[0];

        // start with a simple snippet
        BPatch_arithExpr snip(BPatch_assign, *var, BPatch_constExpr((int)n));
        BPatchSnippetHandle *inst = appThread[n]->insertSnippet(snip, *point);
        if (inst == NULL) {
            printf("  Failed to insert simple snippet.\n");
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return -1;
        }

        // now add a call snippet
        BPatch_Vector<BPatch_snippet *> callArgs;
        BPatch_constExpr arg1(2); callArgs.push_back(&arg1);
        BPatch_constExpr arg2((int)n); callArgs.push_back(&arg2);
        BPatch_funcCallExpr callExpr(*callFunc, callArgs);
        BPatchSnippetHandle *call = 
                appThread[n]->insertSnippet(callExpr, *point);
        if (call == NULL) {
            printf("  Failed to insert call snippet.\n");
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return -1;
        }
    }

    dprintf("Letting %d mutatee processes run.\n", Mutatees);
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    unsigned int numTerminated=0;
    bool terminated[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) terminated[n]=false;

    // monitor the mutatee termination reports
    while (numTerminated < Mutatees) {
	bpatch->waitForStatusChange();
        for (n=0; n<Mutatees; n++)
            if (!terminated[n] && (appThread[n]->isTerminated())) {
                if(appThread[n]->terminationStatus() == ExitedNormally) {
                    int exitCode = appThread[n]->getExitCode();
                    if (exitCode || debugPrint)
                        dprintf("Mutatee %d exited with exit code 0x%x\n", n,
                                exitCode);
                }
                else if(appThread[n]->terminationStatus() == ExitedViaSignal) {
                    int signalNum = appThread[n]->getExitSignal();
                    if (signalNum || debugPrint)
                        dprintf("Mutatee %d exited from signal 0x%d\n", n,
                                signalNum);
                }
                delete appThread[n];
		terminated[n]=true;
                numTerminated++;
            }
    }

    // now read the files to see if the value is what is expected
    bool allCorrect=true;
    int ret[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) {
        ret[n]=readResult(pid[n]);
        if (ret[n] != (int)n) {
            printf("    mutatee process %d produced %d, not %d\n",
                pid[n], ret[n], n);
            allCorrect=false;
        } else {
            dprintf("    mutatee process %d produced expected value %d\n", 
                pid[n], ret[n]);
        }
    }

    if (allCorrect) {
        printf("Passed Test #3 (instrument multiple processes)\n");
        return 0;
    } else {
        printf("**Failed** test #3 (instrument multiple processes)\n");
        return -1;
    }
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    char *pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();
    

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return mutatorTest(pathname, bpatch);
}
