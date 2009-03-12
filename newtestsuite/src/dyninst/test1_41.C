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

// $Id: test1_41.C,v 1.1 2008/10/30 19:19:37 legendre Exp $
/*
 * #Name: test1_41
 * #Desc: Tests whether we lose line information running a mutatee twice
 * #Dep: 
 * #Arch:
 * #Notes:useAttach does not apply
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_statement.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test1_41_Mutator : public DyninstMutator {
  BPatch_exitType expectedSignal;
  int debugPrint;
  const int iterations;
  char *pathname;
  BPatch *bpatch;
  
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();

public:
  test1_41_Mutator();
};
extern "C" DLLEXPORT  TestMutator *test1_41_factory() {
  return new test1_41_Mutator();
}

test1_41_Mutator::test1_41_Mutator()
  : expectedSignal(ExitedNormally), iterations(2) {
  TestMutator();
}

// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test1_41_Mutator::executeTest() {
   unsigned int n=0;
   const char *child_argv[5];
   child_argv[n++] = pathname;
   if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
   child_argv[n++] = const_cast<char*>("-run");
   child_argv[n++] = const_cast<char*>("test1_41"); // run test41 in mutatee
   child_argv[n++] = NULL;

   int counts[iterations];

   // Run the mutatee twice, querying line info each time & store the info
   for (n = 0; n < iterations; n++) {
      dprintf("Starting \"%s\"\n", pathname);
      BPatch_thread *thread = bpatch->createProcess(pathname, child_argv,
                                                    NULL);
      if (!thread) {
         logerror("*ERROR*: unable to create handle for executable\n", n);
         logerror("**Failed** test #41 (repeated line information)\n");
         return FAILED;
      }
      dprintf("Mutatee started, pid=%d\n", n, thread->getPid());
      registerPID(thread->getProcess()->getPid()); // register for cleanup

      BPatch_image *image = thread->getImage();
      if (!image) {
         logerror("*ERROR*: unable to get image from thread\n");
         logerror("**Failed** test #41 (repeated line information)\n");
         return FAILED;
      }
      if (isMutateeFortran(image)) {
         // This shouldn't happen..
         thread->getProcess()->terminateExecution();
         logerror("Skipped test #41 (repeated line information)\n");
         return SKIPPED;
      }

      BPatch_module *module = image->findModule("test1_41_mutatee.c", true);
      if (!module) {
         module = image->findModule("solo_mutatee_boilerplate.c", true);
         if (true) {
            logerror("*ERROR*: unable to get module from image\n");
            logerror("Looking for \"test1_41_solo_me.c\" or \"solo_mutatee_boilerplate.c\". Available modules:\n");
            BPatch_Vector<BPatch_module *> *mods = image->getModules();
            char buffer[512];
            for (unsigned i = 0; i < mods->size(); i++) {
               BPatch_module *mod = (*mods)[i];
               char name[512];
               mod->getName(name, 512);
               sprintf(buffer, "\t%s\n",
                       name);
               logerror(buffer);
            }
		 }
      }

      if (!module) {
		  fprintf(stderr, "%s[%d]:  could not find module solo_mutatee_boilerplate.c\n", FILE__, __LINE__);
         // First try again for 'test1_41_solo_me.c'
         module = image->findModule("test1_41_solo_me.c", true);
         if (!module) {
            logerror("*ERROR*: unable to get module from image\n");
            logerror("Looking for \"test1_41_solo_me.c\" or \"solo_mutatee_boilerplate.c\". Available modules:\n");
            BPatch_Vector<BPatch_module *> *mods = image->getModules();
            char buffer[512];
            for (unsigned i = 0; i < mods->size(); i++) {
               BPatch_module *mod = (*mods)[i];
               char name[512];
               mod->getName(name, 512);
               sprintf(buffer, "\t%s\n",
                       name);
               logerror(buffer);
            }

            logerror("**Failed** test #41 (repeated line information)\n");

            return FAILED;
         }
      }

      char buffer[16384]; // FIXME ugly magic number; No module name should be that long..
      module->getName(buffer, sizeof(buffer));

      BPatch_Vector<BPatch_statement> statements;
      bool res = module->getStatements(statements);
      if (!res) {
         fprintf(stderr, "%s[%d]:  getStatements()\n", __FILE__, __LINE__);
         return FAILED;
      }

      counts[n] = statements.size();
      dprintf("Trial %d: found %d statements\n", n, statements.size());

      thread->getProcess()->terminateExecution();
   }

   // Make sure we got the same info each time we ran the mutatee
   int last_count = -1;
   for (int i = 0; i < iterations; i++) {
      if ((last_count >= 0) && (last_count != counts[i])) {
         logerror("*ERROR*: statement counts didn't match: %d vs. %d\n", last_count, counts[i]);
         logerror("**Failed** test #41 (repeated line information)\n");
         return FAILED;
      }
      last_count = counts[i];
   }

   logerror("Passed test #41 (repeated line information)\n");
   return PASSED;
}

// extern "C" TEST_DLL_EXPORT int test1_41_mutatorMAIN(ParameterDict &param)
test_results_t test1_41_Mutator::setup(ParameterDict &param) {
   pathname = param["pathname"]->getString();
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
   debugPrint = param["debugPrint"]->getInt();

#if defined (sparc_sun_solaris2_4_test)
   // we use some unsafe type operations in the test cases.
   bpatch->setTypeChecking(false);
#endif

   return PASSED;
}
