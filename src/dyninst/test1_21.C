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

// $Id: test1_21.C,v 1.1 2008/10/30 19:18:23 legendre Exp $
/*
 * #Name: test1_21
 * #Desc: findFunction in module
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,alpha_dec_osf4_0_test,i386_unknown_solaris2_5_test,i386_unknown_linux2_0_test,ia64_unknown_linux2_4_test,mips_sgi_irix6_4_test,rs6000_ibm_aix4_1_test,x86_64_unknown_linux2_4_test
 * #Notes: This test uses some special magic for libNameA and libNameB that should probably be altered
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_object.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test1_21_Mutator : public DyninstMutator {
	const char *libNameAroot;
	const char *libNameBroot;
	char libNameA[128], libNameB[128];

	public:
	test1_21_Mutator();
	virtual test_results_t executeTest();
	test_results_t mutatorTest21();
};

test1_21_Mutator::test1_21_Mutator() : 
#if defined(os_windows_test)
    libNameAroot("testA"),
	libNameBroot("testB") 
#else
    libNameAroot("libtestA"),
	libNameBroot("libtestB") 
#endif
{}

extern "C" DLLEXPORT  TestMutator *test1_21_factory() 
{
	return new test1_21_Mutator();
}

//
// Start Test Case #21 - mutator side (findFunction in module)
//
// There is no corresponding failure (test2) testing because the only
// bad input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.

test_results_t test1_21_Mutator::mutatorTest21() 
{
#if defined(os_aix_test) \
	|| defined(os_linux_test) \
	|| defined(os_windows_test) \
        || defined(os_freebsd_test)

	// Lookup the libtestA.so and libtestB.so modules that we've just loaded

	BPatch_object *modA = NULL;
	BPatch_object *modB = NULL;
	BPatch_Vector<BPatch_object *> mods;
	appImage->getObjects(mods);

	if (mods.empty()) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  Mutator couldn't search modules of mutatee\n");
		return FAILED;
	}

	for (unsigned int i = 0; i < mods.size() && !(modA && modB); i++) 
	{
		BPatch_object *m = mods[i];

		// module names sometimes have "_module" appended
                if( m->name().find(libNameAroot) != std::string::npos )
			modA = m;
		else if ( m->name().find(libNameBroot) != std::string::npos ) 
			modB = m;
	}

	if (! modA || ! modB ) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  Mutator couldn't find shlib in mutatee\n");
		flushErrorLog();
		return FAILED;
	}

	// Find the function CALL21_1 in each of the modules
	BPatch_Vector<BPatch_function *> bpmv;

	if (NULL == modA->findFunction("call21_1", bpmv, false, false, true) || !bpmv.size()) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  %s[%d]: Mutator couldn't find a function in %s\n", 
				__FILE__, __LINE__, libNameA);
		return FAILED;
	}

	BPatch_function *funcA = bpmv[0];
	bpmv.clear();

	if (NULL == modB->findFunction("call21_1", bpmv, false, false, true) || !bpmv.size()) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  %s[%d]: Mutator couldn't find a function in %s\n", 
				__FILE__, __LINE__, libNameB);
		return FAILED;
	} 

	BPatch_function *funcB = bpmv[0];

	// Kludgily test whether the functions are distinct
	if (funcA->getBaseAddr() == funcB->getBaseAddr()) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  Mutator cannot distinguish two functions from different shlibs\n");
		return FAILED;
	}

	//  Now test regex search
	//  Not meant to be an extensive check of all regex usage, just that
	//  the basic mechanism that deals with regexes is not broken

	bpmv.clear();
#if !defined(os_windows_test)
	//   regex "^cb" should match all functions that begin with "cb"
	//   We dont use existing "call" functions here since (at least on
	//   linux, we also find call_gmon_start().  Thus the dummy fns.
	if (NULL == modB->findFunction("^cb", bpmv, false, false, true) || (bpmv.size() != 2)) 
	{

		logerror("**Failed test #21 (findFunction in module, regex)\n");
		logerror("  Expected 2 functions matching ^cb, got %d\n",
				bpmv.size());
		char buf[128];
		for (unsigned int i = 0; i < bpmv.size(); ++i) 
			logerror("  matched function: %s\n", 
					bpmv[i]->getName(buf, 128));
		return FAILED;
	}

	bpmv.clear();

	if (NULL == modB->findFunction("^cbll21", bpmv, false, false, true) || (bpmv.size() != 1)) 
	{
		logerror("**Failed test #21 (findFunction in module, regex)\n");
		logerror("  Expected 1 function matching ^cbll21, got %d\n",
				bpmv.size());
		return FAILED;
	}
#endif
	return PASSED;    
#else // Not running on one of the specified platforms
	return SKIPPED;
#endif
}



// Wrapper to call readyTest
test_results_t test1_21_Mutator::executeTest() 
{
	int pointer_size = 0;
#if defined(arch_x86_64_test) || defined (ppc64_linux_test)

	pointer_size = pointerSize(appImage);
#endif

        bool isStatic = appAddrSpace->isStaticExecutable();

	strncpy(libNameA, libNameAroot, 127);
	addLibArchExt(libNameA,127, pointer_size, isStatic);
	strncpy(libNameB, libNameBroot, 127);
	addLibArchExt(libNameB,127, pointer_size, isStatic);

	char libA[128], libB[128];
	snprintf(libA, 128, "./%s", libNameA);
	snprintf(libB, 128, "./%s", libNameB);

	if (! appAddrSpace->loadLibrary(libA)) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libNameA);
		return FAILED;
	}

	if (! appAddrSpace->loadLibrary(libB)) 
	{
		logerror("**Failed test #21 (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libNameB);
		return FAILED;
	}

	return mutatorTest21();
}
