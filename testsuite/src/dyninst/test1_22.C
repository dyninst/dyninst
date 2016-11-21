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

// $Id: test1_22.C,v 1.1 2008/10/30 19:18:25 legendre Exp $
/*
 * #Name: test1_22
 * #Desc: Mutator Side - Replace Function
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,i386_unknown_linux2_0_test,alpha_dec_osf4_0_test,ia64_unknown_linux2_4_test,x86_64_unknown_linux2_4_test
 * #Notes: This test uses libNameA/B magic like test1_21
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_object.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_22_Mutator : public DyninstMutator {
	const char *libNameAroot;
	const char *libNameBroot;
	char libNameA[128], libNameB[128];

	public:
	test1_22_Mutator();
	virtual test_results_t executeTest();
	virtual test_results_t mutatorTest22();
};

test1_22_Mutator::test1_22_Mutator() :
#if defined(os_windows_test)
    libNameAroot("testA"),
	libNameBroot("testB") 
#else
    libNameAroot("libtestA"),
	libNameBroot("libtestB") 
#endif
{
}

extern "C" DLLEXPORT  TestMutator *test1_22_factory() 
{
	return new test1_22_Mutator();
}

//
// Start Test Case #22 - mutator side (replace function)
//
// There is no corresponding failure (test2) testing because the only
// invalid input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.

test_results_t test1_22_Mutator::mutatorTest22() 
{
	char errbuf[1024]; errbuf[0] = '\0';
	BPatch_object *modA = NULL;
	BPatch_object *modB = NULL;

	// Assume that a prior test (mutatorTest21) has loaded the
	// libraries libtestA.so and libtestB.so into the mutator.

	BPatch_Vector<BPatch_object *> mods; 
	appImage->getObjects(mods);

	if(mods.empty())
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't find shlib in mutatee\n");
		return FAILED;
	}

	// Lookup the libtestA.so and libtestB.so modules
	for (unsigned int i = 0; i < mods.size() && !(modA && modB); i++) 
	{
		BPatch_object *m = mods[i];
		// module names sometimes have "_module" appended
                if( m->name().find(libNameAroot) != std::string::npos )
			modA = m;
		else if ( m->name().find(libNameBroot) != std::string::npos )
			modB = m;
	}

	if (! modA || ! modB) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't find dynamically loaded modules\n");
		return FAILED;
	}

	//  Mutatee function replacement scheme:

	//  function      module     replaced    global
	//                         or called?   

	//  call22_1       a.out     replaced         1       global is the index
	//  call22_2       a.out       called         1       of the global variable
	//  call22_3       a.out     replaced         2       in test1.mutatee updated
	//  call22_4    libtestA       called         2       by the function
	//  call22_5A   libtestA     replaced         3
	//  call22_5B   libtestB       called         3
	//  call22_6    libtestA     replaced         4
	//  call22_7       a.out       called         4

	// Both of each pair of functions (e.g., call22_1, call22_2)
	// increments a global variable.  The mutatee will test that the
	// variable has been updated only be the "called" function.

	// Replace an a.out with another a.out function

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_22_call1";
	const char *fn2 = "test1_22_call2";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call22_1func = bpfv[0];

	bpfv.clear();
	if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("    Unable to find function %s\n", fn2);
		return FAILED;
	}

	BPatch_function *call22_2func = bpfv[0];

	if (! appAddrSpace->replaceFunction(*call22_1func, *call22_2func)) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't replaceFunction (a.out -> a.out)\n");
		return FAILED;
	}


	// Replace an a.out function with a shlib function
	bpfv.clear();
	const char *fn3 = "test1_22_call3";
	if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
			|| NULL == bpfv[0]){
		logerror("**Failed test #22 (replace function)\n");
		logerror("    Unable to find function %s\n", fn3);
		return FAILED;
	}

	BPatch_function *call22_3func = bpfv[0];

	BPatch_Vector<BPatch_function *> bpmv;
	if (NULL == modA->findFunction("call22_4", bpmv) || !bpmv.size()) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't find functions in mutatee\n");
		return FAILED;
	}
	BPatch_function *call22_4func = bpmv[0];

	if (! appAddrSpace->replaceFunction(*call22_3func, *call22_4func)) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't replaceFunction (a.out -> shlib)\n");
		return FAILED;
	}

	// Replace a shlib function with a shlib function
	bpmv.clear();

	if (NULL == modA->findFunction("call22_5a", bpmv) || !bpmv.size()) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't find functions in mutatee\n");
		return FAILED;
	}

	BPatch_function *call22_5Afunc = bpmv[0];

	bpmv.clear();

	if (NULL == modB->findFunction("call22_5b", bpmv) || !bpmv.size()) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't find functions in mutatee\n");
		return FAILED;
	}

	BPatch_function *call22_5Bfunc = bpmv[0];

	if (! appAddrSpace->replaceFunction(*call22_5Afunc, *call22_5Bfunc)) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't replaceFunction (shlib -> shlib)\n");
	}

	// Replace a shlib function with an a.out function
	bpmv.clear();

	if (NULL == modA->findFunction("call22_6", bpmv) || !bpmv.size()) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't find functions in mutatee\n");
		return FAILED;
	}

	BPatch_function *call22_6func = bpmv[0];

	bpfv.clear();
	const char *fn4 = "test1_22_call7";

	if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("    Unable to find function %s\n", fn4);
		return FAILED;
	}

	BPatch_function *call22_7func = bpfv[0];

	if (! appAddrSpace->replaceFunction(*call22_6func, *call22_7func)) 
	{
		logerror("**Failed test #22 (replace function)\n");
		logerror("  Mutator couldn't replaceFunction (shlib -> a.out)\n");
		return FAILED;
	}
	return PASSED;
}


// Wrapper to call readyTest

test_results_t test1_22_Mutator::executeTest() 
{
	int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
	pointer_size = pointerSize(appImage);
#endif
        bool isStatic = appAddrSpace->isStaticExecutable();

	strncpy(libNameA, libNameAroot, 127);
	addLibArchExt(libNameA,127, pointer_size, isStatic);
	strncpy(libNameB, libNameBroot, 127);
	addLibArchExt(libNameB,127, pointer_size, isStatic);

	char libA[128], libB[128];
	snprintf(libA, 128, "%s", libNameA);
	snprintf(libB, 128, "%s", libNameB);

	if (! appAddrSpace->loadLibrary(libA)) 
	{
		logerror("**Failed test1_22 (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libA);
		return FAILED;
	}

	if (! appAddrSpace->loadLibrary(libB)) 
	{
		logerror("**Failed test1_22 (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libB);
		return FAILED;
	}

	return mutatorTest22();
} // test1_22_Mutator::executeTest()

