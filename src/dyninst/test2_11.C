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

// $Id: test2_11.C,v 1.1 2008/10/30 19:20:12 legendre Exp $
/*
 * #Name: test2_11
 * #Desc: getDisplacedInstructions
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test2_11_Mutator : public DyninstMutator {
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_11_factory() {
  return new test2_11_Mutator();
}

//
// Test 11 - getDisplacedInstructions
//	This function tests the getDisplacedInstructions instructions methods.
//	Currently this tests is only enabled on AIX platforms.
//
// That's not true at all..  As far as I can tell it's enabled for all
// platforms.  It also doesn't seem to be checking for correct behavior really,
// just that *something* was returned as "displaced instructions".

// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test2_11_Mutator::executeTest() {

  BPatch_Vector<BPatch_function *> found_funcs;
  char *funcname = "test2_11_func1";
    if ((NULL == appImage->findFunction(funcname, found_funcs, 1)) || !found_funcs.size()) {
      logerror("    Unable to find function %s\n", funcname);
      return FAILED;
    }

    if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), funcname);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_entry);

    if (points == NULL) {
	logerror("**Failed** test #11 (getDisplacedInstructions)\n");
	logerror("    unable to locate function \"%s\".\n", funcname);
        return FAILED;
    }

    char buf[128];
    memset(buf, 128, 0);
    int nbytes = (*points)[0]->getDisplacedInstructions(128, buf);
    if (nbytes < 0 || nbytes > 128) {
	logerror("**Failed** test #11 (getDisplacedInstructions)\n");
	logerror("    getDisplacedInstructions returned a strange number of bytes (%d)\n", nbytes);
        return FAILED;
    }
    int i;
    for (i = 0; i < nbytes; i++) {
	if (buf[i] != 0) break;
    }
    if (i == nbytes) {
	logerror("**Failed** test #11 (getDisplacedInstructions)\n");
	logerror("    getDisplacedInstructions doesn't seem to have returned any instructions\n");
        return FAILED;
    }
    // Set the variable test2_11_passed in the mutatee
    BPatch_variableExpr *passed_expr =
        appImage->findVariable("test2_11_passed");
    if (passed_expr == NULL) {
      logerror("**Failed** test #11 (getDisplacedInstructions)\n");
      logerror("    Unable to locate test2_11_passed\n");
      return FAILED;
    } else {
      int pvalue = 1;
      passed_expr->writeValue(&pvalue);
      logerror("Passed test #11 (getDisplacedInstructions)\n");
      return PASSED;
    }
}
