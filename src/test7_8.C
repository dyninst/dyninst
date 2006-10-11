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

// $Id: test7_8.C,v 1.5 2006/10/11 21:54:13 cooksey Exp $
/*
 * #Name: test7_8
 * #Desc: Memory deallocate in child
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


/* Verify that if a variable in the child process is freed with 
   BPatch_thread::free, the corresponding variable in the parent process
   isn't also deleted.

   parent/child: malloc a variable
   parent/child: oneTimeCode(snippetA)  (malloced variable = 10)
   --- fork ---
   parent: insert snippet B  (malloced_var += 3);
   child:  free(getInheritedVariable(malloced_var))
   --- run  ---
   parent: verify malloced_var = 13
   (no way to verify the child variable has indeed been freed)
*/

static BPatch_variableExpr *var7_8p;

static void prepareTestCase8(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      var7_8p = thread->malloc(*(parImage->findType("int")));
      if(doError(&passedTest, (var7_8p==NULL),
		 "  Unable to malloc variable in parent\n")) return;

      BPatch_arithExpr a_expr7_8p(BPatch_assign, *var7_8p,
				  BPatch_constExpr(10));
      thread->oneTimeCode(a_expr7_8p);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_8";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	logerror("    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_8p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_8p || ((*points7_8p).size() == 0),
		 "  Unable to find entry point to \"func7_8\".\n")) return;
      BPatch_point *point7_8p = (*points7_8p)[0];

      BPatch_arithExpr a_expr7_8p(BPatch_plus, *var7_8p, BPatch_constExpr(3));
      BPatch_arithExpr b_expr7_8p(BPatch_assign, *var7_8p, a_expr7_8p);

      thread->insertSnippet(b_expr7_8p, *point7_8p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_variableExpr *var7_8c = thread->getInheritedVariable(*var7_8p);
      thread->free(*var7_8c);
   }
}

static void checkTestCase8(procType proc_type, BPatch_thread * /*thread*/) {
   char varname[50];
   sprintf(varname,"test%d malloced var",8);
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(varname, var7_8p, 13, proc_type)) {
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
    prepareTestCase8(Parent_p, parent, PostFork);
    dprintf("Preparing tests on child\n");
    prepareTestCase8(Child_p,  child,  PostFork);
    dprintf("Fork handler finished (parent %p, child %p)\n", parent, child);
}

/* And verify them when they exit */
static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type) {
    dprintf("Exit func called\n");
    if (thread == parentThread) {
        dprintf("Parent exit reached, checking...\n");
        checkTestCase8(Parent_p, thread);
        parentDone = true;
        dprintf("Parent done\n");
    }
    else if (thread == childThread) {
        dprintf("Child exit reached, checking...\n");
        checkTestCase8(Child_p, thread);
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
   prepareTestCase8(Parent_p, parent, PreFork);
}

static int mutatorTest(BPatch *bpatch, BPatch_thread *appThread)
{
    if ( !setupMessaging(&msgid) )
    {
       passedTest = false;
       delete parentThread;
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
       delete parentThread;
       return passedTest;
    }
    
    if ( !childThread->isTerminated() )
    {
       bpatch->waitForStatusChange();
    }

    // Cleanup child, parent
    delete childThread;
    delete parentThread;

    return passedTest;
}

extern "C" int test7_8_mutatorMAIN(ParameterDict &param)
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

    showFinalResults(passed, 8);
    if ( passed )
       return 0;
    else
       return -1;
}
