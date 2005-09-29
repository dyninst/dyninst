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

// $Id: test7_5.C,v 1.1 2005/09/29 20:39:46 bpellin Exp $
/*
 * #Name: test7_5
 * #Desc: Add snippets to parent & child
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

/* Add two snippets to an already existing snippet in the parent and child
   and see if all of the snippets get run.

   parent/child: globalVariable7_5 initial value = 7
   parent: snippetHandleA  = insert snippetA at pointA   += 9
   --- fork ---
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   parent: add snippetB (+= 11);  add snippetC (+= 13);
   child:  add snippetB' (+= 5);  add snippetC' (+= 3);
   --- run  ---
   parent: verify snippetA, snippetB, and snippetC ran     7+9+11+13 == 40
   child:  verify snippetA', snippetB', and snippetC' ran  7+9+5+3   == 24
*/

void prepareTestCase5(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   static BPatchSnippetHandle *parSnippetHandle5;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_5";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_5p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_5p || ((*points7_5p).size() == 0),
		 "  Unable to find entry point to \"func7_5\".\n")) return;
      BPatch_point *point7_5p = (*points7_5p)[0];

      BPatch_variableExpr *var7_5p = 
	 parImage->findVariable("globalVariable7_5");
      if(doError(&passedTest, (var7_5p==NULL),
		 "  Unable to locate variable globalVariable7_5\n")) return;

      BPatch_arithExpr expr7_5p(BPatch_plus, *var7_5p, BPatch_constExpr(9));
      BPatch_arithExpr b_expr7_5p(BPatch_assign, *var7_5p, expr7_5p);
      parSnippetHandle5 =
	 thread->insertSnippet(b_expr7_5p, *point7_5p, BPatch_callBefore);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_5";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_5p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_5p || ((*points7_5p).size() == 0),
		 "  Unable to find entry point to \"func7_5\".\n")) return;
      BPatch_point *point7_5p = (*points7_5p)[0];

      BPatch_variableExpr *var7_5p = 
	 parImage->findVariable("globalVariable7_5");
      if(doError(&passedTest, (var7_5p==NULL),
		 "  Unable to locate variable globalVariable7_5\n")) return;

      BPatch_arithExpr a_expr7_5p(BPatch_plus, *var7_5p, BPatch_constExpr(11));
      BPatch_arithExpr b_expr7_5p(BPatch_assign, *var7_5p, a_expr7_5p);
      parSnippetHandle5 =
	 thread->insertSnippet(b_expr7_5p, *point7_5p, BPatch_callBefore,
			       BPatch_lastSnippet);

      BPatch_arithExpr c_expr7_5p(BPatch_plus, *var7_5p, BPatch_constExpr(13));
      BPatch_arithExpr d_expr7_5p(BPatch_assign, *var7_5p, c_expr7_5p);
      parSnippetHandle5 =
	 thread->insertSnippet(d_expr7_5p, *point7_5p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_5";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_5c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_5c || ((*points7_5c).size() == 0),
		 "  Unable to find entry point to \"func7_5\".\n")) return;
      BPatch_point *point7_5c = (*points7_5c)[0];

      BPatch_variableExpr *var7_5c = 
	 childImage->findVariable("globalVariable7_5");
      if(doError(&passedTest, (var7_5c==NULL),
		 "  Unable to locate variable globalVariable7_5\n")) return;

      BPatch_arithExpr a_expr7_5c(BPatch_plus, *var7_5c, BPatch_constExpr(5));
      BPatch_arithExpr b_expr7_5c(BPatch_assign, *var7_5c, a_expr7_5c);
      parSnippetHandle5 =
	thread->insertSnippet(b_expr7_5c, *point7_5c, BPatch_callBefore,
			      BPatch_lastSnippet);
      BPatch_arithExpr c_expr7_5c(BPatch_plus, *var7_5c, BPatch_constExpr(3));
      BPatch_arithExpr d_expr7_5c(BPatch_assign, *var7_5c, c_expr7_5c);
      parSnippetHandle5 =
	thread->insertSnippet(d_expr7_5c, *point7_5c, BPatch_callBefore);
   }
}

void checkTestCase5(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_5", 40, proc_type)) {
	 passedTest = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_5", 24, proc_type)) {
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
    prepareTestCase5(Parent_p, parent, PostFork);
    dprintf("Preparing tests on child\n");
    prepareTestCase5(Child_p,  child,  PostFork);
    dprintf("Fork handler finished (parent %p, child %p)\n", parent, child);
}

/* And verify them when they exit */
void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type) {
    dprintf("Exit func called\n");
    if (thread == parentThread) {
        dprintf("Parent exit reached, checking...\n");
        checkTestCase5(Parent_p, thread);
        parentDone = true;
        dprintf("Parent done\n");
    }
    else if (thread == childThread) {
        dprintf("Child exit reached, checking...\n");
        checkTestCase5(Child_p, thread);
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
   prepareTestCase5(Parent_p, parent, PreFork);
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

    showFinalResults(passedTest, 5);

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
