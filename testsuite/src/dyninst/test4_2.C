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

// $Id: test4_2.C,v 1.1 2008/10/30 19:20:49 legendre Exp $
/*char *
 * #Name: test4_2
 * #Desc: Fork Callback
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0_test,alpha_dec_osf4_0_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test4_2_Mutator : public DyninstMutator {
  const unsigned int MAX_TEST;
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test4_2_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
  test_results_t mutatorTest();
};
extern "C" DLLEXPORT TestMutator *test4_2_factory() {
  return new test4_2_Mutator();
}

test4_2_Mutator::test4_2_Mutator()
  : MAX_TEST(4), bpatch(NULL), pathname(NULL) {
}

static bool passedTest;
static int threadCount;
static BPatch_process *mythreads[25];
static BPatch_thread *test2Child;
static BPatch_thread *test2Parent;
static int exited;

static void forkFunc(BPatch_thread *parent, BPatch_thread *child)
{
  dprintf("forkFunc called with parent %p, child %p\n", parent, child);
    BPatch_image *appImage;
    BPatch_Vector<BPatch_function *> bpfv;
    BPatch_Vector<BPatch_snippet *> nullArgs;

    if (child) mythreads[threadCount++] = child->getProcess();

    if (!child) {
        dprintf("in prefork for %d\n", parent->getProcess()->getPid());
    } else {
        dprintf("in fork of %d to %d\n", parent->getProcess()->getPid(), child->getProcess()->getPid());
    }

    if (!child) return;	// skip prefork case

    // Make a race condition always show up -- we don't run
    // until the processes have had a chance.
#if !defined(os_windows_test)
    sleep(1);
#endif
    // That'll make erroneous continues break...

    // insert code into parent
    appImage = parent->getProcess()->getImage();
    assert(appImage);

    const char *fn = "test4_2_func3";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn);
         exit(1);
    }

    BPatch_function *func3_parent = bpfv[0];
    BPatch_funcCallExpr callExpr2(*func3_parent, nullArgs);
 
    bpfv.clear();
    const char *fn2 = "test4_2_func2";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn2);
         exit(1);
    }

    BPatch_function *func2_parent = bpfv[0];
    BPatch_Vector<BPatch_point *> *point2 = func2_parent->findPoint(BPatch_exit);
    assert(point2);

    parent->getProcess()->insertSnippet(callExpr2, *point2);

    dprintf("MUTATEE:  after insert in fork of %d to %d\n", parent->getProcess()->getPid(), child->getProcess()->getPid());
    // insert different code into child
    appImage = child->getProcess()->getImage();
    assert(appImage);

    bpfv.clear();
    const char *fn3 = "test4_2_func4";
    if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn3);
         exit(1);
    }

    BPatch_function *func4_child = bpfv[0];
    BPatch_funcCallExpr callExpr1(*func4_child, nullArgs);

    bpfv.clear();
    const char *fn4 = "test4_2_func2";
    if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn4);
         exit(1);
    }

    BPatch_function *func2_child = bpfv[0];
    BPatch_Vector<BPatch_point *> *point1 = func2_child->findPoint(BPatch_exit);
    assert(point1);

    child->getProcess()->insertSnippet(callExpr1, *point1);

    dprintf("MUTATEE:  after insert2 in fork of %d to %d\n", parent->getProcess()->getPid(), child->getProcess()->getPid());
    test2Child = child;
    test2Parent = parent;
}

static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type)
{
  dprintf("exitFunc called\n");
  bool failedTest = false;
    // Read out the values of the variables.
  int exitCode = thread->getProcess()->getExitCode();

  assert(thread->getProcess()->terminationStatus() == exit_type);
    // Read out the values of the variables.
    exited++;
    if(exit_type == ExitedViaSignal) {
        logerror("Failed test #2 (fork callback)\n");
        logerror("    a process terminated via signal %d\n",
                 thread->getProcess()->getExitSignal());
        exited = 0;
    } else if ((thread->getProcess()->getPid() & 0xFF) != exitCode) {
        logerror("Failed test #2 (fork callback)\n");
        logerror("    exit code was not equal to pid (%d != %d)\n",
                 (thread->getProcess()->getPid() & 0xFF), exitCode);
        exited = 0;
    } else {
        dprintf("test #2, pid %d exited\n", exitCode);
        if ((test2Parent == thread) &&
             !verifyChildMemory(test2Parent->getProcess(), "test4_2_global1", 2000002)) {
            failedTest = true;
        }
        if ((test2Child == thread) &&
             !verifyChildMemory(test2Child->getProcess(), "test4_2_global1", 2000003)) {
            failedTest = true;
        }

        // See if all the processes are done
        if (exited == 2) {
            if (!failedTest) {
                logerror("Passed test #2 (fork callback)\n");
                passedTest = true;
            } else {
                logerror("Failed test #2 (fork callback)\n");
            }
	    // exited = 0; // For the next time through..
        }
    }
}

static void execFunc(BPatch_thread *thread)
{
    logerror("**Failed Test #2\n");
    logerror("    execCallback invoked, but exec was not called!\n");
}

test_results_t test4_2_Mutator::mutatorTest() {
#if defined(i386_unknown_nt4_0_test)
    logerror("Skipping test #2 (fork callback)\n");
    logerror("    not implemented on this platform\n");
    return SKIPPED;
#else

    int n = 0;
    const char *child_argv[MAX_TEST+7];

    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test4_2");
    /* TODO I'd like to automate this part, or wrap it somehow.. */
    child_argv[n] = NULL;

    // Start the mutatee
    logerror("Starting \"%s\"\n", pathname);

    appProc = bpatch->processCreate(pathname, child_argv,NULL);
    dprintf("Process %p created", appProc);
    if (appProc == NULL) {
       logerror("Unable to run test program.\n");
        return FAILED;
    }
    contAndWaitForAllProcs(bpatch, appProc, mythreads, &threadCount);

    if ( !passedTest )
    {
        logerror("**Failed** test #2 (fork callback)\n");
	// FIXME This error message is misleading
        logerror("    fork callback not executed\n");
        return FAILED;
    }
    return PASSED;

#endif
}

test_results_t test4_2_Mutator::executeTest() {
  passedTest = false;
  threadCount = 0;
  exited = 0;

  // Register the proper callbacks for this test
  bpatch->registerPreForkCallback(forkFunc);
  bpatch->registerPostForkCallback(forkFunc);
  bpatch->registerExecCallback(execFunc);
  bpatch->registerExitCallback(exitFunc);

  test_results_t rv = mutatorTest();

  // Remove callbacks upon test completion
  bpatch->registerPreForkCallback(NULL);
  bpatch->registerPostForkCallback(NULL);
  bpatch->registerExecCallback(NULL);
  bpatch->registerExitCallback(NULL);

  return rv;
}

// extern "C" TEST_DLL_EXPORT int test4_2_mutatorMAIN(ParameterDict &param)
test_results_t test4_2_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    return PASSED;
}
