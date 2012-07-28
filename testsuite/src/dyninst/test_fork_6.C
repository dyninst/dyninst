/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: test_fork_6.C,v 1.1 2008/10/30 19:21:38 legendre Exp $
/*
 * #Name: test7_2
 * #Desc: Delete snippet in child
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"
#include "test_lib_test7.h"

#include "dyninst_comp.h"
class test_fork_6_Mutator : public DyninstMutator {
private:
  BPatch *bpatch;

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_fork_6_factory() {
  return new test_fork_6_Mutator();
}

static bool parentDone;
static bool childDone;
static bool passedTest;
static BPatch_thread *parentThread;
static BPatch_thread *childThread;
static int msgid;

/* Make sure deleting a snippet in a child process doesn't delete the
   snippet in the parent process.

   parent: snippetHandleA  = insert snippetA at pointA
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   --- fork ---
   child:  deleteSnippet( snippetHandleA' )
   --- run  ---
   parent: verify snippetHandleA still ran
*/

static void prepareTestCase2(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   static BPatchSnippetHandle *parSnippetHandle2;

   if(proc_type == Parent_p  &&  when == PreFork) {
       BPatch_image *parImage = thread->getProcess()->getImage();
      
      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "test_fork_6_func1";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	logerror("    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_2p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_2p || ((*points7_2p).size() == 0),
		 "  Unable to find entry point to \"test_fork_6_func1\".\n")) return;
      BPatch_point *point7_2p = (*points7_2p)[0];

      BPatch_variableExpr *var7_2p = 
	 parImage->findVariable("test_fork_6_global1");
      if(doError(&passedTest, (var7_2p==NULL),
		 "  Unable to locate variable test_fork_6_global1\n")) return;

      BPatch_arithExpr expr7_2p(BPatch_assign, *var7_2p,BPatch_constExpr(951));

      parSnippetHandle2 =
              thread->getProcess()->insertSnippet(expr7_2p, *point7_2p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
       BPatch_image *childImage = thread->getProcess()->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "test_fork_6_func1";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	logerror("    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_2c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(&passedTest, !points7_2c || ((*points7_2c).size() == 0),
		 "  Unable to find entry point to \"test_fork_6_func1\".\n")) return;
      BPatch_point *point7_2c = (*points7_2c)[0];

      BPatch_Vector<BPatchSnippetHandle *> childSnippets =
	 point7_2c->getCurrentSnippets();
      if(doError(&passedTest, (childSnippets.size()==0),
		 " No snippets were found at test_fork_6_func1\n")) return;

      for(unsigned i=0; i<childSnippets.size(); i++) {
          bool result = thread->getProcess()->deleteSnippet(childSnippets[i]);
	 if(result == false) {
	    logerror("  error, couldn't delete snippet\n");
	    passedTest = false;
	    return;
	 }
      }
   }
}

static void checkTestCase2(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
       if(! verifyProcMemory(thread->getProcess(), "test_fork_6_global1", 951, proc_type)) {
	 passedTest = false;
      }
   } else if(proc_type == Child_p) {
       if(! verifyProcMemory(thread->getProcess(), "test_fork_6_global1", 159, proc_type)) {
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
    prepareTestCase2(Parent_p, parent, PostFork);
    dprintf("Preparing tests on child\n");
    prepareTestCase2(Child_p,  child,  PostFork);
    dprintf("Fork handler finished (parent %p, child %p)\n", parent, child);
}

/* And verify them when they exit */
static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type) {
    dprintf("Exit func called\n");
    if (thread == parentThread) {
        dprintf("Parent exit reached, checking...\n");
        checkTestCase2(Parent_p, thread);
        parentDone = true;
        dprintf("Parent done\n");
    }
    else if (thread == childThread) {
        dprintf("Child exit reached, checking...\n");
        checkTestCase2(Child_p, thread);
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
    assert(parent->getProcess()->isStopped());

   //cerr << "ok, inserting instr\n";
   prepareTestCase2(Parent_p, parent, PreFork);
}

static bool mutatorTest(BPatch *bpatch, BPatch_thread *appThread)
{
    if ( !setupMessaging(&msgid) )
    {
       passedTest = false;
       return passedTest;
    }

    parentThread = appThread;

    initialPreparation(parentThread);
    /* ok, do the fork */;
                         parentThread->getProcess()->continueExecution();

    /* the rest of the execution occurs in postForkFunc() */
    /* Secondary test: we should not have to manually continue
       either parent or child at any point */

    while ( !parentThread->getProcess()->isTerminated() )
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
    
    while ( !childThread->getProcess()->isTerminated() )
    {
       bpatch->waitForStatusChange();
    }

    return passedTest;
}

test_results_t test_fork_6_Mutator::executeTest() {
  // Initialize global variables
  parentDone = false;
  childDone = false;
  passedTest = true;
  parentThread = NULL;
  childThread = NULL;
  msgid = -1;

  // Register callbacks
  bpatch->registerPostForkCallback(postForkFunc);
  bpatch->registerExitCallback(exitFunc);

  bool passed = mutatorTest(bpatch, appThread);

  // Remove callbacks upon test completion
  bpatch->registerPostForkCallback(NULL);
  bpatch->registerExitCallback(NULL);

  showFinalResults(passed, 2);
  if ( passed )
    return PASSED;
  else
    return FAILED;
}

// extern "C" int test7_2_mutatorMAIN(ParameterDict &param)
test_results_t test_fork_6_Mutator::setup(ParameterDict &param) {
#ifndef os_windows_test
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    return PASSED;
#else
    return SKIPPED;
#endif
}
