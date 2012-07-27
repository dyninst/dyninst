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

// $Id: test1_41.C,v 1.1 2008/10/30 19:19:37 legendre Exp $
/*
 * #Name: test1_41
 * #Desc: Tests whether we lose line information running a mutatee twice
 * #Dep: 
 * #Arch:
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
      BPatch_process *proc = bpatch->processCreate(pathname, child_argv,
                                                    NULL);
      if (!proc) {
         logerror("*ERROR*: unable to create handle for executable\n", n);
         logerror("**Failed** test #41 (repeated line information)\n");
         return FAILED;
      }
      dprintf("Mutatee started, pid=%d\n", n, proc->getPid());

      BPatch_image *image = proc->getImage();
      if (!image) {
         logerror("*ERROR*: unable to get image from thread\n");
         logerror("**Failed** test #41 (repeated line information)\n");
         return FAILED;
      }
      if (isMutateeFortran(image)) {
         // This shouldn't happen..
         proc->terminateExecution();
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

      proc->terminateExecution();
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

   return PASSED;
}
