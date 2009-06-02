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

// $Id
/*
 * #Name: snip_change_shlib_var
 * #Desc: use instrumentation to modify variable in shared library
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,i386_unknown_linux2_0_test,alpha_dec_osf4_0_test,ia64_unknown_linux2_4_test,x86_64_unknown_linux2_4_test
 * #Notes: This test uses libNameA/B 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

#ifdef os_windows_test
#define snprintf _snprintf
#endif


class snip_change_shlib_var_Mutator : public DyninstMutator {
	const char *libNameAroot;
	const char *libNameBroot;
	char libNameA[128], libNameB[128];

	public:
	snip_change_shlib_var_Mutator();
	virtual test_results_t executeTest();
	virtual test_results_t mutatorTest();
};

snip_change_shlib_var_Mutator::snip_change_shlib_var_Mutator() : libNameAroot("libtestA"),
									   libNameBroot("libtestB") 
{
}

extern "C" DLLEXPORT  TestMutator *snip_change_shlib_var_factory() 
{
	return new snip_change_shlib_var_Mutator();
}

test_results_t snip_change_shlib_var_Mutator::mutatorTest() 
{
	//  The check function returns 1 on success (value changed as expected)
	//  or 0 on failure.
	const char *check_fname  = "check_snip_change_shlib_var";
	const char *inst_func_name = "scsv1";
	BPatch_Vector<BPatch_function *> funcs;

	appImage->findFunction(inst_func_name, funcs);
	if (!funcs.size())
	{
		fprintf(stderr, "%s[%d]:  failed to find function %s\n", FILE__, __LINE__, inst_func_name);
		return FAILED;
	}
	BPatch_function *inst_func = funcs[0];
	funcs.clear();

	appImage->findFunction(check_fname, funcs);
	if (!funcs.size())
	{
		fprintf(stderr, "%s[%d]:  failed to find function %s\n", FILE__, __LINE__, check_fname);
		return FAILED;
	}
	BPatch_function *check_func = funcs[0];

	const char *vname = "snip_change_shlib_var";
	BPatch_variableExpr *v = appImage->findVariable(vname);
	if (!v)
	{
		fprintf(stderr, "%s[%d]:  could not find variable %s\n", FILE__, __LINE__);
		return FAILED;
	}

	BPatch_Vector<BPatch_point *> *pts	= inst_func->findPoint(BPatch_entry);
	if (!pts || !pts->size())
	{
		fprintf(stderr, "%s[%d]:   failed to find entry point to %s\n", 
				FILE__, __LINE__, inst_func_name);
		return FAILED;
	}
	BPatch_point *entry_point = (*pts)[0];

	pts	= inst_func->findPoint(BPatch_exit);
	if (!pts || !pts->size())
	{
		fprintf(stderr, "%s[%d]:   failed to find exit point to %s\n", 
				FILE__, __LINE__, inst_func_name);
		return FAILED;
	}

	BPatch_point *exit_point  = (*pts)[0];

	const char *check_res_name = "gv_scsv1";
	BPatch_variableExpr *check_result = appImage->findVariable(check_res_name);
	if (!check_result)
	{
		fprintf(stderr, "%s[%d]:  failed to find var %s\n", FILE__, __LINE__, check_res_name);
		return FAILED;
	}

	//  Snippet to assign new value to var in shared lib
	//  inserted at entry of inst_func
	BPatch_constExpr newval(777);
	BPatch_arithExpr my_ass(BPatch_assign, *v, newval);

	if (!appAddrSpace->insertSnippet(my_ass, *entry_point))
	{
		fprintf(stderr, "%s[%d]:  failed to insert snippet\n", FILE__, __LINE__);
		return FAILED;
	}

	//  Snippet to call function in shared lib that checks the value of 
	//  the variable that should have been properly changed.
	//  Return value is assigned to global var in mutatee 
	//  inserted at the exit of inst_func

	BPatch_Vector<BPatch_snippet *> check_args;
	BPatch_funcCallExpr checkCall(*check_func, check_args);
	BPatch_arithExpr checkRes(BPatch_assign, *check_result, checkCall);

	if (!appAddrSpace->insertSnippet(checkRes, *exit_point))
	{
		fprintf(stderr, "%s[%d]:  failed to insert snippet\n", FILE__, __LINE__);
		return FAILED;
	}

	return PASSED;
}

test_results_t snip_change_shlib_var_Mutator::executeTest() 
{
	int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
	pointer_size = pointerSize(appImage);
#endif
	strncpy(libNameA, libNameAroot, 128);
	addLibArchExt(libNameA,128, pointer_size);
	strncpy(libNameB, libNameBroot, 128);
	addLibArchExt(libNameB,128, pointer_size);

	char libA[128], libB[128];
	snprintf(libA, 128, "./%s", libNameA);
	snprintf(libB, 128, "./%s", libNameB);

	if (! appAddrSpace->loadLibrary(libA)) 
	{
		logerror("**Failed snip_change_shlib_var (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libNameA);
		return FAILED;
	}

	if (! appAddrSpace->loadLibrary(libB)) 
	{
		logerror("**Failed snip_change_shlib_var (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libNameB);
		return FAILED;
	}

	return mutatorTest();
} // snip_change_shlib_var_Mutator::executeTest()

