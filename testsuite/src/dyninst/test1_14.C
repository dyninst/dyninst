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

// $Id: test1_14.C,v 1.1 2008/10/30 19:17:43 legendre Exp $
/*
 * #Name: test1_14
 * #Desc: Mutator Side - Replace Function Call
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test1_14_Mutator : public DyninstMutator {
    const char *libNameAroot;
    char libNameA[128];
public:
    test1_14_Mutator();

    virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test1_14_factory() {
  return new test1_14_Mutator();
}

#if !defined(os_windows_test)
test1_14_Mutator::test1_14_Mutator() : libNameAroot("libtestA") {}
#else
test1_14_Mutator::test1_14_Mutator() : libNameAroot("testA") {}
#endif
//
// Start Test Case #14 - mutator side (replace function call)
//
// static int mutatorTest(BPatch_thread *appAddrSpace, BPatch_image *appImage)
// {
test_results_t test1_14_Mutator::executeTest() {
    if ( replaceFunctionCalls(appAddrSpace, appImage, "test1_14_func1",
			      "test1_14_func2", "test1_14_call1", 
			      14, "replace/remove function call", 1) < 0 ) {
        return FAILED;
    }
    if ( replaceFunctionCalls(appAddrSpace, appImage, "test1_14_func1",
			      "test1_14_func3", NULL,
			      14, "replace/remove function call", 1) < 0 ) {
        return FAILED;
    }
    
    int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
    pointer_size = pointerSize(appImage);
#endif

    bool isStatic = appAddrSpace->isStaticExecutable();

    strncpy(libNameA, libNameAroot, 127);
    addLibArchExt(libNameA,127, pointer_size, isStatic);

    char libA[128];
    snprintf(libA, 128, "./%s", libNameA);
    
    if (!appAddrSpace->loadLibrary(libA)) {
        logerror("**Failed test1_14 (replace function call)\n");
        logerror("  Mutator couldn't load %s into mutatee\n", libNameA);
        return FAILED;
    }

    if ( replaceFunctionCalls(appAddrSpace, appImage, "test1_14_func1",
			      "test1_14_func4", "test1_14_call2_libA",
			      14, "replace/remove function call", 1) < 0 ) {
       return FAILED;
    }

    return PASSED;
} // test1_14_Mutator::executeTest()

