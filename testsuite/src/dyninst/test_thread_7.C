/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include <BPatch.h>
#include <BPatch_process.h>
#include <BPatch_thread.h>
#include <BPatch_image.h>
#include <BPatch_function.h>
#include <assert.h>
#include <string.h>

#include "test_lib.h"
#include "BPatch_point.h"

#include "dyninst_comp.h"

#define MAX_ARGS 32

class test_thread_7_Mutator : public DyninstMutator {
protected:
  BPatch *bpatch;
  bool create_proc;
  char *filename;
  BPatch_process *proc;
  char *args[MAX_ARGS];
  unsigned num_args;

  BPatch_process *getProcess();
  void instr_func(BPatch_function *func, BPatch_function *lvl1func);

public:
  test_thread_7_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_thread_7_factory() {
  return new test_thread_7_Mutator();
}

test_thread_7_Mutator::test_thread_7_Mutator()
  : bpatch(NULL), create_proc(true), filename(NULL), num_args(0) {
}

// static FILE *outlog = NULL;
// static FILE *errlog = NULL;


static bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

void test_thread_7_Mutator::instr_func(BPatch_function *func,
				       BPatch_function *lvl1func) {
   BPatch_Vector<BPatch_point *> *points;
   points = func->findPoint(BPatch_entry);
   for (unsigned j=0; j < points->size(); j++)
   {
      BPatch_point *point = (*points)[j];
      BPatch_Vector<BPatch_snippet *> args;
      BPatch_funcCallExpr callToLevel1(*lvl1func, args);
      BPatchSnippetHandle *hndl;
      hndl = proc->insertSnippet(callToLevel1, *point, BPatch_firstSnippet);
      // FIXME Don't think we want to assert here.  It's possible this call
      // might fail for some reason, and we're better off returning a failure
      // code then rather than just crashing the test driver
      assert(hndl);
   }
}

BPatch_process *test_thread_7_Mutator::getProcess() {
  int n = 0;
  args[n++] = filename;

  args[n++] = "-run";
  args[n++] = "test_thread_7";

  // Set up log file!
  args[n++] = "-log";
  args[n++] = const_cast<char*>(getOutputLogFilename());

  args[n] = NULL;

   BPatch_process *proc = NULL;
   if (create_proc) {
      proc = bpatch->processCreate(filename, (const char **) args);
      if(proc == NULL) {
         logerror("%s[%d]: processCreate(%s) failed\n", 
                 __FILE__, __LINE__, filename);
         return NULL;
      }
      registerPID(proc->getPid()); // Register for cleanup
   }
   else
   {
      dprintf(stderr, "%s[%d]: starting process for attach\n", __FILE__, __LINE__);
      int pid = startNewProcessForAttach(filename, (const char **) args,
                                         getOutputLog(), getErrorLog(), true);
      if (pid < 0) {
	 int errnum = errno;
	 errno = 0;
	 char *errstr = strerror(errnum);
	 logerror("%s couldn't be started: %s\n", filename,
		  errno ? "<unknown error>" : errstr);
         return NULL;
      } else if (pid > 0) {
	registerPID(pid); // Register for cleanup
      }
#if defined(os_windows_test)
      P_sleep(1);
#endif
      dprintf(stderr, "%s[%d]: started process, now attaching\n", __FILE__, __LINE__);
      proc = bpatch->processAttach(filename, pid);  
      if(proc == NULL) {
         logerror("%s[%d]: processAttach(%s, %d) failed\n", 
		  __FILE__, __LINE__, filename, pid);
         return NULL;
      }
      dprintf(stderr, "%s[%d]: attached to process\n", __FILE__, __LINE__);
      BPatch_image *appimg = proc->getImage();
      signalAttached(appimg);
   }
   return proc;
}

test_results_t test_thread_7_Mutator::executeTest() {
  memset(args, 0, sizeof (args));

   proc = getProcess();
   if (!proc) {
     return FAILED;
   }

   BPatch_image *image = proc->getImage();
   BPatch_Vector<BPatch_function *> lvl1funcs;
   image->findFunction("test_thread_7_level1", lvl1funcs);
   logerror("finding level1 function...\n");
   if (lvl1funcs.size() != 1)
   {
      logerror("[%s:%u] - Found %d level1 functions.  Expected 1\n",
              __FILE__, __LINE__, lvl1funcs.size());
      return FAILED;
   }
   BPatch_function *lvl1func = lvl1funcs[0];

   BPatch_Vector<BPatch_function *> funcs;
   image->findFunction("test_thread_7_level0", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();
   image->findFunction("test_thread_7_level1", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();
   image->findFunction("test_thread_7_level2", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();
   image->findFunction("test_thread_7_level3", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();
   logerror("found  level0-level3 functions...\n");

   proc->continueExecution();
   logerror("continued execution OK...\n");

   do {
      bpatch->waitForStatusChange();
   } while (!proc->isTerminated());
   logerror("proc terminated, getting exit code...\n");

   int exitCode = proc->getExitCode();
   if (exitCode)
   {
       logstatus("*** Failed test_thread_7 (Multithreaded tramp guards)\n");
       return FAILED;
   }
   else
   {
       logstatus("Passed test_thread_7 (Multithreaded tramp guards)\n");
       logstatus("All tests passed.\n");
   }
   return PASSED;
}

//extern "C" TEST_DLL_EXPORT int test14_1_mutatorMAIN(ParameterDict &param)
test_results_t test_thread_7_Mutator::setup(ParameterDict &param) {
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
   filename = param["pathname"]->getString();

   if ( param["useAttach"]->getInt() != 0 )
   {
      create_proc = false;
   }

   return PASSED;
}
