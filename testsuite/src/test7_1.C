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

// $Id: test7_1.C,v 1.1 2005/09/29 20:39:42 bpellin Exp $
/*
 * #Name: test7_1
 * #Desc: Delete snippet in parent
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


/* Make sure deleting a snippet in a parent process doesn't delete the
   snippet in the child process.

   parent: snippetHandleA  = insert snippetA at pointA
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   --- fork ---
   parent: deleteSnippet( snippetHandleA )
   --- run  ---
   child:  verify snippetHandleA' still ran
*/

void prepareTestCase1(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   static BPatchSnippetHandle *parSnippetHandle1;
   
   if(proc_type == Parent_p  &&  when == PreFork) {
       BPatch_image *parImage = thread->getImage();
       
       BPatch_Vector<BPatch_function *> found_funcs;
       const char *inFunction = "func7_1";
       if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
           fprintf(stderr, "    Unable to find function %s\n",
                   inFunction);
           exit(1);
       }
       
       if (1 < found_funcs.size()) {
           fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
                   __FILE__, __LINE__, found_funcs.size(), inFunction);
       }
       
       BPatch_Vector<BPatch_point *> *point7_1p = found_funcs[0]->findPoint(BPatch_entry);
       
       if(doError(&passedTest, !point7_1p || ((*point7_1p).size() == 0),
                  "  Unable to find entry point to \"func7_1\".\n")) return;
       
       BPatch_variableExpr *var7_1p = 
       parImage->findVariable("globalVariable7_1");
       if(doError(&passedTest, (var7_1p==NULL),
                  "  Unable to locate variable globalVariable7_1\n")) return;
       
       BPatch_arithExpr expr7_1p(BPatch_assign, *var7_1p,BPatch_constExpr(321));
       
       parSnippetHandle1 =
       thread->insertSnippet(expr7_1p, *point7_1p, BPatch_callBefore);
       if(doError(&passedTest, (parSnippetHandle1 == NULL),
                  "  Unable to insert snippet into parent for test 1\n")) return;
   } else if(proc_type == Parent_p  &&  when == PostFork) {
       thread->deleteSnippet(parSnippetHandle1);
   }
}

void checkTestCase1(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_1", 123, proc_type)) {
	 passedTest = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_1", 321, proc_type)) {
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
    prepareTestCase1(Parent_p, parent, PostFork);
    dprintf("Preparing tests on child\n");
    prepareTestCase1(Child_p,  child,  PostFork);
    dprintf("Fork handler finished (parent %p, child %p)\n", parent, child);
}

/* And verify them when they exit */
void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type) {
    dprintf("Exit func called\n");
    if (thread == parentThread) {
        dprintf("Parent exit reached, checking...\n");
        checkTestCase1(Parent_p, thread);
        parentDone = true;
        dprintf("Parent done\n");
    }
    else if (thread == childThread) {
        dprintf("Child exit reached, checking...\n");
        checkTestCase1(Child_p, thread);
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
   prepareTestCase1(Parent_p, parent, PreFork);
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

    showFinalResults(passedTest, 1);

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
