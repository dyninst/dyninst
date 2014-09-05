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

// $Id: test4_4.C,v 1.1 2008/10/30 19:20:54 legendre Exp $
/*
 * #Name: test4_4
 * #Desc: Fork and Exec Callback
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0_test,alpha_dec_osf4_0_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test4_4_Mutator : public DyninstMutator {
  const unsigned int MAX_TEST;
  BPatch *bpatch;
  char *pathname;
  int debugPrint;

public:
  test4_4_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
  virtual test_results_t mutatorTest();
};
extern "C" DLLEXPORT  TestMutator *test4_4_factory() {
  return new test4_4_Mutator();
}

test4_4_Mutator::test4_4_Mutator()
  : MAX_TEST(4), bpatch(NULL), pathname(NULL), debugPrint(0) {
}

static bool passedTest;
static int threadCount;
static BPatch_process *mythreads[25];

static BPatch_thread *test4Child;
static BPatch_thread *test4Parent;

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

       // insert code into parent
       appImage = parent->getProcess()->getImage();
       assert(appImage);

       const char *fn5 = "test4_4_func3";
       if (NULL == appImage->findFunction(fn5, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n", fn5);
	 // FIXME I don't think this should call exit()
	 exit(1);
       }

       BPatch_function *func3_parent = bpfv[0];
       BPatch_funcCallExpr callExpr2(*func3_parent, nullArgs);

       bpfv.clear();
       const char *fn6 = "test4_4_func2";
       if (NULL == appImage->findFunction(fn6, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn6);
	 exit(1);
       }

       BPatch_function *func2_parent = bpfv[0];
       BPatch_Vector<BPatch_point *> *point2 = func2_parent->findPoint(BPatch_exit);
       assert(point2);
       parent->getProcess()->insertSnippet(callExpr2, *point2);

       // code goes into child after in-exec in this test.

       test4Child = child;
}

static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type)
{
  dprintf("exitFunc called\n");
  bool failedTest = false;
    // Read out the values of the variables.
  int exitCode = thread->getProcess()->getExitCode();

  assert(thread->getProcess()->terminationStatus() == exit_type);
    // Read out the values of the variables.
    // FIXME I'm not happy about this static variable.  Make sure it is working
    // right, and maybe try to figure out a different way to do this.
    static int exited = 0;
    exited++;
    if (exit_type == ExitedViaSignal) {
        logerror("Failed test #4 (fork callback)\n");
        logerror("    process exited via signal %d\n",
                 thread->getProcess()->getExitSignal());
        failedTest = true;            
    } else if ((thread->getProcess()->getPid() & 0xff) != exitCode) {
        logerror("Failed test #4 (fork callback)\n");
        logerror("    exit code was not equal to pid\n");
        failedTest = true;
    } else if (test4Parent == thread) {
        dprintf("test #4, pid %d exited\n", exitCode);
        if (!verifyChildMemory(test4Parent->getProcess(),"test4_4_global1",4000002)){
            failedTest = true;
        }
    } else if (test4Child == thread) {
        dprintf("test #4, pid %d exited\n", exitCode);
        if (!verifyChildMemory(test4Child->getProcess(), "test4_4_global1", 4000003)) {
            failedTest = true;
        }
    } else {
        // exit from unknown thread
        logerror("Failed test #4 (fork callback)\n");
        logerror("    exit from unknown pid = %d\n", exitCode);
        failedTest = true;
    }
    // See if all the processes are done
    if (exited == 2) {
        if (!failedTest) {
            logerror("Passed test #4 (fork & exec)\n");
            passedTest = true;
        } else {
            logerror("Failed test #4 (fork & exec)\n");
        }
	exited = 0;
    }
}

static void execFunc(BPatch_thread *thread)
{
  BPatch_Vector<BPatch_function *> bpfv;
  dprintf("in exec callback for %d\n", thread->getProcess()->getPid());

        // insert code into child
	BPatch_Vector<BPatch_snippet *> nullArgs;
        BPatch_image *appImage = thread->getProcess()->getImage();
        assert(appImage);

   const char *fn3 = "test4_4_func4";
	if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  logerror("    Unable to find function %s\n",fn3);
	  // FIXME I don't like the call to exit() here
	  exit(1);
	}

	BPatch_function *func4_child = bpfv[0];
	BPatch_funcCallExpr callExpr1(*func4_child, nullArgs);
	
	bpfv.clear();
	const char *fn4 = "test4_4_func2";
	if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  logerror("    Unable to find function %s\n",fn4);
	  // FIXME I don't like the call to exit() here
	  exit(1);
	}

	BPatch_function *func2_child = bpfv[0];
	BPatch_Vector<BPatch_point *> *point1 = func2_child->findPoint(BPatch_exit);

	assert(point1);
        thread->getProcess()->insertSnippet(callExpr1, *point1);
}

test_results_t test4_4_Mutator::mutatorTest() {
#if defined(i386_unknown_nt4_0_test) \
 || defined(alpha_dec_osf4_0_test)
    logerror("Skipping test #4 (fork & exec)\n");
    logerror("    not implemented on this platform\n");
    return SKIPPED;
#else

    int n = 0;
    const char *child_argv[MAX_TEST+7];

    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test4_4");
    child_argv[n] = NULL;

    // Start the mutatee
    logerror("Starting \"%s\"\n", pathname);

    appProc = bpatch->processCreate(pathname, child_argv);
    if (appProc == NULL) {
	logerror("Unable to run test program: %s.\n", pathname);
        return FAILED;
    }

    contAndWaitForAllProcs(bpatch, appProc, mythreads, &threadCount);

    if ( !passedTest )
    {
        logerror("**Failed** test #4 (fork and exec callback)\n");
        logerror("    fork a exec callback not executed\n");
        return FAILED;
    }
    return PASSED;
#endif
}

test_results_t test4_4_Mutator::executeTest() {
  passedTest = false;
  threadCount = 0;
  test4Child = test4Parent = NULL;
  test_results_t rv;

  // Register the proper callbacks for this test
  bpatch->registerPreForkCallback(forkFunc);
  bpatch->registerPostForkCallback(forkFunc);
  bpatch->registerExecCallback(execFunc);
  bpatch->registerExitCallback(exitFunc);

  rv = mutatorTest();

  // Remove callbacks upon test completion
  bpatch->registerPreForkCallback(NULL);
  bpatch->registerPostForkCallback(NULL);
  bpatch->registerExecCallback(NULL);
  bpatch->registerExitCallback(NULL);

  return rv;
}

// extern "C" DLLEXPORT TEST_DLL_EXPORT int test4_4_mutatorMAIN(ParameterDict &param)
test_results_t test4_4_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    return PASSED;
}
