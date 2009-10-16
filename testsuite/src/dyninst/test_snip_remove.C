/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test_snip_remove.C,v 1.1 2008/10/30 19:19:48 legendre Exp $
/*
 * #Name: test_snip_remove
 * #Desc: Test delete multiple snippets at one instrumentation point
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
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
