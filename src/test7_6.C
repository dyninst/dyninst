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

// $Id: test7_6.C,v 1.6 2008/04/11 23:31:22 legendre Exp $
/*
 * #Name: test7_6
 * #Desc: OneTimeCode in parent & child
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test_lib_test7.h"

static bool parentDone = false;
static bool childDone = false;
static bool passedTest = true;
static BPatch_thread *parentThread = NULL;
static BPatch_thread *childThread = NULL;
static int msgid = -1;

/* Run a oneTimeCode in both the parent and child and see if they both
   happen.

   parent/child: globalVariable7_6 initial value = 21

   parent: run one time code, value += 5
   child:  run one time code, value += 9
   parent: value == 26
   child:  value == 30
*/

static void prepareTestCase6(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_variableExpr *var7_6p = 
	 parImage->findVariable("globalVariable7_6");
      if(doError(&passedTest, (var7_6p==NULL),
		 "  Unable to locate variable globalVariable7_6\n")) return;

      BPatch_arithExpr a_expr7_6p(BPatch_plus, *var7_6p, BPatch_constExpr(5));
      BPatch_arithExpr b_expr7_6p(BPatch_assign, *var7_6p, a_expr7_6p);
      thread->oneTimeCode(b_expr7_6p);
      
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();

      BPatch_variableExpr *var7_6c = 
	 childImage->findVariable("globalVariable7_6");
      if(doError(&passedTest, (var7_6c==NULL),
		 "  Unable to locate variable globalVariable7_6\n")) return;

      BPatch_arithExpr a_expr7_6c(BPatch_plus, *var7_6c, BPatch_constExpr(9));
      BPatch_arithExpr b_expr7_6c(BPatch_assign, *var7_6c, a_expr7_6c);
      thread->oneTimeCode(b_expr7_6c);
   }
}

static void checkTestCase6(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_6", 26, proc_type)) {
	 passedTest = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_6", 30, proc_type)) {
	 passedTest = false;
      }
   }
}



/* We make changes at post-fork */
static void postForkFunc(BPatch_thread *parent, BPatch_thread *child)
{
    //dprintf("in postForkFunc\n");
    /* For later identification */
    childThread = child;
    dprintf("Preparing tests on parent\n");
    prepareTestCase6(Parent_p, parent, PostFork);
    dprintf("Preparing tests on child\n");
    prepareTestCase6(Child_p,  child,  PostFork);
    dprintf("Fork handler finished (parent %p, child %p)\n", parent, child);
}

/* And verify them when they exit */
static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type) {
    dprintf("Exit func called\n");
    if (thread == parentThread) {
        dprintf("Parent exit reached, checking...\n");
        checkTestCase6(Parent_p, thread);
        parentDone = true;
        dprintf("Parent done\n");
    }
    else if (thread == childThread) {
        dprintf("Child exit reached, checking...\n");
        checkTestCase6(Child_p, thread);
        dprintf("Child done\n");
        childDone = true;
    }
    else {
        dprintf("Thread ptr 0x%x, parent 0x%x, child 0x%x\n",
                thread, parentThread, childThread);
        assert(0 && "Unexpected BPatch_thread in exitFunc");
    }
    return;
}

static void initialPreparation(BPatch_thread *parent)
{
   //cerr << "in initialPreparation\n";
   assert(parent->isStopped());

   //cerr << "ok, inserting instr\n";
   prepareTestCase6(Parent_p, parent, PreFork);
}

static int mutatorTest(BPatch *bpatch, BPatch_thread *appThread)
{
    if ( !setupMessaging(&msgid) )
    {
       passedTest = false;
       return passedTest;
    }

    parentThread = appThread;

    initialPreparation(parentThread);
    /* ok, do the fork */;
    parentThread->continueExecution();

    /* the rest of the execution occurs in postForkFunc() */
    /* Secondary test: we should not have to manually continue
       either parent or child at any point */

    while ( !parentThread->isTerminated() ) 
    {
       bpatch->waitForStatusChange();
    }

    // At this point if childThread == NULL the postfork handler failed
    // to run.  Fail gracefully instead of segfaulting on 
    // childThread->isTerminated()
    if (doError(&passedTest, childThread == NULL,
             "childThread == NULL: postForkFunc must not have run\n") )
    {
       return passedTest;
    }
    
    if ( !childThread->isTerminated() )
    {
       bpatch->waitForStatusChange();
    }

    return passedTest;
}

extern "C" int test7_6_mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Get log file pointers
    FILE *outlog = (FILE *)(param["outlog"]->getPtr());
    FILE *errlog = (FILE *)(param["errlog"]->getPtr());
    setOutputLog(outlog);
    setErrorLog(errlog);

    // Register callbacks
    bpatch->registerPostForkCallback(postForkFunc);
    bpatch->registerExitCallback(exitFunc);

    bool passed = mutatorTest(bpatch, appThread);

    // Remove callbacks upon test completion
    bpatch->registerPostForkCallback(NULL);
    bpatch->registerExitCallback(NULL);

    showFinalResults(passed, 6);
    if ( passed )
       return 0;
    else
       return -1;
}
