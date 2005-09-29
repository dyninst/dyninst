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

// $Id: test7_7.C,v 1.1 2005/09/29 20:39:48 bpellin Exp $
/*
 * #Name: test7_7
 * #Desc: Memory allocation in parent & child
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

bool parentDone = false;
bool childDone = false;
bool passedTest = true;
BPatch_thread *parentThread = NULL;
BPatch_thread *childThread = NULL;
int msgid = -1;


/* Verify that the memory created with malloc is per process and isn't
   being shared between a parent and child process.

   parent/child: malloc a variable
   parent/child: oneTimeCode(snippetA)  (malloced variable = 10)
   --- fork ---
   parent: insert snippet B  (malloced_var += 3);
   child:  insert snippet B' (malloced_var += 7);
   --- run  ---
   parent: verify malloced_var = 13
   child:  verify malloced_var = 17
*/

BPatch_variableExpr *var7_7p;
BPatch_variableExpr *var7_7c;

void prepareTestCase7(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      var7_7p = thread->malloc(*(parImage->findType("int")));
      if(doError(&passedTest, (var7_7p==NULL),
		 "  Unable to malloc variable in parent\n")) return;

      BPatch_arithExpr a_expr7_7p(BPatch_assign, *var7_7p,
				  BPatch_constExpr(10));
      thread->oneTimeCode(a_expr7_7p);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_7";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_7p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_7p || ((*points7_7p).size() == 0),
		 "  Unable to find entry point to \"func7_7\".\n")) return;
      BPatch_point *point7_7p = (*points7_7p)[0];

      BPatch_arithExpr a_expr7_7p(BPatch_plus, *var7_7p, BPatch_constExpr(3));
      BPatch_arithExpr b_expr7_7p(BPatch_assign, *var7_7p, a_expr7_7p);
      thread->insertSnippet(b_expr7_7p, *point7_7p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      var7_7c = thread->getInheritedVariable(*var7_7p);

      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_7";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_7c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_7c || ((*points7_7c).size() == 0),
		 "  Unable to find entry point to \"func7_7\".\n")) return;
      BPatch_point *point7_7c = (*points7_7c)[0];

      BPatch_arithExpr a_expr7_7c(BPatch_plus, *var7_7c, BPatch_constExpr(7));
      BPatch_arithExpr b_expr7_7c(BPatch_assign, *var7_7c, a_expr7_7c);

      thread->insertSnippet(b_expr7_7c, *point7_7c, BPatch_callBefore);
   }
}

void checkTestCase7(procType proc_type, BPatch_thread */*thread*/) {
   const int TN = 7;
   char varname[50];
   sprintf(varname,"test%d malloced var",TN);
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(varname, var7_7p, 13, proc_type)) {
	 passedTest = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(varname, var7_7c, 17, proc_type)) {
	 passedTest = false;
      }
   }
}


/* We make changes at post-fork */
void postForkFunc(BPatch_thread *parent, BPatch_thread *child)
{
    //fprintf(stderr, "in postForkFunc\n");
    /* For later identification */
    childThread = child;
    dprintf("Preparing tests on parent\n");
    prepareTestCase7(Parent_p, parent, PostFork);
    dprintf("Preparing tests on child\n");
    prepareTestCase7(Child_p,  child,  PostFork);
    dprintf("Fork handler finished (parent %p, child %p)\n", parent, child);
}

/* And verify them when they exit */
void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type) {
    dprintf("Exit func called\n");
    if (thread == parentThread) {
        dprintf("Parent exit reached, checking...\n");
        checkTestCase7(Parent_p, thread);
        parentDone = true;
        dprintf("Parent done\n");
    }
    else if (thread == childThread) {
        dprintf("Child exit reached, checking...\n");
        checkTestCase7(Child_p, thread);
        dprintf("Child done\n");
        childDone = true;
    }
    else {
        fprintf(stderr, "Thread ptr 0x%x, parent 0x%x, child 0x%x\n",
                thread, parentThread, childThread);
        assert(0 && "Unexpected BPatch_thread in exitFunc");
    }
    return;
}

void initialPreparation(BPatch_thread *parent)
{
   //cerr << "in initialPreparation\n";
   assert(parent->isStopped());

   //cerr << "ok, inserting instr\n";
   prepareTestCase7(Parent_p, parent, PreFork);
}

int mutatorTest(BPatch *bpatch, BPatch_thread *appThread)
{
   // Register Handlers
    bpatch->registerPostForkCallback(postForkFunc);
    bpatch->registerExitCallback(exitFunc);

    RETURNONFAIL(setupMessaging(&msgid));

    parentThread = appThread;

    initialPreparation(parentThread);
    /* ok, do the fork */;
    parentThread->continueExecution();

    /* the rest of the execution occurs in postForkFunc() */
    /* Secondary test: we should not have to manually continue
       either parent or child at any point */

    // Cleanup child parent is cleaned externally
    while ( !parentThread->isTerminated() || !childThread->isTerminated() )
    {
       bpatch->waitForStatusChange();
    }

    delete childThread;
    delete parentThread;

    showFinalResults(passedTest, 7);

    if ( passedTest )
    {
       return 0;
    }
    else
    {
       return -1;
    }
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    return mutatorTest(bpatch, appThread);
    
}
