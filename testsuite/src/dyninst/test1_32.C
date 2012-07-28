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

// $Id: test1_32.C,v 1.1 2008/10/30 19:19:02 legendre Exp $
/*
 * #Name: test1_32
 * #Desc: Recursive Base Tramp
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test1_32_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_32_factory() 
{
	return new test1_32_Mutator();
}

//
// Start Test Case #32 - (recursive base tramp)
//

test_results_t test1_32_Mutator::executeTest() 
{
	const char * func32_2_name = "test1_32_func2";
	const char * func32_3_name = "test1_32_func3";
	const char * func32_4_name = "test1_32_func4";

	BPatch_image * app_image = appImage;

	BPatch_Vector<BPatch_function *> bpfv;

	if (NULL == appImage->findFunction(func32_2_name, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", func32_2_name);
		return FAILED;
	}

	BPatch_function *foo_function = bpfv[0];

	bpfv.clear();

	if (NULL == appImage->findFunction(func32_3_name, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", func32_3_name);
		return FAILED;
	}

	BPatch_function *bar_function = bpfv[0];

	bpfv.clear();

	if (NULL == appImage->findFunction(func32_4_name, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", func32_4_name);
		return FAILED;
	}

	BPatch_function *baz_function = bpfv[0];

	bool old_value = BPatch::bpatch->isTrampRecursive();
	BPatch::bpatch->setTrampRecursive( true );

	BPatch_Vector<BPatch_snippet *> foo_args;
	BPatch_snippet * foo_snippet =
		new BPatch_funcCallExpr( * bar_function,
				foo_args );
	instrument_entry_points( appAddrSpace, app_image, foo_function, foo_snippet );

	BPatch_Vector<BPatch_snippet *> bar_args_1;

	bool mutateeFortran = isMutateeFortran(appImage);
	BPatch_constExpr expr32_2;

	if (mutateeFortran) 
	{
		BPatch_process *p = dynamic_cast<BPatch_process *>(appAddrSpace);
		if (!p)
		{
			fprintf(stderr, "%s[%d]:  error:  address space is not process\n", FILE__, __LINE__);
			abort();
		}
		BPatch_variableExpr *expr32_1 = appAddrSpace->malloc (*appImage->findType ("int"));
		expr32_2 = BPatch_constExpr(expr32_1->getBaseAddr ());

		BPatch_arithExpr oneTimeCodeExpr (BPatch_assign, *expr32_1, BPatch_constExpr(1));      
		p->oneTimeCode (oneTimeCodeExpr);
	} 
	else 
	{
		expr32_2 = BPatch_constExpr(1);
	}

	bar_args_1.push_back (&expr32_2);

	BPatch_snippet * bar_snippet_1 =
		new BPatch_funcCallExpr( * baz_function,
				bar_args_1 );
	instrument_entry_points( appAddrSpace, app_image, bar_function, bar_snippet_1 );

	BPatch_Vector<BPatch_snippet *> bar_args_2;

	BPatch_constExpr expr32_5;

	if (mutateeFortran) 
	{
		BPatch_process *p = dynamic_cast<BPatch_process *>(appAddrSpace);
		if (!p)
		{
			fprintf(stderr, "%s[%d]:  error:  address space is not process\n", FILE__, __LINE__);
			abort();
		}
		BPatch_variableExpr *expr32_4 = appAddrSpace->malloc (*appImage->findType ("int"));
		expr32_5 = BPatch_constExpr(expr32_4->getBaseAddr());

		BPatch_arithExpr expr32_6 (BPatch_assign, *expr32_4, BPatch_constExpr (2));
		p->oneTimeCode (expr32_6);

	} 
	else 
	{
		expr32_5 = BPatch_constExpr(2);
	}

	bar_args_2.push_back(&expr32_5);

	BPatch_snippet * bar_snippet_2 =
		new BPatch_funcCallExpr( * baz_function,
				bar_args_2 );
	instrument_exit_points( appAddrSpace, app_image, bar_function, bar_snippet_2 );

	BPatch::bpatch->setTrampRecursive( old_value );

	return PASSED;
}
