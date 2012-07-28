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

// $Id: test_snip_remove.C,v 1.1 2008/10/30 19:19:48 legendre Exp $
/*
 * #Name: test_snip_remove
 * #Desc: Test delete multiple snippets at one instrumentation point
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test_snip_remove_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test_snip_remove_factory() 
{
	return new test_snip_remove_Mutator();
}

//
// Start Test Case - test delete snippets
//

test_results_t test_snip_remove_Mutator::executeTest() 
{
	BPatch_Vector<BPatch_function *> funcs;
	const char *fn = "test_snip_remove_func";
	if (!appImage->findFunction(fn, funcs) || !funcs.size())
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_Vector<BPatch_point *> *pts = funcs[0]->findPoint(BPatch_entry);
	if (!pts || !pts->size())
	{
		logerror("Unable to find entry point to %s.\n", fn);
		return FAILED;
	}
		
	BPatch_variableExpr *myvar = appImage->findVariable("test_snip_remove_var");
	if (!myvar)
	{
		logerror("Unable to find variable myvar\n");
		return FAILED;
	}

	BPatch_constExpr one_snip(1);
	BPatch_arithExpr add_one_snip(BPatch_plus, *myvar, one_snip);
	BPatch_arithExpr increment_snip(BPatch_assign, *myvar, add_one_snip);

	BPatchSnippetHandle *sh1 = appAddrSpace->insertSnippet(increment_snip, *pts);
	if (!sh1)
	{
		logerror("Failed to insert snippet1\n");
		return FAILED;
	}
	BPatch_constExpr two_snip(2);
	BPatch_arithExpr add_two_snip(BPatch_plus, *myvar, two_snip);
	BPatch_arithExpr increment2_snip(BPatch_assign, *myvar, add_two_snip);

	BPatchSnippetHandle *sh2 = appAddrSpace->insertSnippet(increment2_snip, *pts);
	if (!sh2)
	{
		logerror("Failed to insert snippet2\n");
		return FAILED;
	}
	BPatch_constExpr three_snip(3);
	BPatch_arithExpr add_three_snip(BPatch_plus, *myvar, three_snip);
	BPatch_arithExpr increment3_snip(BPatch_assign, *myvar, add_three_snip);
	BPatchSnippetHandle *sh3 = appAddrSpace->insertSnippet(increment3_snip, *pts);
	if (!sh3)
	{
		logerror("Failed to insert snippet3\n");
		return FAILED;
	}

	if (!appAddrSpace->deleteSnippet(sh1))
	{
		logerror("Failed to delete snippet1\n");
		return FAILED;
	}
	if (!appAddrSpace->deleteSnippet(sh3))
	{
		logerror("Failed to delete snippet3\n");
		return FAILED;
	}

	return PASSED;
}
