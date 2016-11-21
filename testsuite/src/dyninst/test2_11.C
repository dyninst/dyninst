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
#include "BPatch_point.h"

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

   return SKIPPED;
}
