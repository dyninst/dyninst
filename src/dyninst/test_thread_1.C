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

// $Id: test_thread_1.C,v 1.1 2008/10/30 19:22:26 legendre Exp $
/*
 * #Name: test12_1
 * #Desc: rtlib spinlocks
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test12.h"

#include "dyninst_comp.h"
class test_thread_1_Mutator : public DyninstMutator {
public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_thread_1_factory() {
  return new test_thread_1_Mutator();
}

#define TESTNO 1
#define TESTNAME "test_thread_1"
#define TESTDESC "rtlib spinlocks"

static int mutateeXLC;
extern BPatch *bpatch;

static const char *expected_fnames[] = {"call1_1","call1_2","call1_3","call1_4"};
static int test1done = 0;
static int test1err = 0;
template class BPatch_Vector<void *>;
static BPatch_Vector<void *> test1handles;
static BPatch_Vector<BPatch_point *> dyncalls;

// static int mutatorTest(BPatch_thread *appT, BPatch_image *appImage)
test_results_t test_thread_1_Mutator::executeTest() {
  //  Just continue the process and wait for the (mutatee side) test to complete
  BPatch_process *proc = appThread->getProcess();
  int childpid = proc->getPid();
  dprintf("%s[%d]:  mutatee process: %d\n", __FILE__, __LINE__, childpid);

  proc->continueExecution();

  while( !proc->isTerminated() ) {
      if( !bpatch->waitForStatusChange() ) {
          dprintf("%s[%d]: failed to wait for events\n",
                  __FILE__, __LINE__);
          return FAILED;
      }
  }

     if (proc->isTerminated()) {
     switch(proc->terminationStatus()) {
       case ExitedNormally:
         {
          int code = proc->getExitCode();
          dprintf("%s[%d]:  exited normally with code %d\n",
                  __FILE__, __LINE__, code);
          if (code != 0)
	    return FAILED;
          break;
         }
       case ExitedViaSignal:
         {
          int code = proc->getExitSignal();
          dprintf("%s[%d]:  exited with signal %d\n",
                  __FILE__, __LINE__, code);
          return FAILED;
          break;
         }
       case NoExit:
       default:
          dprintf("%s[%d]:  did not exit ???\n",
                  __FILE__, __LINE__);
          return FAILED;
          break;
     };
   }

  PASS_MES(TESTNAME, TESTDESC);
  return PASSED;
}
