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

// $Id: test3_6.C,v 1.1 2008/10/30 19:20:43 legendre Exp $
/*
 * #Name: test3_6
 * #Desc: Create processes (via standard OS methods, not BPatch::createProcess), process events, and kill them, no instrumentation
 * #Dep: 
 * #Arch:
 */

#if !defined(os_windows_test)
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

#include "dyninst_comp.h"
class test3_6_Mutator : public DyninstMutator {
  BPatch_exitType expectedSignal;
  unsigned int Mutatees;
  std::vector<int> pids;
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test3_6_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
  void cleanup();
  
};
extern "C" DLLEXPORT  TestMutator *test3_6_factory() {
  return new test3_6_Mutator();
}

test3_6_Mutator::test3_6_Mutator()
  : Mutatees(3), bpatch(NULL), pathname(NULL) {
#if defined(os_windows_test)
  expectedSignal = ExitedNormally;
#else
  expectedSignal = ExitedViaSignal;
#endif
}

//
// Start Test Case #1 - create processes and process events from each
//     Just let them run a while, then kill them, no instrumentation added.
//

#if !defined (os_windows_test)
static int forkNewMutatee(const char *filename, const char *child_argv[])
{
  int pid;
  static int pgid = 0;
  pid = fork();
  if (pid == 0) {
    // child, do exec
    dprintf("%s[%d]:  before exec in new mutatee %s, pid = %d\n", __FILE__, __LINE__, filename, getpid());
    execv (filename, (char * const *)child_argv);
    //  if we get here, error
    logerror("%s[%d]:  exec failed: %s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }
  else if (pid < 0) {
    //  fork error, fail test
    logerror("%s[%d]:  fork failed: %s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  } 

  return pid;
}
#endif

void test3_6_Mutator::cleanup()
{
#if !defined(os_windows_test)
  for(std::vector<int>::iterator i = pids.begin();
      i != pids.end();
      ++i)
  {
    int result = kill(*i, SIGKILL);
    if(!result) fprintf(stderr, "Failed to kill %d: %s\n", *i, strerror(errno));
    
  }
#endif  
}


// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test3_6_Mutator::executeTest() {
#if !defined (os_windows_test)
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_6"); // run test1 in mutatee
    child_argv[n++] = NULL;

    BPatch_process *appProc[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) {
      appProc[n]=NULL;
      int pid = forkNewMutatee(pathname, child_argv);
      if(pid < 0) 
      {
	cleanup();
	logerror("failed to fork mutatees\n");
	return FAILED;
      }
      pids.push_back(pid);
    }
    



    P_sleep(2);
    //  Attach to them
    for (n=0; n<Mutatees; n++) {
        dprintf("Attaching \"%s\" %d/%d\n", pathname, n, Mutatees);
        appProc[n] = bpatch->processAttach(pathname, pids[n]);
        if (!appProc[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test3_6 (simultaneous multiple-process management - terminate (fork))\n");
	    cleanup();
            return FAILED;
        }
        dprintf("Mutatee %d attached, pid=%d\n", n, appProc[n]->getPid());
    }

    dprintf("Letting mutatee processes run a short while (5s).\n");
    for (n=0; n<Mutatees; n++) appProc[n]->continueExecution();

    P_sleep(5);
    dprintf("Terminating mutatee processes.\n");

    // And kill them
    unsigned int numTerminated=0;
    for (n=0; n<Mutatees; n++) {
        bool dead = appProc[n]->terminateExecution();
        if (!dead || !(appProc[n]->isTerminated())) {
            logerror("**Failed** test3_6 (simultaneous multiple-process management - terminate (fork))\n");
            logerror("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
#if !defined(os_aix_test)
        if(appProc[n]->terminationStatus() != expectedSignal) {
            logerror("**Failed** test3_6 (simultaneous multiple-process management - terminate (fork))\n");
            logerror("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appProc[n]->getExitSignal();
        dprintf("Terminated mutatee [%d] from signal 0x%x\n", n, signalNum);
#endif
        numTerminated++;
    }

    if (numTerminated == Mutatees) {
	logerror("Passed test3_6 (simultaneous multiple-process management - terminate (fork))\n");
        return PASSED;
    }
    cleanup();
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

    
    return PASSED;
}
