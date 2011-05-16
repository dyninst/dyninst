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

// $Id: test_sparc_4.C,v 1.1 2008/10/30 19:22:16 legendre Exp $
/*
 * #Name: test10_4
 * #Desc: ?
 * #Dep: 
 * #Arch: ?
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test_sparc_4_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" TEST_DLL_EXPORT TestMutator *test_sparc_4_factory() {
  return new test_sparc_4_Mutator();
}

//
// Start Test Case #4 
//
//static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test_sparc_4_Mutator::executeTest() {
  if (instrumentToCallZeroArg(appThread, appImage, "test_sparc_4_func",
			      "test_sparc_4_call", 4, "test_sparc_4") != 0) {
    return FAILED;
  } else {
    return PASSED;
  }
}
