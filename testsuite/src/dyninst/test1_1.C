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

// $Id: test1_1.C,v 1.1 2008/10/30 19:17:22 legendre Exp $
/*
 *
 * #Name: test1_1
 * #Desc: Zero Argument Function Call
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test1_1_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest(); // override
};

// Factory function.
extern "C" DLLEXPORT TestMutator* test1_1_factory()
{
  return new test1_1_Mutator();
}

//
// Start Test Case #1 - (zero arg function call)
//
test_results_t test1_1_Mutator::executeTest() {
  const char *funcName = "test1_1_func1_1";
  const char* testName = "zero arg function call";
  int testNo = 1;

  // Find the entry point to the procedure "func1_1"
  
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(funcName, found_funcs))
      || !found_funcs.size()) {
    logerror("    Unable to find function %s\n", funcName);
    return FAILED;
  }
  
  if (1 < found_funcs.size()) {
    logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), funcName);
  }

  BPatch_Vector<BPatch_point *> *point1_1 = found_funcs[0]->findPoint(BPatch_entry);


  if (!point1_1 || ((*point1_1).size() == 0)) {
    logerror("**Failed** test #%d (%s)\n", testNo,testName);
    logerror("    Unable to find entry point to \"%s.\"\n", funcName);
    return FAILED;
  }

  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "test1_1_call1_1";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #%d (%s)\n", testNo, testName);
    logerror("    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *call1_func = bpfv[0];
  
  BPatch_Vector<BPatch_snippet *> call1_args;
  BPatch_funcCallExpr call1Expr(*call1_func, call1_args);

  checkCost(call1Expr);
    appAddrSpace->insertSnippet(call1Expr, *point1_1);
  dprintf("Inserted snippet\n");

  return PASSED;
} // test1_1_Mutator::executeTest()
