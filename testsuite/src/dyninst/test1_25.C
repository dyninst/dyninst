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

// $Id: test1_25.C,v 1.1 2008/10/30 19:18:31 legendre Exp $
/*
 * #Name: test1_25
 * #Desc: Unary Operators
 * #Dep: 
 * #Arch: !mips_sgi_irix6_4_test
 * #Notes: A small part of this test is excluded on most platforms
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_25_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_25_factory() 
{
	return new test1_25_Mutator();
}

//
// Start Test Case #25 - unary operators
//

test_results_t test1_25_Mutator::executeTest() 
{
	// Used as hack for Fortran to allow assignment of a pointer to an int
	BPatch::bpatch->setTypeChecking (false);

	// First verify that we can find a local variable in test1_25_call1
	const char *funcName = "test1_25_call1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs))
			|| !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point25_1 = found_funcs[0]->findPoint(BPatch_entry);

	assert(point25_1);
	//    assert(point25_1 && (point25_1->size() == 1));

	BPatch_variableExpr *gvar[8];

	for (int i=1; i <= 7; i++) 
	{
		char name[80];

		sprintf (name, "test1_25_globalVariable%d", i);
		gvar [i] = findVariable (appImage, name, point25_1);

		if (!gvar[i]) 
		{
			logerror("**Failed** test #25 (unary operaors)\n");
			logerror("  can't find variable %s\n", name);
			return FAILED;
		}
	}

	//     globalVariable25_2 = &globalVariable25_1
#if !defined(rs6000_ibm_aix4_1_test) \
	&& !defined(i386_unknown_linux2_0_test) \
	&& !defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
	&& !defined(ppc64_linux_test) \
	&& !defined(i386_unknown_nt4_0_test) \
        && !defined(os_freebsd_test)

	// without type info need to inform
	BPatch_type *type = appImage->findType("void *");
	assert(type);
	gvar[2]->setType(type);
#endif

	BPatch_arithExpr assignment1(BPatch_assign, *gvar[2],
			BPatch_arithExpr(BPatch_addr, *gvar[1]));

	appAddrSpace->insertSnippet(assignment1, *point25_1);

	// 	   globalVariable25_3 = *globalVariable25_2
	//		Need to make sure this happens after the first one
	BPatch_arithExpr assignment2(BPatch_assign, *gvar[3],
			BPatch_arithExpr(BPatch_deref, *gvar[2]));
	appAddrSpace->insertSnippet(assignment2, *point25_1,  BPatch_callBefore,
			BPatch_lastSnippet);

	// 	   globalVariable25_5 = -globalVariable25_4
	BPatch_arithExpr assignment3(BPatch_assign, *gvar[5],
			BPatch_arithExpr(BPatch_negate, *gvar[4]));
	appAddrSpace->insertSnippet(assignment3, *point25_1);

	// 	   globalVariable25_7 = -globalVariable25_6
	BPatch_arithExpr assignment4(BPatch_assign, *gvar[7],
			BPatch_arithExpr(BPatch_negate, *gvar[6]));
	appAddrSpace->insertSnippet(assignment4, *point25_1);

	// Check removed because MIPS is no longer supported
	// #endif // !MIPS

	BPatch::bpatch->setTypeChecking (true);
	return PASSED;
}

