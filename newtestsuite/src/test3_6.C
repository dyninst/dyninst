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

// $Id: test3_6.C,v 1.3 2008/06/18 19:58:24 carl Exp $
/*
 * #Name: test3_6
 * #Desc: Create processes (via standard OS methods, not BPatch::createProcess), process events, and kill them, no instrumentation
 * #Dep: 
 * #Arch:
 * #Notes:useAttach does not apply
 */

#if !defined(os_windows)
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#define MAX_MUTATEES	32

#include "TestMutator.h"
class test3_6_Mutator : public TestMutator {
  BPatch_exitType expectedSignal;
  unsigned int Mutatees;
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test3_6_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test3_6_factory() {
  return new test3_6_Mutator();
}

test3_6_Mutator::test3_6_Mutator()
  : Mutatees(3), bpatch(NULL), pathname(NULL) {
#if defined(os_windows)
  expectedSignal = ExitedNormally;
#else
  expectedSignal = ExitedViaSignal;
#endif
}

//
// Start Test Case #1 - create processes and process events from each
//     Just let them run a while, then kill them, no instrumentation added.
//

#if !defined (os_windows)
static int forkNewMutatee(const char *filename, const char *child_argv[])
{
  int pid;
  static int pgid = 0;
  pid = fork();
  if (pid == 0) {
    // child, do exec
    dprintf("%s[%d]:  before exec in new mutatee %s, pgid = %d\n", __FILE__, __LINE__, filename, getpgid(0));

    //  sanity check, make sure that all forked processes have the same process group id
    if (!pgid) pgid = getpgid(0);
    else if (pgid != getpgid(0)) {
       logerror("%s[%d]:  Something is broken with the test -- all forked processes should belong to the same group\n",                __FILE__, __LINE__);
       // FIXME Don't call abort here.  We juse want to return failure from
       // this mutator
       abort();
    }

    execv (filename, (char * const *)child_argv);
    //  if we get here, error
    logerror("%s[%d]:  exec failed: %s\n", __FILE__, __LINE__, strerror(errno));
    // FIXME Don't call exit here.  We just want to return failure from this
    // mutator..
    exit (-1);
  }
  else if (pid < 0) {
    //  fork error, fail test
    logerror("%s[%d]:  fork failed: %s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  } else {
    // Parent; register child for cleanup
    registerPID(pid);
  }

  return pid;
}

static bool grandparentForkMutatees(int num, int *pids, const char *filename, const char *child_argv[])
{
    //  this is like fork_mutatee in test_util.C, except it guarantees that mutatees are all
    //  in the same process group.

    //  need a pipe to get grandchild pids back to grandparent
    int filedes[2];
    int result;
    pipe(filedes);

    int childpid = fork();
    if (childpid > 0) {
      //parent -- read grandchild pids
      registerPID(childpid); // Register for cleanup
      for (unsigned int i = 0; i < num; ++i) {
        result = 0;
        do {
           result = read(filedes[0], &pids[i], sizeof(int));
        } while (result == -1 && errno == EINTR);
        if (0 > result) {
           logerror("%s[%d]:  read failed %s\n", __FILE__, __LINE__, strerror(errno));
	   // FIXME Don't abort here.  We just want to return failure from this
	   // mutator
           abort();
        }
        dprintf("%s[%d]:  parent -- have new pid %d\n", __FILE__, __LINE__, pids[i]);
      }

      //  and wait for child exit
      int status;
      int waitpid_ret = waitpid(childpid, &status, 0);
      if (waitpid_ret != childpid) {
        logerror("%s[%d]:  waitpid failed: %s\n", __FILE__, __LINE__, strerror(errno));
	// FIXME Don't exit here.  We just want to return failure from this
	// mutator
        exit (0);
      }
      if (!WIFEXITED(status)) {
         logerror("%s[%d]:  not exited\n", __FILE__, __LINE__);
	 // FIXME Don't exit here.  We just want to return failure from this
	 // mutator
         exit(-1);
      }
      close(filedes[0]);
      close(filedes[1]);
      return true;
    }

    else if (childpid == 0) {
      int gchild_pid;
      //  child -- run as its own session, fork children (mutatees), and exit.
      setsid();
      for (int n=0; n<num; n++) {
        gchild_pid = forkNewMutatee(filename, child_argv);
        if (gchild_pid < 0) {
           logerror("%s[%d]:  failed to fork/exec\n", __FILE__, __LINE__);
           return false;
        }
        dprintf("%s[%d]:  forked mutatee %d\n", __FILE__, __LINE__, gchild_pid);
        //  let parent know the grandchild pid
        if (0 > write(filedes[1], &gchild_pid, sizeof(int))) {
            logerror("%s[%d]:  write failed\n", __FILE__, __LINE__);
	    // FIXME Don't abort here.  We just want to return failure from
	    // this mutator
            abort();
        }
      }
      close (filedes[0]);
      close (filedes[1]);
      // FIXME Don't exit here.  We just want to return failure from this
      // mutator
      exit(0);
   }
   else if (childpid < 0) {
     //  fork error, fail test
     close (filedes[0]);
     close (filedes[1]);
     logerror("%s[%d]:  fork failed: %s\n", __FILE__, __LINE__, strerror(errno));
     return false;
   }
   return true;
}

#endif

// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test3_6_Mutator::execute() {
#if !defined (os_windows)
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_6"); // run test1 in mutatee
    child_argv[n++] = NULL;

    int pids[Mutatees];
    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    // Start the mutatees
    if (!grandparentForkMutatees(Mutatees, pids, pathname, child_argv)) {
      logerror("%s[%d]:  failed to fork mutatees\n", __FILE__, __LINE__);
      exit(1);
    }

    P_sleep(2);
    //  Attach to them
    for (n=0; n<Mutatees; n++) {
        dprintf("Attaching \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->attachProcess(pathname, pids[n]);
        if (!appThread[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test3_6 (simultaneous multiple-process management - terminate (fork))\n");
            MopUpMutatees(n-1,appThread);
            return FAILED;
        }
        dprintf("Mutatee %d attached, pid=%d\n", n, appThread[n]->getPid());
    }

    dprintf("Letting mutatee processes run a short while (5s).\n");
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    P_sleep(5);
    dprintf("Terminating mutatee processes.\n");

    appThread[0]->getProcess(); // ???

    // And kill them
    unsigned int numTerminated=0;
    for (n=0; n<Mutatees; n++) {
        bool dead = appThread[n]->terminateExecution();
        if (!dead || !(appThread[n]->isTerminated())) {
            logerror("**Failed** test3_6 (simultaneous multiple-process management - terminate (fork))\n");
            logerror("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
#if !defined(os_aix) && !defined(os_solaris) && !defined(os_osf)
        if(appThread[n]->terminationStatus() != expectedSignal) {
            logerror("**Failed** test3_6 (simultaneous multiple-process management - terminate (fork))\n");
            logerror("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appThread[n]->getExitSignal();
        dprintf("Terminated mutatee [%d] from signal 0x%x\n", n, signalNum);
#endif
        numTerminated++;
    }

    if (numTerminated == Mutatees) {
	logerror("Passed test3_6 (simultaneous multiple-process management - terminate (fork))\n");
        return PASSED;
    }

    return FAILED;
#else // os_windows
    logerror("Skipped test3_6 (simultaneous multiple-process management - terminate (fork))\n");
    return SKIPPED;
#endif
}

// extern "C" TEST_DLL_EXPORT int test3_6_mutatorMAIN(ParameterDict &param)
test_results_t test3_6_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return PASSED;
}
