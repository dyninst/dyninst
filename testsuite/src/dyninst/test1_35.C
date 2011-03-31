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

// $Id: test1_35.C,v 1.1 2008/10/30 19:19:17 legendre Exp $
/*
 * #Name: test1_35
 * #Desc: Function Relocation
 * #Dep: 
 * #Arch:
 * #Notes: i386_unknown_solaris2_5_test,i386_unknown_linux2_0_test,sparc_sun_solaris2_4_test,x86_64_unknown_linux2_4_test
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_35_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_35_factory() 
{
	return new test1_35_Mutator();
}

// Start Test Case #35 - (function relocation)

test_results_t test1_35_Mutator::executeTest() 
{
#if defined(i386_unknown_linux2_0_test) \
	|| defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
        || defined(os_freebsd_test)

	// Only on Solaris and i386 and AMD64 Linux
	// All of these platforms have assembly versions of call35_1

	if (isMutateeFortran(appImage)) 
	{
		return SKIPPED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	char *fn = "test1_35_call1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("**Failed** test #35 (function relocation)\n");
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *foo_function = bpfv[0];

	BPatch_Vector<BPatch_point *> *point35_1 =  
		foo_function->findPoint(BPatch_subroutine);

	assert(point35_1);

	BPatch_variableExpr *var1 = appImage->findVariable(*(*point35_1)[0], 
			"localVariable35_1");
	BPatch_variableExpr *var2 = appImage->findVariable(*(*point35_1)[0], 
			"localVariable35_2");
	BPatch_variableExpr *var3 = appImage->findVariable(*(*point35_1)[0], 
			"total35_1");
	BPatch_variableExpr *var4 = appImage->findVariable(*(*point35_1)[0], 
			"total35_2");

	if (!var1 || !var2 || !var3 || !var4 ) 
	{
		logerror("**Failed** test #35 (function relocation)\n");
		if (!var1) 
			logerror("  can't find local variable localVariable35_1\n");
		if (!var2) 
			logerror("  can't find local variable localVariable35_2\n");
		if (!var3) 
			logerror("  can't find local variable total35_1\n");
		if (!var4) 
			logerror("  can't find local variable total35_2\n");
		return FAILED;
	}

	BPatch_snippet * snippet35_1 = 
		new BPatch_arithExpr(BPatch_assign, *var1, BPatch_constExpr(7));

	BPatch_snippet * snippet35_2 = 
		new BPatch_arithExpr(BPatch_assign, *var2, BPatch_constExpr(5));

	BPatch_snippet * snippet35_3 = 
		new BPatch_arithExpr(BPatch_assign, *var4, *var3);

	BPatch_point * call_1 = ( (* point35_1)[0] );
	assert( call_1 != 0 );

	BPatch_point * call_2 = ( (* point35_1)[2] );
	assert( call_2 != 0 );

	appAddrSpace->insertSnippet( * snippet35_3, * call_2, BPatch_callAfter, BPatch_firstSnippet );
	appAddrSpace->insertSnippet( * snippet35_2, * call_1, BPatch_callBefore, BPatch_firstSnippet );
	appAddrSpace->insertSnippet( * snippet35_1, * call_1, BPatch_callBefore, BPatch_firstSnippet );

#endif   

	return PASSED;
}    

